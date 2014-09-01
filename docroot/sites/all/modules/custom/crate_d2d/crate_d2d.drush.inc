<?php
/**
 * @file
 * Drush command file.
 */

/**
 * Implements hook_drush_command().
 */
function crate_d2d_drush_command() {
  return array(
    'crate-d2d-migrate-override' => array(
      'drupal dependencies' => array('migrate'),
    ),
    'crate-d2d-migrate-undo-override' => array(
      'drupal dependencies' => array('migrate'),
    ),
  );
}

/**
 * Sets up the overrides so that they can be used.
 */
function drush_crate_d2d_migrate_override() {
  _crate_d2d_override_migrations();
}

/**
 * Undoes overrides.
 */
function drush_crate_d2d_migrate_undo_override() {
  _crate_d2d_reverse_override_migrations();
}