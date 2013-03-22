<?php
/**
 * @file
 * Support for migrating Ning groups to Drupal terms (presumably in the forum
 * vocabulary).
 */

class NingGroupForumMigration extends NingMigration {

  public function __construct($arguments) {
    $arguments['source_feature'] = 'groups';

    parent::__construct($arguments);

    $this->sourceFields += array(
      'id' => 'Unique page ID',
      'contributorName' => 'Internal username of author',
      'title' => 'Page title',
      'description' => 'Page body',
      'createdDate' => 'Page creation date',
      'updatedDate' => 'Page updated date',
      'approved' => 'Y/N',
      'allowInvitationRequests' => 'Y/N',
      'allowInvitations' => 'Y/N',
      'allowMemberMessaging' => 'Y/N',
      'groupPrivacy' => 'public/?',
      'iconUrl' => 'URL of image file',
      'memberCount' => 'Number of members',
      'url' => 'Scrubbed version of title',
      'members' => 'List of members',
    );

    $this->source = new MigrateSourceJSON($this->sourceFile, 'id', $this->sourceFields,
          array('cache_counts' => TRUE));

    // Default to the standard profile's forums vocabulary.
    if (!isset($arguments['bundle'])) {
      $arguments['bundle'] = 'forums';
    }
    $this->destination = new MigrateDestinationTerm($arguments['bundle']);

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'id' =>
          array('type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'description' => 'Unique group ID',
               ),
        ),
        MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('name', 'title');
    $this->addFieldMapping('description', 'description');

    $this->addUnmigratedDestinations(array(
      'format',
      'parent',
      'parent_name',
      'weight',
    ));

    if (module_exists('path')) {
      $this->addUnmigratedDestinations(array('path'));
      if (module_exists('pathauto')) {
        $this->addUnmigratedDestinations(array('pathauto'));
      }
    }
    if (module_exists('redirect')) {
      $this->addUnmigratedDestinations(array('migrate_redirects'));
    }

    $this->addUnmigratedSources(array(
      'allowInvitationRequests',
      'allowInvitations',
      'allowMemberMessaging',
      'approved',
      'contributorName',
      'createdDate',
      'groupPrivacy',
      'iconUrl',
      'memberCount',
      'members',
      'updatedDate',
      'url',
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

    // Keeping these arrays in the row can get in the way.
    unset($row->members);
    unset($row->comments);
  }
}
