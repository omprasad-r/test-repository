<?php
/**
 * @file
 * Support for migrating Ning photos to Drupal nodes.
 */

class NingPhotoMigration extends NingNodeMigration {
  public function __construct($arguments) {
    $arguments['source_feature'] = 'photos';

    $this->sourceFields += array(
      'id' => 'Unique page ID',
      'contributorName' => 'Internal username of author',
      'title' => 'Page title',
      'description' => 'Page body',
      'createdDate' => 'Page creation date',
      'updatedDate' => 'Page updated date',
      'approved' => 'Y/N',
      'location' => 'Location of photo',
      'photoHeight' => 'Height of photo',
      'photoWidth' => 'Width of photo',
      'photoUrl' => 'Full address of image file',
      'rotation' => '90/180/270',
    );

    parent::__construct($arguments);

    $this->addFieldMapping('status', 'approved', FALSE)
         ->callbacks(array($this, 'handleStatus'));

    $this->addUnmigratedSources(array(
      'location',
      'photoHeight',
      'photoWidth',
      'rotation',
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

    if (!empty($row->photoUrl)) {
      if (!$this->copyFile($row->photoUrl)) {
        $row->photoUrl = NULL;
      }
    }
  }
}

/**
 * Migrate comments on photos to Drupal.
 */
class NingPhotoCommentMigration extends NingCommentMigration {
  public function __construct($arguments) {
    $arguments['source_feature'] = 'photos';
    $arguments['parent_id_field'] = 'id';

    parent::__construct($arguments);
  }
}
