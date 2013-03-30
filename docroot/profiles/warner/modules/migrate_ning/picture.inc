<?php
/**
 * @file
 * Support for migrating Ning profile photos to Drupal.
 */

class NingPictureMigration extends NingMigration {
  /**
   * Filename of default picture, which Ning export copies for each member. We
   * want to avoid making many copies, and need to manually set it on the Drupal
   * side, so we ignore the specified picture.
   *
   * @var string
   */
  protected $defaultPicture = '';

  /**
   * Constructor - handles generally-applicable arguments.
   * @param array $arguments
   *  default_picture (optional) - Filename of the default picture on Ning
   *    (e.g., if the Ning filespec looks like members/20001-21000/20988-photo.jpg?crop=1%3A1,
   *    pass 'photo.jpg').
   */
  public function __construct($arguments) {
    $arguments['source_feature'] = 'members';
    parent::__construct($arguments);
    if (isset($arguments['default_picture'])) {
      $this->defaultPicture = $arguments['default_picture'];
    }

    // Just take the necessary members fields.
    $this->sourceFields = array(
      'createdDate' => 'Account creation date',
      'profilePhoto' => 'URL/filespec of profile photo',
      'contributorName' => 'Internal username',
      // Computed to provide a valid destination filename (no query params)
      'filename' => 'Filename relative to destination directory',
    );

    $this->source = new MigrateSourceJSON($this->sourceFile, 'contributorName',
      $this->sourceFields, array('cache_counts' => TRUE, 'cache_key' => $this->machineName));

    $this->destination = new MigrateDestinationFile();

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'contributorName' =>
          array('type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'description' => 'Unique member ID',
               ),
        ),
        MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('value', 'profilePhoto');
    $this->addFieldMapping('timestamp', 'createdDate');
    $this->addFieldMapping('destination_dir')
         ->defaultValue('public://pictures');
    $this->addFieldMapping('destination_file', 'filename');
    $this->addFieldMapping('file_replace')
         ->defaultValue(MigrateFile::FILE_EXISTS_REUSE);

    // Default ownership to the administrator account for now, since we don't
    // know the owner's uid yet - user migration will fill that in.
    $this->addFieldMapping('uid')
         ->defaultValue(1);

    $this->addUnmigratedDestinations(array(
      'preserve_files',
      'source_dir',
    ));

    if (module_exists('redirect')) {
      $this->addUnmigratedDestinations(array('migrate_redirects'));
    }
  }

  /**
   * Implements Migration::prepareRow().
   *
   * @param $row
   *
   * @return bool
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Strip query arguments (like ?crop=1%3A1).
    $stripped_filename = substr($row->profilePhoto, 0, strpos($row->profilePhoto, '?'));
    if ($this->defaultPicture) {
      // If this is a copy of the default picture, skip it
      $filename = substr($stripped_filename, strrpos($stripped_filename, '/') + 1);
      if ($this->useRemoteFiles) {
        if ($filename == $this->defaultPicture) {
          return FALSE;
        }
        $row->filename = $filename;
        $row->profilePhoto = $stripped_filename;
      }
      else {
        $pattern = '|^[0-9]+-' . $this->defaultPicture . '|';
        if (preg_match($pattern, $filename)) {
          return FALSE;
        }
      }
    }

    // Get the full filespec of the file.
    if (!$this->useRemoteFiles) {
      $row->profilePhoto = $this->sourceDirectory . '/' . $stripped_filename;
    }
    if (!$this->copyFile($row->profilePhoto, 'public://pictures/')) {
      return FALSE;
    }
  }
}
