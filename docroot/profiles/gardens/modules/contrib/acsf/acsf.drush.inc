<?php

/**
 * Implements hook_drush_command()
 */
function acsf_drush_command() {
  return array(
    'acsf-build-registry' => array(
      'description' => dt('Rebuilds the ACSF registry.'),
    ),
    'acsf-site-sync' => array(
      'description' => 'Synchronize data with the Factory.',
      'options' => array(
        'data' => 'A base64 encoded php array describing the site generated from the Factory.',
      ),
    ),
    'acsf-get-factory-creds' => array(
      'description' => dt('Print credentials retrieved from the factory.'),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
    ),
    'acsf-site-scrub' => array(
      'description' => dt('Scrubs sensitive information regarding ACSF.'),
    ),
    'go-offline' => array(
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_DATABASE,
      'description' => 'Set a site hard offline via SQL.',
      'aliases' => array('go-off'),
    ),
    'go-online' => array(
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_DATABASE,
      'description' => 'Set a site online via SQL.',
      'aliases' => array('go-on'),
    ),
    'report-complete-async-process' => array(
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
      'description' => 'Send notice back to the Factory when a process completes.',
      'options' => array(
        'data' => 'Serialized PHP data regarding the caller.',
      ),
    ),
    'localhost-curl' => array(
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
    ),
    'acsf-version-get' => array(
      'description' => 'Fetches the version of the acsf moduleset.',
      'arguments' => array(
        'path' => 'The path to the acsf moduleset.',
      ),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUSH, // No bootstrap at all.
    ),
  );
}

/**
 * Drush command. Rebuilds the ACSF registry.
 */
function drush_acsf_build_registry() {
  acsf_build_registry();
  drush_print(dt('The ACSF registry was rebuilt.'));
}

/**
 * Drush command. Scrubs the site database.
 */
function drush_acsf_site_scrub() {
  $config = new AcsfConfigDefault();

  // Ensure that we are testing the scrub cleanly.
  variable_del('acsf_site_scrubbed');

  // Avoid collisions between the Factory and site users when scubbing.
  db_query("UPDATE {users} SET mail = CONCAT('user', uid, '@', MD5(mail), '.example.com'), init = CONCAT('user', uid, '@', MD5(init), '.example.com') WHERE uid > 0");

  $factory_url = preg_replace('@^https?://@', '', $config->getUrl());
  $source_url = preg_replace('@^https?://@', '', $config->getSourceUrl());

  // Replace the source Factory authmap URLs with the new Factory URL.
  if ($factory_url && $source_url) {
    $result = db_query("UPDATE {authmap} SET authname=REPLACE(authname, '$source_url', '$factory_url')");
    drush_print(dt('Replacing authmap entries for !source with !dest.', array('!source' => $source_url, '!dest' => $factory_url)));
  }
  if (!$result) {
    drush_print('Unable to replace any authmap entries.');
  }

  // Remove openid associations.
  db_query('DELETE FROM {openid_association}');

  // Reset the cron key.
  variable_set('cron_key', md5(mt_rand()));

  // Reset the drupal private key.
  variable_del('drupal_private_key');

  // Trigger a menu rebuild.
  variable_set('menu_rebuild_needed', TRUE);

  // Clear sessions and cache tables that might have stale data.
  $truncate_tables = array(
    'sessions',
    'cache',
    'cache_bootstrap',
    'cache_field',
    'cache_filter',
    'cache_form',
    'cache_menu',
    'cache_page',
    'cache_path',
    'cache_block',
    'cache_image',
    'watchdog',
  );

  foreach ($truncate_tables as $table) {
    if (db_table_exists($table)) {
      db_query(sprintf('TRUNCATE {%s}', $table));
    }
  }

  // Ensure that the file_public_path uses the correct convention.
  // @todo this should be removed with DG-11343.
  $db_role = variable_get('gardens_db_name', '');
  if (!empty($db_role)) {
    drush_print(dt('Setting file_public_path to !path', array('!path' => $path)));
    variable_set('file_public_path', "sites/g/files/$db_role/f");
  }

  // Clear any cached values from ACSF variables.
  acsf_variables_cache_refresh();

  // We will handle email addresses and passwords ourselves.
  drush_set_option('sanitize-email', 'no');
  drush_set_option('sanitize-password', 'no');

  // Run the sanitization which allows customers to use hook_drush_sql_sanitize()
  // to provide custom scrubbing.
  drush_sql_sanitize();

  // Mark this database as scrubbed.
  variable_set('acsf_site_scrubbed', 'scrubbed');
}

/**
 * Drush command. Synchronizes data with the Factory.
 *
 * When executed without a --data option, this command will call the Factory to
 * get data. When called with the --data option it will simply digest that data.
 */
function drush_acsf_site_sync() {
  $site = AcsfSite::load();
  $data = drush_get_option('data', NULL);

  // Create an event to gather site stats to send to the Factory.
  $context = array();
  $event = AcsfEvent::create('acsf_stats', $context);
  $event->run();
  $stats = $event->context;

  if ($data) {
    // If data was sent, we can consume it here. Ensure that we are always
    // passing associative arrays here, not objects.
    $site_info = json_decode(base64_decode($data), TRUE);
    if (!empty($site_info) && is_array($site_info)) {
      // Allow other modules to consume the data.
      $context = $site_info;
      $event = AcsfEvent::create('acsf_site_data_receive', $context);
      $event->run();

      // For debugging purpose to be able to tell if the data has been pulled
      // or pushed.
      $site->last_sf_push = time();
      $site->saveSiteInfo($site_info['sf_site']);
    }
  }
  else {
    // If no data was sent, we'll request it.
    $site->refresh($stats);
  }
}

/**
 * Drush command. Run all these queries using direct SQL in case the site is WSOD.
 */
function drush_acsf_go_offline() {
  include_once './includes/lock.inc';
  $query = _drush_acsf_clear_maint();
  // For now hard-code a 10 minute expected offline time.
  $expected = time() + 10*60;
  $query .= "INSERT INTO variable (name, value) VALUES ('maintenance_mode', 'i:1;'); ";
  $query .= "INSERT INTO variable (name, value) VALUES ('acsf_maintenance_time', '". serialize($expected) ."'); ";

  // Get the cron lock to prevent cron from running during an update.
  // Use a large lock timeout because an update can take a long time.
  // All cron processes are stopped before update begins, so the lock will
  // be available.
  lock_acquire('cron', 1200.0);

  return drush_sql_query($query);
}

/**
 * Command callback. Prints factory information.
 */
function drush_acsf_get_factory_creds() {
  if (!class_exists('AcsfConfigDefault')) {
    // Since there might not be a bootstrap, we need to find our config objects.
    $include_path = realpath(dirname(__FILE__));
    require_once $include_path . '/classes/AcsfConfig.inc';
    require_once $include_path . '/classes/AcsfConfigDefault.inc';
  }

  $config = new AcsfConfigDefault();
  $creds = array(
    'url' => $config->getUrl(),
    'username' => $config->getUsername(),
    'password' => $config->getPassword(),
    'url_suffix' => $config->getUrlSuffix(),
  );

  $output = drush_format($creds, NULL, 'json');
  if (drush_get_context('DRUSH_PIPE')) {
    drush_print_pipe($output);
  }
  drush_print($output);
}

/**
 * Run after a go-offline command executes. Verify maintenance_mode variable.
 */
function drush_acsf_post_go_offline() {
  $content = _drush_acsf_get_raw_variable('maintenance_mode');
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
function drush_acsf_go_online() {
  include_once './includes/lock.inc';
  // Determine whether the user intended the site to be in maintenance mode.
  $content = _drush_acsf_get_raw_variable('site_owner_maintenance_mode');
  $query = _drush_acsf_clear_maint();
  if (!empty($content)) {
    $query .= "INSERT INTO variable (name, value) VALUES ('maintenance_mode', 'i:1;'); ";
  }

  // Release cron lock.
  lock_release('cron');
  return drush_sql_query($query);
}

/**
 * Run after a go-online command executes. Verify maintenance_mode variable.
 */
function drush_acsf_post_go_online() {
  $content = _drush_acsf_get_raw_variable('maintenance_mode');
  if (empty($content)) {
    drush_log("Site has been placed online", 'success');
  }
  else {
    $content = _drush_acsf_get_raw_variable('site_owner_maintenance_mode');
    if (empty($content)) {
      drush_log("Site has not been placed online", 'error');
    }
    else {
      drush_log("Site has been left offline as set by the site owner", 'success');
    }
  }
}

/**
 * Drush command callback. Sends a cURL request to localhost for a site.
 */
function drush_acsf_localhost_curl($path) {
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
 * Helper function to build common SQL for clearing maintenance mode.
 */
function _drush_acsf_clear_maint() {
  $query = '';
  $query .= "DELETE FROM variable WHERE name = 'maintenance_mode'; ";
  $query .= "DELETE FROM variable WHERE name = 'acsf_maintenance_time'; ";
  if (drush_drupal_major_version() >= 7) {
    $query .= "DELETE FROM cache_bootstrap WHERE cid = 'variables'; ";
  }
  else {
    $query .= "DELETE FROM cache WHERE cid = 'variables'; ";
  }
  return $query;
}

/**
 * Get the value of the variable directly from the variable table.
 *
 * @param string $variable_name
 *
 * @return string
 *   The raw output of the command-line mysql query.
 */
function _drush_acsf_get_raw_variable($variable_name) {
  $result = drush_db_select('variable', 'value', 'name = :name', array(':name' => $variable_name));
  return drush_db_result($result);
}

/**
 * Drush command. Reports process completion back to the factory.
 */
function drush_acsf_report_complete_async_process() {
  $data = unserialize(drush_get_option('data', NULL));

  if (empty($data->callback) || empty($data->object_id) || empty($data->acsf_path)) {
    drush_set_error('Data error', 'Requires serialized object in --data argument with $data->callback and $data->object_id populated.');
    exit(1);
  }

  // Since this does not bootstrap drupal fully, we need to manually require the
  // classes necessary to send a message to the Factory.
  require_once $data->acsf_path . '/classes/AcsfConfig.inc';
  require_once $data->acsf_path . '/classes/AcsfConfigDefault.inc';
  require_once $data->acsf_path . '/classes/AcsfMessage.inc';
  require_once $data->acsf_path . '/classes/AcsfMessageRest.inc';
  require_once $data->acsf_path . '/classes/AcsfMessageResponse.inc';
  require_once $data->acsf_path . '/classes/AcsfMessageResponseRest.inc';

  $arguments = array(
    'wid' => $data->object_id,
    'signal' => 1,
    'state' => isset($data->state) ? $data->state : NULL,
    'data' => $data,
  );

  try {
    // We do not have a Drupal bootstrap at this point, so we need to use
    // AcsfConfigDefault to obtain the shared credentials.
    $config = new AcsfConfigDefault();
    $message = new AcsfMessageRest('POST', $data->callback, $arguments, $config);
    $message->send();
  }
  catch (AcsfMessageFailedResponseException $e) {
    syslog(LOG_ERR, 'Unable to contact the factory via AcsfMessage.');
  }
}

/**
 * Drush command. Fetches the version of the acsf moduleset.
 */
function drush_acsf_version_get($path) {
  $version = '0.0';
  $acsf_file_path = rtrim($path, '/') . '/acsf.info';
  if (file_exists($acsf_file_path)) {
    $info = drush_drupal_parse_info_file($acsf_file_path);
    $version = isset($info['acsf_version']) ? $info['acsf_version'] : '0.1';
  }
  print $version;
}
