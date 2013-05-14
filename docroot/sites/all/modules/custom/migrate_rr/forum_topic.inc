<?php
/**
 * @file
 * Forum node/comment migration classes for Roadrunner.
 */

/**
 * Forum category class for Roadrunner.
 */
class RRForumCategoryMigration extends RRMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t("Migrate forum categories from the source database");

    $fields = array(
      // From categories
      'id' => 'Unique category ID',
      'name' => 'Category name',
      'description' => 'Category description',
      'permalink' => 'Category URL suffix',
      'created_at' => 'When category was created',
      'updated_at' => 'When category was last updated',
      'requires_admin' => '1/0',
      'requires_publisher' => '1/0',
      'fanclub_id' => 'Fanclub this belongs to',
      'public_at' => 'Timestamp',
      'member_at' => 'Always NULL',
      'premium_at' => 'Timestamp',
      'legacy_id' => 'NULL except a single 1 value',
      'position' => 'Ordering of categories',
      'is_visible' => 'Whether the category is visible',
      'last_topic_id' => 'Last topic added',
      'last_board_post_id' => 'Last board post added',
      'last_comment_id' => 'Last comment added',
      'legacy_source' => 'Usually NULL',
      'mobile_roadie_id' => 'Always NULL',
    );

    $query = $this->connection
      ->select('categories', 'c')
      ->fields('c')
      ->condition('fanclub_id', $arguments['artist_id'])
      ->condition('is_visible', 1)
      ->orderBy('position');

    $this->source = new MigrateSourceSQL($query, $fields, NULL,
      array('map_joinable' => FALSE));

    $this->destination = new MigrateDestinationTerm('forums');

    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'id' => array('type' => 'int',
                        'unsigned' => TRUE,
                        'not null' => TRUE,
                        'description' => 'Category ID',
                       )
        ),
        MigrateDestinationTerm::getKeySchema()
      );

    $this->addFieldMapping('name', 'name');
    $this->addFieldMapping('description', 'description');
    $this->addFieldMapping('path', 'permalink')
         ->description('Migrate as-is, or as redirect, ...?');

    $this->addUnmigratedDestinations(array(
      'format',
      'parent',
      'parent_name',
      'pathauto',
      'weight',
    ));

    $this->addUnmigratedSources(array(
      'created_at',
      'fanclub_id',
      'is_visible',
      'last_board_post_id',
      'last_comment_id',
      'last_topic_id',
      'legacy_id',
      'legacy_source',
      'member_at',
      'mobile_roadie_id',
      'position',
      'premium_at',
      'public_at',
      'requires_admin',
      'requires_publisher',
      'updated_at',
    ));
  }

  /**
   * Implements Migration::complete().
   *
   * @param $term
   * @param $row
   */
  public function complete($term, $row) {
    // Treat the categories as containers - i.e., users can't post to them.
    $containers = variable_get('forum_containers', array());
    if (!in_array($term->tid, $containers)) {
      $containers[] = $term->tid;
      variable_set('forum_containers', $containers);
    }
  }
}


/**
 * Forum topic class for Roadrunner.
 */
class RRForumTopicMigration extends RRMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t("Migrate forum topics from the source database");
    $this->dependencies[] = $this->group->getName() . 'ForumCategory';

    $fields = array(
      // From topics
      'id' => 'Unique topic ID',
      'user_id' => 'User who created the topic?',
      'title' => 'Topic name',
      'board_posts_count' => 'Number of posts in the topic',
      'comments_count' => 'Number of comments in the topic',
      'description' => 'Category description',
      'description_html' => 'Always NULL',
      'status' => 'Always NULL',
      'permalink' => 'Topic URL suffix',
      'position' => 'Ordering of topics',
      'created_at' => 'When topic was created',
      'updated_at' => 'When topic was last updated',
      'category_id' => 'Parent category (forum container)',
      'requires_admin' => '1/0',
      'requires_publisher' => '1/0',
      'fanclub_id' => 'Fanclub this belongs to',
      'public_at' => 'Timestamp',
      'member_at' => 'Usually NULL',
      'premium_at' => 'Timestamp',
      'legacy_id' => 'Mostly NULL',
      'is_visible' => 'Whether the topic is visible',
      'display_type' => 'bio, blog, forum, gallery, ...',
      'last_post_at' => 'Mostly NULL',
      'last_board_post_id' => 'Last board post added',
      'last_comment_id' => 'Last comment added',
      'auto_post' => 'Always 0',
      'auto_post_feature_type' => 'Usually NULL or empty',
      'legacy_source' => 'Usually NULL',
      'mobile_roadie_id' => 'Always NULL',
    );

    $query = $this->connection
      ->select('topics', 't')
      ->fields('t')
      ->condition('fanclub_id', $arguments['artist_id'])
      ->condition('is_visible', 1)
      ->orderBy('position');

    $this->source = new MigrateSourceSQL($query, $fields, NULL,
      array('map_joinable' => FALSE));

    $this->destination = new MigrateDestinationTerm('forums');

    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'id' => array('type' => 'int',
                        'unsigned' => TRUE,
                        'not null' => TRUE,
                        'description' => 'Topic ID',
                       )
        ),
        MigrateDestinationTerm::getKeySchema()
      );

    $this->addFieldMapping('name', 'title');
    $this->addFieldMapping('description', 'description');
    $this->addFieldMapping('parent', 'category_id')
         ->sourceMigration($this->group->getName() . 'ForumCategory');

    $this->addUnmigratedDestinations(array(
      'format',
      'parent_name',
      'path',
      'pathauto',
      'tac_redirect_403',
      'weight',
    ));

    $this->addUnmigratedSources(array(
      'auto_post',
      'auto_post_feature_type',
      'board_posts_count',
      'category_id',
      'comments_count',
      'created_at',
      'description_html',
      'display_type',
      'fanclub_id',
      'is_visible',
      'last_board_post_id',
      'last_comment_id',
      'last_post_at',
      'legacy_id',
      'legacy_source',
      'member_at',
      'mobile_roadie_id',
      'permalink',
      'position',
      'premium_at',
      'public_at',
      'requires_admin',
      'requires_publisher',
      'status',
      'updated_at',
      'user_id',
    ));
  }
}
