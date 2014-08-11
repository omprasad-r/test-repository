<?php


function migrate_ceelo_migrate_api() {
  $api = array(
    'api' => 2,
    'groups' => array(
      'ceelo' => array(
        'title' => t('Ceelo Green Content'),
        'default_format'    => 'full_html',
        'default_user'      => 1,
        'source_dir'        => variable_get('ceelo_migrate_files_path', '/data/files/files'),
      ),
    ),
    'migrations' => array(
      'Incentive' => array(
        'class_name' => 'TeslaIncentiveMigration',
        'group_name' => 'tesla',
        'source_type'      => 'incentive',
        'destination_type' => 'incentive',
        'description'      => 'Incentive Migration',
        'user_migration'    => 'User',
      ),
    ),
  );

  return $api;
}