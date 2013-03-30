<?php
/**
 * @file
 * Migration class for importing blog posts from ONEsite to Drupal Gardens.
 */
 

class MigrateOnesiteExampleBlogPost extends OnesiteBlogPostMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_blog_category')
         ->defaultValue('My Blog Category');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);
    $this->addUnmigratedDestinations(array(
      'field_blog_category:source_type', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
    ));
  }
}
