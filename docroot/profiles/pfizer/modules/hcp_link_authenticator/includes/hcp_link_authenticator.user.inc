<?php
/**
 * @file
 * File to handle the users functions.
 */

/**
 * Defines the HCP auth username.
 */
define('HCP_LINK_AUTH_USER', 'user_hcp');

/**
 * Loads a user by name if the user exists, otherwise creates it.
 *
 * @param $username
 *   The constant user's name.
 */
function _hcp_link_authenticator_load_user($username) {
  global $user;

  $user = user_load_by_name($username);

  // There is no access checking involved here.
  if ($user == FALSE) {
    $edit = array(
      'name' => $username,
      'pass' => user_password(),
      'mail' => $username . '@pfizer.com',
      'status' => 1,
      'init' => $username . '@pfizer.com',
    );

    // Create the user.
    user_save(NULL, $edit);

    $user = user_load_by_name($username);
  }

  user_login_finalize();
}

/**
 * Returns the name of the HCP user.
 *
 * @param $username
 *   The constant user's name.
 * @param $count
 *   An internal number that will increase the user's name.
 * @return
 *   The HCP user's name.
*/
function _hcp_link_authenticator_get_username($username = HCP_LINK_AUTH_USER, $count = 0) {
  // If the configuration variable containing the user's name exists, return it.
  if (!is_null(variable_get('hcp_link_authenticator_user'))) {
    return variable_get('hcp_link_authenticator_user');
  }
  // Otherwise, check to see if the username exists in the system. If it does
  // not exist, use the given username. If it does exist, add an incremental
  // numeric suffix until we find a name that doesn't exist in the system and
  // use that.
  else {
    // Check to see if the given username exists in the system. If it does not
    // exist, use the given username.
    if (user_load_by_name($username) == FALSE) {
      variable_set('hcp_link_authenticator_user', $username);
      return $username;
    }
    // If it does exist, add a numeric suffix and increase it until we get to an
    // available name.
    else {
      $count++;
      return _hcp_link_authenticator_get_username($username . $count, $count);
    }
  }
}

