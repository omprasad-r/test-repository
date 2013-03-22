<?php

class PhotoMigration extends WarnerMediaMigration {
  // The value of the thumbnail URL for the current run - just used to
  // support fixing up ning.com photos
  protected $thumbnail;

  public function __construct($group, $live = TRUE) {
    // Fields specific to the photo type
    $specific_fields = array(
      'thumbnail' => '',
    );
    parent::__construct($specific_fields, 'photo', 'photo', $live);
    $this->description = t('Import photo entities from XML into Drupal');
    // Need to have a destination for the callbacks to be called
    $this->addFieldMapping('dummy_thumbnail', 'thumbnail')
         ->xpath('thumbnail')
         ->callbacks(array($this, 'fixThumbnail'));
    $this->addFieldMapping('field_photo_asset', 'asset')
         ->arguments(array('file_function' => 'file_link', 'file_replace' => FILE_EXISTS_RENAME))
         ->callbacks(array($this, 'applyUrlDecode'), array($this, 'hashFilename'))
         ->xpath('asset/@href');
  }

  /**
   * All photos have a filename of thumbnail_8.jpg - using the default
   * file_copy function, the automatic deduping in core uses sequence numbers,
   * generating (eventually) thousands of file_exists() calls for each
   * migrated node - in the production Gardens environment, this performance
   * unacceptable. So, what we do instead is copy the file ourselves to
   * a hashed filename, and ask the file handler to link to it.
   *
   * @param $filename
   * @return string
   */
  protected function hashFilename($filename) {
    $hashed_filename = 'public://' . hash('md4', $filename) . '.jpg';
    // Ning URLs don't work directly, use the hacked thumbnail value
    if (strpos($filename, 'api.ning.com') === FALSE) {
      @copy($filename, $hashed_filename);
    }
    else {
      @copy($this->thumbnail, $hashed_filename);
    }
    return $hashed_filename;
  }

  protected function fixThumbnail($value) {
    $value = str_replace('.jpg', '_8.jpg', $value);
    $this->thumbnail = $value;
    return $value;
  }
}

class PhotoNonLiveMigration extends PhotoMigration {
  public function __construct($group) {
    parent::__construct($group, FALSE);
    $this->description = t('Import non-live photo entities from XML into Drupal');
  }
}
