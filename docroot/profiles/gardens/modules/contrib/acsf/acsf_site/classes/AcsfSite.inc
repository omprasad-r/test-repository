<?php

/**
 * @file
 * This class contains information about this site and its relationship to the
 * Site Factory.
 */

class AcsfSiteMissingIdentifierException extends Exception {}

class AcsfSite {

  protected $info = array();

  /**
   * Constructor.
   *
   * @param Int $nid
   *   The nid from the Site Factory.
   */
  public function __construct($nid = NULL) {
    $this->info = acsf_vget_group('site_info');

    if ($nid !== NULL) {
      $this->info['nid'] = $nid;
    }

    if ($nid === NULL) {
      throw new AcsfSiteMissingIdentifierException('Cannot instantiate AcsfSite without a nid from the Site Factory. Ensure that it is passed to the constructor or set in acsf_site_nid.');
    }
  }

  /**
   * Setter.
   *
   * @param String $key
   *   The key of the internal storage to look up.
   * @param Unknown $value
   *   The value to stor internally.
   */
  public function __set($key, $value) {
     $this->info[$key] = $value;
  }

  /**
   * Getter.
   *
   * @param String $key
   *   The key of the internal storage to look up.
   *
   * @return Unknown
   */
  public function __get($key) {
    if (isset($this->info[$key])) {
      return $this->info[$key];
    }
  }

  /**
   * Refreshes the site information from the Site Factory.
   *
   * @return Bool
   */
  public function refresh() {
    acsf_vset('acsf_site_nid', $this->info['nid'], 'site_info');

    try {
      $message = new AcsfMessageXmlRpc('acquia.gardens.phone.home', array($this->info['nid']));
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
      acsf_vset('acsf_site_info', $site_info, 'site_info');
      $this->event->context['acsf_site_info'] = $site_info;
      return TRUE;
    }

  }

}
