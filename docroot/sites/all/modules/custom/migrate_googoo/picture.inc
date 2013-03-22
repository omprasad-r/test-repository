<?php
/**
 * @file
 * Picture migration class for googoodolls import.
 */

class GooGooPictureMigration extends GooGooMigration {
  public function __construct() {
    parent::__construct();

    $this->description = 'Import of user avatars from googoodolls.com';

    $fields = array(
      // smf_members table
      'avatar' => 'Values like GGD/icon5.jpg',
    );
    $query = Database::getConnection('default', 'googoo_smf')
             ->select('smf_members', 'm')
             ->fields('m', array('avatar'))
             ->condition('avatar', ' ', '<>')
             ->distinct();
    $this->source = new MigrateSourceSQL($query, $fields, NULL,
          array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationFile('image');
    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'avatar' =>
          array('type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'description' => 'Avatar filename',
               ),
      ),
      MigrateDestinationFile::getKeySchema()
    );

    $this->addFieldMapping('value', 'avatar');
    $this->addFieldMapping('source_dir')
         ->defaultValue('http://23.23.120.51/member/forum/avatars/');
    $this->addFieldMapping('destination_dir')
         ->defaultValue('public://legacy');
    $this->addFieldMapping('destination_file', 'avatar');
    $this->addFieldMapping('file_replace')
         ->defaultValue(MigrateFile::FILE_EXISTS_REUSE);
    $this->addFieldMapping('preserve_files')
         ->defaultValue(TRUE);
    $this->addFieldMapping('uid')
         ->defaultValue($this->defaultAuthor);

    $this->addUnmigratedDestinations(array(
      'field_license',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
      'media_description', 'media_description:format', 'media_description:language',
      'media_title', 'media_title:language',
      'migrate_redirects',
      'timestamp',
    ));
  }
}
