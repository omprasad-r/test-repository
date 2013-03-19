<?php
// $Id$

/**
 * @file
 * The ThemeBuilderLog class and convenience function.
 */

class ThemeBuilderLog {
  protected static $instance = NULL;

  /**
   * Returns an instance of the ThemeBuilderLog class.
   */
  public static function getInstance() {
    if (empty(ThemeBuilderLog::$instance)) {
      ThemeBuilderLog::$instance = new ThemeBuilderLog();
    }
    return ThemeBuilderLog::$instance;
  }

  /**
   * Callback used to log messages to the server log via AJAX.  This is the
   * callback referenced in hook_menu() in the themebuilder_log module.
   */
  public static function logAjax() {
    $result = array();
    $result['error'] = '';
    
    if (isset($_REQUEST['logEntry'])) {
      $entry = $_REQUEST['logEntry'];
      $level = intval($entry['level']);
      if ($level === THEME_BUILDER_LOG_LEVEL_OFF) {
        // This is not a valid log level for logging a message.
        return;
      }
      $includeRequestDetails = strtolower($entry['includeRequestDetails']) === 'true';
      ThemeBuilderLog()->log($level, $entry['message'], $includeRequestDetails, $entry['info'], $entry['tag']);
    }
    themebuilder_compiler_json_output($result);
  }

  /**
   * Logs a gardens error message.  These messages will be alerted through
   * nagios.
   *
   * @param {String} $message
   *   A string containing a static message that is exactly the same
   *   for every single instance of the problem being logged.
   * @param {String} $info
   *   A string containing any additional info associated with the error.
   */
  public function gardensError($message, $info = NULL) {
    $this->log(THEME_BUILDER_LOG_LEVEL_ERROR, $message, TRUE, $info, 'GardensError');
  }

  /**
   * Logs an error message.
   *
   * @param {String} $message
   *   A string containing a static message that is exactly the same
   *   for every single instance of the problem being logged.
   * @param {String} $info
   *   A string containing any additional info associated with the error.
   */
  public function error($message, $info = NULL) {
    $this->log(THEME_BUILDER_LOG_LEVEL_ERROR, $message, TRUE, $info);
  }

  /**
   * Logs a gardens warning message.  These messages will be alerted
   * through nagios, but will have the 'GardensWarning' text to work
   * nicely with the email filters described here: https://i.acquia.com/wiki/gardens-e-mail-alert-system-what-you-should-be-doing
   *
   * @param {String} $message
   *   A string containing a static message that is exactly the same
   *   for every single instance of the problem being logged.
   * @param {String} $info
   *   A string containing any additional info associated with the error.
   */
  public function gardensWarning($message, $info = NULL) {
    $this->log(THEME_BUILDER_LOG_LEVEL_ERROR, $message, TRUE, $info, 'GardensWarning');
  }

  /**
   * Logs a warning message.
   *
   * @param {String} $message
   *   A string containing a static message that is exactly the same
   *   for every single instance of the problem being logged.
   * @param {String} $info
   *   A string containing any additional info associated with the error.
   */
  public function warning($message, $info = NULL) {
    $this->log(THEME_BUILDER_LOG_LEVEL_WARNING, $message, TRUE, $info);
  }

  /**
   * Logs an info message.
   *
   * @param {String} $message
   *   A string containing a static message that is exactly the same
   *   for every single instance of the problem being logged.
   * @param {String} $info
   *   A string containing any additional info associated with the error.
   */
  public function info($message, $info = NULL) {
    $this->log(THEME_BUILDER_LOG_LEVEL_INFO, $message, TRUE, $info);
  }

  /**
   * Logs a trace message.
   *
   * @param {String} $message
   *   The log message.
   */
  public function trace($message) {
    $this->log(THEME_BUILDER_LOG_LEVEL_TRACE, $message, FALSE);
  }

  /**
   * Logs a timing message.
   *
   * @param {String} $message
   *   The log message.
   */
  public function timing($message) {
    $this->log(THEME_BUILDER_LOG_LEVEL_TIMING, $message, FALSE);
  }

  /**
   * Logs the specified message at the specified log level.
   *
   * @param {int} $level
   *   The log level.  For example, THEME_BUILDER_LOG_LEVEL_ERROR.  All log
   *   levels can be found in themebuilder_log.module.
   * @param {String} $message
   *   The log message.
   * @param {boolean} $includeRequestDetails
   *   If true, request details will be added to the log message to make
   *   following up on the issue easier.
   * @param {String} $info
   *   A string containing any additional info associated with the error.
   * @param {String} $tag
   *   A tag that wraps the static part of the message.  This is used
   *   to aid in parsing the logs.  Example: 'GardensError', or
   *   'GardensWarning'.
   */
  public function log($level, $message, $includeRequestDetails = TRUE, $info = NULL, $tag = NULL) {
    if (themebuilder_log_enabled($level)) {
      $requestDetails = ($includeRequestDetails ? $this->getRequestDetails() : '');
      $tag = empty($tag) ? '' : "$tag: ";
      $info = empty($info) ? '' : $info;
      $out = sprintf('%s%s %s%s (level: %s) - %s',
        $tag, $message, $tag, $info, $this->getLevelText($level), $requestDetails);
      syslog($this->getSyslogPriority($level), $out);
    }  
  }

  /**
   * Returns a text string that represents the specified log level.  Note that
   * the result is a non-localized string, suitable for logging purposes.
   *
   * @param {int} $level
   *   The log level.
   * @return
   *   A string representation of the specified level.
   */
  private function getLevelText($level) {
    switch ($level) {
      case THEME_BUILDER_LOG_LEVEL_ERROR:
        return 'Error';

      case THEME_BUILDER_LOG_LEVEL_WARNING:
        return 'Warning';

      case THEME_BUILDER_LOG_LEVEL_INFO:
        return 'Info';

      case THEME_BUILDER_LOG_LEVEL_TRACE:
        return 'Trace';

      case THEME_BUILDER_LOG_LEVEL_TIMING:
        return 'Timing';

      case THEME_BUILDER_LOG_LEVEL_ALL:
        return 'All';

      case THEME_BUILDER_LOG_LEVEL_NONE:
        return 'None';
    }
    return 'Unknown';
  }

  /**
   * Returns the syslog priority associated with the specified log level.
   *
   * @param {int} $level
   *   The log level.
   * @return
   *   An integer representing an appropriate syslog level for the specified
   *   logging level.
   */
  private function getSyslogPriority($level) {
    switch ($level) {
      case THEME_BUILDER_LOG_LEVEL_ERROR:
        return LOG_ERR;

      case THEME_BUILDER_LOG_LEVEL_WARNING:
        return LOG_WARNING;

      case THEME_BUILDER_LOG_LEVEL_INFO:
      case THEME_BUILDER_LOG_LEVEL_TRACE:
      case THEME_BUILDER_LOG_LEVEL_TIMING:
      case THEME_BUILDER_LOG_LEVEL_ALL:
      case THEME_BUILDER_LOG_LEVEL_NONE:
        return LOG_INFO;
    }
  }

  /**
   * Returns a string that contains site and database information.
   *
   * @return
   *   A string that provides enough information about the site to make
   *   following up on the issue easier.
   */
  private function getRequestDetails() {
    global $conf;
    global $base_url;
    $dbMaster = 'DB master: unknown';
    if (isset($conf['acquia_hosting_site_info']) && isset($conf['acquia_hosting_site_info']['db'])) {
      $db = $conf['acquia_hosting_site_info']['db'];
      $master = 'unknown';
      if (isset($db['db_url_ha'])) {
        $master = reset(array_keys($db['db_url_ha']));
      }
      $name = $db['name'];
      $dbMaster = "DB master: $master; name: $name";
      // Add the site name to this mix
    }
    $result = "Site: $base_url; $dbMaster";
    return $result;
  }
}
