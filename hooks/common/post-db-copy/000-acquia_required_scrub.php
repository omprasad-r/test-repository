#!/usr/bin/env php
<?php

function error($message) {
  fwrite(STDERR, $message);
  exit(1);
}

$site = $argv[1];    // AH site group
$env = $argv[2];     // AH site env
$db_role = $argv[3]; // Database name

fwrite(STDERR, "Scrubbing site database. Site: $site, Env: $env, Db Role: $db_role\n");

// Load the drupal settings to get the db connection info. The D6 version is
// relatively safe to open outside of a Drupal context as it doesn't call out
// to other functions like the D7 version does.
$conf['acquia_use_early_cache'] = TRUE;
require_once sprintf('/mnt/www/site-php/%s.%s/D6-%s-%s-settings.inc', $site, $env, $env, $db_role);

// Connection info.
$user = $conf['acquia_hosting_site_info']['db']['user'];
$pass = $conf['acquia_hosting_site_info']['db']['pass'];
$db_name = $conf['acquia_hosting_site_info']['db']['name'];
$hosts = array_keys($conf['acquia_hosting_site_info']['db']['db_url_ha']);
$host = array_shift($hosts);

$link = mysql_connect($host, $user, $pass)
    or error('Could not connect: ' . mysql_error());
fwrite(STDERR, "Connecting to db: $db_name\n");
mysql_select_db($db_name) or error('Could not select database');

// Get the site name from the database.
$result = mysql_query('SELECT value FROM variable WHERE name = "gardens_misc_standard_domain"');
$value = mysql_result($result, 0);
mysql_close($link);
$standard_domain = unserialize($value);
$site_name = preg_replace('@^([^.]+)\..+@', '$1', $standard_domain);
if (empty($site_name)) {
  error('Could not retrieve standard domain from database.');
}
fwrite(STDERR, "Site name: $site_name\n");

// Get the target url suffix from the gardener.
$command = sprintf('drush @%1$s.%2$s -r /var/www/html/%1$s.%2$s/docroot -i /var/www/html/%1$s.%2$s/docroot/profiles/gardens/modules/acquia/gardens_misc gardens-get-gardener-creds --pipe', escapeshellarg($site), escapeshellarg($env));
fwrite(STDERR, "Executing command: $command\n");

$creds = json_decode(trim(shell_exec($command)));
$url_suffix = $creds->url_suffix;
if (empty($url_suffix)) {
  error('Could not retrieve site factory url suffix.');
}

$new_domain = "$site_name.$url_suffix";

$command = sprintf("drush @%s.%s -r /var/www/html/%s.%s/docroot -l %s -y gardens-sql-sanitize", escapeshellarg($site), escapeshellarg($env), escapeshellarg($site), escapeshellarg($env), escapeshellarg($new_domain));
print "Executing $command";
$result = shell_exec($command);
print $result;
// TODO: exit(1) if not scrubbed?
