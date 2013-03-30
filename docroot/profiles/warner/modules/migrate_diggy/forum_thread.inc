<?php
/**
 * @file
 * Migration class for importing forum threads from ONEsite to Drupal Gardens.
 */

class MigrateDiggyForumThread extends OnesiteForumThreadMigration {
  public function __construct($arguments) {
    $this->sourceFields['title_url'] = 'Legacy URL for redirects';
    parent::__construct($arguments);

    $this->addFieldMapping('migrate_redirects', 'title_url', FALSE);

    $this->addUnmigratedDestinations(array('field_is_pro'));
  }


  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $title_url = trim(preg_replace('|[^a-z0-9- ]+|i', '', $row->thread_title));
    $title_url = strtolower(str_replace(' ', '-', $title_url));
    $row->title_url = substr('go/thread/view/' . $row->forum_category_id . '/'
      . $row->thread_id . '/' . $title_url, 0, 255);
  }
}
