<?php

/**
 * @file
 *
 * Mock Engage web API for DG sites.
 *
 * @see http://rpxnow.com/docs
 */

class MockRPX {

  /**
   * Fetches Engage user profile information for an authenticated user.
   *
   * @param string $token
   *   The token POST'ed to janrain_client_token_handler() upon user authentication.
   * @param string $api_key
   *   The API key for the Engage app we are using.
   * @param boolean $extended
   *   If TRUE, extended user profile data will be requested from Engage.
   *
   * @return
   *   An array of (auth_info) data about the user; FALSE if an
   *   error occured.
   *
   * @see janrain_client_token_handler()
   * @see http://rpxnow.com/docs
   */
  static function auth_info($token, $api_key, $extended = FALSE) {
    $data['token'] = $token;
    $data['apiKey'] = $api_key;
    $data['format'] = 'json';

    if ($extended) {
      $data['extended'] = 'true';
    }

    self::saveLastCall(__FUNCTION__, $data);

    $result = array(
      'stat' => 'ok',
    );
    return $result;
  }

  /**
   * Performs an providers request to the Engage API.
   *
   * @param $realm
   *   Engage realm for which are looking up configured providers for.
   *
   * @return
   *   An array containing the following data:
   *     signin: list of the sign-in providers configured for the RP.
   *     social: list of the social sharing providers configured for the RP.
   *     stat:   status message ('ok' or 'fail'; if 'fail' an 'err' field will
   *       also be containing the error message).
   */
  static function providers($realm) {
    self::saveLastCall(__FUNCTION__, $realm);

    $networks = array_keys(janrain_client_authorized_publishing_providers());

    $providers = 
    $result = array(
      'stat' => 'ok',
      'signin' => array_merge(array('google'), $networks),
      'social' => array(
      ),
      'shareWidget' => array(
        'share' => $networks,
      ),
    );
    return $result;
  }

  /**
   * Performs a map request.
   *
   * @param string $api_key
   *   The API key for the Engage App instance we are using.
   * @param string $authname
   *   The identifier we are mapping to a Drupal uid.
   * @param string $uid
   *   The Drupal user ID we are mapping to.
   * @return
   *   An array of RPX request info (stat, err, etc.)
   *
   * @see http://rpxnow.com/docs
   */
  static function map($api_key, $authname, $uid) {
    $data['apiKey'] = $api_key;
    $data['identifier'] = $authname;
    $data['primaryKey'] = $uid;
    $data['overwrite'] = 'true';

    self::saveLastCall(__FUNCTION__, $data);

    $result = array(
      'stat' => 'ok',
    );
    return $result;
  }

  /**
   * Performs an unmap request.
   *
   * @param string $api_key
   *   The API key for the Engage App instance we are using.
   * @param string $authname
   *   The identifier we are unlinking from a Drupal uid.
   * @param string $uid
   *   The Drupal user ID we are unlinking for.
   * @param boolean $all
   *   Set to TRUE if we should unmap all identifiers mapped to $uid (for example,
   *   when user is deleted.)
   *
   * @return
   *   An array of RPX request info (stat, err, etc.)
   *
   * @see http://rpxnow.com/docs
   */
  static function unmap($api_key, $authname, $uid, $all = FALSE) {
    $data['apiKey'] = $api_key;
    $data['primaryKey'] = $uid;

    if ($all) {
      $data['all_identifiers'] = 'true';
    }
    else {
      $data['identifier'] = $authname;
    }

    self::saveLastCall(__FUNCTION__, $data);

    $result = array(
      'stat' => 'ok',
    );
    return $result;
  }

  /**
   * Performs an activity request.
   *
   * @param string $api_key
   *   The API key for the Engage App instance we are using.
   * @param string $authname
   *   The user's identifier we are posting with.
   * @param string $activity
   *   The activity structure, JSON-encoded.
   * @return
   *   An array containing the following data:
   *     stat:   status message ('ok' or 'fail'; if 'fail' an 'err' field will
   *       also be containing the error message).
   *
   * @see http://rpxnow.com/docs
   */
  static function activity($api_key, $authname, $activity) {
    $data['apiKey'] = $api_key;
    $data['identifier'] = $authname;
    $data['truncate'] = 'true';
    $data['url_shortening'] = 'false';
    $data['prepend_name'] = 'false';
    $data['activity'] = $activity;

    self::saveLastCall(__FUNCTION__, $data);    

    $result = array(
      'stat' => 'ok',
    );
    return $result;
  }

  /**
   * Helper function for the Engage web API wrappers.
   */
  static function report_http_error($result) {
    watchdog('janrain_client', 'Janrain Engage web API seems to be inaccessible due to "%error".', array('%error' => $result->code . ' ' . $result->error), WATCHDOG_WARNING);
    drupal_set_message(t('An HTTP error occurred.'), 'error');
  }

  static function saveLastCall($method, $data) {
    watchdog(__CLASS__, __CLASS__ . ":: $method " . print_r($data, TRUE));
    $last = variable_get('mock_rpx_last_calls', array());
    $last[] = array('method' => $method, 'data' => $data);
    variable_set('mock_rpx_last_calls', $last);
  }

  static function getLastCall() {
    $last = variable_get('mock_rpx_last_calls', array());
    return end($last);
  }

  static function getLastCalls() {
    return variable_get('mock_rpx_last_calls', array());
  }

  static function clearLastCalls() {
    $last = variable_get('mock_rpx_last_calls', array());
    variable_del('mock_rpx_last_calls');
    return $last;
  }
}
