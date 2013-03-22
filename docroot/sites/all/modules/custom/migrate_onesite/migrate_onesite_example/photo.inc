<?php
/**
 * @file
 * Migration class for importing photos from ONEsite to Drupal Gardens.
 */

class MigrateOnesiteExamplePhoto extends OnesitePhotoMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_photo_category')
         ->defaultValue('Fan Photos');
    $this->addFieldMapping('field_photo_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:destination_dir',
          'field_photo_asset:destination_file', 'field_photo_asset:file_replace',
          'field_photo_asset:preserve_files', 'field_photo_asset:data',
        'field_photo_category:source_type', 'field_photo_category:ignore_case',
    ));
  }
}
