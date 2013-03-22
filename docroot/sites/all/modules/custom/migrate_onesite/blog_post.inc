<?php
/**
 * @file
 * Migration class for importing blog posts from ONEsite to Drupal. May be extended,
 * particularly to add project-specific field mappings.
 */

class OnesiteBlogPostMigration extends OnesiteNodeMigration {
  public function __construct($arguments) {
    // Default the bundle to the core blog type.
    if (!isset($arguments['bundle'])) {
      $arguments['bundle'] = 'blog';
    }
    parent::__construct($arguments);
    $this->description = 'Migration of blog posts from OneSite';

    $this->sourceFields = array(
      'post_id' => 'Unique blog post ID',
      'blog_id' => 'ID of containing blog',
      'author_id' => 'ID of post author',
      'node_assoc' => 'Values look like UNIX timestamps',
      'post_date' => 'Date posted',
      'mod_date' => 'Date modified',
      'title' => 'Blog post title',
      'title_url' => 'Tail end of path alias',
      'body' => 'Body of blog post',
      'category' => 'Category of blog post (only 3 non-zero values)',
      'num_direct_views' => 'View count?',
      'wordcount' => 'Unused, ignore',
      'pages' => 'Unused, ignore',
      'status' => 'deleted/draft/public - for draft, set to unpublished?',
      'is_module' => 'Unused, ignore',
      'is_page' => 'Unused, ignore',
    );

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('blog_post', 'bp')
             ->fields('bp')
             ->condition('bp.status', 'deleted', '<>')
             ->orderBy('bp.post_date');
    $this->source = new MigrateSourceSQL($query, $this->sourceFields, NULL,
      array('map_joinable' => FALSE));

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'post_id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Unique blog post ID',
                'alias' => 'bp',
               ),
        ),
        MigrateDestinationNode::getKeySchema()
    );

    $this->highwaterField = array(
      'name' => 'post_date',
      'alias' => 'bp',
      'type' => 'string',
    );

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('body', 'body');

    $format_mapping = $this->addFieldMapping('body:format');
    if (isset($arguments['default_format'])) {
      $format_mapping->defaultValue($arguments['default_format']);
    }

    $this->addFieldMapping('uid', 'author_id')
         ->sourceMigration($arguments['user_migration'])
         ->defaultValue(1);
    $this->addFieldMapping('created', 'post_date');
    $this->addFieldMapping('changed', 'mod_date');
/* @todo
    $this->addFieldMapping('migrate_redirects', 'title_url', FALSE)
         ->callbacks(array($this, 'fixPath'));
*/
    $this->addFieldMapping('status', 'status')
         ->callbacks(array($this, 'handleStatus'));
    $this->addFieldMapping('totalcount', 'num_direct_views', FALSE);

    $this->addUnmigratedSources(array(
      'blog_id',
      'category',
      'is_module',
      'is_page',
      'node_assoc',
      'pages',
      'wordcount',
    ));
  }

  /**
   * Implementation of Migration::prepareRow().
   *
   * @param $row
   *
   * @return bool
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    // ONEsite modified timestamps may be 0 - if so, set to created timestamp.
    if (!$row->mod_date) {
      $row->mod_date = $row->post_date;
    }
  }
}

class OnesiteBlogPostCommentMigration extends OnesiteCommentMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = 'Migration of blog post comments';

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
    $query->innerJoin('forum_topic_lookup', 'ftl', 'dc.discussion_id=ftl.topic_id');
    $query->fields('ftl', array('xref_id'));
    $query->condition('ftl.type', 'blog_post');
    // Skip the first post in a thread, it's the thread body
/*    $query->innerJoin('discussion', 'd',
                      'd.discussion_id=dc.discussion_id AND dc.post_id<>d.first_post_id');
*/
    $this->source = new MigrateSourceSQL($query, $this->sourceFields, NULL,
          array('map_joinable' => FALSE));
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'post_id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Unique blog post comment ID',
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
    $this->addFieldMapping('nid', 'xref_id')
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
      'discussion_id',
      'node_id',
      'public',
      'rating',
      'title',
      'user_ip',
    ));

  }
}
