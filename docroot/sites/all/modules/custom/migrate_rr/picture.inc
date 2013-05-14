<?php
/**
 * @file
 * Picture migration class for Roadrunner.
 */

/**
 * Picture migration class for Roadrunner.
 */
class RRPictureMigration extends RRMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t("Migrate user picturess from the source database");

    $fields = array(
      // From users
      'id' => 'Unique user ID',
      'avatar_file_name' => 'Filename of avatar',
    );

    $query = $this->connection
      ->select('users', 'u')
      ->fields('u', array('id', 'avatar_file_name', 'updated_at'))
      ->isNotNull('u.avatar_file_name')
      ->orderBy('updated_at');
    $query->innerJoin('memberships', 'm', 'u.id=m.user_id');
    $query->condition('m.fanclub_id', $arguments['artist_id']);

    $this->source = new MigrateSourceSQL($query, $fields, NULL,
      array('map_joinable' => FALSE));

    $this->destination = new MigrateDestinationFile('image');

    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'id' => array('type' => 'int',
                             'unsigned' => TRUE,
                             'not null' => TRUE,
                             'description' => 'User ID',
                            )
        ),
        MigrateDestinationFile::getKeySchema()
      );

    $this->highwaterField = array(
      'name' => 'updated_at',
      'alias' => 'u',
    );

    $this->addFieldMapping('value', 'avatar_file_name');
    $this->addFieldMapping('destination_file', 'avatar_file_name');
    $this->addFieldMapping('source_dir')
         ->defaultValue('http://assets.network.roadrunnerrecords.com/system/users/avatars/');
    $this->addFieldMapping('destination_dir')
         ->defaultValue('public://pictures');
    $this->addFieldMapping('uid')
         ->description('TODO: rewrite in user complete function')
         ->issueGroup('Acquia')
         ->issuePriority(MigrateFieldMapping::ISSUE_PRIORITY_MEDIUM);
    $this->addFieldMapping('timestamp', 'updated_at');
    $this->addFieldMapping('file_replace')
         ->defaultValue(MigrateFile::FILE_EXISTS_REUSE);
    $this->addFieldMapping('preserve_files')
         ->defaultValue(1);

    $this->addUnmigratedDestinations(array(
      'field_license',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
      'media_description', 'media_description:language', 'media_description:format',
      'media_title', 'media_title:language',
      'path',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $row->avatar_file_name = $row->id . '/thumb_' . $row->avatar_file_name;
  }
}
