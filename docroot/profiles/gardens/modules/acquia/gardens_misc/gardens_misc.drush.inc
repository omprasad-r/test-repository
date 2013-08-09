<?php

/**
 * Implements hook_drush_command().
 */
function gardens_misc_drush_command() {
  $items = array(
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
