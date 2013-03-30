<?php
/**
 * @file
 *
 */

class NingWarnerGroupMigration extends NingGroupForumMigration {
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Gucci: Skip all but the two featured groups. Remove this to import the rest of
    // the groups.
    if (variable_get('migrate_ning_picture', '') == 'Guccinewalbumcover.jpg') {
      if ($row->title != 'True Gucci Fans' && $row->title != 'Mr. Zone 6 Mixtape') {
        return FALSE;
      }
    }
  }
}
