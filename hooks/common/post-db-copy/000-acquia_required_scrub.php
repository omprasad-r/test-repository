#!/usr/bin/env php
<?php

$site = $argv[1];    // AH site group
$env = $argv[2];     // AH site env
$db_role = $argv[3]; // Database name

fwrite(STDERR, "Scrubbing site database. Site: $site, Env: $env, Db Role: $db_role\n");

// Get the db connection.
require dirname(__FILE__) . '/../../acquia/db_connect.php';
$link = get_db($site, $env, $db_role);

// Get the site name from the database.
$result = mysql_query('SELECT value FROM acsf_variables WHERE name = "acsf_site_info"');
$value = mysql_result($result, 0);
mysql_close($link);
$site_info = unserialize($value);
$site_name = $site_info['site_name'];
if (empty($site_name)) {
  error('Could not retrieve standard domain from database.');
}
fwrite(STDERR, "Site name: $site_name\n");

// Locate the acsf module to get the factory creds.
$acsf_location = realpath(dirname(trim(shell_exec(sprintf('find /var/www/html/%s.%s/docroot/ -name acsf.drush.inc', $site, $env)))));
if (empty($acsf_location)) {
  error('Could not locate the ACSF module.');
}

// Get the target url suffix from the gardener.
$command = sprintf('AH_SITE_GROUP=%1$s AH_SITE_ENVIRONMENT=%2$s drush5 @%1$s.%2$s -r /var/www/html/%1$s.%2$s/docroot -i %3$s acsf-get-factory-creds --pipe', escapeshellarg($site), escapeshellarg($env), escapeshellarg($acsf_location));
fwrite(STDERR, "Executing command: $command\n");

$creds = json_decode(trim(shell_exec($command)));
$url_suffix = $creds->url_suffix;
if (empty($url_suffix)) {
  error('Could not retrieve site factory url suffix.');
}

// Create a new standard domain name.
$new_domain = "$site_name.$url_suffix";

// Scrub the ACSF modules.
$command = sprintf("drush5 @%s.%s -r /var/www/html/%s.%s/docroot -l %s -y acsf-site-scrub", escapeshellarg($site), escapeshellarg($env), escapeshellarg($site), escapeshellarg($env), escapeshellarg($new_domain));
print "Executing $command";
$result = shell_exec($command);
print $result;

// TODO: exit(1) if not scrubbed?
