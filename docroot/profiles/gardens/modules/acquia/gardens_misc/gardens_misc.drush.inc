<?php

/**
 * Implements hook_drush_command().
 */
function gardens_misc_drush_command() {
  $items = array(
    'gardens-sql-sanitize' => array(
      'description' => dt("Scrub the current site's database in place. NOT for use on production sites."),
    ),
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
 * Command callback. Runs drush's sql-sanitize command with some extra data.
 *
 * This command requires the staged-gardener and standard-domain options to
 * be passed in. They are used in gardens_misc_drush_sql_sync_sanitize.
 */
function drush_gardens_misc_gardens_sql_sanitize() {
  // Allow modules enabled in the site to add sanitization operations via the
  // sql-sync hook functions.
  drush_bootstrap_max(DRUSH_BOOTSTRAP_DRUPAL_SITE);

  // Skip the built-in email and password sanitizing (we have our own).
  drush_set_option('sanitize-email', 'no');
  drush_set_option('sanitize-password', 'no');

  // Run the sanitization.
  drush_sql_sanitize();
}

/**
 * Implements hook_drush_sql_sync_sanitize().
 */
function gardens_misc_drush_sql_sync_sanitize() {
  // TODO: Make sure the staged-gardener and standard-domain options exist,
  // and error out if they don't.

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
  if (!class_exists('AcsfConfigDefault')) {
    // Since there might not be a bootstrap, we need to find our config objects.
    require_once DRUPAL_ROOT . '/profiles/gardens/modules/contrib/acsf/classes/AcsfConfig.inc';
    require_once DRUPAL_ROOT . '/profiles/gardens/modules/contrib/acsf/classes/AcsfConfigDefault.inc';
  }

  $config = new AcsfConfigDefault();
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
function drush_hosting_mollom_keys_get() {
  $public = variable_get('mollom_public_key');
  $private = variable_get('mollom_private_key');
  if (!$public || !$private) {
    drush_hosting_mollom_keys_replace();
  }
  else {
    drush_log("Mollom keys already existed - not replaced", 'warning');
  }
}

/**
 * drush command callback. Get from the Gardener and save mollom keys even if they already exist.
 */
function drush_hosting_mollom_keys_replace() {
  $nid = gardens_client_get_site_identifier();
  try {
    $message = new AcsfMessageXmlRpc('GET', 'acquia.gardens.get.site.mollom.keys', array($nid));
    $message->send();
    $mollom_keys = $message->getResponseBody();
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

