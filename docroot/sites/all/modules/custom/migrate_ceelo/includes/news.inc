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
      array('NewsID', 'PK'),
      array('NewsTypeID', '?'),
      array('Title', 'Title'),
      array('SubTitle', 'Subtitle - Mostly empty'),
      array('Summary', 'Summary field'),
      array('Article', 'Full article text'),
      array('DateArticle', 'createdish'),
      array('DateRelease', 'What we are using for created'),
      array('DateArchive', 'Date archived?'),
      array('Active', 'Status'),
      array('Tags', 'Tags'),
      array('URL', 'url'),
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
    $this->addFieldMapping('field_tags', 'Tags')->separator(',');
    $this->addFieldMapping('field_tags:create_term')->defaultValue(TRUE);

    $this->addFieldMapping('migrate_redirects', 'URL');
  }
}