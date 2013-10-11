<?php

/**
 * @file
 * This file regenerates the APC cache from JSON sites data.
 *
 * This script can only be called via curl from localhost, specifying the
 * correct port number for the site.  A GET parameter containing info about the
 * JSON file for verification must be provided in the format mtime-size-inode. eg:
 *
 *   INFO=`stat -c '%Y-%s-%i' /mnt/files/<sitename>.<env>/files-private/sites.json`; curl "http://localhost:<port>/apc_rebuild.php?i=$INFO"
 *
 * The full line to add this cron job should look like this (including
 * appropriate escaping):
 *
 *  ./fields-provision.php --cron-add <sitename>:* /5:*:*:*:* --server-filter all --cmd 'INFO=`stat -c "\%Y-\%s-\%i" /mnt/files/<sitegroup>.<env>/files-private/sites.json`; curl "http://localhost:<port>/apc_rebuild.php?i=$INFO"'
 *
 * Note that in the above line, the * /5 should not contain spaces, but removing
 * the space from this comment results in the PHP comment block ending
 * prematurely.
 */

$file = "/mnt/files/{$_ENV['AH_SITE_GROUP']}.{$_ENV['AH_SITE_ENVIRONMENT']}/files-private/sites.json";
if (!file_exists($file)) {
  syslog(LOG_ERR, sprintf('APC cache update could not be executed, as the JSON file [%s] is missing.', $file));
  die('Missing sites file.');
}

// We also pass some info about the source file for minimal authentication.
$info = implode('-', array(filemtime($file), filesize($file), fileinode($file)));
if (empty($_GET['i']) || $_GET['i'] !== $info) {
  syslog(LOG_ERR, sprintf('APC cache update verification parameter [%s] does not match actual data [%s].', $_GET['i'], $info));
  die('Invalid.');
}

include_once('./sites/sites.php');

if (!empty($_GET['domains'])) {
  $domains = explode(',', $_GET['domains']);
  gardens_site_data_refresh_domains($domains);
  syslog(LOG_INFO, sprintf('Updated APC cache for [%s].', $_GET['domains']));
}
else {
  gardens_site_data_refresh_all();
  syslog(LOG_INFO, 'Updated APC cache for all domains.');
}
