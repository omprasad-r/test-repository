<?php

class AudioMigration extends WarnerMediaMigration {
  public function __construct($group, $live = TRUE) {
    // Fields specific to the audio type
    $specific_fields = array(
      'rating' => '',
      'lyrics' => '',
      'download_url' => '',
      'video_url' => '',
      'thumbnail' => '',
    );
    parent::__construct($specific_fields, 'audio', 'audio', $live);
    $this->description = t('Import audio entities from XML into Drupal');

    $this->addFieldMapping('field_rating')
         ->description('Constructed at run-time');
    $this->addFieldMapping('field_audio_asset')
         ->issueGroup('DNM');
    $this->addFieldMapping('field_legacy_url', 'asset')
         ->xpath('asset/@href')
         ->callbacks(array($this, 'fixAsset'));
    $this->addFieldMapping('field_lyrics', 'lyrics')
         ->xpath('songLyrics')
         ->callbacks(array($this, 'fixCharset'))
         ->arguments(array('format' => $this->plain_text));
    $this->addFieldMapping('field_download_url', 'download_url')
         ->xpath('songDownloadUrl');
    $this->addFieldMapping('field_video_url', 'video_url')
         ->xpath('songVideoUrl');
    $this->addFieldMapping('field_audio_image', 'thumbnail')
         ->arguments(array('file_function' => 'file_copy', 'file_replace' => FILE_EXISTS_RENAME))
         ->xpath('thumbnail')
         ->callbacks(array($this, 'fixThumbnail'));
  }

  protected function fixCharset($string) {
    $string = iconv('Windows-1252', 'UTF-8', $string);
    $string = str_replace('Â', '', $string);
    return $string;
  }

  protected function fixAsset($value) {
    if (is_array($value)) {
      $value = $value[0];
    }
    $value = urldecode($value);
    return $value;
  }

  protected function fixThumbnail($value) {
    $value = str_replace('.jpg', '_8.jpg', $value);
    return $value;
  }

  public function complete($node, $row) {
    $this->handleRating($node, $row);
  }
}

class AudioNonLiveMigration extends AudioMigration {
  public function __construct($group) {
    parent::__construct($group, FALSE);
    $this->description = t('Import non-live audio entities from XML into Drupal');
  }
}
