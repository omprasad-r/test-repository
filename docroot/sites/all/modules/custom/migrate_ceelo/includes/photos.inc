<?php

class CeeloPhotoMigration extends CeeloMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->destination = new MigrateDestinationNode('photo');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'PhotoID' =>
        array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'Photo ID from TSV',
        ),
    ), MigrateDestinationNode::getKeySchema());

    $this->source = new MigrateSourceCSV('data/photos.txt', array(
      'PhotoID',
      'Gallery',
      'Category',
      'Photo',
      'URL',
    ), array(
      'delimiter'   => "\t",
      'header_rows' => 1,
    ));

    $this->addFieldMapping('field_photo_category', 'Category');
    $this->addFieldMapping('field_photo_category:create_term')->defaultValue(TRUE);

    $this->addFieldMapping('title', 'Photo');

    // TODO: Figure out the url grabbiness.
  }
}