<?php

/**
 * @file cron script; runs automatically on Gardens web nodes.
 *
 * This script runs continually and looks for new Gardens sites associated with
 * this Hosting site that need to be installed or configured, then installs or
 * configures them.
 */

// Initialize logging.
openlog('gardens-installer-worker', LOG_PID, LOG_DAEMON);

$current_directory = dirname(__FILE__);
include_once $current_directory . '/install_gardens.inc';

// Rely on the fact that Hosting sites are installed in directories given by
// their site name.
$hosting_site_name = basename($current_directory);
// This is misnamed: parent_hosting_site_name should be the sitegroup
$parent_hosting_site_name = $argv[1];
// Obtain the current process owner in order to perform sanity checks against the
// hosting site name.
$user_info = posix_getpwuid(posix_geteuid());

// We can no longer check the site name contains the site group name, but we can
// check that the unix user and site group match.
if (empty($parent_hosting_site_name) || $parent_hosting_site_name != $user_info['name']) {
  gardens_log_and_alert_if_necessary($hosting_site_name, "[TAG] Hosting site group and process username do not match.[TAG] sitegroup: $parent_hosting_site_name user: {$user_info['name']}",
    'hosting_site_user_mismatch', array('warning' => 1, 'error' => 1));
  exit;
}

// Note: we no longer want to rely on magic naming conventions to determine if
// site installs should run on this hosting site.  It will be passed as an argument
// by cron to the parent process and passed through to this script.

try {
  $hosting_site_environment = acquia_gardens_get_site_environment();
}
catch (Exception $e) {
  // This server should not try to install new sites.
  syslog(LOG_ERR, 'gardens_installer_worker.php was invoked without the AH_SITE_ENVIRONMENT variable set.');
  exit();
}

// Make sure all Gardens sites are initialized (i.e. have their sites dir).
try {
  // When a webnode is launched, some tasks (notably domain configuration) need
  // to occur once. We use the "server_initialized" file to keep track of that
  // for each tangle. Note that we need a separate file for the upgrade tangle
  // (e.g. tangle001_up) to ensure that the sync command runs separately for
  // both tangles, rather than picking one at random. Furthermore, although
  // most automated domain syncing tasks do not need to happen on the upgrade
  // tangle (because we sync domains manually when running Gardens updates and
  // then do not allow end users to manage their domains during that time),
  // this one is an exception because if a server relaunches during the
  // upgrade, we want the upgrade tangle (which is serving live sites at that
  // time) to immediately get the correct domains in place, without requiring
  // any manual intervention.
  if (!file_exists("/mnt/tmp/{$hosting_site_name}/server_initialized")) {
    acquia_gardens_full_domain_sync(NULL, $hosting_site_environment);
    touch("/mnt/tmp/{$hosting_site_name}/server_initialized");
  }
  // Determine whether we should trigger performance logging based on the
  // presence of a file.
  $performance_logging = file_exists("/mnt/tmp/{$hosting_site_name}/install_gardens_do_performance_logging");
  // Perform the actual initialization.
  install_gardens_initialize($parent_hosting_site_name, $hosting_site_environment, FALSE, $performance_logging);
}
catch (Exception $e) {
  // Something went badly wrong, so bail out. We only want to raise a Nagios
  // alert if this happens a number of times in a short period.
  gardens_log_and_alert_if_necessary($hosting_site_name, "[TAG] AN-22784 - Hosting site failed to initialize Gardens sites [TAG] site: {$hosting_site_name}, message: " . $e->getMessage(), 'failed_to_initialize_gardens_sites');
  exit;
}

// End of script.
exit;

/**********************************************************
 * Utility functions
 */

/**
 * Create a pid file for this process, and find the install limit.
 *
 * @param $lock_dir
 *   A writeble directory to hold pid files.
 * @param $mypid
 *   A numeric process ID.
 *
 * @return
 *  Integer limit value.
 */
function gardens_pidfile_setup($lock_dir, $mypid) {
  if (!is_dir($lock_dir)) {
    mkdir($lock_dir, 0755, TRUE);
  }
  // Create a pid file with my pid.
  file_put_contents("$lock_dir/$mypid", time());
  // Default sane value.
  $limit = 6;
  // A way to dynamically change this number is setting a value in the limit file.
  $new_limit = @file_get_contents("$lock_dir/limit");
  if ($new_limit !== FALSE) {
    $new_limit = trim($new_limit);
    if (is_numeric($new_limit)) {
      $limit = intval($new_limit);
    }
  }
  return $limit;
}

/**
 * Try to unlink any files corresponding to a non-running process.
 *
 * @param $pid_files
 *   An array of filenames, such as returned by glob(), where the basename
 *   is a process ID.
 */
function gardens_pidfile_cleanup($pid_files, $hosting_site_name) {
  // Try to clean up for the next time this script runs.  Look for processes
  // owned by the gardens site unix user.
  $output = trim(shell_exec('ps -o pid= -u ' . escapeshellarg($hosting_site_name)));
  $running_pids = explode("\n", $output);
  foreach ($pid_files as $filename) {
    $pid = basename($filename);
    // Check if the file exists before trying to delete it, to prevent warnings
    // (it might have finished running and deleted itself in the meantime).
    if (!in_array($pid, $running_pids) && file_exists($filename)) {
      $time = file_get_contents($filename);
      unlink($filename);
      syslog(LOG_ERR, "Unlinked gardens install pid file $filename which was created " . date('c', $time));
    }
  }
}

/**
 * Write an error message to syslog, and trigger a Nagios alert only if this
 * happens a number of times in a short period.
 *
 * @param $hosting_site_name
 *   The name of the Hosting site that is triggering the error.
 * @param $message
 *   A message describing the error.
 * @param $error_id
 *   A string identifying the error; used to name files in the filesystem that
 *   keep track of and control the behavior for that error.
 * @param $threshold
 *   (optional) Can be passed with optional keys 'warning', 'error', and
 *   'seconds' to override the default alert thresholds.
 */
function gardens_log_and_alert_if_necessary($hosting_site_name, $message, $error_id, $threshold = array()) {
  $count_dir = "/mnt/tmp/{$hosting_site_name}/install_gardens_nagios_counts";
  if (!is_dir($count_dir)) {
    mkdir($count_dir, 0755, TRUE);
  }
  // Default to warning when there are three errors per minute and throwing an
  // error when there are six per minute. (These numbers can be dynamically
  // changed by setting values in the appropriate files for each parameter.
  // Note that 'error' must always be greater than 'warning'.)
  $threshold += array(
    'warning' => 3,
    'error' => 6,
    'seconds' => 60,
  );
  foreach (array_keys($threshold) as $property) {
    $new_value = @file_get_contents("{$count_dir}/{$error_id}.threshold.{$property}");
    if ($new_value !== FALSE) {
      $new_value = trim($new_value);
      if (is_numeric($new_value)) {
        $threshold[$property] = $new_value;
      }
    }
  }
  $count_file = "{$count_dir}/{$error_id}.counts";
  $current_errors = @file_get_contents($count_file);
  $timestamps = empty($current_errors) ? array() : array_filter(explode(',', $current_errors));
  $timestamps[] = time();
  // Write the updated file right away. We only need to keep enough entries to
  // be able to check the alert next time, but we also keep an extra one to
  // allow the code below to be a bit simpler.
  $relevant_timestamps = array_slice($timestamps, -$threshold['error']);
  file_put_contents($count_file, implode(',', $relevant_timestamps));
  // Now determine whether the error message should trigger a Nagios alert, and
  // if so, what kind.
  $alert_types = array(
    'error' => 'GardensError',
    'warning' => 'GardensWarning',
  );
  foreach ($alert_types as $type => $alert_message) {
    $threshold_time = $threshold[$type];
    $checked_timestamps = array_slice($relevant_timestamps, -$threshold_time);
    $alert = (count($checked_timestamps) >= $threshold_time && ($threshold_time < 2 || end($checked_timestamps) - reset($checked_timestamps) <= $threshold['seconds']));
    if ($alert) {
      if (strpos($message, 'Gardens') !== 0) {
        // The message Does not already contain GardensError or
        // GardensWarning at the beginning of the string.

        // The error message may have the string '[TAG]' embedded to
        // bound the static part of the string.  This indicates where
        // the GardensError or GardensWarning string should be.
        $test_message = str_replace('[TAG]', "{$alert_message}:", $message, $replace_count);
        if ($replace_count == 2) {
          // We replaced two instances of '[TAG]', this properly
          // forming the error message with a static component and a
          // dynamic component.
          $message = $test_message;
        }
        else {
          // This is not optimal because the message may include a
          // dynamic component that our alert aggregator will not be
          // able to place into a common bucket.  Just wrap the whole
          // message.
          $message = "{$alert_message}: $message {$alert_message}:";
        }
      }
      // Stop on the first (highest severity) alert we trigger.
      break;
    }
    $message = $message . " At least {$threshold_time} failures in the last {$threshold['seconds']} seconds";
  }
  syslog(LOG_ERR, $message);
}
