<?php

include_once(dirname(__FILE__) . '/sites.inc');

// TTL set to 6 minutes to allow a cron to run full refreshes every 5 mins
define('GARDENS_SITE_DATA_TTL', 1800);
define('GARDENS_SITE_DATA_VERSION_KEY', '_GARDENS_SITE_DATA_VERSION_STRING');
// It is possible to set this config var in php.ini via hosting/fields-provision.
define('GARDENS_SITE_DATA_USE_APC', get_cfg_var('gardens.disable_apc_for_sites_php') != 1);

$host = rtrim($_SERVER['HTTP_HOST'], '.');
$dir = implode('.', array_reverse(explode(':', $host)));

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
