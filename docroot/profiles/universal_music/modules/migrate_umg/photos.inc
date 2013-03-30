<?php
/**
 * @file
 * Photo migration classes (from umg_cms) for UMG.
 */

/**
 * Class for photo migrations from umg_cms.
 */
class UMGPhotoMigration extends UMGMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t('Migrate photos from the source database to media entities');

    $fields = array(
      // From photogalleryphotos
      'PhotoID' => 'Unique photo ID',
      'PhotoGalleryID' => 'ID of the gallery this photo belongs to',
      'DisplayOrder' => 'Sort order in a gallery',
      'PhotoUrl' => 'Location of the photo',
      'Caption' => 'Photo caption',
      'Title' => 'Photo title',
      'Author' => 'Username of author',
      'ThumbnailUrl' => 'Location of the thumbnail',
      'Status' => 'Usually 2 except on one record (0)',
      'ParentPhotoID' => '',
      'IsFolder' => 'Always 1',
      'CreateDate' => 'Photo creation date',
      'ModifyDate' => 'Photo modification date',
      'AuthorID' => 'ID of author',
      'PhotoDate' => 'Photo taken date?',
      'IsNewsFolder' => 'Always 1',
    );

    $query = $this->connection
      ->select('photogalleryphotos', 'p')
      ->fields('p');
    $query->innerJoin('photogalleryartist', 'pga',
      'p.PhotoGalleryID = pga.PhotoGalleryID AND pga.ArtistID = :aid',
      array(':aid' => $arguments['artist_id'])
    );
    // Check to see if the parent ID is valid.
    $query->leftJoin('photogalleryphotos', 'p2',
      'p.ParentPhotoID = p2.PhotoID');
    $query->addField('p2', 'PhotoID', 'ParentExists');

    $this->source = new MigrateSourceSQL($query, $fields, NULL,
      array('map_joinable' => FALSE));

    $this->destination = new MigrateDestinationMedia('image');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'PhotoID' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'Photo ID',
        ),
      ),
      MigrateDestinationMedia::getKeySchema()
    );

    $this->addFieldMapping('media_title', 'Title');
    $this->addFieldMapping('preserve_files')->defaultValue(FALSE);
    $this->addFieldMapping('timestamp', 'CreateDate');
    $this->addFieldMapping('media_description', 'Caption');
    $this->addFieldMapping('uid')
      ->defaultValue($arguments['default_author_id']);
    $this->addFieldMapping('value', 'PhotoUrl')
      ->callbacks(array($this, 'prepUrl'));
    $this->addFieldMapping('destination_file')
      ->issueGroup(t('DNM'));

    $this->addFieldMapping('field_license')
      ->description(t('What license should we default to?'))
      ->issueGroup('Stakeholder');

    $this->addFieldMapping('source_dir')
      ->description('Differs from record to record - handled programmatically')
      ->issueGroup('DNM');

    $this->addFieldMapping('media_title:language')
      ->description(t(''))
      ->issueGroup(t('TODO'));

    $this->addFieldMapping('media_description:format')
      ->description(t(''))
      ->issueGroup(t('TODO'));

    $this->addFieldMapping('media_description:language')
      ->description(t(''))
      ->issueGroup(t('TODO'));

    $this->addFieldMapping('destination_dir')
      ->description(t(''))
      ->issueGroup(t('TODO'));

    $this->addFieldMapping('file_replace')
      ->description(t(''))
      ->issueGroup(t('TODO'));

    $this->addFieldMapping(NULL, 'PhotoGalleryID')
      ->description(t(''))
      ->issueGroup(t('TODO'));

    $this->addFieldMapping(NULL, 'DisplayOrder')
      ->description(t(''))
      ->issueGroup(t('TODO'));

    $this->addFieldMapping(NULL, 'Author')
      ->description(t('Not migrating users'))
      ->issueGroup(t('DNM'));

    $this->addFieldMapping(NULL, 'AuthorID')
      ->description(t('Not migrating users'))
      ->issueGroup(t('DNM'));

    $this->addFieldMapping(NULL, 'ThumbnailUrl')
      ->description(t('Thumbnails are re-generated in Drupal'))
      ->issueGroup(t('DNM'));

    $this->addFieldMapping(NULL, 'Status')
      ->description(t(''))
      ->issueGroup(t('TODO'));

    $this->addFieldMapping(NULL, 'ParentPhotoID')
      ->description(t(''))
      ->issueGroup(t('TODO'));

    $this->addFieldMapping(NULL, 'IsFolder')
      ->description(t(''))
      ->issueGroup(t('TODO'));

    $this->addFieldMapping(NULL, 'ModifyDate')
      ->description(t(''))
      ->issueGroup(t('TODO'));

    $this->addFieldMapping(NULL, 'PhotoDate')
      ->description(t(''))
      ->issueGroup(t('TODO'));

    $this->addFieldMapping(NULL, 'IsNewsFolder')
      ->description(t(''))
      ->issueGroup(t('TODO'));
  }

  /**
   * Looks at the URL value and determines how it needs to be prepended
   */
  protected function prepUrl($value)  {
    $pos = strpos($value, '/');
    if ($pos === FALSE) {
      $url = 'http://cache.interscope.com/images/local/gallery/' . $value;
    }
    else {
      $url = 'http://cache.interscope.com' . $value;
    }
    if (!$this->urlExists($url)) {
      $url = str_replace('/gallery/', '/500/', $url);
    }
    return $url;
  }

  protected function urlExists($url) {
    $headers = @get_headers($url);
    if (!$headers) {
      return FALSE;
    }
    else {
      return TRUE;
    }
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) == FALSE) {
      return FALSE;
    }

    // Fallback to the thumbnail if there's no main URL.
    if (empty($row->PhotoUrl)) {
      $row->PhotoUrl = $row->ThumbnailUrl;
    }
  }

  /**
   * Implements Migration::complete().
   *
   * @param $file
   * @param $row
   */
  public function complete($file, $row) {
    // Handle orphaned children - create (or find) a parent gallery and assign
    // them to it.
    if ($row->ParentPhotoID > 0 && !$row->ParentExists) {
      $gallery_title = 'Gallery ' . $row->ParentPhotoID;
      // See if this gallery already exists
      $query = new EntityFieldQuery();
      $query->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'media_gallery')
        ->propertyCondition('title', $gallery_title);
      $result = $query->execute();
      if (isset($result['node'])) {
        $nid = reset(array_keys($result['node']));
        $gallery_node = node_load($nid);
      }
      else {
        $gallery_node = new stdClass;
        $gallery_node->uid = $this->arguments['default_author_id'];
        $gallery_node->title = $gallery_title;
        $gallery_node->type = 'media_gallery';
        $gallery_node->language = LANGUAGE_NONE;
      }
      $gallery_node->media_gallery_media[LANGUAGE_NONE][] =
        array('fid' => $file->fid);
      node_save($gallery_node);
    }
  }
}
