<?php

/**
 * @file
 *
 * Engage web API for DG sites.
 *
 * @see http://rpxnow.com/docs
 */

class RPX {
  const providers_api_suffix = '/api/v2/providers';
  const providers_api_prefix = 'https://';
  const map_api = 'https://rpxnow.com/api/v2/map';
  const unmap_api = 'https://rpxnow.com/api/v2/unmap';
  const auth_api = 'https://rpxnow.com/api/v2/auth_info';
  const activity_api = 'https://rpxnow.com/api/v2/activity';

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

    $options = array(
      'data' => http_build_query($data, '', '&'),
      'headers' => array('Content-Type' => 'application/x-www-form-urlencoded'),
      'method' => 'POST',
    );

    $result = drupal_http_request(RPX::auth_api, $options);

    if(isset($result->error)) {
      RPX::report_http_error($result);
      return FALSE;
    }

    return json_decode($result->data, TRUE);
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
    $url = RPX::providers_api_prefix . $realm . RPX::providers_api_suffix . '/?format=json' ;

    $options = array(
      'max_redirects' => 5,
    );

    $result = drupal_http_request($url, $options);

    if (isset($result->error)) {
      RPX::report_http_error($result);
      return FALSE;
    }

    if (!isset($result->data)) {
      return FALSE;
    }

    // Request was a success.
    return json_decode($result->data, TRUE);
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

    $options = array(
      'data' => http_build_query($data, '', '&'),
      'headers' => array('Content-Type' => 'application/x-www-form-urlencoded'),
      'method' => 'POST',
      'max_redirects' => 5,
    );

    $result = drupal_http_request(RPX::map_api, $options);

    if (isset($result->error)) {
      RPX::report_http_error($result);
    }

    return json_decode($result->data, TRUE);
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

    $options = array(
      'data' => http_build_query($data, '', '&'),
      'headers' => array('Content-Type' => 'application/x-www-form-urlencoded'),
      'method' => 'POST',
      'max_redirects' => 5,
    );

    $result = drupal_http_request(RPX::unmap_api, $options);

    if (isset($result->error)) {
      RPX::report_http_error($result);
    }

    return json_decode($result->data, TRUE);
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

    $options = array(
      'data' => http_build_query($data, '', '&'),
      'headers' => array('Content-Type' => 'application/x-www-form-urlencoded'),
      'method' => 'POST',
      'max_redirects' => 5,
    );

    $result = drupal_http_request(RPX::activity_api, $options);

    if (isset($result->error)) {
      RPX::report_http_error($result);
    }

    return json_decode($result->data, TRUE);
  }

  /**
   * Helper function for the Engage web API wrappers.
   */
  static function report_http_error($result) {
    watchdog('janrain_client', 'Janrain Engage web API seems to be inaccessible due to "%error".', array('%error' => $result->code . ' ' . $result->error), WATCHDOG_WARNING);
    drupal_set_message(t('An HTTP error occurred.'), 'error');
  }
}
