<?php

/**
 * @file
 * Hooking into standard migrate_d2d classes to support Nonesuch-specific use
 * cases.
 */

/**
 * Implements hook_migrate_api_alter().
 */
function migrate_nonesuch_migrate_api() {
  $api = array(
    'api' => 2,
    /* @todo: Enable when runnable
    'migrations' => array(
      'MigrateNonesuchQueueGallery' => array(
        'class_name' => 'MigrateNonesuchGallery',
        'group_name' => 'NonesuchRecords',
        'source_version' => 5,
        'source_connection' => 'NonesuchRecords_legacy',
        'destination_type' => 'media_gallery',
      ),
    ),*/
  );
  return $api;
}

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
  return $api;
}
