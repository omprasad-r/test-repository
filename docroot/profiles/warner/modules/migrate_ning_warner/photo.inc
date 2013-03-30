<?php
/**
 * @file
 *
 */

class NingWarnerPhotoMigration extends NingPhotoMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_is_pro', 'is_pro');
    $this->addFieldMapping('field_photo_asset', 'photoUrl');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileUri');
    $this->addFieldMapping('field_photo_asset:file_replace')
         ->defaultValue(MigrateFileUri::FILE_EXISTS_REUSE);
    $this->addFieldMapping('field_photo_asset:title', 'title');

    $this->addFieldMapping('field_photo_category', 'category');
    $this->addFieldMapping('field_photo_category:create_term')
         ->defaultValue(TRUE);
    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:destination_dir',
          'field_photo_asset:destination_file', 'field_photo_asset:preserve_files',
          'field_photo_asset:source_dir', 'field_photo_asset:data',
        'field_photo_category:source_type', 'field_photo_category:ignore_case',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    if (migrate_ning_warner_is_admin($row->contributorName)) {
      $row->is_pro = 1;
      $row->category = 'Official';
    }
    else {
      $row->is_pro = 0;
      $row->category = 'Fans';
    }
  }
}
