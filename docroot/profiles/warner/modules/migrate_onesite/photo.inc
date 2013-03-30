<?php
/**
 * @file
 * Migration class for importing photos from ONEsite to Drupal. May be
 * extended, particularly to add project-specific field mappings.
 */

class OnesitePhotoMigration extends OnesiteNodeMigration {
  /**
   * @param array $arguments
   *  image_field (options) - Name of a Drupal image field to hold the migrated
   *    image.
   */
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = 'Migration of photos from OneSite';

    $this->sourceFields = array(
      'photo_id' => 'Unique photo ID - strangely, encrypted',
      'owner_id' => 'ID of photo owner',
      'blog_id' => 'ID of containing blog',
      'filename' => 'Local filename',
      'caption' => 'Photo caption',
      'viewable' => 'active/deleted/public',
      'content_status' => 'Ignore deleted',
      // Need to pick up the owner's domain and subdir from os_user
      'domain' => 'Owner domain',
      'subdir' => 'Owner subdirectory (also used as usename)',
    );

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('photo', 'p')
             ->fields('p')
             ->condition('p.content_status', 'deleted', '<>');
    $query->innerJoin('user', 'u', 'p.owner_id=u.user_id');
    $query->fields('u', array('domain', 'subdir'));
    $query->leftJoin('photo_gal', 'pg', 'p.gallery=pg.gallery_id AND p.blog_id=pg.blog_id');
    $query->fields('pg', array('gallery_name'));
    $this->source = new MigrateSourceSQL($query, $this->sourceFields, NULL,
          array('map_joinable' => FALSE));

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'photo_id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Unique photo ID',
                'alias' => 'p',
               ),
        ),
        MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('title', 'caption');
    $this->addFieldMapping('uid', 'owner_id')
         ->sourceMigration($arguments['user_migration'])
         ->defaultValue(1);
    if (isset($arguments['image_field'])) {
      $field = $arguments['image_field'];
      $this->addFieldMapping($field, 'filename');
      $this->addFieldMapping("{$field}:file_class")
           ->defaultValue('MigrateFileUri');
      $this->addFieldMapping("{$field}:title", 'caption');
      $this->addFieldMapping('status', 'content_status')
           ->callbacks(array($this, 'handleStatus'));
      $this->addFieldMapping("{$field}:source_dir", 'subdir');
    }

    $this->addUnmigratedDestinations(array(
      'body', 'body:format',
      'changed',
      'created',
    ));

    $this->addUnmigratedSources(array(
      'blog_id',
      'domain',
      'viewable',
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

    // Construct the directory holding this image from the domain and the user's
    // subdirectory.
    $row->subdir =
      'http://fast1.onesite.com/'. $row->domain . '/user/' . $row->subdir . '/';

    // Odd case - the general algorithm suggests it should end in _, but it
    // doesn't in reality.
    if ($row->gallery_name == '- Jayy Nett !') {
      $gallery_folder = '_jayy_nett';
    }
    else {
      // The photo might be in a subdirectory based on the gallery name, check
      // there first. Since accessing a non-existent file returns a 69-byte 1x1
      // PNG image, read 100 bytes to tell whether there's really a file there.
      $gallery_folder = strtolower($row->gallery_name);
      $gallery_folder = str_replace(' ', '_', $gallery_folder);
      $gallery_folder = preg_replace('|[^a-z0-9\._,]|', '', $gallery_folder);
    }
    $subdir = $row->subdir . $gallery_folder . '/';
    if ($handle = @fopen($subdir . $row->filename, 'r')) {
      $taste = fread($handle, 100);
      fclose($handle);
      if (strlen($taste) > 90) {
        $row->subdir = $subdir;
      }
      else {
        // Also try the top level
        $subdir = 'http://fast1.onesite.com/'. $row->domain . '/';
        if ($handle = @fopen($subdir . $row->filename, 'r')) {
          $taste = fread($handle, 100);
          fclose($handle);
          if (strlen($taste) > 90) {
            $row->subdir = $subdir;
          }
        }
      }
    }
  }
}

class OnesitePhotoCommentMigration extends OnesiteCommentMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = 'Migration of photo comments';

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
    $query->condition('ftl.type', 'photo');
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
