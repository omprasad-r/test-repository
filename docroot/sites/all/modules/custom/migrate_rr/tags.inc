<?php
/**
 * @file
 * Tag migration class for Roadrunner.
 */

/**
 * Tag migration class for Roadrunner.
 */
class RRTagsMigration extends RRMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t("Migrate tags from the source database");

    $fields = array(
      // From tags
      'id' => 'Unique tag ID',
      'name' => 'Tag name',
      'permalink' => 'URL suffix',
    );

    $query = $this->connection
      ->select('tags', 't')
      ->fields('t');
    $query->innerJoin('taggings', 'ts', 't.id=ts.tag_id');
    $query->innerJoin('board_posts', 'bp', 'ts.taggable_id=bp.id');
    $query->condition('bp.fanclub_id', $arguments['artist_id']);
    $query->distinct();

    $this->source = new MigrateSourceSQL($query, $fields, NULL,
      array('map_joinable' => FALSE));

    $this->destination = new MigrateDestinationTerm('tags');

    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'id' => array('type' => 'int',
                        'unsigned' => TRUE,
                        'not null' => TRUE,
                        'description' => 'Tag ID',
                       )
        ),
        MigrateDestinationTerm::getKeySchema()
      );

    $this->addFieldMapping('name', 'name');

    $this->addUnmigratedDestinations(array(
      'description',
      'format',
      'parent',
      'parent_name',
      'path',
      'pathauto',
      'weight',
    ));

    $this->addUnmigratedSources(array(
      'permalink',
    ));
  }
}
