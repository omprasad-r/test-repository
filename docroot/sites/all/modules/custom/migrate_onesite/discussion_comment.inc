<?php
/**
 * @file
 * Migration class for importing discussion comments from ONEsite to Drupal.
 * May be extended, particularly to add project-specific field mappings.
 */

class OnesiteDiscussionCommentMigration extends OnesiteCommentMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = 'Migration of discussion comments';

    $this->sourceFields = array(
      'node_id' => 'Unused, ignore',
      'post_id' => 'Unique discussion comment ID',
      'discussion_id' => 'Related discussion ID',
      'title' => 'Title of the comment',
      'content' => 'Body of the comment',
      'user_ip' => 'IP address comment was posted from',
      'created_by' => 'Author of the comment',
      'date_created' => 'Date of the comment',
      'public' => 'No useful data',
      'status' => 'What do values of 0 and 7 mean?',
      'rating' => 'values 0, 7, 8, 9',
    );

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('discussion_comment', 'dc')
             ->fields('dc')
             ->condition('dc.status', 'deleted', '<>');
    // Skip the first post in a thread, it's the thread body
    $query->innerJoin('discussion', 'd',
                      'd.discussion_id=dc.discussion_id AND dc.post_id<>d.first_post_id');
    $this->source = new MigrateSourceSQL($query, $this->sourceFields, NULL,
          array('map_joinable' => FALSE));
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'post_id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Unique discussion comment ID',
               ),
        ),
        MigrateDestinationComment::getKeySchema()
    );

    $this->addFieldMapping('comment_body', 'content')
         ->callbacks(array($this, 'handleQuotes'));

    $format_mapping = $this->addFieldMapping('comment_body:format');
    if (isset($arguments['default_format'])) {
      $format_mapping->defaultValue($arguments['default_format']);
    }

    $this->addFieldMapping('created', 'date_created');
    $this->addFieldMapping('changed', 'date_created');
    $this->addFieldMapping('nid', 'discussion_id')
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
      'node_id',
      'public',
      'rating',
      'title',
      'user_ip',
    ));
  }
}
