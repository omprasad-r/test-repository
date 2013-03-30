<?php
/**
 * @file
 * News Story Type migration classes (from umg_cms) for UMG.
 */

/**
 * Class for news story type migrations from umg_cms.
 */
class UMGNewsStoryTypeMigration extends UMGMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t('Migrate news story types from the source database to taxonomy terms');

    $fields = array(
      // From newsstoriestypes
      'NewsStoryTypeID' => 'Unique story type ID',
      'NewsStoryType' => 'News story type title',
    );

    $query = $this->connection
             ->select('newsstorytypes', 'n')
             ->fields('n');

    $this->source = new MigrateSourceSQL($query, $fields, NULL,
      array('map_joinable' => FALSE));

    $this->destination = new MigrateDestinationTerm('tags');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'NewsStoryTypeID' => array(
          'type' => 'int',
          'unsigned' => 255,
          'not null' => TRUE,
          'description' => 'News story type ID',
        )
      ),
      MigrateDestinationTerm::getKeySchema()
    );

    $this->addFieldMapping('name', 'NewsStoryType')
      ->description(t('Title of the news story type'));
    $this->addFieldMapping(NULL, 'NewsStoryTypeID')
      ->description(t('This info will be generated in Drupal'))
      ->issueGroup(t('DNM'));
    $this->addFieldMapping('format')
      ->description(t('No source data'))
      ->issueGroup(t('DNM'));
    $this->addFieldMapping('weight')
      ->description(t('No source data'))
      ->issueGroup(t('DNM'));
    $this->addFieldMapping('parent')
      ->description(t('No source data'))
      ->issueGroup(t('DNM'));
    $this->addFieldMapping('description')
      ->description(t('No source data'))
      ->issueGroup(t('DNM'));
    $this->addFieldMapping('parent_name')
      ->description(t('No source data'))
      ->issueGroup(t('DNM'));
    $this->addFieldMapping('path')
      ->description(t('No source data'))
      ->issueGroup(t('DNM'));
    $this->addFieldMapping('pathauto')
      ->description(t('No source data'))
      ->issueGroup(t('DNM'));

  }
}
