#!/usr/bin/env php
<?php

if (!drupal_is_cli()) {
  exit(1);
}

$site = $argv[1];    // AH site group
$env = $argv[2];     // AH site env
$db_role = $argv[3]; // Database name

fwrite(STDERR, "Scrubbing site database. Site: $site, Env: $env, Db Role: $db_role\n");

// Get the db connection.
require dirname(__FILE__) . '/../../acquia/db_connect.php';
$link = get_db($site, $env, $db_role);

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
$command = sprintf('drush5 @%1$s.%2$s -r /var/www/html/%1$s.%2$s/docroot -i /var/www/html/%1$s.%2$s/docroot/profiles/gardens/modules/acquia/gardens_misc gardens-get-gardener-creds --pipe', escapeshellarg($site), escapeshellarg($env));
fwrite(STDERR, "Executing command: $command\n");

$creds = json_decode(trim(shell_exec($command)));
$url_suffix = $creds->url_suffix;
if (empty($url_suffix)) {
  error('Could not retrieve site factory url suffix.');
}

$new_domain = "$site_name.$url_suffix";

$command = sprintf("drush5 @%s.%s -r /var/www/html/%s.%s/docroot -l %s -y gardens-sql-sanitize", escapeshellarg($site), escapeshellarg($env), escapeshellarg($site), escapeshellarg($env), escapeshellarg($new_domain));
print "Executing $command";
$result = shell_exec($command);
print $result;
// TODO: exit(1) if not scrubbed?
