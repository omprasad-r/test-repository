<?php
/**
 * @file
 * Support for migrating Ning discussions to Drupal forum nodes.
 */

class NingDiscussionMigration extends NingNodeMigration {
  protected $groupMigration;

  public function __construct($arguments) {
    $arguments['source_feature'] = 'discussions';

    $this->sourceFields += array(
      'id' => 'Unique page ID',
      'contributorName' => 'Internal username of author',
      'title' => 'Page title',
      'description' => 'Page body',
      'createdDate' => 'Page creation date',
      'updatedDate' => 'Page updated date',
      'approved' => 'Y/N',
      'groupId' => 'Containing group (forum)',
      'fileAttachments' => 'URL of attachedfiles',
      'isPrivate' => '1/empty',
    );

    parent::__construct($arguments);

    $this->softDependencies[] = $arguments['group_migration'];
    $this->addFieldMapping('taxonomy_forums', 'groupId')
         ->sourceMigration($arguments['group_migration']);
    $this->addFieldMapping('taxonomy_forums:source_type')
         ->defaultValue('tid');
    $this->addUnmigratedDestinations(array(
        'taxonomy_forums:create_term', 'taxonomy_forums:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'approved',
      'fileAttachments',
      'isPrivate',
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
    // Empirical evidence suggested published forum content has an empty
    // isPrivate, approved of Y
    if (empty($row->isPrivate) && $row->approved == 'Y') {
      $row->publishStatus = NODE_PUBLISHED;
    }
    else {
      $row->publishStatus = NODE_NOT_PUBLISHED;
    }
    if (empty($row->groupId)) {
      return FALSE;
    }
  }
}

/**
 * Migrate comments on discussions to Drupal.
 */
class NingDiscussionCommentMigration extends NingCommentMigration {
  public function __construct($arguments) {
    $arguments['source_feature'] = 'discussions';
    $arguments['parent_id_field'] = 'id';

    parent::__construct($arguments);

  }
}
