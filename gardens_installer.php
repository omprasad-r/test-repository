<?php

/**
 * @file This script is run from cron and dispatches gardens site installers. It
 * should run as the hosting site user.  Usage:
 *   php gardens_installer.php hosting_site_user (install|initialize)
 */

/**
 * This script was adapted from fields-config-gardens.php.  The following was not
 * ported over from fields-config-gardens.php:
 *   1. The lock around the process which forks installer workers as it wasn't found
 *      to be neccessary.
 *   2. File globbing to run the installer worker in all locations found - we just
 *      run a single specific installer from each cron job now.
 *   3. logic for determining the hosting site user from the directory name.
 *   4. syncing of credentials (assumed done in a speparate creds deploy task).
 */

/**
 * Timeout for this script in seconds.
 */
define('GARDENS_INSTALLER_TIMEOUT', 60);

/**
 * Delay between forking installers in seconds.
 */
define('GARDENS_INSTALLER_DELAY', 5);

require_once('install_gardens.inc');

$start = time();

// Initialize logging.
openlog('gardens-installer', LOG_PID, LOG_DAEMON);

// The only valid arguments are install and initialize
$op = (empty($argv[2]) ? 'initialize' : $argv[2]);
$site = $argv[1];
if (empty($site) || !in_array($op, array('install', 'initialize')) || preg_match('/[^a-zA-Z0-9_-]/', $site)) {
  syslog(LOG_ERR, 'alert="gardens_installer_error" Invalid argument for gardens_installer.php. Op: ' . $op . "; Site: $site");
  exit;
}

$install_worker_file = escapeshellarg(dirname(__FILE__) . '/gardens_installer_worker.php');

$gardens_site_environment = acquia_gardens_get_site_environment();
do {
  // Capturing output and return value would be pointless here.
  exec(sprintf('AH_SITE_ENVIRONMENT=%s /usr/bin/php %s %s %s >/dev/null &', $gardens_site_environment, $install_worker_file, $site, $op));
  sleep(GARDENS_INSTALLER_DELAY);
} while (time() - $start < GARDENS_INSTALLER_TIMEOUT);
