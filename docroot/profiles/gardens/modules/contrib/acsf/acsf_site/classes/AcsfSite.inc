<?php

/**
 * @file
 * This class contains information about this site and its relationship to the
 * Site Factory.
 */

class AcsfSiteMissingIdentifierException extends Exception {}

class AcsfSite {

  static $cache;

  protected $info = array();

  /**
   * Factory: loads the current site.
   *
   * This method will load from static cache if available.
   */
  public static function load() {
    if (!empty(self::$cache)) {
      return self::$cache;
    }
    self::$cache = new static();
    return self::$cache;
  }

  /**
   * Constructor.
   *
   * @param int $nid
   *   The nid from the Site Factory (Optional).
   */
  public function __construct($nid = NULL) {
    if ($nid === NULL) {
      $nid = acsf_vget('acsf_site_nid');
      if (empty($nid) && !empty($GLOBALS['gardens_site_settings']['conf']['gardens_site_id'])) {
        // If we do not have the nid, we can retrieve it from the sites.json.
        $nid = $GLOBALS['gardens_site_settings']['conf']['gardens_site_id'];
        acsf_vset('acsf_site_nid', $nid, 'site_info');
      }
    }

    if (empty($nid)) {
      throw new AcsfSiteMissingIdentifierException('Cannot instantiate AcsfSite without a nid from the Site Factory. Ensure that it is passed to the constructor or set in acsf_site_nid.');
    }

    // Init with the stored values.
    $this->initStoredSiteInfo();

    // Merge in any global values from the sites.json.
    if (!empty($GLOBALS['gardens_site_settings']['conf'])) {
      $this->mergeSiteInfo($GLOBALS['gardens_site_settings']['conf']);
    }

    $this->nid = $nid;
  }

  /**
   * Setter.
   *
   * @param string $key
   *   The key of the internal storage to look up.
   * @param mixed $value
   *   The value to store internally.
   */
  public function __set($key, $value) {
     $this->info[$key] = $value;
  }

  /**
   * Getter.
   *
   * @param string $key
   *   The key of the internal storage to look up.
   *
   * @return mixed
   */
  public function __get($key) {
    if (isset($this->info[$key])) {
      return $this->info[$key];
    }
  }

  /**
   * Refreshes the site information from the Site Factory.
   *
   * @param array $data
   *   (Optional) Data that should be sent to the factory.
   *
   * @return bool
   *   Returns TRUE if the data fetch was successful.
   */
  public function refresh($data = array()) {
    try {
      $site_id = !empty($this->nid) ? $this->nid : $GLOBALS['gardens_site_settings']['conf']['gardens_site_id'];
      $arguments = array(
        'site_id' => $site_id,
        'site_data' => $data,
      );
      $message = new AcsfMessageRest('GET', 'site-api/v1/sync/' . $site_id, $arguments);
      $message->send();
      $site_info = $message->getResponseBody();
    }
    catch (AcsfMessageFailedResponseException $e) {
      // No need to fail, we can retry the site info call.
    }

    if (empty($site_info)) {
      watchdog('acsf_site', 'Could not retrieve site information after installation.', array(), WATCHDOG_CRITICAL);
      return FALSE;
    }
    else {
      // Allow other modules to consume the data.
      $context = $site_info;
      $event = AcsfEvent::create('acsf_site_data_receive', $context);
      $event->run();

      $this->saveSiteInfo($site_info['sf_site']);
      return TRUE;
    }
  }

  /**
   * Removes all local information and refreshes data from the Factory.
   *
   * This is a destructive API function, so use sparingly - only if you can
   * guarantee that there is no loss of required data. Since others may be using
   * this as a storage engine, there could be additional local information saved
   * that you are unaware of.
   */
  public function clean() {
    $this->info = array();
    return $this->refresh();
  }

  /**
   * Saves the internal state of this site.
   */
  public function save() {
    $context = $this->info;
    $event = AcsfEvent::create('acsf_site_data_save', $context);
    $event->run();
    $this->info = $event->context;

    acsf_vset('acsf_site_info', $this->info, 'site_info');
  }

  /**
   * Initializes the internal state of this site.
   */
  private function initStoredSiteInfo() {
    $this->info = array();

    $site_info = acsf_vget_group('site_info');
    foreach ($site_info as $key => $value) {
      if (!is_array($value)) {
        $this->mergeSiteInfo(array($key => $value));
      }
      else {
        $this->mergeSiteInfo($value);
      }
    }
  }

  /**
   * Merges new site information into the internal state.
   *
   * @param array $site_info
   *   The information to merge.
   */
  public function mergeSiteInfo($site_info) {
    if (!empty($site_info)) {
      foreach ($site_info as $key => $value) {
        $this->info[$key] = $value;
      }
    }
  }

  /**
   * Saves new site information.
   *
   * @param array $site_info
   *   The information to merge.
   */
  public function saveSiteInfo($site_info) {
    $this->mergeSiteInfo($site_info);
    $this->last_sf_refresh = time();
    $this->save();
  }

}
