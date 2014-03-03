<?php
/**
 * @file
 * Turn image nodequeues into gallery nodes for Nonesuch migration.
 */

class MigrateNonesuchGallery extends DrupalNode5Migration {
  /**
   * @param array $arguments
   */
  public function __construct(array $arguments) {

    $this->sourceFields += array(
      'images' => 'List of referenced image nodes in queue',
      'timestamp' => 'Timestamp of first image in queue',
    );

    parent::__construct($arguments);
    $this->dependencies[] = 'NonesuchRecordsNodeimage';

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'sqid' => array('type' => 'int',
                       'unsigned' => TRUE,
                       'not null' => TRUE,
                       'description' => 'Source subqueue ID',
                       'alias' => 'sq',
                      ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('uid', NULL, FALSE)
         ->defaultValue(1);
    $this->addFieldMapping('title', 'title', FALSE);
    $this->addFieldMapping('created', 'timestamp', FALSE);
    $this->addFieldMapping('changed', 'timestamp', FALSE);
    $this->addFieldMapping('status', NULL, FALSE)
         ->defaultValue(1);
    $this->addFieldMapping('media_gallery_media', 'images')
         ->sourceMigration('NonesuchRecordsFile');
    $this->addFieldMapping('media_gallery_media:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('media_gallery_media:preserve_files')
         ->defaultValue(TRUE);
    // @todo: field_artist_node needs to be an entity reference to an artist
    // node. All we have to go on is the subqueue title, will that match directly to
    // artist names?

    $this->removeFieldMapping('comment');
    $this->removeFieldMapping('daycount');
    $this->removeFieldMapping('is_new');
    $this->removeFieldMapping('log');
    $this->removeFieldMapping('path');
    $this->removeFieldMapping('pathauto');
    $this->removeFieldMapping('promote');
    $this->removeFieldMapping('revision');
    $this->removeFieldMapping('revision_uid');
    $this->removeFieldMapping('sticky');
    $this->removeFieldMapping('timestamp');
    $this->removeFieldMapping('totalcount');
    $this->removeFieldMapping(NULL, 'vid');
    $this->removeFieldMapping(NULL, 'moderate');

    $this->addUnmigratedDestinations(array(
      'comment',
      'daycount',
      'is_new',
      'language',
      'log',
      'media_gallery_allow_download',
      'media_gallery_block_columns',
      'media_gallery_block_rows',
      'media_gallery_collection', 'media_gallery_collection:source_type',
        'media_gallery_collection:create_term', 'media_gallery_collection:ignore_case',
      'media_gallery_columns',
      'media_gallery_description',
      'media_gallery_description:format',
      'media_gallery_description:language',
      'media_gallery_expose_block',
      'media_gallery_format',
      'media_gallery_image_info_where',
      'media_gallery_lightbox_extras',
      'media_gallery_media:language',
      'media_gallery_media:title',
      'media_gallery_media:data',
      'media_gallery_rows',
      'metatag_abstract',
      'metatag_author',
      'metatag_canonical',
      'metatag_copyright',
      'metatag_description',
      'metatag_fb:admins',
      'metatag_fb:app_id',
      'metatag_generator',
      'metatag_image_src',
      'metatag_keywords',
      'metatag_news_keywords',
      'metatag_og:country-name',
      'metatag_og:description',
      'metatag_og:email',
      'metatag_og:fax_number',
      'metatag_og:image',
      'metatag_og:latitude',
      'metatag_og:locality',
      'metatag_og:longitude',
      'metatag_og:phone_number',
      'metatag_og:postal-code',
      'metatag_og:region',
      'metatag_og:site_name',
      'metatag_og:street-address',
      'metatag_og:title',
      'metatag_og:type',
      'metatag_og:url',
      'metatag_og:video',
      'metatag_og:video:height',
      'metatag_og:video:secure_url',
      'metatag_og:video:type',
      'metatag_og:video:width',
      'metatag_original-source',
      'metatag_publisher',
      'metatag_robots',
      'metatag_shortlink',
      'metatag_title',
      'metatag_twitter:card',
      'metatag_twitter:creator',
      'metatag_twitter:creator:id',
      'metatag_twitter:description',
      'metatag_twitter:image',
      'metatag_twitter:image:height',
      'metatag_twitter:image:width',
      'metatag_twitter:player',
      'metatag_twitter:player:height',
      'metatag_twitter:player:stream',
      'metatag_twitter:player:stream:content_type',
      'metatag_twitter:player:width',
      'metatag_twitter:site',
      'metatag_twitter:site:id',
      'metatag_twitter:title',
      'metatag_twitter:url',
      'path',
      'pathauto',
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
      'changed',
      'created',
      'language',
      'log',
      'nid',
      'path',
      'promote',
      'revision',
      'revision_uid',
      'status',
      'sticky',
      'tnid',
      'uid',
    ));
  }

  /**
   * Query the subqueue
   *
   * @return QueryConditionInterface
   */
  protected function query() {
    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('nodequeue_subqueue', 'sq')
             ->fields('sq', array('sqid', 'title'));
    $query->innerJoin('nodequeue_types', 't', 'sq.qid=t.qid');
    $query->condition('t.type', 'image');

    return $query;
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

    // Pull the nodes in this subqueue.
    // @todo: Actually, we need to get the fids of the images referenced by
    // these nodes...
    $query = Database::getConnection('default', $this->sourceConnection)
      ->select('nodequeue_nodes', 'n')
      ->fields('n', array('nid', 'timestamp'))
      ->condition('sqid', $row->sqid)
      ->orderBy('position')
      ->execute();
    $row->images = array();
    $first = TRUE;
    foreach ($query as $queue_row) {
      $row->images[] = $queue_row->nid;
      if ($first) {
        $first = FALSE;
        $row->timestamp = $queue_row->timestamp;
      }
    }
    return TRUE;
  }
}
