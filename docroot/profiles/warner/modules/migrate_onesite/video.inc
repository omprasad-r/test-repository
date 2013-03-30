<?php

class OnesiteVideoMigration extends OnesiteNodeMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = 'Migration of videos from OneSite';

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('core_video', 'v')
             ->fields('v')
             ->condition('status', 'ready')
             ->condition('video_system', 'youtube');
    $this->source = new MigrateSourceSQL($query, $this->sourceFields, NULL,
          array('map_joinable' => FALSE));

    $this->map = new MigrateSQLMap($this->machineName,
      array(
           'video_id' =>
             array('type' => 'int',
                   'unsigned' => TRUE,
                   'not null' => TRUE,
                   'description' => 'Unique video ID',
                  ),
           ),
        MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('body', 'description');
    $format_mapping = $this->addFieldMapping('body:format');
    if (isset($arguments['default_format'])) {
      $format_mapping->defaultValue($arguments['default_format']);
    }
    $this->addFieldMapping('uid', 'uploaded_by')
         ->sourceMigration($arguments['user_migration'])
         ->defaultValue(1);
    $this->addFieldMapping('created', 'date_uploaded');
    $this->addFieldMapping('changed', 'last_updated');
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);
    if (isset($arguments['video_field'])) {
      $field = $arguments['video_field'];
      $this->addFieldMapping($field, 'external_video_id');
      $this->addFieldMapping("{$field}:file_class")
           ->defaultValue('MigrateExtrasFileYoutube');
      $this->addFieldMapping("{$field}:title", 'title');
    }

    $this->addUnmigratedDestinations(array(
        "{$field}:language", "{$field}:data",
    ));

    $this->addUnmigratedSources(array(
      'aspect_ratio',
      'available_extensions',
      'bitrate',
      'blog_id',
      'external_thumbnail_url',
      'external_video_info',
      'filename',
      'gallery_id',
      'height',
      'moderated_by',
      'node_assoc',
      'num_views',
      'original_filename',
      'preview_name',
      'status',
      'video_system',
      'width',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $row->external_video_id = 'http://www.youtube.com/v/' . $row->external_video_id;
  }
}

class OnesiteVideoCommentMigration extends OnesiteCommentMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = 'Migration of video comments';

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
    $query->condition('ftl.type', 'video');
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
