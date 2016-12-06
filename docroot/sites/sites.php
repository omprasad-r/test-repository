<?php

/**
 * @file
 * Configuration file for Drupal's multi-site directory aliasing feature.
 */

if (!function_exists('acsf_hooks_includes')) {
  /**
   * Scans a factory-hooks sub-directory and returns PHP files to be included.
   *
   * @param string $hook_name
   *   The name of the hook whose files should be returned.
   *
   * @return string[]
   *   A list of customer-defined hook files to include.
   */
  function acsf_hooks_includes($hook_name) {
    $hook_pattern = sprintf('%s/../factory-hooks/%s/*.php', getcwd(), $hook_name);
    return glob($hook_pattern, GLOB_NOSORT);
  }
}

// Include custom sites.php code from factory-hooks/pre-sites-php.
foreach (acsf_hooks_includes('pre-sites-php') as $pre_hook) {
  include_once $pre_hook;
}

if (!function_exists('is_acquia_host')) {
  /**
   * Checks whether the site is on Acquia Hosting.
   *
   * @return bool
   *   TRUE if the site is on Acquia Hosting, otherwise FALSE.
   */
  function is_acquia_host() {
    return file_exists('/var/acquia');
  }
}

// HTTP_HOST can be empty during early drush bootstrap. Also, check that we're
// on an Acquia server so we don't run this code for local development.
if (empty($_SERVER['HTTP_HOST']) || !is_acquia_host()) {
  return;
}

require_once dirname(__FILE__) . '/g/sites.inc';

// Drush site-install gets confused about the uri when we specify the
// --sites-subdir option. The HTTP_HOST is set incorrectly and we can't
// find it in the sites.json. By specifying the --acsf-install-uri option
// with the value of the standard domain, we can catch that here and
// correct the uri argument for drush site installs.
if (drupal_is_cli() && function_exists('drush_get_option') && ($http_host = drush_get_option('acsf-install-uri', FALSE))) {
  // The acsf-install-uri argument contains a pure domain string, one without a
  // leading http:// string. To make sure that the parse_url properly identifies
  // the host portion, add a http:// string.
  $host = $_SERVER['HTTP_HOST'] = parse_url('http://' . $http_host, PHP_URL_HOST);
  $acsf_uri_path = parse_url('http://' . $http_host, PHP_URL_PATH);
  $acsf_uri_path .= '/index.php';
}
else {
  $host = rtrim($_SERVER['HTTP_HOST'], '.');
  $acsf_uri_path = $_SERVER['SCRIPT_NAME'] ? $_SERVER['SCRIPT_NAME'] : $_SERVER['SCRIPT_FILENAME'];
}

$acsf_host = implode('.', array_reverse(explode(':', $host)));
$acsf_uri_path_fragments = explode('/', $acsf_uri_path);
$acsf_uri_path_fragments = array_diff($acsf_uri_path_fragments, array('index.php'));
// Only check the first path fragment, then the base domain.
$acsf_uri_path_fragments = array_slice($acsf_uri_path_fragments, 0, 2);
$data = NULL;
for ($i = count($acsf_uri_path_fragments); $i > 0; $i--) {
  $dir = $acsf_host . implode('.', array_slice($acsf_uri_path_fragments, 0, $i));
  $acsf_uri = $acsf_host . implode('/', array_slice($acsf_uri_path_fragments, 0, $i));
  if (!GARDENS_SITE_DATA_USE_APC) {
    // gardens_site_data_refresh_one() will do a full parse if the domain is in
    // the file at all and a single line parse fails.
    $data = gardens_site_data_refresh_one($acsf_uri);
  }
  elseif (($data = gardens_site_data_cache_get($acsf_uri)) !== 0) {
    if (empty($data)) {
      // Note - when set to use APC, we never parse the whole file on a web
      // request, but we do attempt to parse out the one requested.
      $data = gardens_site_data_refresh_one($acsf_uri);
    }
  }
  if ($data) {
    break;
  }
}

// A value of zero either from the cache or when attempting to refresh indicates
// that the host is known to not exist and was cached as such - we don't need to
// refresh, just fail.
if ($data === 0) {
  return;
}

$GLOBALS['gardens_site_settings'] = $data['gardens_site_settings'];
$sites[$dir] = $data['dir'];

// Include custom sites.php code from factory-hooks/pre-sites-php.
foreach (acsf_hooks_includes('post-sites-php') as $post_hook) {
  include_once $post_hook;
}
