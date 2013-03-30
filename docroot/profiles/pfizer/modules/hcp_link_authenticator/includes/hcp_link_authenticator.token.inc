<?php
/**
 * @file
 * Handles the authenication token.
 */

/**
 * Sends the token to central auth and handles the response.
 *
 * @param $token
 *   Blowfish-encrypted ciphertext.
 */
function hcp_link_authenticator_link($token) {
  $options = array(
    'timeout' => 2,
    'headers' => array(
      'Content-Type' => 'application/json'
    )
  );

  $domain = $_SERVER['HTTP_HOST'];
  $url = variable_get('hcp_link_authenticator_central_link') . "link/$token/$domain";

  // Send the token to central auth.
  $response = drupal_http_request($url, $options);

  // Handle failure states silently by redirecting to the front page.
  if ($response->code != 200) {
    drupal_goto('<front>');
  }

  // Decode the JSON response data.
  $json = json_decode($response->data);

  // Check the JSON response code.
  if ($json->code == 0) {
    _hcp_link_authenticator_auth($json->target);
  }
  else {
    drupal_goto('<front>');
  }
}

/**
 * Authenticates the user and redirects them to the target path.
 *
 * @param $token
 *   Encrypted token string.
 */
function _hcp_link_authenticator_auth($token) {
  // Load the user and mcrypt include files.
  module_load_include('inc', 'hcp_link_authenticator', 'includes/hcp_link_authenticator.user');
  module_load_include('inc', 'hcp_link_authenticator', 'includes/hcp_link_authenticator.mcrypt');

  // Parse the query string into an array.
  parse_str(_hcp_link_authenticator_decrypt($token), $query);

  // If no target path was specified, redirect to the front page.
  if (empty($query['targetPath'])) {
    drupal_goto('<front>');
  }

  // Get or set the default user.
  $username = _hcp_link_authenticator_get_username();

  // Load the user.
  _hcp_link_authenticator_load_user($username);

  // If the user is now logged in, redirect them to the target path.
  if (user_is_logged_in()) {
    drupal_goto($query['targetPath']);
  }

  drupal_goto('<front>');
}

