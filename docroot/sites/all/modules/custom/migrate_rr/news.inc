<?php
/**
 * @file
 * News post/comment migration classes for Roadrunner.
 */

require_once drupal_get_path('module', 'migrate_rr') . '/bbcode-filter.inc';

/**
 * News post class for Roadrunner.
 */
class RRNewsMigration extends RRMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t("Migrate news posts from the source database");
    $group_name = $this->group->getName();
    $this->dependencies[] = $group_name . 'User';
    $this->dependencies[] = $group_name . 'ForumTopic';

    $fields = array(
      // From board_posts
      'id' => 'Unique board post ID',
      'user_id' => 'Post author',
      'title' => 'Title of the post',
      'permalink' => 'Post URL suffix',
      'description' => 'Equivalent to teaser?',
      'body' => 'Body of the post',
      'post_image_file_name' => 'Filename of image for post',
      'post_image_content_type' => 'MIME type of the image',
      'post_image_file_size' => 'Size of the image',
      'url' => '? Usually NULL',
      'attachment' => 'NULL, 0, URL, iframe, ...?',
      'views' => 'Number of views of the post',
      'comments_count' => 'Number of comments on the post',
      'ratings_count' => 'Usually NULL',
      'created_at' => 'When post was created',
      'updated_at' => 'When post was last updated',
      'published_at' => 'When post was published',
      'flags_count' => 'Always (?) 0',
      'rating_average' => 'Usually NULL',
      'topic_id' => 'Parent topic (forum)',
      'sticky' => 'Whether post is sticky',
      'requires_admin' => '1/0',
      'requires_publisher' => '1/0',
      'fanclub_id' => 'Fanclub this belongs to',
      'public_at' => 'Timestamp',
      'member_at' => 'Always NULL',
      'premium_at' => 'Timestamp',
      'legacy_id' => 'NULL except a single 1 value',
      'pinned' => 'How does this differ from sticky?',
      'is_visible' => 'Whether the post is visible',
      'display_type' => 'bio, blog, forum, ...',
      'last_comment_at' => 'Timestamp of last comment',
      'released_on' => 'Datestamp',
      'position' => 'Ordering of posts (?)',
      'last_comment_id' => 'Last comment added',
      'ip_id' => 'Always (?) NULL',
      'display_poll' => '1/0',
      'premium_priority' => 'Numeric value',
      'visible_for' => 'premium, public, ...',
      'spam_status' => 'ham, haml, ...',
      'email' => 'Email address of poster?',
      'author' => 'Always (?) NULL',
      'download_url' => 'Always (?) NULL or empty',
      'legacy_source' => 'Usually NULL',
      'video_asset_file_name' => 'Usually NULL',
      'video_asset_content_type' => 'Usually NULL',
      'video_asset_file_size' => 'Usually NULL',
      'video_asset_updated_at' => 'Usually NULL',
      'video_job_id' => 'Usually NULL',
      'video_encoded_state' => 'Usually NULL',
      'video_output_url' => 'Usually NULL',
      'video_thumbnail_output_url' => 'Usually NULL',
      'video_aspect_ratio' => 'Usually NULL',
      'audio_asset_file_name' => 'Usually NULL',
      'audio_asset_content_type' => 'Usually NULL',
      'audio_asset_file_size' => 'Usually NULL',
      'audio_asset_updated_at' => 'Usually NULL',
      'video_width' => 'Usually NULL',
      'video_height' => 'Usually NULL',
      'facebook_post_id' => 'Always (?) NULL',
      'mobile_roadie_id' => 'Always NULL',
      'snook_score' => '?',
      'snook_debug' => '?',
      'remote_source_url' => 'Always (?) NULL or empty',
      // Dynamically generated
      'tags' => 'Tag terms'
    );

    $query = $this->connection
      ->select('board_posts', 'bp')
      ->fields('bp')
      ->condition('fanclub_id', $arguments['artist_id'])
      ->condition('topic_id', array(184, 550, 376), 'IN')
      ->orderBy('updated_at');

    $this->source = new MigrateSourceSQL($query, $fields, NULL,
      array('map_joinable' => FALSE));

    $this->destination = new MigrateDestinationNode('blog');

    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'id' => array('type' => 'int',
                        'unsigned' => TRUE,
                        'not null' => TRUE,
                        'description' => 'News post ID',
                       )
        ),
        MigrateDestinationNode::getKeySchema()
      );

    $this->highwaterField = array(
      'name' => 'updated_at',
      'alias' => 'bp',
    );

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('body', 'body')
         ->callbacks('_bbcode_filter_process');
    $this->addFieldMapping('body:summary', 'description');
    $this->addFieldMapping('body:format')
         ->defaultValue('6');
    $this->addFieldMapping('uid', 'user_id')
         ->sourceMigration($group_name . 'User');
    $this->addFieldMapping('created', 'created_at');
    $this->addFieldMapping('changed', 'updated_at');
    $this->addFieldMapping('field_blog_category', 'topic_id')
         ->sourceMigration($group_name . 'ForumTopic');
    $this->addFieldMapping('field_blog_category:source_type')
         ->defaultValue('tid');
    $this->addFieldMapping('sticky', 'sticky');
    $this->addFieldMapping('status', 'is_visible');
    $this->addFieldMapping('field_is_pro')
         ->defaultValue(0);
    $this->addFieldMapping('totalcount', 'views');
    $this->addFieldMapping('field_tags', 'tags')
         ->sourceMigration($group_name . 'Tags');
    $this->addFieldMapping('field_tags:source_type')
         ->defaultValue('tid');

    $this->addUnmigratedDestinations(array(
      'body:language',
      'comment',
      'daycount',
        'field_blog_category:create_term', 'field_blog_category:ignore_case',
        'field_tags:create_term', 'field_tags:ignore_case',
      'is_new',
      'language',
      'log',
      'path',
      'pathauto',
      'promote',
      'revision',
      'revision_uid',
      'timestamp',
      'tnid',
      'translate',
    ));

    $this->addUnmigratedSources(array(
      'attachment',
      'audio_asset_file_name', 'audio_asset_content_type', 'audio_asset_file_size',
        'audio_asset_updated_at',
      'author',
      'comments_count',
      'display_poll',
      'display_type',
      'download_url',
      'email',
      'facebook_post_id',
      'fanclub_id',
      'flags_count',
      'ip_id',
      'last_comment_at',
      'last_comment_id',
      'legacy_id',
      'legacy_source',
      'member_at',
      'mobile_roadie_id',
      'permalink',
      'pinned',
      'position',
      'post_image_file_name', 'post_image_content_type', 'post_image_file_size',
      'premium_at',
      'premium_priority',
      'public_at',
      'published_at',
      'rating_average',
      'ratings_count',
      'released_on',
      'remote_source_url',
      'requires_admin', 'requires_publisher',
      'snook_score', 'snook_debug',
      'spam_status',
      'url',
      'video_asset_file_name', 'video_asset_content_type', 'video_asset_file_size',
        'video_asset_updated_at', 'video_job_id', 'video_encoded_state',
        'video_output_url', 'video_aspect_ratio', 'video_thumbnail_output_url',
        'video_duration_in_ms', 'video_width', 'video_height',
      'views',
      'visible_for',
    ));
  }

  /**
   * Implements Migration::prepareRow().
   *
   * @param $row
   *
   * @return bool
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Gather tags referenced by this post
    $query = $this->connection
      ->select('taggings', 't')
      ->fields('t', array('tag_id'))
      ->isNotNull('tag_id')
      ->condition('taggable_id', $row->id);
    $row->tags = $query->execute()->fetchCol();

    // Append attachment contents
    if (isset($row->attachment)) {
      $attachment = trim($row->attachment);
      if ($attachment) {
        $row->body .= "<br />\n" . $attachment;
      }
    }
  }
}

/**
 * Forum comment class for Roadrunner.
 */
class RRNewsCommentMigration extends RRMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t("Migrate news comments from the source database");
    $group_name = $this->group->getName();
    $this->dependencies[] = $group_name . 'News';

    $fields = array(
      // From comments
      'id' => 'Unique comment ID',
      'user_id' => 'Comment author',
      'board_post_id' => 'Parent post',
      'body' => 'Body of the comment',
      'created_at' => 'When comment was created',
      'updated_at' => 'When comment was last updated',
      'flags_count' => 'Always (?) 0',
      'legacy_id' => '?',
      'spam_status' => 'ham, haml, ...',
      'ip_id' => 'Always (?) NULL',
      'email' => 'Email address of poster?',
      'author' => 'Always (?) NULL',
      'url' => '? Usually NULL',
      'legacy_source' => 'Usually NULL',
      'commentable_type' => 'Always (?) BoardPost',
      'commentable_id' => 'Board post ID',
      'mobile_roadie_id' => 'Always NULL',
      'snook_score' => '?',
      'snook_debug' => '?',
    );

    $query = $this->connection
      ->select('comments', 'c')
      ->fields('c');
    $query->innerJoin('board_posts', 'bp', 'c.board_post_id=bp.id');
    $query->condition('bp.fanclub_id', $arguments['artist_id']);
    $query->condition('topic_id', array(184, 550), 'IN');

    $this->source = new MigrateSourceSQL($query, $fields, NULL,
      array('map_joinable' => FALSE));

    $this->destination = new MigrateDestinationComment('comment_node_blog');

    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'id' => array('type' => 'int',
                        'unsigned' => TRUE,
                        'not null' => TRUE,
                        'description' => 'News comment ID',
                       )
        ),
        MigrateDestinationComment::getKeySchema()
      );

    $this->addFieldMapping('nid', 'board_post_id')
         ->sourceMigration($group_name . 'News');
    $this->addFieldMapping('uid', 'user_id')
         ->sourceMigration($group_name . 'User');
    $this->addFieldMapping('comment_body', 'body');
    $this->addFieldMapping('comment_body:format')
         ->defaultValue('6');
    $this->addFieldMapping('created', 'created_at');
    $this->addFieldMapping('changed', 'updated_at');
    $this->addFieldMapping('status')
         ->defaultValue(COMMENT_PUBLISHED);

    $this->addUnmigratedDestinations(array(
        'comment_body:language',
      'homepage',
      'hostname',
      'language',
      'mail',
      'name',
      'path',
      'pid',
      'subject',
      'thread',
    ));

    $this->addUnmigratedSources(array(
      'author',
      'commentable_id', 'commentable_type',
      'email',
      'flags_count',
      'ip_id',
      'legacy_id',
      'legacy_source',
      'mobile_roadie_id',
      'snook_score', 'snook_debug',
      'spam_status',
      'url',
    ));
  }
}
