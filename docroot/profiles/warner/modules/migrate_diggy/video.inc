<?php
/**
 * @file
 * Migration class for importing photos from ONEsite to Drupal Gardens.
 */

class MigrateDiggyVideo extends OnesiteVideoMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_legacy_url', 'external_video_id');
    $this->addFieldMapping('field_is_pro', 'is_pro')
         ->defaultValue(0);
    $this->addFieldMapping('field_video_category', 'video_category')
         ->defaultValue('Fans');
    $this->addFieldMapping('field_video_category:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_video_duration', 'duration');

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
      'field_download_url', 'field_download_url:language',
        'field_legacy_url:language',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
        'field_video_category:source_type', 'field_video_category:ignore_case',
        'field_video_duration:language',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // These are the IDs of the diggy and diggy_webcrew accounts
    if ($row->uploaded_by == 118164219 || $row->uploaded_by == 121331489) {
      $row->video_category = 'Official';
    }
  }
}
