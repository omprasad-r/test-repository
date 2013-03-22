<?php
/**
 * @file
 *
 */

class NingWarnerVideoMigration extends NingVideoMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_is_pro', 'is_pro');
    $this->addFieldMapping('field_video_asset', 'embedCode');
    $this->addFieldMapping('field_video_asset:file_class')
         ->defaultValue('MigrateExtrasFileYoutube');
    $this->addFieldMapping('field_video_asset:title', 'title');

    $this->addFieldMapping('field_video_category', 'category');
    $this->addFieldMapping('field_video_category:create_term')
         ->defaultValue(TRUE);

    $this->addFieldMapping('field_legacy_url', 'videoAttachmentUrl');

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
      'field_download_url', 'field_download_url:language',
        'field_video_asset:language', 'field_video_asset:data',
        'field_video_category:source_type', 'field_video_category:ignore_case',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
        'field_legacy_url:language',
      'field_video_duration', 'field_video_duration:language',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    if (isset($row->embedCode)) {
      if (preg_match('|http://(www.)?youtube.com/[^\\"]+|', $row->embedCode, $matches)) {
        $row->videoAttachmentUrl = $matches[0];
        // file_managed table only takes 255 characters for uri, and some of
        // these have long argument lists - remove everything from the first
        // &amp; onward.
        if (($offset = strpos($matches[0], '&amp;')) > 0) {
          $row->embedCode = substr($matches[0], 0, $offset);
        }
        else {
          $row->embedCode = $matches[0];
        }
        // Some URLs have a /e/ or /embed/ that the Media implementation (or
        // oembed) doesn't like, translate to /v/
        $row->embedCode = str_replace('/e/', '/v/', $row->embedCode);
        $row->embedCode = str_replace('/embed/', '/v/', $row->embedCode);
      }
      else {
        return FALSE;
      }
    }
    else {
      return FALSE;
    }

    if (empty($row->title)) {
      $row->title = 'Video';
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
