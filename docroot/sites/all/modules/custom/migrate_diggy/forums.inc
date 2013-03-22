<?php
/**
 * @file
 * Migration class for importing forums from ONEsite to Drupal Gardens.
 */

class MigrateDiggyForums extends OnesiteForumsMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addUnmigratedDestinations(array('tac_redirect_403'));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    if ($row->forum_title == 'Network Content Discussions') {
      return FALSE;
    }

    if ($row->forum_title == 'Basic Discussions' ||
        $row->forum_title == 'General Discussion') {
      $row->forum_title = 'Discussions';
    }
  }
}
