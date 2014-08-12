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

    $this->source = new MigrateSourceCSV(dirname(__FILE__) . '/../data/news.txt', array(
      'NewsID' => 'PK',
      'NewsTypeID' => '?',
      'Title' => 'Title',
      'SubTitle' => 'Subtitle - Mostly empty',
      'Summary' => 'Summary field',
      'Article' => 'Full article text',
      'DateArticle' => 'createdish',
      'DateRelease' => 'What we are using for created',
      'DateArchive' => 'Date archived?',
      'Active' => 'Status',
      'Tags' => 'Tags',
      'URL' => 'url',
    ), array(
      'delimiter'   => "\t",
      'header_rows' => 1,
    ));

    $this->addFieldMapping('title', 'Title');
    $this->addFieldMapping('body', 'Article');
    $this->addFieldMapping('body:summary', 'Summary');
    $this->addFieldMapping('body:format')->defaultValue('full_html');
    $this->addFieldMapping('created', 'DateRelease');
    $this->addFieldMapping('status', 'Active');
    $this->addFieldMapping('tags', 'Tags')->separator(',');
    $this->addFieldMapping('tags:create_term')->defaultValue(TRUE);
  }
}