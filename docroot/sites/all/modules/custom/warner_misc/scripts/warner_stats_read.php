<?php

if (PHP_SAPI !== 'cli') {
  exit(1);
}

if (empty($argv[1]) || $argv[1] == 'help') {
echo <<<EOD
This script obtains a day's log file from each tangle and aggregates them.

Usage:
$ ./warner_stats_read.php help|day-of-week [all] [test]
  - day-of-week (e.g. 1 for monday, 7 for sunday)
  - all - Optional, print each site's individual stats for each time. If this is omitted, only the aggregated total will be displayed.
  - test - Optional, use http://shawncolvin.wmg-egardens.dev locally to retrieve stats.
e.g. warner_stats_read.php 1 all

EOD;
  exit(1);
}

// Ping one site per tangle to print the aggregated data
// from that tangle. For testing locally, we'll use some
// fake sites.
if (isset($argv[3]) && $argv[3] == 'test') {
  $sites = array(
    'http://shawncolvin.wmg-egardens.dev', // Testing
  );
}
else {
  $sites = array(
    // @todo - move these to clone sites so that we aren't affecting "real" ones.
    'http://www.shawncolvin.com/',                  // managed-11 (tangle001)
    'http://www.simpleplan.com',                    // managed-46 (tangle002)
    'http://jeffthebrotherhood.wmg-gardens.com/',   // managed-78 (tangle003)
  );
}

// Specify what day - date("N")
$day = isset($argv[1]) ? $argv[1] : 1;
$all = isset($argv[2]) ? TRUE : FALSE;

$total = array();
// Ping each tangle and aggregate their data by timestamp.
foreach ($sites as $site) {
  $stats = get_site_stats($site, $day);

  // Aggregate data.
  foreach ($stats as $timestamp => $data) {
    if (!isset($total[$timestamp]['total'])) {
      $total[$timestamp]['total'] = 0;
    }

    $total[$timestamp]['total'] += $data['total'];
    unset($data['total']);
    if ($all) {
      foreach ($data as $site => $count) {
        $total[$timestamp][$site] = $count;
      }
    }
  }
}

// Output the data.
foreach ($total as $timestamp => $sites) {
  foreach ($sites as $site => $count) {
    print date('H:i', $timestamp) . ",$site,$count \n";
  }
}

/**
 * Helper function to get the site's data.
 */
function get_site_stats($url, $day) {
  $hash = md5('gardens-' . $url);
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url . '/report-session-count/' . $day . '/' . $hash);
  curl_setopt($ch, CURLOPT_HEADER, 0);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  $json = curl_exec($ch);
  curl_close($ch);

  if ($json) {
    return json_decode($json, TRUE);
  }
}

