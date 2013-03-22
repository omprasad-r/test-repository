<?php

class VideoMigration extends WarnerMediaMigration {
  public function __construct($group, $live = TRUE) {
    // Fields specific to the video type
    $specific_fields = array(
      'rating' => '',
      'duration' => '',
    );
    parent::__construct($specific_fields, 'video', 'video', $live);
    $this->description = t('Import video entities from XML into Drupal');

    $this->addFieldMapping(NULL, 'rating')
         ->xpath('rating/rating_value')
         ->description('Will we use fivestar module for this?')
         ->issueGroup('VML');
    $this->addFieldMapping('field_legacy_url', 'asset')
         ->xpath('asset/@href')
         ->callbacks(array($this, 'decodeUrl'));
    $this->addFieldMapping('field_video_duration', 'duration')
         ->xpath('asset/duration');
/*    $this->addFieldMapping("field_video_asset", 'asset')
         ->arguments(array('file_function' => 'file_link'));*/
//         ->callbacks('urldecode', array($this, 'stripArgs'))
//         ->xpath('asset/@href');
  }
  public function prepareRow($row) {
    // Make sure we actually have an asset
    $href = $row->xml->xpath('asset/@href');
    if (!is_array($href)) {
      return FALSE;
    }
    $row->asset = urldecode((string)reset($href));
  }

  protected function decodeUrl($value) {
    if (is_array($value)) {
      $value = reset($value);
    }
    return urldecode($value);
  }
  protected function stripArgs($url) {
    $stripped_url = substr($url, 0, strpos($url, '?'));
    return $stripped_url;
  }

  public function prepare($node, $row) {
    $asset = (string)reset($row->xml->xpath('asset/@href'));
//    $asset = $this->stripArgs(urldecode());
    // Skip non-Youtube content
    if (!strpos($asset, 'youtube.com')) {
      $this->fid = NULL;
      // Unpublish UGC Eos content
      if ($row->isPro != 'true') {
        $node->status = NODE_NOT_PUBLISHED;
      }
      return;
    }
//    $youtube_code = substr($asset, strrpos($asset, '/') + 1);
//    $asset = 'youtube://v/' . $youtube_code;
    $asset = urldecode($asset);
    // Unpublish invalid fan videos
    if ($row->isPro == 'false') {
      $data = oembedcore_oembed_data($asset);
      // Type is 'video' for good ones, 'link' for bad ones
      if ($data->type == 'link') {
        $node->status = NODE_NOT_PUBLISHED;
      }
    }
    $asset = 'oembed://' . urlencode($asset);

    // Until the migrate media module integration is ready, do it the hard way
    db_merge('file_managed')
      ->key(array('uri' => $asset))
      ->fields(array(
                 'uid' => $node->uid,
                 'filename' => 'video title',
                 'filemime' => 'video/oembed',
                 'filesize' => 0,
                 'status' => 1,
                 'timestamp' => time(),
                 'type' => 'video',
               ))
      ->execute();
    $fid = db_select('file_managed', 'f')
           ->fields('f', array('fid'))
           ->condition('uri', $asset)
           ->execute()
           ->fetchField();

    if ($fid) {
      $node->field_video_asset = array(
        LANGUAGE_NONE => array(
          array('fid' => $fid)
        )
      );
      $this->fid = $fid;
    }
    else {
      $this->fid = NULL;
    }
  }
  public function complete($node, $row) {
    $this->handleRating($node, $row);
    if ($this->fid) {
      $file = file_load($this->fid);
      file_usage_add($file, 'media', 'node', $node->nid);
    }
  }
}

class VideoNonLiveMigration extends VideoMigration {
  public function __construct($group) {
    parent::__construct($group, FALSE);
    $this->description = t('Import non-live video entities from XML into Drupal');
  }
}