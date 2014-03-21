<?php

/**
 * @file
 * Hooking into standard migrate_d2d classes to support Nonesuch-specific use
 * cases.
 */

/**
 * Implements hook_migrate_api_alter().
 */
/*
function migrate_nonesuch_migrate_api() {
  $api = array(
    'api' => 2,
    'migrations' => array(
      'MigrateNonesuchQueueGallery' => array(
        'class_name' => 'MigrateNonesuchGallery',
        'group_name' => 'NonesuchRecords',
        'source_version' => 5,
        'source_connection' => 'NonesuchRecords_legacy',
        'destination_type' => 'media_gallery',
      ),
    ),
  );

  // Deregister the gallery migration we're overriding.
  $machine_name = 'NonesuchRecordsNodegallery';
  db_drop_table('migrate_map_' . strtolower($machine_name));
  db_drop_table('migrate_message_' . strtolower($machine_name));
  db_delete('migrate_status')
    ->condition('machine_name', $machine_name)
    ->execute();
  db_delete('migrate_field_mapping')
    ->condition('machine_name', $machine_name)
    ->execute();
  return $api;
}*/

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
