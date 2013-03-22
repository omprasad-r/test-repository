<?php
/**
 * @file
 * Forum migration classes for googoodolls import.
 */

require_once drupal_get_path('module', 'migrate_googoo') . '/bbcode-filter.inc';

class GooGooForumTopicMigration extends GooGooMigration {
  public function __construct() {
    parent::__construct();

    $this->description = 'Import of forum topics from smf database';
    $this->dependencies = array('GooGooUser');

    $fields = array(
      // smf_messages table
      'ID_MSG' => 'Unique message ID',
      'ID_TOPIC' => 'Topic ID - unique for forum topics',
      'ID_BOARD' => 'Forum ID',
      'posterTime' => 'UNIX timestamp',
      'ID_MEMBER' => 'Member ID',
      'subject' => 'Title',
      'posterName' => 'Member username',
      'posterEmail' => 'Member email',
      'posterIP' => 'Member IP',
      'smileysEnabled' => 'Boolean, only 3 zeros',
      'modifiedTime' => 'UNIX timestamp',
      'modifiedName' => 'Username who modified',
      'body' => 'Body text',
      'icon' => 'angry, cheesy, exclamation, ...',
      // smf_boards table
      'forum_name' => 'Forum name',
    );
    // Topics are distinguished from comments by being marked as the
    // first message in smf_topics.
    $query = Database::getConnection('default', 'googoo_smf')
             ->select('smf_messages', 'm')
             ->fields('m');
    $query->innerJoin('smf_topics', 't', 'm.ID_MSG=t.ID_FIRST_MSG');
    $query->innerJoin('smf_boards', 'b', 'm.ID_BOARD=b.ID_BOARD');
    $query->addField('b', 'name', 'forum_name');
    $this->source = new MigrateSourceSQL($query, $fields, NULL,
          array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationNode('forum');
    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'ID_TOPIC' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Topic ID',
                'alias' => 'm',
               ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('title', 'subject')
         ->callbacks('html_entity_decode');
    $this->addFieldMapping('uid', 'posterName')
         ->sourceMigration('GooGooUser')
         ->defaultValue($this->defaultAuthor);
    $this->addFieldMapping('created', 'posterTime');
    $this->addFieldMapping('changed', 'modifiedTime');
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);
    $this->addFieldMapping('field_is_pro')
         ->defaultValue(0);
    $this->addFieldMapping('body', 'body')
         ->callbacks('_bbcode_filter_process');
    $this->addFieldMapping('body:format')
         ->defaultValue('6');
    $this->addFieldMapping('taxonomy_forums', 'forum_name');
    $this->addFieldMapping('taxonomy_forums:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
        'body:summary', 'body:language',
      'comment',
      'daycount',
      'is_new',
      'language',
      'log',
      'migrate_redirects',
      'path',
      'pathauto',
      'promote',
      'revision',
      'revision_uid',
      'sticky',
        'taxonomy_forums:source_type', 'taxonomy_forums:ignore_case',
      'timestamp',
      'tnid',
      'totalcount',
      'translate',
    ));

    $this->addUnmigratedSources(array(
      'icon',
      'ID_BOARD',
      'ID_MEMBER',
      'ID_MSG',
      'modifiedName',
      'posterEmail',
      'posterIP',
      'smileysEnabled',
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

    // Default 0 modified times to the creation time.
    if (empty($row->modifiedTime)) {
      $row->modifiedTime = $row->posterTime;
    }
  }
}

class GooGooForumCommentMigration extends GooGooMigration {
  public function __construct() {
    parent::__construct();

    $this->description = 'Import of forum comments from smf database';
    $this->dependencies = array('GooGooForumTopic', 'GooGooUser');

    $fields = array(
      // smf_messages table
      'ID_MSG' => 'Unique message ID',
      'ID_TOPIC' => 'Topic ID - unique for forum topics',
      'ID_BOARD' => 'Forum ID',
      'posterTime' => 'UNIX timestamp',
      'ID_MEMBER' => 'Member ID',
      'subject' => 'Title',
      'posterName' => 'Member username',
      'posterEmail' => 'Member email',
      'posterIP' => 'Member IP',
      'smileysEnabled' => 'Boolean, only 3 zeros',
      'modifiedTime' => 'UNIX timestamp',
      'modifiedName' => 'Username who modified',
      'body' => 'Body text',
      'icon' => 'angry, cheesy, exclamation, ...',
    );
    // Topics are distinguished from comments by being marked as the
    // first message in smf_topics.
    $query = Database::getConnection('default', 'googoo_smf')
             ->select('smf_messages', 'm')
             ->fields('m');
    $query->leftJoin('smf_topics', 't', 'm.ID_MSG=t.ID_FIRST_MSG');
    $query->isNull('t.ID_TOPIC');
    $this->source = new MigrateSourceSQL($query, $fields, NULL,
          array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationComment('comment_node_forum');
    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'ID_MSG' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Comment ID',
                'alias' => 'm',
               ),
      ),
      MigrateDestinationComment::getKeySchema()
    );

    $this->addFieldMapping('nid', 'ID_TOPIC')
         ->sourceMigration('GooGooForumTopic');
    $this->addFieldMapping('uid', 'posterName')
         ->sourceMigration('GooGooUser')
         ->defaultValue($this->defaultAuthor);
    $this->addFieldMapping('created', 'posterTime');
    $this->addFieldMapping('changed', 'modifiedTime');
    $this->addFieldMapping('status')
         ->defaultValue(COMMENT_PUBLISHED);
    $this->addFieldMapping('hostname', 'posterIP');
    $this->addFieldMapping('name', 'posterName');
    $this->addFieldMapping('mail', 'posterEmail');
    $this->addFieldMapping('comment_body', 'body')
         ->callbacks('_bbcode_filter_process');
    $this->addFieldMapping('comment_body:format')
         ->defaultValue('6');

    $this->addUnmigratedDestinations(array(
        'comment_body:language',
      'homepage',
      'language',
      'migrate_redirects',
      'pid',
      'subject',
      'thread',
    ));

    $this->addUnmigratedSources(array(
      'icon',
      'ID_BOARD',
      'ID_MEMBER',
      'modifiedName',
      'smileysEnabled',
      'subject',  // Subjects are just Re: plus the topic title
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

    // Default 0 modified times to the creation time.
    if (empty($row->modifiedTime)) {
      $row->modifiedTime = $row->posterTime;
    }
  }
}
