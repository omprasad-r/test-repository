<?php
/**
 * @file
 *
 */

class NingWarnerDiscussionMigration extends NingDiscussionMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_is_pro')
         ->defaultValue(0);

    if (variable_get('migrate_ning_picture', '') == 'NewBoyzAlbumCover.jpg') {
      $this->addFieldMapping('taxonomy_forums', NULL, FALSE)
           ->defaultValue('General discussion');
      $this->addFieldMapping('taxonomy_forums:source_type', NULL, FALSE);
      $this->addFieldMapping('taxonomy_forums:create_term', NULL, FALSE)
           ->defaultValue(1);
    }
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Gucci: Skip discussions not in the two featured groups
    if (variable_get('migrate_ning_picture', '') == 'Guccinewalbumcover.jpg') {
      if ($row->groupId != '3685062:Group:47276' && $row->groupId != '3685062:Group:169319') {
        return FALSE;
      }
    }
  }
}
