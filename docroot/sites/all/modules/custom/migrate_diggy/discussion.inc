<?php
/**
 * @file
 * Migration class for importing discussions from ONEsite to Drupal Gardens.
 */

class MigrateDiggyDiscussion extends OnesiteDiscussionMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addUnmigratedDestinations(array('field_is_pro'));
  }
}
