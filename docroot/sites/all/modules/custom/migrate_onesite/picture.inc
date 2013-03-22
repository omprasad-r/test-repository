<?php
/**
 * @file
 * Migration class for importing user pictures from ONEsite to Drupal. May be
 * extended, particularly to add project-specific field mappings.
 */

class OnesitePictureMigration extends OnesiteMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = 'Migration of user pictures from ONEsite';

    $this->sourceFields = array(
      // user table - all we care about
      'user_id' => 'Unique user ID',
      'subdir' => 'Appears to be username',
      // Generated
      'filename' => 'Generated filename',
    );

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('user', 'u')
             ->fields('u', array('user_id', 'subdir'))
             ->condition('account_status', 'deleted', '<>');

    $this->source = new MigrateSourceSQL($query, $this->sourceFields, NULL,
          array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationFile();
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'user_id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Unique user ID',
                'alias' => 'u',
               ),
        ),
        MigrateDestinationFile::getKeySchema()
    );

    $this->addFieldMapping('value', 'subdir');
    $this->addFieldMapping('destination_dir')
         ->defaultValue('public://pictures');
    $this->addFieldMapping('destination_file', 'filename');
    $this->addFieldMapping('file_replace')
         ->defaultValue(FILE_EXISTS_RENAME);

    $this->addUnmigratedDestinations(array(
      'preserve_files',
      'source_dir',
      'timestamp',
      'uid',
    ));
  }

  /**
   * Implementation of Migration::prepareRow().
   *
   * @param $row
   *
   * @return bool
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $row->filename = $row->subdir . '.jpg';
    $row->subdir = 'http://fast1.onesite.com/diggyworld.com/user/' .
                          $row->subdir . '/profile.jpg?type=user&ts=1207-0854';
  }
}
