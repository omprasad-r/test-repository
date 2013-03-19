#!/usr/bin/env php
<?php

/*
 * A simple php cli script that creates lots and lots of garden sites. 
 * 
   Put this script in gardener docroot, then run it from the shell like so
   
   ./garden_grow.php 100 5000
   
*/

// Default settings
$uid = 9991;
$start = 2;           // New site naming starts from this number
$num_sites = 10;      // How many additional Gardens to create
$start_time = time();

// Command line arguments (positional arguments because this is a hack)
if ($argc) {
  if (isset($argv[1])) {
    $start = $argv[1];
  }
  if (isset($argv[2])) {
    $num_sites = $argv[2];
  }
}
print "DEBUG: start: $start, num_sites: $num_sites\n";

echo "DEBUG: Bootstrapping Drupal...\n";
include_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

gardens_signup_includes();
if (!defined('GARDENS_SIGNUP_DEFAULT_URL_PROTOCOL')) {
  echo "ERROR: Could not include gardens_signup.inc\n";
  exit(1);
}

echo "DEBUG: Creating many Gardens sites...\n";
for ($i=$start; $i<($start+$num_sites); $i++) {
  $site_name = 'kurttest' . $i;
  $domain = $site_name . '.kurt.acquia-sites.com';
  $start_time_task = time();
  // echo "DEBUG: gardens_signup_get_url_from_domain($domain) = ". gardens_signup_get_url_from_domain($domain) ."\n";
  // gardens_signup_add_site($uid, $domain = NULL, $site_name, $template = NULL, $features = NULL, $operation = GARDENS_SIGNUP_SITE_OPERATION_INSTALL_AND_CONFIGURE)
  $node = gardens_signup_add_site(
      $uid, 
      $domain,
      $site_name, 
      NULL, 
      NULL, 
      GARDENS_SIGNUP_SITE_OPERATION_INSTALL_AND_CONFIGURE);
  
  if (!$node) {
    echo "ERROR: gardens_signup_add_site() did not return a node object.\n";
    exit(1);
  }
  
  // Garden site created
  $time_now = time();
  $elapsed = $time_now - $start_time_task;
  $total_elapsed = $time_now - $start_time;
  echo "DEBUG: $total_elapsed $elapsed Created $site_name\n";
}


?>
