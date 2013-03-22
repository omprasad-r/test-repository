<?php
/**
 * @file
 * Support for migrating Ning blogs to Drupal nodes.
 */

class NingBlogMigration extends NingNodeMigration {
  protected $combos = array();
  public function __construct($arguments) {
    $arguments['source_feature'] = 'blogs';

    $this->sourceFields += array(
      'id' => 'Unique page ID',
      'contributorName' => 'Internal username of author',
      'title' => 'Page title',
      'description' => 'Page body',
      'createdDate' => 'Page creation date',
      'updatedDate' => 'Page updated date',
      'isPrivate' => '1/empty',
      'approved' => 'Y/N',
      'publishStatus' => 'publish/draft/queued',
      'publishTime' => 'Time to publish',
      'publishWhen' => 'later/now',
    );

    parent::__construct($arguments);

    $this->addFieldMapping('status', 'publishStatus', FALSE);

    $this->addUnmigratedSources(array(
      'approved',
      'isPrivate',
      'publishTime',
      'publishWhen',
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

    // Empirical evidence suggested published blog content has an empty
    // isPrivate, approved of Y, and publishStatus of publish or draft
    if (empty($row->isPrivate) && $row->approved == 'Y' &&
        ($row->publishStatus == 'publish' || $row->publishStatus == 'draft')) {
      $row->publishStatus = NODE_PUBLISHED;
    }
    else {
      $row->publishStatus = NODE_NOT_PUBLISHED;
    }
  }
}

/**
 * Migrate comments on blogs to Drupal.
 */
class NingBlogCommentMigration extends NingCommentMigration {
  public function __construct($arguments) {
    $arguments['source_feature'] = 'blogs';
    $arguments['parent_id_field'] = 'id';

    parent::__construct($arguments);
  }
}
