<?php

/**
 * Implements hook_drush_command()
 */
function acsf_drush_command() {
  return array(
    'acsf-registry-rebuild' => array(
      'description' => dt('Rebuilds the ACSF registry.'),
      'callback' => 'drush_acsf_build_registry',
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
 * Drush command. Run all these queries using direct SQL in case the site is WSOD.
 */
function drush_acsf_go_offline() {
  $query = _drush_acsf_clear_maint();
  // For now hard-code a 10 minute expected offline time.
  $expected = time() + 10*60;
  $query .= "INSERT INTO variable (name, value) VALUES ('maintenance_mode', 'i:1;'); ";
  $query .= "INSERT INTO variable (name, value) VALUES ('acsf_maintenance_time', '". serialize($expected) ."'); ";
  return drush_sql_query($query);
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
  // Determine whether the user intended the site to be in maintenance mode.
  $content = _drush_acsf_get_raw_variable('site_owner_maintenance_mode');
  $query = _drush_acsf_clear_maint();
  if (!empty($content)) {
    $query .= "INSERT INTO variable (name, value) VALUES ('maintenance_mode', 'i:1;'); ";
  }
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

