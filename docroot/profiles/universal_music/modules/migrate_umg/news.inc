<?php
/**
 * @file
 * News migration class for UMG.
 */

/**
 * News migration class for UMG.
 */
class UMGNewsMigration extends UMGMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t("Migrate news stories from the source database to blog entries");

    $this->dependencies = array($this->getGroup()->getName() . 'NewsStoryType');

    $fields = array(
      // From newsstories
      'NewsStoryID' => 'Unique story ID',
      'Title' => 'Story title',
      'TitleUrl' => 'Usually blank, if present an external URL',
      'IsNewWindow' => 'Always 1',
      'Abstract' => 'Story teaser',
      'PublishedDate' => 'When story was published',
      'Copy' => 'Content of story ',
      'ImageUrl' => 'Local filespec of related image',
      'ImageLinkUrl' => 'Always empty',
      'IconUrl' => 'Always empty',
      'NewsStoryTypeID' => 'Reference to newsstorytypes table',
      'Status' => 'Usually 2 (published?), sometimes 0 (unpublished?)',
      'CreateDate' => 'Story creation date',
      'ModifyDate' => 'Story modification date',
      'IsSiteWide' => 'Always 1',
      'Author' => 'Username of author',
      'AuthorID' => 'ID of author',
      'CountryID' => 'Two-letter country code, usually XX or blank',
      'OpenDate' => 'Usually NULL',
      'CloseDate' => 'Usually NULL',
      'IsPinned' => 'Always 1',
      'PhotoGalleryID' => 'Usually NULL',
      'PhotoGalleryFolderID' => 'Usually NULL',
    );

    $query = $this->connection
      ->select('newsstories', 'n')
      ->fields('n');

    $query->innerJoin('newsartist', 'na', 'n.NewsStoryID = na.NewsStoryID AND na.ArtistID = :aid',
                      array(':aid' => $arguments['artist_id']));

    $this->source = new MigrateSourceSQL($query, $fields, NULL,
      array('map_joinable' => FALSE));

    $this->destination = new MigrateDestinationNode('blog');

    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'NewsStoryID' => array('type' => 'int',
                             'unsigned' => TRUE,
                             'not null' => TRUE,
                             'description' => 'News ID',
                            )
        ),
        MigrateDestinationNode::getKeySchema()
      );

    $this->addFieldMapping('title', 'Title');
    $this->addFieldMapping('body', 'Copy');
    $this->addFieldMapping('body:summary', 'Abstract');
    $this->addFieldMapping('body:format')
         ->defaultValue('full_html');
    $this->addFieldMapping('status', 'Status')
         ->callbacks(array($this, 'handleStatus'));
    $this->addFieldMapping('created', 'CreateDate');
    $this->addFieldMapping('changed', 'ModifyDate');
    $this->addFieldMapping('uid')
         ->defaultValue($arguments['default_author_id']);
    $this->addFieldMapping('field_tags', 'NewsStoryTypeID')
         ->sourceMigration($this->getGroup()->getName() . 'NewsStoryType');
    $this->addFieldMapping('field_tags:source_type')
         ->defaultValue('tid');

    $this->addFieldMapping(NULL, 'ImageUrl')
      ->description(t('Will a field be created for this?'))
      ->issueGroup(t('TODO'));

    $this->addUnmigratedDestinations(array(
        'body:language',
      'comment',
        'field_tags:create_term', 'field_tags:ignore_case',
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
      'tnid',
      'translate',
    ));

    $this->addUnmigratedSources(array(
      'Author',
      'AuthorID',
      'CloseDate',
      'CountryID',
      'IconUrl',
      'ImageLinkUrl',
      'IsNewWindow',
      'IsPinned',
      'IsSiteWide',
      'OpenDate',
      'PhotoGalleryFolderID',
      'PhotoGalleryID',
      'PublishedDate',
      'TitleUrl',
    ));
  }

  protected function handleStatus($value) {
    if ($value == 2) {
      return NODE_PUBLISHED;
    }
    else {
      return NODE_NOT_PUBLISHED;
    }
  }

  public function prepareRow($row) {
    // Rewrite any img tags referencing local files to point to a media
    // tag for the migrated file

    $row->Copy = preg_replace_callback('|<img +(.*?)>|i',
      array($this, 'replaceImgs'), $row->Copy);
    $row->Abstract = preg_replace_callback('|<img +(.*?)>|i',
      array($this, 'replaceImgs'), $row->Abstract);
  }

  /**
   * If we have an image reference, replace it with media tags if we can.
   *
   * @param array $matches
   */
  protected function replaceImgs(array $matches) {
    // Default to the original <img> tag.
    $result = $matches[0];

    // The src parameter is required
    if (preg_match('|src=[\'"]http://cache.umusic.com/(.*?)[\'"]|i', $result, $src_matches)) {
      $src = $src_matches[1];
      $src_dir = 'http://cache.umusic.com/';
    }
    else if (preg_match('|src=[\'"]http://www.interscope.com/(.*?)[\'"]|i', $result, $src_matches)) {
      $src = $src_matches[1];
      $src_dir = 'http://www.interscope.com/';
    }
    else if (preg_match('|src=[\'"]http://cache.interscope.com/(.*?)[\'"]|i', $result, $src_matches)) {
      $src = $src_matches[1];
      $src_dir = 'http://cache.interscope.com/';
    }
    else if (preg_match('|src=[\'"]http://' . $this->artistSiteUrl . '/(.*?)[\'"]|i', $result, $src_matches)) {
      $src = $src_matches[1];
      $src_dir = 'http://' . $this->artistSiteUrl . '/';
    }
    else {
      return $result;
    }


    $fid = db_select('file_managed', 'f')
           ->fields('f', array('fid'))
           ->condition('filename', urldecode(basename($src)))
           ->execute()
           ->fetchField();
    if (!$fid) {
      $arguments = array(
        'source_dir' => $src_dir,
        'destination_dir' => 'public://legacy_images',
        'destination_file' => $src,
        'file_replace' => MigrateFile::FILE_EXISTS_REUSE,
        'preserve_files' => TRUE,
      );
      $source = new MigrateFileUri($arguments);
      $file = $source->processFile($src, $arguments['default_author_id']);
      $fid = $file->fid;

    }

    $attributes = array(
      'class' => 'media-image',
      'typeof' => 'foaf:Image',
      'style' => '',
    );

    // Build the media tag
    $img_info = array(
      'type' => 'media',
      'view_mode' => 'media_large',
      'fid' => $fid,
      'attributes' => array($attributes),
    );
    $result = '[[' . drupal_json_encode($img_info) . ']]';

    return $result;
  }

}
