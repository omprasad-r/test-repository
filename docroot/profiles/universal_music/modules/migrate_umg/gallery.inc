<?php
/**
 * @file
 * Gallery migration classes (from umg_cms) for UMG.
 */

/**
 * Common base class for node migrations from TBL_pages_cms.
 */
class UMGGalleryMigration extends UMGMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t("Migrate galleries from the source database to gallery content type");

    $this->dependencies = array($this->getGroup()->getName() . 'Photos');

    $this->destination = new MigrateDestinationNode('media_gallery');

    $query = $this->connection
      ->select('photogalleryphotos', 'p')
      ->fields('p');
    $query->innerJoin('photogallery', 'pg', 'pg.PhotoGalleryID = p.PhotoGalleryID');
    $query->innerJoin('photogalleryartist', 'ga',
      'ga.PhotoGalleryID = pg.PhotoGalleryID AND ga.ArtistID = :aid',
      array(':aid' => $arguments['artist_id']));
    $query->condition('p.ParentPhotoID', '0');
    // Skip empty galleries
    $query->innerJoin('photogalleryphotos', 'p2', 'p.PhotoID = p2.ParentPhotoID');
    $query->distinct();

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
      'ParentPhotoID' => '0 signifies a collection',
      'IsFolder' => 'Always 1',
      'CreateDate' => 'Photo creation date',
      'ModifyDate' => 'Photo modification date',
      'AuthorID' => 'ID of author',
      'PhotoDate' => 'Photo taken date?',
      'IsNewsFolder' => 'Always 1',
      // Constructed fields
      'photo_list' => 'List of photos to include in the gallery',
    );

    $this->source = new MigrateSourceSQL($query, $fields, NULL,
      array('map_joinable' => FALSE));

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'PhotoID' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'Use parent photo ID as key',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('title', 'Title');
    $this->addFieldMapping('media_gallery_description', 'Caption');
    $this->addFieldMapping('status', 'Status')
         ->callbacks(array($this, 'handleStatus'));
    $this->addFieldMapping('uid')
      ->defaultValue($arguments['default_author_id']);

    $this->addFieldMapping('media_gallery_media', 'photo_list')
      ->sourceMigration($this->getGroup()->getName() . 'Photos');
    $this->addFieldMapping('media_gallery_media:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('media_gallery_media:title', 'Caption');

    $this->addFieldMapping('created', 'CreateDate');
    $this->addFieldMapping('changed', 'ModifyDate');

    $this->addUnmigratedDestinations(array(
      'comment',
      'is_new',
      'language',
      'log',
      'media_gallery_allow_download',
      'media_gallery_block_columns',
      'media_gallery_block_rows',
      'media_gallery_collection', 'media_gallery_collection:source_type',
        'media_gallery_collection:create_term', 'media_gallery_collection:ignore_case',
      'media_gallery_columns',
        'media_gallery_description:format', 'media_gallery_description:language',
      'media_gallery_expose_block',
      'media_gallery_format',
      'media_gallery_image_info_where',
      'media_gallery_lightbox_extras',
        'media_gallery_media:language', 'media_gallery_media:data',
      'media_gallery_rows',
      'path',
      'pathauto',
      'promote',
      'revision',
      'revision_uid',
      'sticky',
      'tnid',
      'translate',
    ));

    $this->addUnmigratedSources(array(
      'Author',
      'AuthorID',
      'DisplayOrder',
      'IsFolder',
      'IsNewsFolder',
      'ParentPhotoID',
      'PhotoDate',
      'PhotoGalleryID',
      'PhotoUrl',
      'Status',
      'ThumbnailUrl',
    ));
  }

  /**
   * Callback to interpret source status values.
   *
   * @param $value
   *
   * @return int
   */
  protected function handleStatus($value) {
    if ($value == 2) {
      return NODE_PUBLISHED;
    }
    else {
      return NODE_NOT_PUBLISHED;
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

    $query = $this->connection
      ->select('photogalleryphotos', 'p')
      ->fields('p', array('PhotoID'))
      ->condition(db_or()
                  ->condition('p.ParentPhotoID', $row->PhotoID)
                  ->condition('p.PhotoID', $row->PhotoID))
      ->orderBy('ParentPhotoID', 'ASC')
      ->orderBy('DisplayOrder', 'ASC');

    $result = $query->execute();
    $row->photo_list = $result->fetchCol(0);
  }
}
