<?php

// $Id: $

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

  // Cause a gardens site to call the gardener.
  $items['et-phone-home'] = array(
    'description' => "Cause a gardens site to call the gardener for site-specific data.",
    'aliases' => array('phone-home'),
  );

  $items['mollom-keys-get'] = array(
    'description' => 'Get from the Gardener and save mollom keys if they are currently missing',
    'aliases' => array('mk-get'),
  );

  $items['mollom-keys-replace'] = array(
    'description' => 'Get from the Gardener and save mollom keys even if they already exist',
    'aliases' => array('mk-repl'),
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

