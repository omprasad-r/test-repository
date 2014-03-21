<?php

/**
 * @file
 * Hooking into standard migrate_d2d classes to support Nonesuch-specific use
 * cases.
 */

/**
 * Implements hook_migrate_api_alter().
 */
function migrate_nonesuch_migrate_api_alter(&$api) {
  // Make sure the album migration uses our overridden class, to handle
  // UPCs and prices.
  db_update('migrate_status')
    ->fields(array('class_name' => 'MigrateNonesuchAlbum'))
    ->condition('machine_name', 'NonesuchRecordsNodealbum')
    ->execute();

  // Ditto for gallery migration.
  db_update('migrate_status')
    ->fields(array('class_name' => 'MigrateNonesuchGallery'))
    ->condition('machine_name', 'NonesuchRecordsNodegallery')
    ->execute();

  return $api;
}
