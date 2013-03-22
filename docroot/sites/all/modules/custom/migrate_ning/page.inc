<?php
/**
 * @file
 * Support for migrating Ning pages to Drupal nodes.
 */

class NingPageMigration extends NingNodeMigration {
  public function __construct($arguments) {
    $arguments['source_feature'] = 'pages';

    $this->sourceFields += array(
      'id' => 'Unique page ID',
      'contributorName' => 'Internal username of author',
      'title' => 'Page title',
      'description' => 'Page body',
      'createdDate' => 'Page creation date',
      'updatedDate' => 'Page updated date',
    );

    parent::__construct($arguments);

  }
}

/**
 * Migrate comments on pages to Drupal.
 */
class NingPageCommentMigration extends NingCommentMigration {
  public function __construct($arguments) {
    $arguments['source_feature'] = 'pages';
    $arguments['parent_id_field'] = 'id';

    parent::__construct($arguments);
  }
}
