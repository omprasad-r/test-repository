<?php

/**
 * Implements hook_drush_command().
 */
function gardens_misc_drush_command() {
  $items = array(
    'mollom-keys-get' => array(
      'description' => 'Get from the Gardener and save mollom keys if they are currently missing',
      'aliases' => array('mk-get'),
    ),
    'mollom-keys-replace' => array(
      'description' => 'Get from the Gardener and save mollom keys even if they already exist',
      'aliases' => array('mk-repl'),
    ),
  );
  return $items;
}

/**
 * Implements hook_drush_sql_sync_sanitize().
 */
function gardens_misc_drush_sql_sync_sanitize() {
  $static_scrub = file_get_contents(DRUPAL_ROOT . '/../gsite-scrub.sql');
  drush_sql_register_post_sync_op('staging-static-scrub',
    dt('Scrub Gardens site database'),
    $static_scrub);
}

/**
 * Retrieves information about the gardener.
 *
 * @return array
 *   An array of gardener information.
 */
function drush_gardens_misc_get_gardener_info() {
  if (!class_exists('\Acquia\Acsf\AcsfConfigDefault')) {
    // Since there might not be a bootstrap, we need to find our config objects.
    require_once DRUPAL_ROOT . '/profiles/gardens/modules/contrib/acsf/src/Acquia/Acsf/AcsfConfig.php';
    require_once DRUPAL_ROOT . '/profiles/gardens/modules/contrib/acsf/src/Acquia/Acsf/AcsfConfigDefault.php';
  }

  $config = new \Acquia\Acsf\AcsfConfigDefault();
  return array(
    'url' => $config->getUrl(),
    'username' => $config->getUsername(),
    'password' => $config->getPassword(),
    'url_suffix' => $config->getUrlSuffix(),
    'source_url' => $config->getSourceUrl(),
  );
}

/**
 * drush command callback. Get from the Gardener and save mollom keys if they are currently missing.
 */
function drush_gardens_misc_mollom_keys_get() {
  $public = variable_get('mollom_public_key');
  $private = variable_get('mollom_private_key');
  if (!$public || !$private) {
    drush_gardens_misc_mollom_keys_replace();
  }
  else {
    drush_log("Mollom keys already existed - not replaced", 'warning');
  }
}

/**
 * drush command callback. Get from the Gardener and save mollom keys even if they already exist.
 */
function drush_gardens_misc_mollom_keys_replace() {
  $site = \Acquia\Acsf\AcsfSite::load();
  $nid = $site->site_id;
  try {
    $message = new \Acquia\Acsf\AcsfMessageRest('GET', 'site-api/v1/mollom/keys/' . $nid);
    $message->send();
    $response = $message->getResponseBody();
    $mollom_keys = $response['keys'];
  }
  catch (Exception $e) {
    watchdog('gardens_misc', $e->getMessage(), array(), WATCHDOG_ERROR);
  }

  if ($mollom_keys) {
    variable_set('mollom_public_key', $mollom_keys['public_key']);
    variable_set('mollom_private_key', $mollom_keys['private_key']);
    // Reset mollom status to force a key check.
    if (function_exists('_mollom_status')) {
      _mollom_status(TRUE);
    }
    drush_log("Set mollom keys", 'success');
  }
  else {
    drush_log("Failed to set mollom keys", 'error');
  }
}

