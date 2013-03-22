<?php
/**
 * @file
 * Support for migrating Ning music items to Drupal nodes.
 */

class NingMusicMigration extends NingNodeMigration {
  protected $combos = array();
  public function __construct($arguments) {
    $arguments['source_feature'] = 'music';

    $this->sourceFields += array(
      'id' => 'Unique page ID',
      'contributorName' => 'Internal username of author',
      'title' => 'Page title',
      'createdDate' => 'Page creation date',
      'updatedDate' => 'Page updated date',
      'approved' => 'Y/N',
      'album' => 'Album title',
      'artist' => 'Name of artist',
      'artworkUrl' => 'Image for the track',
      'audioUrl' => 'URL of the music file',
      'duration' => 'Duration of the track (in ms)',
      'trackTitle' => 'Title of the track',
    );

    parent::__construct($arguments);

    $this->addFieldMapping('status', 'approved', FALSE)
         ->callbacks(array($this, 'handleStatus'));
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

    // Copy the music file and the image if present.
    if (!empty($row->audioUrl)) {
      if (!$this->copyFile($row->audioUrl)) {
        $row->audioUrl = NULL;
      }
    }
    if (!empty($row->artworkUrl)) {
      if (!$this->copyFile($row->artworkUrl)) {
        $row->artworkUrl = NULL;
      }
    }
  }
}

/**
 * Migrate comments on music to Drupal.
 */
class NingMusicCommentMigration extends NingCommentMigration {
  public function __construct($arguments) {
    $arguments['source_feature'] = 'music';
    $arguments['parent_id_field'] = 'id';

    parent::__construct($arguments);
  }
}
