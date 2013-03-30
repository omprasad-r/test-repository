<?php
/**
 * @file
 *
 */

class NingWarnerBlogMigration extends NingBlogMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_is_pro')
         ->defaultValue(1);
    $this->addFieldMapping('field_blog_category')
         ->defaultValue('Official');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);
    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:ignore_case',
      'field_featured',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    if (!migrate_ning_warner_is_admin($row->contributorName)) {
      return FALSE;
    }
  }
}

class NingWarnerBlogFanMigration extends NingBlogMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_is_pro')
         ->defaultValue(1);
    $this->addFieldMapping('taxonomy_forums')
         ->defaultValue('General');
    $this->addFieldMapping('taxonomy_forums:create_term')
         ->defaultValue(TRUE);
    $this->addUnmigratedDestinations(array(
        'taxonomy_forums:source_type', 'taxonomy_forums:ignore_case',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    if (migrate_ning_warner_is_admin($row->contributorName)) {
      return FALSE;
    }
    if (empty($row->title)) {
      $row->title = ' ';
    }
  }
}

/**
 * Migrate comments on blogs to Drupal.
 */
class NingWarnerBlogCommentMigration extends NingBlogCommentMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'BlogFan';

    $this->addFieldMapping('nid', 'parentId', FALSE)
         ->sourceMigration(array('Blog', 'BlogFan'));
  }
}
