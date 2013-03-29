<?php

/**
 * Update settings script - run this manually to update all settings.php files.
 *
 * NOTE: You must have the AH_SITE_ENVIRONMENT variable set properly
 * to run this script.
 */

// Initialize logging.
openlog('gardens-update-settings', LOG_PID, LOG_DAEMON);

// Define the files we will use.
$autorun_file = __FILE__;
$current_directory = dirname($autorun_file);

include_once $current_directory . '/install_gardens.inc';

// Rely on the fact that Hosting sites are installed in directories given by
// their site name.
$hosting_site_name = basename($current_directory);

try {
  $hosting_site_environment = acquia_gardens_get_site_environment();
}
catch (Exception $e) {
  // This server should not try to update settings files.
  syslog(LOG_ERR, 'update_settings.php was invoked without the AH_SITE_ENVIRONMENT variable set.');
  exit();
}

// Make sure all Gardens sites are initialized (i.e. have their sites dir). and
// update their settings.php file if needed.
try {
  install_gardens_initialize($hosting_site_name, $hosting_site_environment, TRUE);
}
catch (Exception $e) {
  // Something went badly wrong.
  print "Hosting site $hosting_site_name failed to initialize Gardens sites with the following error message: " . $e->getMessage() . "\n";
  exit;
}

