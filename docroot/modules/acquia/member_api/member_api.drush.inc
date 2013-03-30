<?php

/**
 * Implements hook_drush_command().
 */
function member_api_drush_command() {
  $items = array();
  $items['member_api-get-key'] = array(
    'description' => 'Retrieves an API key and secret for a given user, generating it if one did not already exist.',
    'arguments' => array(
      'user' => 'The user ID for which to retrieve an API key.'
    ),
    'options' => array(

    ),
  );
  return $items;
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_member_api_get_key_validate($uid = NULL) {
  if (empty($uid) || !user_load($uid)) {
    return drush_set_error('Please provide a valid user ID.');
  }
}

/**
 * Prints an API key and secret for the given user.
 *
 * @param $uid
 *   A valid user ID.
 */
function drush_member_api_get_key($uid) {
  $api_consumer = member_api_consumer_get($uid);
  if (empty($api_consumer)) {
    $account = user_load($uid);
    $api_consumer = member_api_consumer_create($account);
    drush_print("New API key and secret generated for user $uid.");
  }
  drush_print("Key: " . $api_consumer->key);
  drush_print("Secret: " . $api_consumer->secret);
}
