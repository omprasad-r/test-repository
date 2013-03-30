<?php

/**
 * @file
 *  Drush commands for the Antivirus IEContentAnalyzer module.
 */

/**
 * Implements hook_drush_command().
 */
function antivirus_iecontentanalyzer_drush_command() {
  $items['antivirus-iecontentanalyzer-download'] = array(
    'description' => dt('Downloads the IEContentAnalyzer library from MediaWiki.'),
    'arguments' => array(
      'path' => dt('Optional. A path to the download folder. If omitted Drush will use the default location (sites/all/libraries/IEContentAnalyzer).'),
    ),
  );

  return $items;
}

/**
 * Implements hook_drush_pm_post_download().
 */
function antivirus_iecontentanalyzer_drush_pm_post_download($project, $release) {
  if ($project == 'antivirus_iecontentanalyzer') {
    drush_antivirus_iecontentanalyzer_download();
  }
}

/**
 * Drush command callback for IEContentAnalyzer library download.
 *
 * @see drush_devel_download()
 */
function drush_antivirus_iecontentanalyzer_download() {
  $args = func_get_args();
  if (isset($args[0])) {
    $path = $args[0];
  }
  else {
    $path = drush_get_context('DRUSH_DRUPAL_ROOT');
    if (module_exists('libraries')) {
      $path .= '/' . libraries_get_path('IEContentAnalyzer') . '/IEContentAnalyzer';
    }
    else {
      $path .= '/' . drupal_get_path('module', 'antivirus_iecontentanalyzer') . '/IEContentAnalyzer';
    }
  }

  if (is_dir($path) && file_exists($path . '/IEContentAnalyzer.php')) {
    drush_log('IEContentAnalyzer already present. No download required.', 'ok');
  }
  else {
    drupal_mkdir($path);
    if (drush_shell_exec('svn export ' . ANTIVIRUS_IECONTENTANALYZER_URL . ' ' . $path . '/IEContentAnalyzer.php')) {
      drush_log(dt('IEContentAnalyzer has been exported via svn to @path.', array('@path' => $path)), 'success');
    }
    else {
      drush_log(dt('Drush was unable to export IEContentAnalyzer to @path.', array('@path' => $path)), 'error');
    }
  }
}

