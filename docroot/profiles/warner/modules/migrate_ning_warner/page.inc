<?php
/**
 * @file
 *
 */

class NingWarnerPageMigration extends NingPageMigration {
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
}
