#!/usr/bin/env php
<?php

define('CHECK_TIMEOUT', 300);
define('CHECK_INTERVAL', 10);
define('CHECK_VARIABLE', 'gsite_database_scrubbed');
define('CHECK_VALUE', 'scrubbed');

$site = $argv[1];    // AH site group
$env = $argv[2];     // AH site env
$db_name = $argv[3]; // Database name

echo "Blocking cloud hooks until site scrub is complete. Site: $site, Env: $env, Db: $db_name\n";

$timeout = time() + CHECK_TIMEOUT;

// Load the drupal settings to get the db connection info. The D6 version is
// relatively safe to open outside of a Drupal context as it doesn't call out
// to other functions like the D7 version does.
$conf['acquia_use_early_cache'] = TRUE;
require_once sprintf('/mnt/www/site-php/spartacus.live/D6-%s-settings.inc', $db_name);

// Connection info.
$user = $conf['acquia_hosting_site_info']['db']['user'];
$pass = $conf['acquia_hosting_site_info']['db']['pass'];
$hosts = array_keys($conf['acquia_hosting_site_info']['db']['db_url_ha']);
$host = array_shift($hosts);

$link = mysql_connect($host, $user, $pass, $mysql_port)
    or die('Could not connect: ' . mysql_error());
echo "Connecting to db: $db_name\n";
mysql_select_db($db_name) or die('Could not select database');

// Look up the scrubbed status and wait if it is incomplete.
// 'gsite_database_scrubbed', 's:8:\"scrubbed\";'
$value = '';
while (time() < $timeout) {
  if ($result = mysql_query(sprintf('SELECT value FROM variable WHERE name = "%s"', CHECK_VARIABLE))) {
    @$value = mysql_result($result, 0);
    if (strstr($value, CHECK_VALUE)) {
      echo "Site Factory site scrub complete.\n";
      break;
    }
  }
  echo "Site Factory site scrub incomplete, waiting.\n";
  sleep(CHECK_INTERVAL);
}

// Exit non-zero if we were never able to confirm the scrub.
if (empty($value)) {
  echo "Unable to confirm site scrub.\n";
  exit(1);
}

mysql_close($link);

