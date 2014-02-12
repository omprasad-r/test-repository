<?php

if (empty($_SERVER['HTTP_HOST'])) {
  // This only happens during early drush bootstrap.
  return;
}

require_once(dirname(__FILE__) . '/g/sites.inc');

// Drush site-install gets confused about the uri when we specify the
// --sites-subdir option. The HTTP_HOST is set incorrectly and we can't
// find it in the sites.json. By specifying the --acsf-install-uri option
// with the value of the standard domain, we can catch that here and
// correct the uri argument for drush site installs.
if (drupal_is_cli() && function_exists('drush_get_option') && ($http_host = drush_get_option('acsf-install-uri', FALSE))) {
  $host = $_SERVER['HTTP_HOST'] = $http_host;
  // Match the expected drupal sites.php key. @see conf_path().
  $dir = implode('.', array_reverse(explode(':', $host)));
}
else {
  $host = rtrim($_SERVER['HTTP_HOST'], '.');
  // Match the expected drupal sites.php key. @see conf_path().
  $dir = implode('.', array_reverse(explode(':', $host)));
}

if (!GARDENS_SITE_DATA_USE_APC) {
  // gardens_site_data_refresh_one() will do a full parse if the domain is in
  // the file at all and a single line parse fails.
  $data = gardens_site_data_refresh_one($host);
}
elseif (($data = gardens_site_data_cache_get($host)) !== 0) {
  if (empty($data)) {
    // Note - when set to use APC, we never parse the whole file on a web
    // request, but we do attempt to parse out the one requested.
    $data = gardens_site_data_refresh_one($host);
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
