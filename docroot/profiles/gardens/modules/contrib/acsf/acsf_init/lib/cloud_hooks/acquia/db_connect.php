<?php

/**
 * @file
 * This file provides helper functions for running Acquia Cloud hooks.
 */

/**
 * Exit on error.
 *
 * @param String $message
 *   A message to write to sdderr.
 */
function error($message) {
  fwrite(STDERR, $message);
  exit(1);
}

/**
 * Initiates a connection to a specified database.
 *
 * In some cases, like cloud hooks, we might need to connect to the drupal database where there is no drupal bootstrap. For example, we might need to retrieve a drush compatible uri value before we can run a drush command on a site.
 *
 * @param String $site
 *   The AH site name.
 * @param String $env
 *   The AH site environment.
 * @param String $db_role
 *   The 'role' of the AH database.
 *
 * @return Resource
 *   The database resource, if the connection was established.
 */
function get_db($site, $env, $db_role) {
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
  //fwrite(STDERR, "Connecting to db: $db_name\n");
  mysql_select_db($db_name) or error('Could not select database');

  return $link;
}

