<?php

/**
 * Implements hook_drush_command().
 */
function gardens_misc_drush_command() {
  $items = array(
    'gardens-get-gardener-creds' => array(
      'description' => dt('Print credentials retrieved from the gardener.'),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
    ),
    'gardens-sql-sanitize' => array(
      'description' => dt("Scrub the current site's database in place. NOT for use on production sites."),
      'options' => array(
        'staged-gardener' => array(
          'description' => dt('The domain of the staged gardener.'),
        ),
        'standard-domain' => array(
          'description' => dt('The domain of the staged site.'),
        ),
      ),
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

  $query = array();
  $staged_gardener = drush_get_option('staged-gardener');
  $standard_domain = drush_get_option('standard-domain');

  if (empty($staged_gardener) || empty($standard_domain)) {
    $creds = drush_gardens_misc_get_gardener_info();
    $staged_gardener = preg_replace('@^https?://@', '', $creds['url']);

    $site_name = preg_replace('@^([^.]+)\..+@', '$1', variable_get('gardens_misc_standard_domain'));
    $standard_domain = sprintf('%s.%s', $site_name, $creds['url_suffix']);
  }

  $original_gardener = variable_get('acquia_gardens_gardener_url');
  $original_gardener = preg_replace('@^https?://@', '', $original_gardener);
  $gardener_url = "https://$staged_gardener";

  $query[] = "UPDATE authmap SET authname=REPLACE(authname, '$original_gardener', '$staged_gardener');";
  $query[] = "DELETE FROM variable WHERE name = 'acquia_gardens_gardener_url';";
  $query[] = "INSERT INTO variable (name, value) VALUES ('acquia_gardens_gardener_url', '" . serialize($gardener_url) . "');";
  $query[] = "DELETE FROM variable WHERE name = 'gardens_misc_standard_domain';";
  $query[] = "INSERT INTO variable (name, value) VALUES ('gardens_misc_standard_domain', '" . serialize($standard_domain) . "');";

  // During the staging process, we use a general SQL scrub file that also
  // removes the file_public_path. The file path does not change during staging,
  // so this needs to be put back here.
  $db_role = variable_get('gardens_db_name', '');
  if (!empty($db_role)) {
    $query[] = "INSERT INTO variable (name, value) VALUES ('file_public_path', '" . serialize("sites/g/files/$db_role/f") . "');";
  }

  drush_sql_register_post_sync_op('staging-dynamic-scrub',
    dt('Reset Gardener-related variables'),
    implode("\n", $query));
}

/**
 * Retrieves information about the gardener.
 *
 * @return array
 *   An array of gardener information.
 */
function drush_gardens_misc_get_gardener_info() {
  if (!function_exists('_acquia_gardens_xmlrpc_creds')) {
    $path = DRUPAL_ROOT .'/../library';
    @include_once("$path/acquia_gardens_xmlrpc.inc");
  }

  $creds = _acquia_gardens_xmlrpc_creds('gardener');
  return $creds;
}

/**
 * Command callback. Prints gardener information.
 */
function drush_gardens_misc_gardens_get_gardener_creds() {
  $creds = drush_gardens_misc_get_gardener_info();
  $output = drush_format($creds, NULL, 'json');
  if (drush_get_context('DRUSH_PIPE')) {
    drush_print_pipe($output);
  }
  drush_print($output);
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
  if ($creds = drush_get_context('DRUSH_DB_CREDENTIALS')) {
    $nid = ltrim($creds['name'], 'g');
    $mollom_keys = acquia_gardens_call('get.site.mollom.keys', array($nid));
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
  else {
    return drush_set_error('HOSTING_ERROR', "Error - can't get DB credentials");
  }
}


