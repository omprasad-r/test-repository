<?php

class CeeloNewsMigration extends CeeloMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->destination = new MigrateDestinationNode('blog');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'NewsID' =>
        array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'News ID from TSV',
        ),
    ), MigrateDestinationNode::getKeySchema());

    $this->source = new MigrateSourceCSV(dirname(__FILE__) . '/data/news.txt', array(
      'NewsID',
      'NewsTypeID',
      'Title',
      'SubTitle',
      'Summary',
      'Article',
      'DateArticle',
      'DateRelease',
      'DateArchive',
      'Active',
      'Tags',
      'URL',
    ), array(
      'delimiter'   => "\t",
      'header_rows' => 1,
    ));

    $this->addFieldMapping('title', 'Title');
    $this->addFieldMapping('body', 'Article');
    $this->addFieldMapping('body:summary', 'Summary');
    $this->addFieldMapping('created', 'DateRelease');
    $this->addFieldMapping('status', 'Active');
    $this->addFieldMapping('tags', 'Tags')->separator(',');
    $this->addFieldMapping('tags:create_term')->defaultValue(TRUE);
  }
}