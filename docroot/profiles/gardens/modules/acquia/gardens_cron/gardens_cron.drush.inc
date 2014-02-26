<?php

/**
 * @file
 * Drush commands for the gardens cron module.
 */

/**
 * Implements hook_drush_command().
 */
function gardens_cron_drush_command() {
  $items['gardens-cron'] = array(
    'description' => 'Runs cron on a specified list of modules.',
    'arguments' => array(
      'module_list' => 'A comma-separated list of modules to run cron on.',
    ),
  );

  return $items;
}

/**
 * Drush command callback; runs cron.
 */
function drush_gardens_cron($module_list) {
  if (!empty($module_list)) {
    $modules = explode(',', $module_list);
    gardens_cron_select_cron($modules);
  }
}
