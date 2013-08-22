<?php

require_once dirname(__FILE__) . '/gardens.drush.lib.inc';
// $Id: $

/**
 * @file
 *   Example drush command.
 *
 *   Shows how to make your own drush command.
 *
 *   You can copy this file to any of the following
 *     1. A .drush folder in your HOME folder.
 *     2. Anywhere in a folder tree below an active module on your site.
 *     3. In an arbitrary folder specified with the --include option.
 */

function hosting_drush_init() {
  gardens_drush_includes();
}

/**
 * Implementation of hook_drush_command().
 *
 * In this hook, you specify which commands your
 * drush module makes available, what it does and
 * description.
 *
 * Notice how this structure closely resembles how
 * you define menu hooks.
 *
 * @See drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function hosting_drush_command() {
  $items = array();

  // the key in the $items array is the name of the command.
  $items['move-domains-to-staging'] = array(
    // Don't need to bootstap anything for hosting API calls.
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
    // a short description of your command
    'description' => "Move all the domains for all gsites on this tangle to the staging site.",
    'options' => array(
      'suffix' => 'A string to append after a _ to derive the staging site name.  Defaults to "up".',
    ),
    'aliases' => array('mdts'),
  );

  // the key in the $items array is the name of the command.
  $items['move-domains-to-production'] = array(
    // Don't need to bootstap anything for hosting API calls.
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
    // a short description of your command
    'description' => "Move all the domains for all gsites on this tangle from the staging site to the production site.",
    'options' => array(
      'suffix' => 'A string to append after a _ to derive the staging site name.  Defaults to "up".',
    ),
    'aliases' => array('mdtp'),
  );

  // the key in the $items array is the name of the command.
  $items['move-gsite-domains'] = array(
    // Don't need to bootstap anything for hosting API calls.
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
    // a short description of your command
    'description' => "Move all the domains for a gsite to this tangle from the staging site.",
    'options' => array(
      'suffix' => 'A string to append after a _ to derive the staging site name.  Defaults to "up".',
    ),
    'aliases' => array('mgd'),
  );
  $items['go-offline'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_DATABASE,
    'description' => 'Set a site hard offline via SQL.',
    'aliases' => array('go-off'),
  );
  $items['go-online'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_DATABASE,
    'description' => 'Set a site online via SQL.',
    'aliases' => array('go-on'),
  );
  $items['mollom-keys-get'] = array(
    'description' => 'Get from the Gardener and save mollom keys if they are currently missing',
    'aliases' => array('mk-get'),
  );
  $items['mollom-keys-replace'] = array(
    'description' => 'Get from the Gardener and save mollom keys even if they already exist',
    'aliases' => array('mk-repl'),
  );
  // Set the site's file_public_path to refer to its first domain (i.e., the
  // *.drupalgardens.com domain).
  $items['set-gardens-filepath'] = array(
    'description' => "Sets a site's public file directory to refer to its *.drupalgardens.com domain.",
  );
  // Cause a gardens site to call the gardener.
  $items['et-phone-home'] = array(
    'description' => "Cause a gardens site to call the gardener for site-specific data.",
    'aliases' => array('phone-home'),
  );
  $items['domain-config'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
    'description' => 'Adds or removes domains for a gardens site',
    'options' => array(
      'remove' => 'Remove',
      'simulate' => 'Simulate',
    ),
  );
  $items['domain-sync'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
    'description' => 'Sync up domains from the gardener',
    'aliases' => array('doms'),
    'options' => array(
      'cleanup-deleted' => 'Cleanup Deleted Symlinks',
    ),
  );
  $items['domain-sync-full'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
    'description' => 'Sync all domains from the gardener for a site/env',
    'aliases' => array('dom-full'),
    'arguments' => array(
      'site' => 'Hosting sitgroup name',
      'env' => 'Hosting environment name',
      'live_env' => 'Hosting environment name for the live environment of the tangle',
    ),
    'required-arguments' => TRUE,
  );

  $items['localhost-curl'] = array(
    'description' => "Makes a localhost curl request.",
    'arguments' => array(
      'path' => 'The path of the URL.',
    ),
    'required-arguments' => TRUE,
    'options' => array(
      'port' => array(
        'description' => 'Port to use (default is 8081).',
        'example-value' => '8082',
      ),
      'timeout' => array(
        'description' => 'cURL request timeout (default is 120 sec)',
        'example-value' => '300',
      ),
      'host-header' => array(
        'description' => 'Host header (default taken from the alias, or --uri, or omitted).',
        'example-value' => 'foo.drupalgardens.com',
      ),
    ),
    'examples' => array(
      'drush @sitename curl /index.php' => 'Use the URL from the slias and curl index.php.',
    ),
    'aliases' => array('curl'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH, // No bootstrap at all.
  );
  return $items;
}

/**
 * Implementation of hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function hosting_drush_help($section) {
  switch ($section) {
    case 'drush:move-gsite-domains':
      return dt("Move all the domains for a gsite to this tangle.");
    case 'drush:go-offline':
      return dt("Set a site hard offline via SQL.");
    case 'drush:go-online':
      return dt("Set a site online via SQL.");
  }
}

/**
 * Helper function to build common SQL for clearing maintenance mode.
 */
function _drush_hosting_clear_maint() {
  $query = '';
  $query .= "DELETE FROM variable WHERE name = 'maintenance_mode'; ";
  $query .= "DELETE FROM variable WHERE name = 'scarecrow_maintenance_time'; ";
  if (drush_drupal_major_version() >= 7) {
    $query .= "DELETE FROM cache_bootstrap WHERE cid = 'variables'; ";
  }
  else {
    $query .= "DELETE FROM cache WHERE cid = 'variables'; ";
  }
  return $query;
}

/**
 * Drush command. Run all these queries using direct SQL in case the site is WSOD.
 */
function drush_hosting_go_offline() {
  $query = _drush_hosting_clear_maint();
  // For now hard-code a 10 minute expected offline time.
  // in drush5 table prefix processing is off by default. See http://drupal.org/node/1219850.
  // Thus table names are not wrapped with {} otherwise we get SQL errors.  This is fine
  // since Gardens never uses table prefixing.
  $expected = time() + 10*60;
  $query .= "INSERT INTO variable (name, value) VALUES ('maintenance_mode', 'i:1;'); ";
  $query .= "INSERT INTO variable (name, value) VALUES ('scarecrow_maintenance_time', '". serialize($expected) ."'); ";
  return drush_sql_query($query);
}

/**
 * Run after a go-offline command executes. Verify maintenance_mode variable.
 */
function drush_hosting_post_go_offline() {
  $content = _drush_hosting_get_raw_variable('maintenance_mode');
  if (strpos($content, 'i:1;') !== FALSE) {
    drush_log("Site has been placed offline", 'success');
  }
  else {
    drush_log("Site has not been placed offline", 'error');
  }
}


/**
 * Drush command. Run all these queries using direct SQL in case the site is WSOD.
 */
function drush_hosting_go_online() {
  // Determine whether the user intended the site to be in maintenance mode.
  $content = _drush_hosting_get_raw_variable('site_owner_maintenance_mode');
  $query = _drush_hosting_clear_maint();
  if (!empty($content)) {
    $query .= "INSERT INTO variable (name, value) VALUES ('maintenance_mode', 'i:1;'); ";
  }
  return drush_sql_query($query);
}

/**
 * Get the raw mysql command-line output of a SELECT query on {variables}.
 *
 * @param string $variable_name
 * @return string
 *   The raw output of the command-line mysql query.
 */
function _drush_hosting_get_raw_variable($variable_name) {
  $ret = _drush_hosting_sql_query("SELECT value FROM variable WHERE name='$variable_name';");
  if ($ret['status'] !== 0) {
    throw new Exception('Unable to continue, mysql server is unavailable or query failed to execute. Status: ' . $ret['status'] . ' Output: ' . $ret['output']);
  }
  return $ret['output'];
}

/**
 * Run after a go-online command executes. Verify maintenance_mode variable.
 */
function drush_hosting_post_go_online() {
  $content = _drush_hosting_get_raw_variable('maintenance_mode');
  if (empty($content)) {
    drush_log("Site has been placed online", 'success');
  }
  else {
    $content = _drush_hosting_get_raw_variable('site_owner_maintenance_mode');
    if (empty($content)) {
      drush_log("Site has not been placed online", 'error');
    }
    else {
      drush_log("Site has been left offline as set by the site owner", 'success');
    }
  }
}

function _drush_hosting_check_prepare() {
  if (!function_exists('acquia_fields_call')) {
    return drush_set_error('HOSTING_ERROR', 'function acquia_fields_call() does not exist');
  }
  $current_directory = dirname(dirname(__FILE__));
  $files = "$current_directory/docroot/files/ms-drush";
  if (!file_exists("$files/active_domains_by_site.json.txt")) {
    return drush_set_error('HOSTING_ERROR', "You must run 'ms-drush --prepare' before running any hosting commands.");
  }
}

function drush_hosting_pre_move_domains_to_staging() {
  _drush_hosting_check_prepare();
}

function drush_hosting_move_domains_to_staging() {
  $current_directory = dirname(dirname(__FILE__));
  $files = "$current_directory/docroot/files/ms-drush";
  $json = @file_get_contents("$files/active_domains_by_site.json.txt");
  if (empty($json)) {
    return drush_set_error('HOSTING_ERROR', "Did not retrieve any data for active_domains_by_site.");
  }
  $site_domains = json_decode($json);
  $options = drush_get_context('options');
  $options += array('suffix' => 'up');
  // Rely on the fact that Hosting sites are installed in directories given by
  // their site name.
  $hosting_site_name = basename($current_directory);
  if (strpos($hosting_site_name, '_') !== FALSE) {
    return drush_set_error('HOSTING_ERROR', "You should only be running this on a live tangle, not a staging tangle.");
  }
  $staging_site_name = $hosting_site_name . '_' . $options['suffix'];
  drush_print("Moving domains to staging site");
  try {
    acquia_fields_call('move.all.domains', array($hosting_site_name, $staging_site_name));
  }
  catch (Exception $e) {
    return drush_set_error('HOSTING_ERROR', $e->getMessage());
  }
  try {
    $site_info = acquia_fields_call('find.one', array('site', array('name' => $staging_site_name)));
    unset($site_info['parent_site']);
  }
  catch (Exception $e) {
    return drush_set_error('HOSTING_ERROR', $e->getMessage());
  }  
  // Move back wildcard domains.
  $domains_back = array();
  foreach ($site_info['domains'] as $name) {
    if (strpos($name, '*') !== FALSE) {
      $domains_back[] = $name;
    }
  }
  if ($domains_back) {
    try {
      acquia_fields_call('move.domains', array($staging_site_name, $hosting_site_name, $domains_back));
    }
    catch (Exception $e) {
      return drush_set_error('HOSTING_ERROR', $e->getMessage());
    }
  }
}

function drush_hosting_pre_move_domains_to_production() {
  _drush_hosting_check_prepare();
}

function drush_hosting_move_domains_to_production() {
  $current_directory = dirname(dirname(__FILE__));
  $files = "$current_directory/docroot/files/ms-drush";
  $json = @file_get_contents("$files/active_domains_by_site.json.txt");
  if (empty($json)) {
    return drush_set_error('HOSTING_ERROR', "Did not retrieve any data for active_domains_by_site.");
  }
  $site_domains = json_decode($json);
  $options = drush_get_context('options');
  $options += array('suffix' => 'up');
  // Rely on the fact that Hosting sites are installed in directories given by
  // their site name.
  $hosting_site_name = basename($current_directory);
  if (strpos($hosting_site_name, '_') !== FALSE) {
    return drush_set_error('HOSTING_ERROR', "You should only be running this on a live tangle, not a staging tangle.");
  }
  $staging_site_name = $hosting_site_name . '_' . $options['suffix'];
  drush_print("Moving domains to production from staging site");
  try {
    acquia_fields_call('move.all.domains', array($staging_site_name, $hosting_site_name));
  }
  catch (Exception $e) {
    return drush_set_error('HOSTING_ERROR', $e->getMessage());
  }
}

/**
 * Example drush command callback. This is where the action takes place.
 *
 * The function name should be same as command name but with dashes turned to
 * underscores and 'drush_commandfile_' prepended, where 'commandfile' is
 * taken from the file 'commandfile.drush.inc', which in this case is 'example'.
 * Note also that a simplification step is also done in instances where
 * the commandfile name is the same as the beginning of the command name,
 * "drush_example_example_foo" is simplified to just "drush_example_foo".
 * To also implement a hook that is called before your command, implement
 * "drush_hook_pre_example_foo".  For a list of all available hooks for a
 * given command, run drush in --debug mode.
 *
 * If for some reason you do not want your hook function to be named
 * after your command, you may define a 'callback' item in your command
 * object that specifies the exact name of the function that should be
 * called.  However, the specified callback function must still begin
 * with "drush_commandfile_" (e.g. 'callback' => "drush_example_foo_execute").
 * All hook functions are still called (e.g. drush_example_pre_foo_execute,
 * and so on.)
 *
 * In this function, all of Drupal's API is (usually) available, including
 * any functions you have added in your own modules/themes.
 *
 */

function drush_hosting_pre_move_gsite_domains() {
  _drush_hosting_check_prepare();
}

/**
 * drush command callback. Moves a garden site's domains from a staging site to current.
 */
function drush_hosting_move_gsite_domains() {
  $current_directory = dirname(dirname(__FILE__));
  $files = "$current_directory/docroot/files/ms-drush";
  $json = @file_get_contents("$files/active_domains_by_site.json.txt");
  if (empty($json)) {
    return drush_set_error('HOSTING_ERROR', "Did not retrieve any data for active_domains_by_site.");
  }
  $site_domains = json_decode($json);
  $alias = drush_get_context('alias');
  // Drush 3.0 makes us use a leading @ for all aliases.
  $gsite_name = ltrim($alias['#name'], '@');
  if (empty($alias['#name']) || !isset($site_domains->{$gsite_name})) {
    return drush_set_error('HOSTING_ERROR', "You must run hosting drush commands using a gNNNN type site name alias.");
  }
  $domains = $site_domains->{$gsite_name};
  if (empty($domains)) {
    return drush_set_error('HOSTING_ERROR', "Empty list of domains for {$alias['#name']}.");
  }
  foreach ($domains as $i => $domain) {
    if (preg_match('/^g[0-9]+\.gardenssite$/', $domain)) {
      unset($domains[$i]);
      drush_log("Not moving temporary domain:  $domain\n", 'warning');
    }
  }
  if (empty($domains)) {
    return;
  }
  $options = drush_get_context('options');
  $options += array('suffix' => 'up');
  // Rely on the fact that Hosting sites are installed in directories given by
  // their site name.
  $hosting_site_name = basename($current_directory);
  if (strpos($hosting_site_name, '_') !== FALSE) {
    return drush_set_error('HOSTING_ERROR', "You should only be running this on a live tangle, not a staging tangle.");
  }
  $staging_site_name = $hosting_site_name . '_' . $options['suffix'];
  // Load basic site information for the Hosting site and its staging site, and
  // cache it in the filesystem (since it will be the same for all Gardens
  // sites on this tangle).
  $site_base = array();
  foreach (array($hosting_site_name, $staging_site_name) as $site_name) {
    $filename = "$files/site_base_$site_name.txt";
    $contents = @file_get_contents($filename);
    if ($contents) {
      $site_base[$site_name] = unserialize($contents);
    }
    else {
      $sites = acquia_fields_call('find.base', array('site', array('name' => $site_name)));
      $site_base[$site_name] = reset($sites);
      $contents = serialize($site_base[$site_name]);
      file_put_contents($filename, $contents);
    }
    if (empty($site_base[$site_name]['default_fqdn'])) {
      return drush_set_error('HOSTING_ERROR', "Can't find a FQDN for $site_name.");
    }
  }
  drush_print("Moving domains for {$gsite_name}: " . implode(', ', $domains));
  try {
    try {
      // The most likely scenario is that we can move all domains at once.
      acquia_fields_call('move.domains', array($staging_site_name, $hosting_site_name, $domains));
      $moved_domains = $domains;
    }
    catch (Exception $e) {
      // If there was an error, the site's domains may not be where we think
      // they are. Go through and check each domain, moving it from the staging
      // site to the main site unless it's already there.
      drush_log("Could not move all domains at once but will try them one by one. Error was " . $e->getMessage(), 'warning');
      $trial = 1;
      $moved_domains = array();
      do {
        try {
          $failed = FALSE;
          foreach ($domains as $domain) {
            $existing_record = acquia_fields_call('find.one', array('site_domain', array('name' => $domain)));
            if (!empty($existing_record['site_id']) && $existing_record['site_id'] == $site_base[$hosting_site_name]['id']) {
              // The domain is already there. Do nothing.
            }
            elseif (!empty($existing_record['site_id']) && $existing_record['site_id'] == $site_base[$staging_site_name]['id']) {
              // The domain still needs to be moved. Try moving it now.
              acquia_fields_call('move.domains', array($staging_site_name, $hosting_site_name, array($domain)));
              $moved_domains[] = $domain;
            }
            else {
              // The domain doesn't exist or is on some other tangle. Something
              // is really wrong with this Gardens site, so bail out.
              throw new Exception("Domain $domain is not associated with $hosting_site_name or $staging_site_name.");
            }
          }
        }
        catch (Exception $e) {
          // We ran into an error while trying to move the domains one-by-one.
          // It could be something serious, or it could be a temporary failure
          // in the RPC calls to the Master. Try three times and then give up.
          $failed = TRUE;
          if ($trial < 3) {
            // The most recent domain we encountered during the above foreach()
            // loop will still be in $domain; use it in the message.
            drush_log("(trial $trial/3) Error occurred while processing domain $domain: " . $e->getMessage(), 'warning');
            $trial++;
          }
          else {
            // Third time fail for real.
            throw new Exception("Error occurred while processing $domain: " . $e->getMessage());
          }
        }
      } while ($failed);
    }
    // If we made it here, we expect all the site's domains to be on the
    // staging site. Verify that they're there before considering this command
    // successful.
    $test = uniqid('', TRUE);
    file_put_contents("$files/$test.txt", $test);
    $delay = 0;
    do {
      $failed = FALSE;
      if ($delay > 90) {
        // Bail out.
        unlink("$files/$test.txt");
        throw new Exception("Waited too long for domains to move.");
      }
      sleep(5);
      $delay += 5;
      // We use the default_fqdn so that we don't have to worry about DNS.
      $default_fqdn = $site_base[$hosting_site_name]['default_fqdn'];
      foreach ($domains as $domain) {
        $ch = curl_init("http://$default_fqdn/files/ms-drush/$test.txt");
        curl_setopt($ch, CURLOPT_HTTPHEADER, array('Host: ' . $domain)); 
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
        curl_setopt($ch, CURLOPT_TIMEOUT, 3);
        $content = curl_exec($ch);
        $errno = curl_errno($ch);
        curl_close($ch);
        if ($errno || empty($content) || $content != $test) {
          $failed = TRUE;
          break;
        }
      }
    } while ($failed);
    // Cleanup.
    unlink("$files/$test.txt");
  }
  catch (Exception $e) {
    // The Drush command failed. Move all domains back to where they originally
    // were, and set an error.
    $message = "Domains could not be moved for {$gsite_name}: " . $e->getMessage();
    if (!empty($moved_domains)) {
      try {
        acquia_fields_call('move.domains', array($hosting_site_name, $staging_site_name, array_unique($moved_domains)));
      }
      catch (Exception $e) {
        $message .= "\nADDITIONAL ERROR OCCURRED FOR {$gsite_name} WHILE ROLLING BACK; DOMAINS MAY NO LONGER BE POINTING TO THE ORIGINAL CODE: " . $e->getMessage();
      }
    }
    return drush_set_error('HOSTING_ERROR', $message);
  }
  drush_log("Moved domains for {$gsite_name}: " . implode(', ', $domains), 'success');
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
    $mollom_keys = gardens_drush_call_gardener('get.site.mollom.keys', array($nid));
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

/**
 * Drush command callback to set a site's public file directory to its first (i.e. *.drupalgardens.com) domain.
 */
function drush_hosting_set_gardens_filepath() {
  // Load the list of site domains and validate that this command is being run
  // correctly.
  $root_directory = dirname(dirname(__FILE__));
  $files = "$root_directory/docroot/files/ms-drush";
  if (!file_exists("$files/active_domains_by_site.json.txt")) {
    return drush_set_error('HOSTING_ERROR', "You must run 'ms-drush --prepare --use-gardens-domains' before running the set-gardens-filepath command.");
  }
  $json = @file_get_contents("$files/active_domains_by_site.json.txt");
  if (empty($json)) {
    return drush_set_error('HOSTING_ERROR', "Did not retrieve any data for active_domains_by_site.");
  }
  $site_domains = json_decode($json);
  $alias = drush_get_context('alias');
  if (empty($alias['#name'])) {
    return drush_set_error('HOSTING ERROR', "The set-gardens-filepath command can only be run using a gNNNN type site name alias.");
  }
  $gsite_name = ltrim($alias['#name'], '@');
  if (!isset($site_domains->{$gsite_name})) {
    return drush_set_error('HOSTING_ERROR', "The site alias {$alias['#name']} was not found in active_domains_by_site.json.txt.");
  }
  $domains = $site_domains->{$gsite_name};
  if (empty($domains)) {
    return drush_set_error('HOSTING_ERROR', "Empty list of domains for {$alias['#name']}.");
  }

  // Force the site's file directory to use the first domain in the list (i.e.,
  // the *.drupalgardens.com one). Also make sure that is the same domain the
  // current Drush command is running under, or when we flush caches below the
  // wrong domain can wind up places where it shouldn't, e.g. in the {system}
  // table.
  $domain = reset($domains);
  $drush_domain = parse_url($alias['uri']);
  if ($drush_domain['host'] != $domain) {
    return drush_set_error('HOSTING_ERROR', "The set-gardens-filepath command requires Drush to run under the $domain URI for this site ({$drush_domain['host']} was used instead). You may need to run 'ms-drush --prepare --use-gardens-domains' first.");
  }

  // Don't do this for preinstalled sites.
  if (preg_match('/^g[0-9]+\.gardenssite$/', $domain)) {
    drush_log("Not setting file public path for temporary domain $domain\n", 'warning');
    return;
  }

  // Set the 'file_public_path' variable and clear caches if necessary.
  $old_file_public_path = variable_get('file_public_path');
  $new_file_public_path = "sites/$domain/files";
  if ($old_file_public_path != $new_file_public_path) {
    variable_set('file_public_path', $new_file_public_path);
    // Clear the site's caches, so the correct domain is used e.g. for image
    // style paths in the menu router table and for modules and themes in the
    // {system} table.
    drupal_flush_all_caches();
    drush_log("Changed the public file directory from $old_file_public_path to $new_file_public_path", 'success');
  }
  else {
    drush_log("The public file directory was already set to $new_file_public_path", 'success');
  }
}

/**
 * drush command callback. Get the Gardens site to call the Gardener.
 */
function drush_hosting_et_phone_home() {
  if (function_exists('gardens_client_phone_home') && gardens_client_phone_home()) {
    drush_log("Got data from the gardener", 'success');
  }
  else {
    drush_log("Failed to phone home to the gardener", 'error');
  }     
}

/**
 * drush command callback. Configure the domains on a site.
 *
 * @param string $site_id
 *  In the format g123
 *
 * @param domains string
 *  A comma-separated string of the valid domains for this site.
 */
function drush_hosting_domain_config($site_id, $domains = '') {
  drush_hosting_include_install_gardens();
  $hosting_site_name = _drush_hosting_get_hosting_site_name();
  $domains = explode(',', $domains);

  // This is the format some existing code expects
  $domains = array($site_id => $domains);
  if (drush_get_option('remove')) {
    // We're removing these domains.
    return acquia_gardens_remove_site_domains($domains, _drush_hosting_get_hosting_site_name());
  }
  else {
    return acquia_gardens_ensure_domains($domains, _drush_hosting_get_hosting_site_name());
  }
}

/**
 * drush command callback.  Syncs up all domain symlinks.
 *
 * This would typically be run when a machine boots and then every hour or so
 * in case we had a sync failure and want to self heal.
 *
 */
function drush_hosting_domain_sync() {
  drush_hosting_include_install_gardens();
  $hosting_site_name = _drush_hosting_get_hosting_site_name();
  $hosting_site_group = _drush_hosting_get_hosting_site_group();
  $environment = acquia_gardens_get_site_environment();
  
  $active_domains = acquia_gardens_call('get.active.domains.by.site', array($hosting_site_group, $environment));

  syslog(LOG_INFO, "Got " . count($active_domains) . " domains from the gardener");
  $valid_domains = acquia_gardens_ensure_domains($active_domains, $hosting_site_name);
  // These are slower operations we may not want to run so frequently.
  if (drush_get_option('cleanup-deleted')) {
    $deleted = array();
    // Remove temporary domain links used during the install.
    foreach (glob(acquia_gardens_site_directory_symlink('*.gardenssite')) as $link) {
      if (!isset($valid_domains[basename($link)]) && is_link($link)) {
        // Only unlink if there is another link pointing to the same site directory.
        $target = readlink($link);
        if ($target && file_exists("$target/installed") && in_array($target, $valid_domains, TRUE)) {
          unlink($link);
          $deleted[] = $link;
        }
      }
    }

    syslog(LOG_INFO, "Deleted " . count($deleted) .' domains:\n' . implode("," , $deleted));
  }
}

/**
 * Environment-aware full domain sync command for a sitegroup/environment
 *
 * @param String $hosting_site_group
 *   A hosting site group name.
 * @param String $environment
 *   A hosting environment name.
 * @param String $live_environment
 *   A hosting environment name for the corresponding live environment of the
 *   tangle.
 */
function drush_hosting_domain_sync_full($hosting_site_group, $environment, $live_environment) {
  if (empty($hosting_site_group) || empty($environment) || empty($live_environment)) {
    return drush_set_error("BAD_ARGUMENTS", dt('Missing required argument.'));
  }
  drush_hosting_include_install_gardens();
  $active_domains = acquia_gardens_call('get.active.domains.by.site', array($hosting_site_group, $environment));
  $valid_domains = acquia_gardens_ensure_domains($active_domains, $hosting_site_group, $environment, $live_environment);
  syslog(LOG_INFO, "Got " . count($active_domains) . " domains from the gardener for [$hosting_site_group.$environment]");
  // We no longer preinstall, so there is no need to remove preinstall domains
  // any more.
}

/**
 * drush command callback. cURL me maybe.
 *
 */
function drush_hosting_localhost_curl($path) {
  $alias = drush_get_context('alias');
  $uri = drush_get_option('uri');
  $host_header = drush_get_option('host-header');
  $port = drush_get_option('port', '8081');
  if (empty($host_header) && !empty($uri)) {
    if (strpos($uri, '://')) {
      $host_header = parse_url($uri, PHP_URL_HOST);
    }
    else {
      $host_header = current(explode('/', $uri));
    }
  }
  $path = ltrim($path, '/');

  $ch = curl_init("http://localhost:{$port}/{$path}");
  curl_setopt($ch, CURLOPT_HTTPHEADER, array('Host: ' . $host_header));
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
  curl_setopt($ch, CURLOPT_TIMEOUT, (int) drush_get_option('timeout', 120));
  $content = curl_exec($ch);
  $errno = curl_errno($ch);
  $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);

  if ($errno) {
    drush_set_error('HOSTING_ERROR', 'cURL error: ' . curl_error($ch));
  }
  elseif ($code != 200) {
    drush_set_error('HOSTING_ERROR', 'received code: ' . $code);
  }
  curl_close($ch);
  drush_print($content);
}

/**
 * Returns the hosting_site_name by looking at the directory name.
 *
 * If we are on a staging tangle (e.g. tangle001_up) this function returns the
 * parent site's name (e.g. tangle001) instead, since that is what is required
 * by all the API functions.
 *
 * This is not very elegant, so if there is a better way to do it and you're reading
 * this, please do it.
 *
 * @return string
 */
function _drush_hosting_get_hosting_site_name() {
  $hosting_site_name = basename(dirname(realpath(DRUPAL_ROOT)));

  // Filter for an optional "_up" or "up" to map update environments
  return preg_replace('/_?up$/', '', $hosting_site_name);
}

/**
 * Returns the hosting site group.
 *
 * @todo this "API" doesn't seem reliable. We need to find a more permanent
 * solution for deriving the AH site group when there is no drush site context.
 */
function _drush_hosting_get_hosting_site_group() {
  $hosting_site_name = basename(dirname(realpath(DRUPAL_ROOT)));

  $hosting_site_group = file_get_contents(sprintf('/var/www/site-php/%s/ah-site-group', $hosting_site_name));

  return !empty($hosting_site_group) ? $hosting_site_group : $hosting_site_name;
}

/**
 * Creates any required domain symlinks in the sites directory from a given list.
 *
 * @param array $active_domains
 *   An array keyed by DB role containing an array of domains alloted to that
 *   site.
 *   Example:
 *     array(
 *       'g123' => array('domain1.com', 'domain2.org', etc),
 *       'g321' => array('othersite.drupalgardens.com')
 *     );
 * @param string $hosting_site_name
 *   The hosting site identifier (e.g. tangle001), or a sitegroup name if
 *   $environment is also specified.
 * @param String $environment
 *   (Optional) A hosting environment name.
 *
 * @return array
 *  An array of domains in the format array('domainname.com' => '/path/to/sites/g1234');
 */
function acquia_gardens_ensure_domains($active_domains, $hosting_site_name, $environment = '', $live_environment = '') {
  $valid_domains = array();
  foreach ($active_domains as $gardens_site_id => $domains) {
    if (empty($environment)) {
      $site_directory = acquia_gardens_site_directory_legacy($hosting_site_name, $gardens_site_id);
    }
    else {
      $site_directory = acquia_gardens_site_directory($hosting_site_name, $live_environment, $gardens_site_id);
    }
    foreach ($domains as $domain) {
      if (empty($domain)) {
        continue;
      }
      $valid_domains[$domain] = $site_directory;
      // Create any new symlinks pointing from the domain to the Gardens site.
      // If an error occurs here, something is seriously wrong, and the site is
      // probably not going to be accessible via that domain on this web node.
      // However, we don't want to stop processing or installation of other
      // sites, so we simply log it as a critical Gardens error and move on.
      try {
        $site_directory_symlink = acquia_gardens_site_directory_symlink($domain);
        if (drush_get_option('s')) {
          echo "\nensure symlink {$site_directory_symlink} => {$site_directory}\n";
        }
        else {
          acquia_gardens_ensure_symlink($site_directory, $site_directory_symlink);
        }
      }
      catch (Exception $e) {
        syslog(LOG_ERR, "GardensError: AN-22477 - Hosting site failed to set up a Gardens site domain GardensError: site: $hosting_site_name.$environment (live env: $live_environment); error message: " . $e->getMessage());
      }
    }
  }
  return $valid_domains;
}

/**
 * Helper function to include the install_gardens.inc file which contains several
 * functions we are using.
 */
function drush_hosting_include_install_gardens(){
  // Many of the functions we need are in this massive include file.
  // instead of disecting it, I'm just going to include it.
  require_once dirname(realpath(DRUPAL_ROOT)) . '/install_gardens.inc';
}

/**
 * Helper for _drush_hosting_sql_query();
 * @param string $query
 * @param array  $db_spec
 * @return string
 */
function _drush_hosting_build_sql_query($query, $db_spec = NULL) {
  $scheme = _drush_sql_get_scheme($db_spec);

  // Inject table prefixes as needed.
  if (drush_drupal_major_version() >= 7) {
    $query = Database::getConnection()->prefixTables($query);
  }
  else {
    $query = db_prefix_tables($query);
  }

  // Convert mysql 'show tables;' query into something pgsql understands
  if (($scheme == 'pgsql') && ($query == 'show tables;')) {
    $query = "select tablename from pg_tables where schemaname='public';";
  }

  // Save $query to a tmp file. We will redirect it in.
  if ($file = drush_save_data_to_temp_file($query)) {
    switch ($scheme) {
      case 'mysql':
        $exec = 'mysql' . (drush_get_context('DRUSH_VERBOSE') ? ' -v' : '');
        $exec .= _drush_sql_get_credentials($db_spec);
        $exec .= ' ' . drush_get_option('extra');
        $exec .= " < $file";
        break;
      case 'pgsql':
        $exec = 'psql';
        $exec .= _drush_sql_get_credentials($db_spec);
        $exec .= (drush_get_context('DRUSH_VERBOSE') ? '' : ' -q');
        $exec .= ' ' . (drush_get_option('extra') ? drush_get_option('extra') : "--no-align --field-separator=$'\t' --pset footer=off");
        $exec .= " --file $file";
        break;
    }
    return $exec;
  }
}

/**
 * Essentially a clone fo _drush_sql_query, but using exec() instead of system
 * so the return status is discoverable.
 * @param string $query
 * @param array $db_spec
 * @return array
 *  Indexed by output (the output of the query) and status (return status).
 */
function _drush_hosting_sql_query($query, $db_spec = NULL) {
    $exec = _drush_hosting_build_sql_query($query, $db_spec);
    $output = array();
    $status = NULL;
    exec($exec, $output, $status);
    // This doesn't work because drush_op can't pass vars by reference.
    //drush_op('exec', $exec, $output, $status);
    $return = array('output' => implode("\n", $output), 'status' => $status);
    return $return;
}
