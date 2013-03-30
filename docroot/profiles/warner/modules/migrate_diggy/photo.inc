<?php
/**
 * @file
 * Migration class for importing photos from ONEsite to Drupal Gardens.
 */

class MigrateDiggyPhoto extends OnesitePhotoMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_photo_category', 'gallery_name');
    $this->addFieldMapping('field_photo_category:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_photo_caegory:ignore_case')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:destination_dir',
          'field_photo_asset:destination_file', 'field_photo_asset:file_replace',
          'field_photo_asset:preserve_files', 'field_photo_asset:data',
        'field_photo_category:source_type',
      'field_is_pro',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // These are the IDs of the diggy and diggy_webcrew accounts
    if ($row->owner_id == 118164219 || $row->owner_id == 121331489 ||
        $row->owner_id == 121332769) {
      if (empty($row->gallery_name)) {
        $row->gallery_name = 'Official';
      }
    }
    elseif ($row->gallery_name != 'Cellstream') {
      $row->gallery_name = 'Fans';
    }
  }
}
