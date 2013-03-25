#!/usr/bin/env php
<?php

// Dumps data about javascript libraries to a csv file.
// Run this from the root directory of the gardens codebase.


$columns = array('site', 'id', 'type', 'scope', 'name', 'weight', 'uri');

chdir('docroot/sites');

if (!empty($argv[1])) {
  $sites = glob($argv[1]); 
}
else {
  $sites = glob('*.acquia-sites.com');
}

$outfile = '/tmp/js_lib_' . uniqid() . '.csv';

$fp = fopen($outfile, 'w');

fputcsv($fp, $columns, "|");

foreach ($sites as $name) {

  $site = 'http://' . $name;
  $json = shell_exec("drush -l $site eval '\$d=variable_get(\"javascript_libraries_custom_libraries\"); echo json_encode(\$d);'");
  $data = json_decode($json, TRUE);
  foreach ($data as $id => $lib) {
    $row = array();
    $lib['site'] = $name;
    foreach ($columns as $key) {
      $row[] = isset($lib[$key]) ? $lib[$key] : '';
    }
    fputcsv($fp, $row, "|");
  }
}

fclose($fp);

echo "\nresults in $outfile\n";
