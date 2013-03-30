<?php
/**
 * @file
 * Migration class for importing forum threads from ONEsite to Drupal. May be
 * extended, particularly to add project-specific field mappings.
 */

class OnesiteForumThreadMigration extends OnesiteNodeMigration {
  /**
   * @param array $arguments
   *  forum_migration (required) - Machine name of the migration from forums
   *    to forum terms.
   */
  public function __construct($arguments) {
    // Default the bundle to the core forum type.
    if (!isset($arguments['bundle'])) {
      $arguments['bundle'] = 'forum';
    }
    parent::__construct($arguments);
    $this->description = 'Migration of forum threads from OneSite';
    $this->dependencies[] = $arguments['forum_migration'];

    $this->sourceFields = array(
      'thread_id' => 'Unique thread ID',
      'forum_id' => 'ID for containing forum - 3 values present',
      'forum_category_id' => 'Unused, ignore',
      'blog_id' => 'Unused, ignore',
      'node_id' => 'Unused, ignore',
      'thread_title' => 'Title of thread',
      'first_post_id' => 'ID of first thread post',
      'last_post_id' => 'ID of last thread post',
      'sticky' => 'Unused, ignore',
      'anonymous' => 'Unused, ignore',
      'num_posts' => 'Number of posts in thread',
      'expires' => 'Unused, ignore',
      'last_hot_date' => 'Unused, ignore',
      'created_by' => 'User ID of thread author',
      'date_created' => 'Date thread posted',
      'date_created_rev' => 'Negative timestamps?',
      'num_views' => 'Number of page views',
      'scaled_rating' => 'Unused, ignore',
      'status' => 'Skip deleted',
      'is_public' => 'Unused, ignore',
      'owner' => 'Unused, ignore',
      'moderator' => 'Unused, ignore',
      'friend' => 'Unused, ignore',
      'group_member' => 'Unused, ignore',
      'member' => 'Unused, ignore',
      'non_member' => 'Unused, ignore',
      // From forum_post - the first post is actually the body for the thread
      'post_content' => 'Body of first post, to use as node body'
    );

    // The post that begins a thread contains what in Drupal we would consider
    // the body of a forum node, so we pull that from forum_post.
    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('forum_thread', 'ft')
             ->fields('ft')
             ->condition('ft.status', 'deleted', '<>')
             ->orderBy('ft.date_created');
    $query->innerJoin('forum_post', 'fp', 'ft.first_post_id=fp.post_id');
    $query->fields('fp', array('post_content'));
    $this->source = new MigrateSourceSQL($query, $this->sourceFields, NULL,
          array('map_joinable' => FALSE));

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'thread_id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Unique thread ID',
                'alias' => 'ft',
               ),
        ),
        MigrateDestinationNode::getKeySchema()
    );

    $this->highwaterField = array(
      'name' => 'date_created',
      'alias' => 'ft',
      'type' => 'string',
    );

    $this->addFieldMapping('title', 'thread_title');
    $this->addFieldMapping('body', 'post_content')
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
    $this->addFieldMapping('taxonomy_forums', 'forum_id')
         ->sourceMigration($arguments['forum_migration']);
    $this->addFieldMapping('taxonomy_forums:source_type')
         ->defaultValue('tid');
    $this->addFieldMapping('status', 'status')
         ->callbacks(array($this, 'handleStatus'));
    $this->addFieldMapping('sticky', 'sticky', FALSE);

    $this->addUnmigratedDestinations(array(
        'taxonomy_forums:create_term', 'taxonomy_forums:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'anonymous',
      'blog_id',
      'date_created_rev',
      'expires',
      'first_post_id',
      'forum_category_id',
      'friend',
      'group_member',
      'is_public',
      'last_hot_date',
      'last_post_id',
      'member',
      'moderator',
      'node_id',
      'non_member',
      'num_posts',
      'num_views',
      'owner',
      'scaled_rating',
      'thread_id',
    ));
  }
}
