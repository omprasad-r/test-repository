<?php
/**
 * This class is responsible for determining which server a themebuilder
 * session should be locked to.  The affinity cookie must be set before
 * entering the themebuilder, but should not be set unless the user is using
 * the themebuilder.
 *
 * The ah_app_server is a cookie which causes subsequent requests to go to the
 * specified webnode.  Because multiple simultaneous writes to a theme
 * directory are likely to cause theme corruption, we use a semaphore to
 * serialize writes to the theme files.  This semaphore is webnode-specific,
 * so it is important that all themebuilder sessions on a particular site are
 * serviced by the same webnode. It is important to set this cookie before the
 * user even enters the themebuilder edit mode because the request to enter
 * edit mode also uses the semaphore.
 */
class AffinityServer {
  const TIMEOUT = 300; // 5 minutes, measured in seconds
  // Holds the server information, which includes the webnode hostname and the
  // timestamp of this data's expiration.
  private static $serverInfo;
  // Holds the hostname retrieved from the current webnode.
  private static $hostname;

  /**
   * Verifies the cookie is set and matches existing themes.  If the
   * verification fails, the themebuilder session will be forcibly closed.
   *
   * @param $end_request
   *   When the cookie cannot be set correctly for the current page request,
   *   the default behavior is to log an error and end the page request with a
   *   503 internal server error. Set this parameter to FALSE to cause the
   *   function to throw a ThemeBuilderWrongWebnodeException instead, if you
   *   want to do your own error handling.
   *
   * @throws ThemeBuilderWrongWebnodeException
   *   In the case described above.
   * @throws ThemeBuilderException
   *   For all other errors.
   */
  public function verify($end_request = TRUE) {
    if (!isset($_COOKIE['ah_app_server'])) {
      // The cookie is not set.  The user could have deleted it or this can
      // happen if the user exits the browser in a themebuilder session and
      // then returns at a later time.  Set the cookie and see if the
      // themebuilder can recover.
      $webnode = $this->getWebnode();
      $cookie_webnode = $webnode['webnode'];
      setcookie('ah_app_server', $cookie_webnode, 0, '/');

      // We don't stop the request here because there is a chance that
      // the request was sent to the correct webnode despite the lack
      // of the affinity cookie.
    }
    else {
      $cookie_was_set = TRUE;
      $cookie_webnode = $_COOKIE['ah_app_server'];
    }

    $current_webnode = $this->getHostname();
    $session_webnode = $this->getWebnodeFromSession();

    // If there is any mismatch, send an error code.  The themebuilder will
    // attempt to send the request several times before eventually closing.
    if ($cookie_webnode !== $current_webnode || (!empty($session_webnode) && $session_webnode !== $current_webnode)) {
      if (!empty($cookie_was_set)) {
        // The user already had a cookie, but it led them astray and put them
        // on the wrong web node. Bad cookie! Kill it.
        $this->clearCookie();
      }
      $message = 'Themebuilder cookie mismatch error.';
      $info = "Cookie webnode is $cookie_webnode, session webnode is $session_webnode, current webnode is $current_webnode.";
      if ($end_request) {
        ThemeBuilderLog()->error($message, $info);
        header('HTTP/1.0 503 Internal error', true, 503);
        exit();
      }
      else {
        throw new ThemeBuilderWrongWebnodeException($message);
      }
    }
    // Updates the variable if needed.
    $this->getWebnode();
  }

  /**
   * Checks for the existence of the affinity cookie and verifies that
   * it matches the current webnode.  If there is a problem, the
   * request fails.  This is the correct behavior in cases in which we
   * are opening the themebuilder but the target webnode has gone out
   * of rotation so the request got sent to the wrong webnode.  Also
   * we have seen cases in which IE does not send the cookie at all
   * when opening the themebuilder, which has catastrophic
   * consequences.
   */
  public function checkAndFail() {
    if (!isset($_COOKIE['ah_app_server'])) {
      // The cookie is not set.  The user could have deleted it or this can
      // happen if the user exits the browser in a themebuilder session and
      // then returns at a later time.
      ThemeBuilderLog()->error("Themebuilder cookie is missing.");
      header('HTTP/1.0 503 Internal error', true, 503);
      exit();
    }

    $cookie_webnode = $_COOKIE['ah_app_server'];
    $current_webnode = $this->getHostname();

    // If there is any mismatch, send an error code.  The themebuilder will
    // attempt to send the request several times before eventually closing.
    if ($cookie_webnode !== $current_webnode) {
      ThemeBuilderLog()->error('Themebuilder cookie mismatch error.', "Cookie webnode is $cookie_webnode, current webnode is  $current_webnode.");
      header('HTTP/1.0 503 Internal error', true, 503);
      exit();
    }
  }

  /**
   * Returns the preferred webnode.
   *
   * NOTE: If a webserver is taken out of rotation for any reason, this could
   * leave references in the themebuilder_session table that will continue to
   * try to put subsequent themebuilder edit sessions onto the nonexistent
   * webnode.  This can be addressed if needed by adding a webnode blacklist,
   * implemented entirely in code so the effect would be immediate when
   * needed.
   *
   * @return
   *   An array representing the webnode with a unix timestamp indicating the
   *   expiration time of that information.
   */
  public function getWebnode() {
    if (!empty(self::$serverInfo)) {
      return self::$serverInfo;
    }
    $serverInfo = $this->getWebnodeFromDB();
    if (empty($serverInfo)) {
      $server = $this->getWebnodeFromSession();
      if (empty($server)) {
        $server = $this->getHostname();
      }
      $serverInfo = array('webnode' => $server, 'time' => REQUEST_TIME + self::TIMEOUT);
    }
    self::$serverInfo = $serverInfo;
    $this->updateWebnodeTimeout();
    return $serverInfo;
  }

  /**
   * Clears the affinity server cookie so that subsequent requests can be load
   * balanced across all available webnodes.
   */
  public function clearCookie() {
    setcookie('ah_app_server', '', 0, '/');
  }

  /**
   * Gets the webnode from the themebuilder session table.  Note that all of
   * the webnodes in that table should be the same string.
   *
   * @return
   *   A string representing the webnode, or NULL if there are no entries in
   *   the table that contain a webnode string.
   * @throws ThemeBuilderException
   *   If the webnodes in the themebuilder_session table do not match.
   * @param {boolean} $retry
   *   If TRUE, a retry will be attempted if it was not possible to
   *   get the webnode in the first attempt.
   */
  private function getWebnodeFromSession($retry = TRUE) {
    $session_webnodes = db_select('themebuilder_session', 'ts')
      ->fields('ts', array('webnode'))
      ->distinct()
      ->execute()
      ->fetchCol();
    if (empty($session_webnodes)) {
      return NULL;
    }
    elseif (count($session_webnodes) == 1) {
      return end($session_webnodes);
    }
    elseif (count($session_webnodes) > 1 && $retry) {
      $themeCorrector = new ThemeCorrector(DRUPAL_ROOT . '/' . themebuilder_compiler_get_custom_theme_folder(), array('MultipleWebnodeSessionElf'));
      $fixed = $themeCorrector->fix();
      if ($fixed) {
        return $this->getWebnodeFromSession(FALSE);
      }
    }
    ThemeBuilderLog()->gardensError('AN-23711 - Themebuilder multiple session webnodes error.', 'Multiple webnodes are specified in the themebuilder session table.  Forcibly closing the themebuilder.');
    throw new ThemeBuilderException(t("Oops.  !service_name_long was unable to process your request (internal error).  The ThemeBuilder must close now.  After closing, you can try changing your site's appearance again.  If you have seen this message multiple times, please contact !service_name_long support for assistance.", array(
      '!service_name_long' => variable_get('gardens_service_name_long', 'Drupal Gardens'),
    )), THEMEBUILDER_ERROR_RECOVERABLE, array(ThemeBuilderException::HANDLER_ALERT_AND_CLOSE));
  }

  /**
   * Refreshes the webnode entry in the variable table.  This entry will have
   * a timeout so it won't live forever.
   */
  private function updateWebnodeTimeout() {
    if (empty($this->serverInfo)) {
      return;
    }
    variable_set('themebuilder_webnode',
      array('webnode' => $this->serverInfo['webnode'],
        'time' => REQUEST_TIME + self::TIMEOUT));
  }

  /**
   * Returns the webnode hostname and information time retrieved from the
   * variable table.
   */
  private function getWebnodeFromDB() {
    $webnode = NULL;
    $info = variable_get('themebuilder_webnode', NULL);
    if (!empty($info['webnode']) && REQUEST_TIME <= $info['time']) {
      $webnode = $info['webnode'];
    }
    return $webnode;
  }

  /**
   * Gets the hostname either from a cache file or from a function call.
   *
   * @return
   *   The hostname.
   */
  public function getHostname() {
    if (empty(self::$hostname)) {
      $hostname_full = php_uname('n');
      $hostname = explode('.', $hostname_full);
      self::$hostname = $hostname[0];
    }
    return self::$hostname;
  }
}
