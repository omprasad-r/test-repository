<?php

/**
 * @file
 * Drush commands for the Antivirus module.
 */

/**
 * Implements hook_drush_command().
 */
function antivirus_drush_command() {
  $items = array();

  $items['antivirus-scan'] = array(
    'description' => 'Scan files directories for viruses.',
    'aliases' => array('av-scan', 'avs'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  return $items;
}

/**
 * Implements hook_drush_help().
 */
function antivirus_drush_help($section) {
  switch ($section) {
    case 'antivirus:antivirus-scan':
      return dt('This command scans uploaded files directories for viruses.');
  }
}

/**
 * Scans uploaded files directories for viruses.
 */
function drush_antivirus_scan() {
  antivirus_scan(array('TestScanner'))->addTargets(array('public:///'))->scan();
  return drush_print('Scanning...') . drush_print('Done!');
}

