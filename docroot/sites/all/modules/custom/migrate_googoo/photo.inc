<?php
/**
 * @file
 * Photo migration classes for googoodolls import.
 */

class GooGooPhotoMoblogMigration extends GooGooMigration {
  public function __construct() {
    parent::__construct();

    $this->description = 'Import of photos from moblog';

    $fields = array(
      // mafia table
      'id' => 'Unique ID',
      'view' => 'yes/no/pic',
      'pic' => 'Filename relative to member/moblog/pics',
      'type' => 'pic/vid',
      'title' => 'Always empty',
      'descrip' => 'Always empty',
      'date' => 'Date of photo',
      'time' => 'Time of photo',
      'tags' => 'Always empty',
      'comms' => 'Number of comments?',
      'special' => 'Always empty',
    );
    $query = Database::getConnection('default', 'googoo_moblog')
             ->select('mafia', 'm')
             ->fields('m')
             ->condition('view', 'yes')
             ->condition('type', 'pic');
    $this->source = new MigrateSourceSQL($query, $fields, NULL,
          array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationNode('photo');
    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Photo ID',
               ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('field_photo_asset', 'pic');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileUri');
    $this->addFieldMapping('field_photo_asset:source_dir')
         ->defaultValue('http://23.23.120.51/member/moblog/pics/');
    $this->addFieldMapping('field_photo_asset:destination_dir')
         ->defaultValue('public://legacy');
    $this->addFieldMapping('field_photo_asset:destination_file', 'pic');
    $this->addFieldMapping('field_photo_asset:file_replace')
         ->defaultValue(MigrateFile::FILE_EXISTS_REUSE);
    $this->addFieldMapping('field_photo_asset:preserve_files')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_photo_asset:title', 'title');

    $this->addFieldMapping('title', 'pic');
    $this->addFieldMapping('uid')
         ->defaultValue($this->defaultAuthor);
    $this->addFieldMapping('created', 'date');
    $this->addFieldMapping('changed', 'date');
    $this->addFieldMapping(NULL, 'time')
         ->description('Appended to date');
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);
    $this->addFieldMapping('field_is_pro')
         ->defaultValue(1);
    $this->addFieldMapping('field_photo_category')
         ->defaultValue('Fan Photos');
    $this->addFieldMapping('field_photo_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'body', 'body:summary', 'body:format', 'body:language',
      'comment',
      'daycount',
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language','field_photo_asset:data',
        'field_photo_category:source_type', 'field_photo_category:ignore_case',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
      'is_new',
      'language',
      'log',
      'migrate_redirects',
      'path',
      'promote',
      'revision',
      'revision_uid',
      'sticky',
      'timestamp',
      'tnid',
      'totalcount',
      'translate',
    ));

    $this->addUnmigratedSources(array(
      'comms',
      'descrip',
      'special',
      'tags',
      'type',
      'view',
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

    $this->handleDate($row);
  }
}

class GooGooPhotoMoblogCommentMigration extends GooGooMigration {
  public function __construct() {
    parent::__construct();

    $this->description = 'Import of photo comments from moblog';
    $this->dependencies = array('GooGooPhotoMoblog', 'GooGooUser');

    $fields = array(
      // comments table
      'id' => 'Unique ID',
      'for_id' => 'ID of photo being commented on',
      'open' => 'Always open',
      'poster' => 'Username of poster',
      'url' => 'Always http://',
      'comment' => 'Text of the comment',
      'date' => 'Date of the comment',
      'time' => 'Time of the comment',
      'ip' => 'Poster IP address',
    );
    $query = Database::getConnection('default', 'googoo_moblog')
             ->select('comments', 'c')
             ->fields('c');
    $query->innerJoin('mafia', 'm', 'c.for_id=m.id');
    $query->condition('m.view', 'yes')
          ->condition('m.type', 'pic');
    $this->source = new MigrateSourceSQL($query, $fields, NULL,
          array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationComment('comment_node_photo');
    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Comment ID',
                'alias' => 'c',
               ),
      ),
      MigrateDestinationComment::getKeySchema()
    );

    $this->addFieldMapping('nid', 'for_id')
         ->sourceMigration('GooGooPhotoMoblog');
    $this->addFieldMapping('uid', 'poster')
         ->sourceMigration('GooGooUser');
    $this->addFieldMapping('created', 'date');
    $this->addFieldMapping('changed', 'date');
    $this->addFieldMapping(NULL, 'time')
         ->description('Appended to date');
    $this->addFieldMapping('status')
         ->defaultValue(COMMENT_PUBLISHED);
    $this->addFieldMapping('hostname', 'ip');
    $this->addFieldMapping('name', 'poster');
    $this->addFieldMapping('comment_body', 'comment');
    $this->addFieldMapping('comment_body:format')
         ->defaultValue('6');

    $this->addUnmigratedDestinations(array(
        'comment_body:language',
      'homepage',
      'language',
      'mail',
      'migrate_redirects',
      'pid',
      'subject',
      'thread',
    ));

    $this->addUnmigratedSources(array(
      'open',
      'url',
    ));
  }

  /**
   * Implements Migration:;prepareRow().
   *
   * @param $row
   *
   * @return bool
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $this->handleDate($row);
  }
}

class GooGooPhotoMigration extends GooGooMigration {
  public function __construct() {
    parent::__construct();

    $this->description = 'Import of photos from smf database';
    $this->dependencies = array('GooGooUser');

    $fields = array(
      // cpg_pictures table
      'pid' => 'Unique ID',
      'aid' => 'Album ID',
      'filepath' => 'Folder containing photo',
      'filename' => 'Filename',
      'filesize' => 'Size of file',
      'total_filesize' => '?',
      'pwidth' => 'Photo width',
      'pheight' => 'Photo height',
      'hits' => 'Views?',
      'mtime' => 'Timestamp in yyyy-mm-dd hh:mm:ss form',
      'ctime' => 'UNIX timestamp',
      'owner_id' => 'ID of user',
      'owner_name' => 'Username of user',
      'pic_rating' => 'Rating',
      'votes' => 'Votes',
      'title' => 'Title',
      'caption' => 'Caption',
      'keywords' => 'Keywords',
      'approved' => 'Always YES',
      'user1' => 'Always empty',
      'user2' => 'Always empty',
      'user3' => 'Always empty',
      'user4' => 'Always empty',
      'url_prefix' => 'Always 0',
      'randpos' => '0-3',
      'pic_raw_ip' => 'IP address',
      'pic_hdr_ip' => 'Another IP address',
      // cpg_albums table
      'album_title' => 'Title of photo album',
    );
    $query = Database::getConnection('default', 'googoo_smf')
             ->select('cpg_pictures', 'p')
             ->fields('p');
    $query->leftJoin('cpg_albums', 'a', 'p.aid=a.aid');
    $query->addField('a', 'title', 'album_title');
    $this->source = new MigrateSourceSQL($query, $fields, NULL,
          array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationNode('photo');
    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'pid' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Photo ID',
               ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('field_photo_asset', 'filename');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileUri');
    $this->addFieldMapping('field_photo_asset:source_dir')
         ->defaultValue('http://23.23.120.51/member/gallery/albums/');
    $this->addFieldMapping('field_photo_asset:destination_dir')
         ->defaultValue('public://legacy');
    $this->addFieldMapping('field_photo_asset:destination_file', 'filename');
    $this->addFieldMapping('field_photo_asset:file_replace')
         ->defaultValue(MigrateFile::FILE_EXISTS_REUSE);
    $this->addFieldMapping('field_photo_asset:preserve_files')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_photo_asset:title', 'caption');
    $this->addFieldMapping(NULL, 'filepath')
         ->description('Prepended to filename');

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('uid', 'owner_name')
         ->sourceMigration('GooGooUser');
    $this->addFieldMapping('created', 'ctime');
    $this->addFieldMapping('changed', 'ctime');
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);
    $this->addFieldMapping('field_is_pro')
         ->defaultValue(0);
    $this->addFieldMapping('field_photo_category', 'album_title')
         ->callbacks(array($this, 'handleCategory'));
    $this->addFieldMapping('field_photo_category:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_rating', 'pic_rating')
         ->callbacks(array($this, 'handleRating'));

    $this->addUnmigratedDestinations(array(
      'body', 'body:summary', 'body:format', 'body:language',
      'comment',
      'daycount',
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language','field_photo_asset:data',
        'field_photo_category:source_type', 'field_photo_category:ignore_case',
      'field_secondary_title', 'field_secondary_title:language',
      'is_new',
      'language',
      'log',
      'migrate_redirects',
      'path',
      'promote',
      'revision',
      'revision_uid',
      'sticky',
      'timestamp',
      'tnid',
      'totalcount',
      'translate',
    ));

    $this->addUnmigratedSources(array(
      'aid',
      'approved',
      'filesize',
      'hits',
      'keywords',
      'mtime',
      'owner_id',
      'pheight',
      'pic_hdr_ip',
      'pic_raw_ip',
      'pwidth',
      'randpos',
      'total_filesize',
      'url_prefix',
      'user1',
      'user2',
      'user3',
      'user4',
      'votes',
    ));
  }

  /**
   * Translate the rating value.
   *
   * @param $value
   *
   * @return float
   */
  protected function handleRating($value) {
    return $value/2000;
  }

  protected function handleCategory($value) {
    switch (drupal_strtolower($value)) {
      case 'submitted pictures':
      case 'let love in':
      case 'gutterflower':
      case 'dizzy up the girl':
      case 'stay with you video':
        return 'Exclusive';
      default:
        return 'Fan Photos';
    }
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

    if (empty($row->title)) {
      $row->title = $row->filename;
    }

    $row->filename = $row->filepath . $row->filename;
  }
}

class GooGooPhotoCommentMigration extends GooGooMigration {
  public function __construct() {
    parent::__construct();

    $this->description = 'Import of photo comments from smf database';
    $this->dependencies = array('GooGooPhoto', 'GooGooUser');

    $fields = array(
      // comments table
      'pid' => 'ID of photo being commented on',
      'msg_id' => 'Unique ID',
      'msg_author' => 'Username of poster',
      'msg_body' => 'Text of the comment',
      'msg_date' => 'Date of the comment',
      'msg_raw_ip' => 'IP address',
      'msg_hdr_ip' => 'IP address',
      'author_md5_id' => 'Always empty',
      'author_id' => 'User ID of author',
    );
    $query = Database::getConnection('default', 'googoo_smf')
             ->select('cpg_comments', 'c')
             ->fields('c');
    $this->source = new MigrateSourceSQL($query, $fields, NULL,
          array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationComment('comment_node_photo');
    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'msg_id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Comment ID',
                'alias' => 'c',
               ),
      ),
      MigrateDestinationComment::getKeySchema()
    );

    $this->addFieldMapping('nid', 'pid')
         ->sourceMigration('GooGooPhoto');
    $this->addFieldMapping('uid', 'msg_author')
         ->sourceMigration('GooGooUser');
    $this->addFieldMapping('created', 'msg_date');
    $this->addFieldMapping('changed', 'msg_date');
    $this->addFieldMapping('status')
         ->defaultValue(COMMENT_PUBLISHED);
    $this->addFieldMapping('hostname', 'msg_raw_ip');
    $this->addFieldMapping('name', 'msg_author');
    $this->addFieldMapping('comment_body', 'msg_body');
    $this->addFieldMapping('comment_body:format')
         ->defaultValue('6');

    $this->addUnmigratedDestinations(array(
        'comment_body:language',
      'homepage',
      'language',
      'mail',
      'migrate_redirects',
      'pid',
      'subject',
      'thread',
    ));

    $this->addUnmigratedSources(array(
      'author_id',
      'author_md5_id',
      'msg_hdr_ip',
    ));
  }
}
