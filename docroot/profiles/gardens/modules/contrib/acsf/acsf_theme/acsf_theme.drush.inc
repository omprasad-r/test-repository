<?php

/**
 * Implements hook_drush_command()
 */
function acsf_theme_drush_command() {
  return array(
    'acsf-theme-changed' => array(
      'description' => dt('Sends notification to the factory that a theme has changed.'),
    ),
  );
}

/**
 * Drush command. Rebuilds the ACSF registry.
 */
function drush_acsf_theme_changed() {
  $response = _acsf_theme_change_notification();
  drush_print($response['message']);
}
