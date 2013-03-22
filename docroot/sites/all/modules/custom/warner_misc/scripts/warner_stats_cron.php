<?php

if (PHP_SAPI !== 'cli') {
  exit(1);
}

$time = time();

switch ($argv[1]) {
  case 'flush':
    // On a flush action, we need to truncate the
    // current day's log.
    $dir = '/var/log/warner';
    $file = 'warner_stats_' . date('N', $time) . '.log';
    $fp = fopen($dir . '/' . $file, 'w+');
    fwrite($fp, '');
    fclose($fp);
  // Intentional fall through so that either case will invoke exec_drush().
  case 'run':
    exec_drush($argv[1], $time);
    break;
  default;
echo <<<EOD
This script logs current session counts for warner sites. It also
aggregates the past 24 hours of session counts to a log file on the
server.

Usage:
$ ./warner_stats_cron.php [help|run|flush]
- run - log the current number of active sessions for each site.
- flush - write the session counts to a log file.
- help - this screen.

EOD;
    break;
}

/**
 * Helper function to get all sites from the gardener.
 */
function get_all_sites() {
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, "https://www.wmg-gardens.com/admin/reports/sites/json/a5c3d83d6a25814419b39a36abe52ad4");
  curl_setopt($ch, CURLOPT_HEADER, 0);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  $json = curl_exec($ch);
  curl_close($ch);

  if ($json) {
    return json_decode($json, TRUE);
  }
}

/**
 * Execute the drush command on all sites.
 */
function exec_drush($arg, $time) {
  // We need to track our profile and sites directories for drush, etc.,
  // so ensure that we are starting from this location.
  $current_dir = dirname(__FILE__);
  chdir($current_dir);

  chdir('../../../');
  $profile_dir = getcwd();

  chdir('../../sites/');
  $sites_dir = getcwd();

  // Iterate over each site and if it is on this tangle, execute
  // the drush command in that site's directory.
  foreach (get_all_sites() as $nid => $site) {
    $dir = preg_replace('/https?:\/\//', '', $site['url']);
    $cmd = '';

    if (is_dir($dir) || is_link($dir)) {
      switch ($arg) {
        case 'run':
          // To attempt to line up the entries on different servers, we'll
          // round down to the nearest five-minute interval.
          $min = date('i', $time);
          $min_round = floor($min/5) * 5;
          $time = mktime(date('H', $time), $min_round, 0, date('n', $time), date('j', $time), date('Y', $time));

          // Compile the current stats into the database.
          $cmd = '/usr/local/bin/drush -i ' . $profile_dir . ' --uri=http://' . $dir . ' warner-stats-run ' . $time;
          break;
        case 'flush':
          // Compile the database stats into a log file.
          $cmd = '/usr/local/bin/drush -i ' . $profile_dir . ' --uri=http://' . $dir . ' warner-stats-flush ' . date('N', $time) . ' ' . $nid . ' ' . $site['url'];
          break;
      }

      if (!empty($cmd)) {
        $out = shell_exec($cmd);
      }
    }
  }
}
