<?php
/**
 * @file
 * Support for migrating Ning videos to Drupal nodes.
 */

class NingVideoMigration extends NingNodeMigration {
  public function __construct($arguments) {
    $arguments['source_feature'] = 'videos';

    $this->sourceFields += array(
      'id' => 'Unique page ID',
      'contributorName' => 'Internal username of author',
      'title' => 'Page title',
      'description' => 'Page body',
      'createdDate' => 'Page creation date',
      'updatedDate' => 'Page updated date',
      'approved' => 'Y/N',
      'videoAttachmentUrl' => 'Full address of video file',
      'embedCode' => 'object tag for embedded video',
    );

    parent::__construct($arguments);

    $this->addFieldMapping('status', 'approved', FALSE)
         ->callbacks(array($this, 'handleStatus'));
  }
}

/**
 * Migrate comments on videos to Drupal.
 */
class NingVideoCommentMigration extends NingCommentMigration {
  public function __construct($arguments) {
    $arguments['source_feature'] = 'videos';
    $arguments['parent_id_field'] = 'id';

    parent::__construct($arguments);

  }
}
