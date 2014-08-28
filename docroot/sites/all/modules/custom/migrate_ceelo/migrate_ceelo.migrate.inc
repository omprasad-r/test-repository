<?php


function migrate_ceelo_migrate_api() {
  $api = array(
    'api' => 2,
    'destination handlers' => array(
      'MigrateRedirectEntityHandler',
    ),
    'groups' => array(
      'ceelo' => array(
        'title' => t('Ceelo Green Content'),
        'default_format'    => 'full_html',
        'default_user'      => 'Webcrew',
        'source_dir'        => variable_get('ceelo_migrate_files_path', '/data/files/files'),
      ),
    ),
    'migrations' => array(
      'News' => array(
        'class_name'  => 'CeeloNewsMigration',
        'group_name'  => 'ceelo',
        'description' => 'News Migration',
      ),
      'Photo' => array(
        'class_name'   => 'CeeloPhotoMigration',
        'group_name'   => 'ceelo',
        'description'  => 'Photo Migration',
      ),
      'Video' => array(
        'class_name'   => 'CeeloVideoMigration',
        'group_name'   => 'ceelo',
        'description'  => 'Photo Migration',
      ),
    ),
  );

  return $api;
}