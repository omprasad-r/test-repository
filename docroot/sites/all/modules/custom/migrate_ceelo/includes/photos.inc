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

    $this->source = new MigrateSourceCSV(dirname(__FILE__) . '/../data/photos.txt', array(
      array('PhotoID', 'Artificial Primary Key'),
      array('Gallery', 'Gallery Name'),
      array('Category', 'Photo Category'),
      array('Photo', 'Photo url'),
      array('URL', 'Canonical Url'),
    ), array(
      'delimiter'   => "\t",
      'header_rows' => 1,
    ));

    $this->addFieldMapping('field_photo_category', 'Category');
    $this->addFieldMapping('field_photo_category:create_term')->defaultValue(TRUE);

    $this->addFieldMapping('title', 'Photo');

    $this->addFieldMapping('field_photo_asset', 'URL');
    $this->addFieldMapping('field_photo_asset:file_class')->defaultValue('MigrateFileUri');
    $this->addFieldMapping('field_photo_asset:urlencode')->defaultValue(FALSE);
  }
}