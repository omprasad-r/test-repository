<?php
/**
 * @file
 * Migration class for importing forum posts from ONEsite to Drupal as comments.
 * May be extended, particularly to add project-specific field mappings.
 */

class OnesiteForumPostMigration extends OnesiteCommentMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = 'Migration of forum posts from OneSite as comments';

    $this->sourceFields = array(
      'post_id' => 'Unique post ID',
      'thread_id' => 'Containing thread',
      'forum_id' => 'ID for containing forum - 3 values present',
      'node_id' => 'Unused, ignore',
      'post_title' => 'Title of post - actually, looks more like a teaser',
      'post_content' => 'Body of the post',
      'date_created' => 'Date thread posted',
      'created_by' => 'Author of the post',
      'status' => 'Skip deleted - how to handle empty and 0 values?',
      'num_views' => 'Number of page views - unused',
      'scaled_rating' => '0, 2, 3',
    );

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('forum_post', 'fp')
             ->fields('fp')
             ->condition('fp.status', 'deleted', '<>')
             ->orderBy('fp.date_created');
    // Skip the first post in a thread, it's the thread body
    $query->innerJoin('forum_thread', 'ft',
                      'fp.thread_id=ft.thread_id AND fp.post_id<>ft.first_post_id');
    $this->source = new MigrateSourceSQL($query, $this->sourceFields, NULL,
          array('map_joinable' => FALSE));
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'post_id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Unique comment ID',
               ),
        ),
        MigrateDestinationComment::getKeySchema()
    );

    $this->highwaterField = array(
      'name' => 'date_created',
      'alias' => 'fp',
      'type' => 'string',
    );

    $this->addFieldMapping('comment_body', 'post_content')
         ->callbacks(array($this, 'handleQuotes'));

    $format_mapping = $this->addFieldMapping('comment_body:format');
    if (isset($arguments['default_format'])) {
      $format_mapping->defaultValue($arguments['default_format']);
    }

    $this->addFieldMapping('created', 'date_created');
    $this->addFieldMapping('changed', 'date_created');
    $this->addFieldMapping('nid', 'thread_id')
         ->sourceMigration($arguments['parent_migration']);
    $this->addFieldMapping('uid', 'created_by')
         ->sourceMigration($arguments['user_migration'])
         ->defaultValue(1);
    $this->addFieldMapping('status', 'status')
         ->callbacks(array($this, 'handleStatus'));

    $this->addUnmigratedDestinations(array(
        'comment_body:language',
      'subject',
    ));

    $this->addUnmigratedSources(array(
      'forum_id',
      'node_id',
      'num_views',
      'post_title',
      'scaled_rating',
      'thread_id',
    ));
  }
}
