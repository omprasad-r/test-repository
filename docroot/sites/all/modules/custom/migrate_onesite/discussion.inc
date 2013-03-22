<?php

/**
 * @file
 * Migration class for importing discussions from ONEsite to Drupal. May be
 * extended, particularly to add project-specific field mappings.
 */

class OnesiteDiscussionMigration extends OnesiteNodeMigration {
  /**
   * @param array $arguments
   *  forum_migration (required) - Machine name of the migration from discussions
   *    to forum topics.
   */
  public function __construct($arguments) {
    // Default the bundle to the core forum type.
    if (!isset($arguments['bundle'])) {
      $arguments['bundle'] = 'forum';
    }
    parent::__construct($arguments);
    $this->description = 'Migration of discussions from OneSite';

    $this->sourceFields = array(
      'node_id' => 'Unused, ignore',
      'discussion_id' => 'Unique discussion ID',
      'num_replies' => 'Number of replies',
      'first_post_id' => 'ID of first post',
      'last_post_id' => 'ID of last post',
      'created_by' => 'ID of author',
      'date_created' => 'Date discussion created (string)',
      'public' => 'Unused, ignore',
      'status' => 'Always active',
      'num_views' => 'Unused, ignore',
    );

    // The post that begins a discussion contains what in Drupal we would consider
    // the body of a forum node, so we pull that from discussion_comment.
    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('discussion', 'd')
             ->fields('d')
             ->condition('d.status', 'deleted', '<>');
    $query->innerJoin('discussion_comment', 'dc', 'd.first_post_id=dc.post_id');
    $query->fields('dc', array('title', 'content'));
    $this->source = new MigrateSourceSQL($query, $this->sourceFields, NULL,
          array('map_joinable' => FALSE));

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'discussion_id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Unique discussion ID',
                'alias' => 'd',
               ),
        ),
        MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('body', 'content')
         ->callbacks(array($this, 'handleQuotes'));

    $format_mapping = $this->addFieldMapping('body:format');
    if (isset($arguments['default_format'])) {
      $format_mapping->defaultValue($arguments['default_format']);
    }

    $this->addFieldMapping('created', 'date_created');
    $this->addFieldMapping('changed', 'date_created');
    $this->addFieldMapping('uid', 'created_by')
         ->sourceMigration($arguments['user_migration'])
         ->defaultValue(1);
    $this->addFieldMapping('taxonomy_forums')
         ->defaultValue('Discussions');
    $this->addFieldMapping('taxonomy_forums:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('status', 'status')
         ->callbacks(array($this, 'handleStatus'));

    $this->addUnmigratedDestinations(array(
        'taxonomy_forums:source_type', 'taxonomy_forums:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'first_post_id',
      'last_post_id',
      'node_id',
      'num_replies',
      'num_views',
      'public',
    ));
  }
}
