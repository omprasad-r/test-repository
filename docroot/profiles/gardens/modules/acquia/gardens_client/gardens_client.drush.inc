<?php
/**
 * Implementation of hook_drush_command().
 *
 * @See drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function gardens_client_drush_command() {
  $items = array();

  $items['gardens-stats'] = array(
    'description' => "List all the available features for your site.",
    'aliases' => array('gs'),
    'options' => array(
      '-d' => 'Debug stat sending, does not send, just prints stats to console',
    ),
  );
  $items['gardens-move-cluster'] = array(
    'description' => dt('Modifies the db to work in a new cluster.  It is necessary to put the site together first by copying the files and the database and including 2 tables in the database that together form a user map from the original site to the new site.  These tables are "drupalgardens" and "egardens", and are removed at the successful completion of this script.'),
    'arguments' => array(
      'gardener_url' => dt('An optional argument that specifies the gardener url.  By default the stage is used to derive the gardener url, but if a custom domain is being used specify the correct domain here.'),
      'usermapfile' => dt('An optional argument that specifies users that must be present on the site during the migration.  These users will be connected to use openid.  The format is one user per line:  username;mail;gardener_uid'),
    ),
    'examples' => array(
      'Migration example' => 'drush gardens-move-cluster https://www.wmg-gardens.com'
    ),
  );
  $items['gardens-add-openid-users'] = array(
    'description' => dt('Adds openid users to the gardens site.'),
    'arguments' => array(
      'gardener_url' => dt('An optional argument that specifies the gardener url.  By default the stage is used to derive the gardener url, but if a custom domain is being used specify the correct domain here.'),
      'usermapfile' => dt('The filename of the users file, which identifies the users to add.  The format is one user per line: username,mail,gardener_uid'),
    ),
    'examples' => array(
      'Adding openid users example' => 'drush gardens-add-openid-users /tmp/users.txt'
    ),
  );

  $items['gardens-enable-local-logins'] = array(
    'description' => dt('Properly enables local logins on the site.'),
    'arguments' => array(),
  );

  $items['gardens-ensure-module-dependencies'] = array(
    'description' => dt('Ensure that all dependencies for a given module are enabled'),
    'arguments' => array('module_name' => dt('Name of the module to ensure dependencies on.')),
    'aliases' => array('gemd'),
  );

  $items['gardens-flood-protection-add'] = array(
    'description' => dt('Add flood protection to user registration or forgotten password form.'),
    'arguments' => array(
      'form_type' => dt('The form to be protected. Either user-registration or forgotten-password.'),
    ),
    'options' => array(
      'limit' => dt('The number of attempts to allow in a given time period without triggering flood protection.'),
      'period' => dt('The time period in seconds in which the attempts are checked. Also affects blocking period when flood protection has triggered.'),
    ),
    'aliases' => array('gfpa'),
  );
  $items['gardens-flood-protection-remove'] = array(
    'description' => dt('Remove flood protection from user registration or forgotten password form.'),
    'arguments' => array(
      'form_type' => dt('The form to be unprotected. Either user-registration or forgotten-password.'),
    ),
    'aliases' => array('gfpr'),
  );

  // Cause a gardens site to call the gardener.
  $items['et-phone-home'] = array(
    'description' => "Cause a gardens site to call the gardener for site-specific data.",
    'aliases' => array('phone-home'),
  );

  return $items;
}

/**
 * Implementation of hook_drush_help().
 */
function gardens_client_drush_help($section) {
  switch ($section) {
    case 'drush:gardens-stats':
      return dt("Sends stats to the gardener");
    case 'drush:gardens-move-cluster':
      return dt('Modifies this site after the db and files have been moved to a new cluster.');
    case 'drush:gardens-add-openid-users':
      return dt('Adds the specified users as openid users to the site.');
    case 'drush:gardens-enable-local-logins':
      return dt('Enables local logins for the site.');
    case 'drush:gardens-flood-protection-add':
      return dt('Add flood protection to user registration or forgotten password form.');
    case 'gardens-flood-protection-remove':
      return dt('Remove flood protection from user registration or forgotten password form.');
  }
}

/**
 * Sends stats to the gardener.
 */
function drush_gardens_client_gardens_stats() {
  $debug = drush_get_option(array('d'), FALSE);
  $path = dirname(__FILE__);
  include_once("$path/stats.inc");
  $stats = gardens_client_generate_site_stats();
  if ($debug) {
    var_dump($stats);
    return;
  }
  $rpc_result = gardens_client_call_gardener('set.site.stats', array($stats));
  if ($rpc_result !== NULL) {
    variable_set('gardens_stats_time', REQUEST_TIME);
  }
  else {
    throw new Exception('GardensError: AN-22473 - Failed to send site stats for site GardensError:' .$_SERVER['HTTP_HOST']);
  }
  // Also get data from the gardener.
  if (!gardens_client_phone_home()) {
    if (gardens_client_gardener_unreachable_alert()) {
      throw new Exception('GardensError: Failed to retrieve Gardener data for site GardensError:' .$_SERVER['HTTP_HOST']);
    }
  }
  else {
    gardens_client_gardener_unreachable_alert(TRUE);
  }
}

function drush_gardens_client_gardens_move_cluster($gardener_url = NULL, $usermapfile = NULL) {
  drush_log('Running the gardens-move-cluster command...', 'ok');
  $stage = gardens_client_stage_name_get();
  if (empty($stage)) {
    drush_log('Unable to determine the stage from the hostname.', 'error');
    return;
  }

  if (!db_table_exists('drupalgardens')) {
    drush_log('The drupalgardens user table does not exist; make sure to add /mnt/tmp/gardensusers.sql before the migration!', 'error');
    return;
  }

  // Scrub variables
  variable_del('drupal_private_key');
  variable_del('file_public_path');
  variable_set('menu_rebuild_needed', TRUE);
  variable_set('gardens_misc_flush_all_caches', TRUE);

  db_query('TRUNCATE {sessions}');
  db_query('TRUNCATE {themebuilder_session}');

  db_query('UPDATE {users} SET theme = 0 where theme != 0');

  $caches = array(
    'cache',
    'cache_bootstrap',
    'cache_field',
    'cache_filter',
    'cache_form',
    'cache_menu',
    'cache_page',
    'cache_path',
    'cache_block',
    'cache_file_styles',
    'cache_image',
    'cache_media_xml',
    'cache_styles',
  );
  foreach ($caches as $cache) {
    if (db_table_exists($cache)) {
      db_query('TRUNCATE {' . $cache . '}');
    }
  }

  $original_gardener_url = variable_get('acquia_gardens_gardener_url', NULL);
  if (empty($gardener_url)) {
    $gardener_url = 'https://gardener.' . $stage . '.acquia-sites.com';
  }
  variable_set('acquia_gardens_gardener_url', $gardener_url);
  drush_log("Set the acquia_gardens_gardener_url to ${gardener_url}.", 'ok');
  variable_del('cron_last');
  drush_log("Cleared the cron_last variable.", 'ok');
  variable_del('gardens_stats_time');
  drush_log("Cleared the gardens_stats_time variable.", 'ok');

  // Add users if provided.
  try {
    $new_users = _gardens_client_parse_usermapfile($usermapfile);
    _gardens_client_add_users($new_users);
  }
  catch (Exception $e) {
  }

  // Modify the authmap so users will be able to authenticate against the
  // gardener open id server.
  if ($original_gardener_url !== NULL) {
    $entries = db_query('SELECT aid, uid, authname FROM {authmap} WHERE authname LIKE :url_pattern', array(':url_pattern' => "$original_gardener_url%"));
    while ($auth_entry = $entries->fetchObject()) {
      $matches = array();
      $match_count = preg_match('/^.*\/user\/([0-9]+)\/identity$/', $auth_entry->authname, $matches);
      if (count($matches) == 2) {
	$old_uid = $matches[1];
	$map = db_query('SELECT e.uid as uid, e.name as name FROM {drupalgardens} d JOIN {egardens} e on d.name = e.name WHERE d.uid = :old_uid', array(':old_uid' => $old_uid))->fetchObject();
	if (empty($map)) {
          $name = db_query('SELECT name FROM {users} where uid = :uid', array(':uid' => $auth_entry->uid))->fetchField();
	  drush_log("Could not find a uid mapping for user ${name}, deleting the entry from the authmap.", 'warning');
	  db_query('DELETE FROM {authmap} WHERE aid = :aid', array(':aid' => $auth_entry->aid));
	}
	else {
	  // Change the authname to use the new gardener.
	  $new_authname = str_replace($original_gardener_url, $gardener_url, $auth_entry->authname);
	  $new_authname = "${gardener_url}/user/" . $map->uid . '/identity';
	  db_query('UPDATE {authmap} SET authname = :authname WHERE aid = :aid', array(':authname' => $new_authname, ':aid' => $auth_entry->aid));
	  drush_log('Updated authmap for user ' . $map->name, 'ok');
	}
      }
      else {
        $name = db_query('SELECT name FROM {users} WHERE uid = :uid', array(':uid' => $auth_entry->uid))->fetchField();
	drush_log('Unable to extract the user id from auth string ' . $auth_entry->authname . '. Skipping user ' . $name, 'error');
      }
    }
  }

  // Make sure every user in the user table has an authmap entry.  All
  // existing authmap entries have been modified, so only look at users
  // that have no entry.
  _gardens_client_add_authmap_entries($gardener_url, $new_users);

  // Drop the tables that provide a mapping of drupalgardens gardener uids to
  // the new cluster's gardener uids.
  db_drop_table('drupalgardens');
  db_drop_table('egardens');

  acsf_openid_allow_local_user_logins();
  drush_log('Enabled local logins.', 'ok');
  drush_log('Migration complete', 'success');
}

function drush_gardens_client_gardens_add_openid_users($gardener_url, $usermapfile = NULL) {
  drush_log('Running the gardens-add-openid-users command...', 'ok');
  $new_users = _gardens_client_parse_usermapfile($usermapfile);
  _gardens_client_add_users($new_users);

  // Now we need an authmap entry for each user.
  _gardens_client_add_authmap_entries($gardener_url, $new_users);
}

/**
 * Returns the gardener uid for the specified account name.
 *
 * @param {String} $name
 *   The user name.
 * @return {String}
 *   The gardener user id associated with the specified user name.
 */
function _gardens_client_get_gardener_uid_from_name($name) {
  if (empty($name)) {
    throw new Exception('Name not provided');
  }
  if (!db_table_exists('egardens')) {
    throw new Exception('egardens table not found.');
  }
  $gardener_uid = db_query('SELECT uid FROM {egardens} WHERE name = :name', array(':name' => $name))->fetchField();
  if (empty($gardener_uid)) {
    throw new Exception('Gardener account not found.');
  }
  drush_log("Local account $name uses gardener uid $gardener_uid", 'ok');
  return $gardener_uid;
}

/**
 * Parses the specified usermap file.
 *
 * The user map file is a text file with one line per user in the following form:
 *   login name;mail
 */
function _gardens_client_parse_usermapfile($usermapfile) {
  $result = array();
  if (!file_exists($usermapfile)) {
    throw new Exception("The user map file $usermapfile does not exist.");
  }

  $userdata = explode("\n", file_get_contents($usermapfile));
  foreach ($userdata as $entry) {
    $elements = explode(';', $entry);
    $u = new StdClass();
    $u->name = $elements[0];
    $u->mail = $elements[1];
    $result[] = $u;
  }
  return $result;
}

/**
 * Creates accounts for the specified users.
 *
 * @param {Array} $new_users
 *   An array of objects, each of which identifies a new user with the login name and email address.
 */
function _gardens_client_add_users($new_users) {
  $current_uid = db_query('SELECT max(uid) FROM {users}')->fetchField() + 5;
  $timestamp = time();
  foreach ($new_users as $u) {
    $existing_user = db_query('SELECT uid, name, mail from {users} WHERE name = :name', array(':name' => $u->name))->fetchObject();
    if (!$existing_user) {
      // Need to add the user here.
      db_query('INSERT INTO {users} (uid, name, mail, status, created) VALUES (:uid, :name, :mail, :status, :created)', array(':uid' => $current_uid, ':name' => $u->name, ':mail' => $u->mail, ':status' => 1, ':created' => $timestamp));
      drush_log('Added user \'' . $u->name . '\'.', 'ok');
      $current_uid += 5;
    }
  }
}

/**
 * Adds authmap entries for the specified users so they can log in using openid.
 *
 * @param {String} $gardener_url
 *   The url to the gardener.
 * @param {Array} $users
 *   An array of objects, each of which represents a user which should
 *   have an authmap entry.
 */
function _gardens_client_add_authmap_entries($gardener_url, $users) {
  // Make sure every user in the user table has an authmap entry.  All
  // existing authmap entries have been modified, so only look at users
  // that have no entry.
  if (!empty($users)) {
    $user_names = array();
    foreach ($users as $user) {
      $user_names[] = $user->name;
    }
    $local_users = db_query('SELECT uid, name FROM {users} WHERE uid NOT IN (SELECT uid FROM {authmap}) AND name in (:names)', array(':names' => $user_names));
  }
  else {
    // Add all of the users found in the users table.
    $local_users = db_query('SELECT uid, name FROM {users} WHERE uid NOT IN (SELECT uid FROM {authmap})');
  }
  $authmap_insert = 'INSERT INTO {authmap} (uid, authname, module) VALUES (:uid, :authname, :module)';
  $user_lookup = 'SELECT uid FROM {users} WHERE name = "%s"';
  $authmap_template = '%s/user/%s/identity';
  while ($u = $local_users->fetchObject()) {
    try {
      // If there is a mapping between a local user and a gardener
      // user, create a new authmap entry.
      $gardener_uid = _gardens_client_get_gardener_uid_from_name($u->name);
      $authname = sprintf($authmap_template, $gardener_url, $gardener_uid);

      // Add the entry.
      db_query($authmap_insert, array(':uid' => $u->uid, ':authname' => $authname, ':module' => 'openid'));
    }
    catch (Exception $e) {
      // If there is no mapping, we aren't going to worry about it.
    }
  }
}

function drush_gardens_enable_local_logins() {
  acsf_openid_allow_local_user_logins();
  drush_log('Enabled local logins.', 'success');
}

/**
 * Drush command to ensure all dependencies are enabled for a given module.
 *
 * @param $module
 *   Drupal machine-readable name for the module to be checked.
 */
function drush_gardens_ensure_module_dependencies($module) {
  if (module_exists($module)) {
    // For a single module, parsing info seems more efficent than doing full
    // module data rebuilds just to get the dependencies graph.  This means we
    // won't capture nested dependencies here though, but that isn't the point of
    // this function.
    $info = drupal_parse_info_file(drupal_get_path('module', $module) . "/$module.info");
    $to_enable = array();
    if (!empty($info['dependencies'])) {
      foreach ($info['dependencies'] as $dependency) {
        if (!module_exists($dependency)) {
          $to_enable[] = $dependency;
          drush_print(dt('Missing dependency !dep added to list.', array('!dep' => $dependency)));
        }
      }
    }

    if (!empty($to_enable)) {
      module_enable($to_enable);
      drush_print(dt('Enabled all dependencies.'));
    }
    else {
      drush_print(dt('Nothing to do. All dependencies already enabled.'));
    }
  }
  else {
    // This command will likely only be run automatically, so the most sensible
    // option if the specified module is not enabled is to enable it and warn.
    drush_print(dt('Target module was not enabled.  Enabling now.'));
    module_enable(array($module));
  }
}

/**
 * Adds flood protection to user registration or forgotten password form.
 */
function drush_gardens_client_gardens_flood_protection_add($form_type = '') {
  $allowed_input = array('user-registration', 'forgotten-password');
  if (in_array($form_type, $allowed_input)) {
    $flood_name = str_replace('-', '_', $form_type);
    // For grepping: gardens_misc_forgotten_password_flood_protection
    // gardens_misc_user_registration_flood_protection
    variable_set('gardens_misc_' . $flood_name . '_flood_protection', TRUE);

    $limit = drush_get_option('limit', FALSE);
    if ($limit) {
      variable_set('gardens_misc_persistent_flood_limit_' . $flood_name, $limit);
    }
    else {
      $limit = variable_get('gardens_misc_persistent_flood_limit_' . $flood_name, 10);
    }

    $period = drush_get_option('period', FALSE);
    if ($period) {
      variable_set('gardens_misc_persistent_flood_expire_' . $flood_name, $period);
    }
    else {
      $period = variable_get('gardens_misc_persistent_flood_expire_' . $flood_name, 3600);
    }

    drush_print(dt('Flood protection has been enabled for the @form form. More than @limit attempts over @period seconds will trigger the flood control.', array(
      '@form' => str_replace('-', ' ', $form_type),
      '@limit' => $limit,
      '@period' => $period,
    )));
  }
  else {
    drush_print(dt('Invalid input, the allowed values are: @allowed_input.', array('@allowed_input' => implode(', ', $allowed_input))));
  }
}

/**
 * Removes flood protection from user registration or forgotten password form.
 */
function drush_gardens_client_gardens_flood_protection_remove($form_type) {
  $allowed_input = array('user-registration', 'forgotten-password');
  if (in_array($form_type, $allowed_input)) {
    // For grepping: gardens_misc_forgotten_password_flood_protection
    // gardens_misc_user_registration_flood_protection
    variable_del('gardens_misc_' . str_replace('-', '_', $form_type) . '_flood_protection');

    drush_print(dt('Flood protection has been disabled for the @form form.', array('@form' => str_replace('-', ' ', $form_type))));
  }
  else {
    drush_print(dt('Invalid input, the allowed values are: @allowed_input.', array('@allowed_input' => implode(', ', $allowed_input))));
  }
}

/**
 * drush command callback. Get the Gardens site to call the Gardener.
 */
function drush_gardens_client_et_phone_home() {
  if (function_exists('gardens_client_phone_home') && gardens_client_phone_home()) {
    drush_log("Got data from the gardener", 'success');
  }
  else {
    drush_log("Failed to phone home to the gardener", 'error');
  }
}

