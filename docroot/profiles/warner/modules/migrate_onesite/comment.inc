<?php
/**
 * @file
 * Migration class for importing general comments from ONEsite to Drupal. May be
 * extended, particularly to add project-specific field mappings.
 */

class OnesiteGeneralCommentMigration extends OnesiteCommentMigration {
  /**
   * @param array $arguments
   *  parent_type (required) - ONEsite type ('blog_post' etc.) to migrate
   *    comments for.
   */
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->sourceFields = array(
      'comment_id' => 'Unique comment ID',
      'blog_id' => 'Parent blog ID',
      'author_id' => 'ID of comment author',
      'xref_id' => 'ID of thing being commented on?',
      'type' => 'Type of thing being commented on - mostly blog_post',
      'post_id' => 'Always 0',
      'photo_id' => 'No useful data',
      'video_id' => 'No useful data',
      'comment' => 'Text of comment',
      'title' => 'Comment title - only five non-empty',
      'date_added' => 'Date of comment',
      'status' => 'Status of comment, should we unpublish pending?',
      'view_status' => 'Mostly "unread"',
    );
    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('comment', 'c')
             ->fields('c')
             ->condition('status', 'deleted', '<>')
             ->condition('type', $arguments['parent_type']);

    $this->source = new MigrateSourceSQL($query, $this->sourceFields, NULL,
          array('map_joinable' => FALSE));
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'comment_id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Unique comment ID',
               ),
        ),
        MigrateDestinationComment::getKeySchema()
    );

    $this->addFieldMapping('nid', 'xref_id')
         ->sourceMigration($arguments['parent_migration']);
    $this->addFieldMapping('uid', 'author_id')
         ->sourceMigration($arguments['user_migration'])
         ->defaultValue(1);
    $this->addFieldMapping('subject', 'title');
    $this->addFieldMapping('created', 'date_added');
    $this->addFieldMapping('changed', 'date_added');
    $this->addFieldMapping('status', 'status')
         ->callbacks(array($this, 'handleStatus'));
    $this->addFieldMapping('comment_body', 'comment');

    $format_mapping = $this->addFieldMapping('comment_body:format');
    if (isset($arguments['default_format'])) {
      $format_mapping->defaultValue($arguments['default_format']);
    }

    $this->addUnmigratedDestinations(array(
      'comment_body:language',
    ));

    $this->addUnmigratedSources(array(
      'blog_id',
      'photo_id',
      'post_id',
      'type',
      'video_id',
      'view_status'
    ));
  }
}
