<?php

/**
 * Command-line script to install Gardens in a Hosting environment.
 *
 * NOTE: This is potentially dangerous, and should only be used on development
 * clusters (or if you really really really know what you're doing)!
 *
 * Example:
 * php hosting_install_gardens.php tangle001 g1000 mysite.gsteamer.acquia-sites.com [other-parameters]
 *
 * This will run an installation on the current server, creating a Gardens site
 * with ID 'g1000' as part of the Hosting tangle 'tangle001', accessible via
 * 'mysite.gsteamer.acquia-sites.com'.
 *
 * (Any other parameters provided to the script are passed along to
 * install_gardens.php and used there in the standard way.)
 *
 * This script only works if the Gardens site ID corresponds to a database that
 * has already been registered with Hosting. Furthermore, the site will only be
 * accessible at the subsequent URL if the domain is at some point registered
 * with Hosting as well.
 */

require_once dirname(__FILE__) . '/install_gardens.inc';

if (empty($argv[1]) || empty($argv[2]) || empty($argv[3])) {
  print "Error: Required parameters not provided.\n";
  exit(1);
}

array_shift($argv);
$hosting_site_name = array_shift($argv);
$gardens_site_id = array_shift($argv);
$domain = array_shift($argv);
$domains = array($domain);

try {
  $hosting_site_environment = acquia_gardens_get_site_environment();
}
catch (Exception $e) {
  // This server should not try to install new sites.
  print 'hosting_install_gardens.php was invoked without the AH_SITE_ENVIRONMENT variable set.';
  exit();
}

// If we are still getting installation failures after two minutes have passed,
// we'll assume something went wrong.
$max_duration = 120;
$max_end_time = time() + $max_duration;

while (time() < $max_end_time) {
  try {
    // Make sure the shared sites directory is in place.
    $sites_directory = acquia_gardens_sites_directory($hosting_site_name);
    acquia_gardens_ensure_directory($sites_directory);

    // Prepare the directory for installation.
    $site_directory = acquia_gardens_site_directory($hosting_site_name, $gardens_site_id);
    acquia_gardens_prepare_for_installation($site_directory, $hosting_site_name, $hosting_site_environment, $gardens_site_id, $domains);

    // Run the external installation script, passing along any additional
    // parameters that were provided, and a random user password.
    $user_password1 = _acquia_gardens_user_password();
    $user_password2 = _acquia_gardens_user_password();
    $parameters = implode(' ', $argv);
    $command = "php -d memory_limit=128M /var/www/html/{$hosting_site_name}/install_gardens.php user1_pass=\"{$user_password1}\" user2_pass=\"{$user_password2}\" $parameters url=\"http://{$domain}\"";
    print "Running the following command to install $domain:\n$command\n";
    exec($command, $output, $return_var);

    if (empty($return_var)) {
      print $output;
      exit;
    }
    else {
      // Indicate that the installation failed for now. Note that we throw an
      // exception here and catch it below because this is not the only place
      // where an exception might be thrown; e.g., the other functions called
      // above might throw them, and we need to catch those also.
      throw new Exception("Drupal command line installation failed.");
    }
  }
  catch (Exception $e) {
    sleep(2);
    $time_left = $max_end_time - time();
    print "Installation was unsuccessful. Trying again for $time_left more seconds.\nError message was: {$e->getMessage()}\n";
  }
}

// If we get here, something went wrong.
print "INSTALLATION FAILURE: Could not install Gardens after trying for $max_duration seconds.\n";
exit(1);
