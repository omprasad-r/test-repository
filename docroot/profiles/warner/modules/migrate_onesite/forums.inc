<?php
/**
 * @file
 * Migration class for importing forums from ONEsite to Drupal. May be
 * extended, particularly to add project-specific field mappings.
 */

class OnesiteForumsMigration extends OnesiteMigration {
  /**
   * @param array $arguments
   *  forum_vocabulary (optional) - Name of the forum vocabulary to populate
   *    from ONEsite forum titles. Defaults to 'forums'.
   */
  public function __construct($arguments) {
    if (!isset($arguments['forum_vocabulary'])) {
      $arguments['forum_vocabulary'] = 'forums';
    }
    parent::__construct($arguments);
    $this->description = 'Migration of forums from OneSite';

    $this->sourceFields = array(
      'forum_id' => 'Unique forum ID',
      'forum_category_id' => '3 133903, 1 135977',
      'parent_id' => 'Unused, ignore',
      'children' => 'Unused, ignore',
      'blog_id' => 'Unused, ignore',
      'node_id' => 'Unused, ignore',
      'forum_title' => 'Title of forum',
      'forum_description' => 'Description of forum',
      'last_post_id' => 'ID of last post',
      'num_threads' => 'Number of threads in forum',
      'num_posts' => 'Number of posts in forum',
      'expires' => 'No useful data',
      'created_by' => 'Unused, ignore',
      'unknown' => 'Undocumented field',
      'date_created' => 'Date forum created',
      'num_views' => 'Unused, ignore',
      'scaled_rating' => 'Unused, ignore',
      'status' => 'Always active',
      'owner' => 'Unused, ignore',
      'moderator' => 'Unused, ignore',
      'friend' => 'Unused, ignore',
      'group_member' => 'Unused, ignore',
      'member' => 'Unused, ignore',
      'non_member' => 'Unused, ignore',
    );

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('forums', 'f')
             ->fields('f');
    $this->source = new MigrateSourceSQL($query, $this->sourceFields, NULL,
          array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationTerm($arguments['forum_vocabulary']);

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'forum_id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Unique forum ID',
               ),
        ),
        MigrateDestinationTerm::getKeySchema()
    );

    $this->addFieldMapping('name', 'forum_title');
    $this->addFieldMapping('description', 'forum_description');

    $this->addUnmigratedDestinations(array(
      'format',
      'migrate_redirects',
      'parent',
      'parent_name',
      'path',
      'pathauto',
      'weight'
    ));

    $this->addUnmigratedSources(array(
      'blog_id',
      'children',
      'created_by',
      'date_created',
      'expires',
      'friend',
      'forum_category_id',
      'group_member',
      'last_post_id',
      'member',
      'moderator',
      'node_id',
      'non_member',
      'num_posts',
      'num_threads',
      'num_views',
      'owner',
      'parent_id',
      'scaled_rating',
      'status',
      'unknown',
    ));
  }
}
