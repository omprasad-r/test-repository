<?php
/**
 * @file
 * Migration class for importing blog posts from ONEsite to Drupal Gardens.
 */

class MigrateDiggyBlogPost extends OnesiteBlogPostMigration {
  public function __construct($arguments) {
    $this->sourceFields['tags'] = 'Tags';
    parent::__construct($arguments);

    $this->addFieldMapping('field_blog_category')
         ->defaultValue('News');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('pathauto', NULL, FALSE)
         ->defaultValue(1);
    $this->addFieldMapping('migrate_redirects', 'title_url', FALSE);
    $this->addFieldMapping('field_tags', 'tags');
    $this->addFieldMapping('field_tags:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:ignore_case',
      'field_is_pro',
        'field_tags:source_type', 'field_tags:ignore_case',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $username = Database::getConnection('default', $this->sourceConnection)
                 ->select('user', 'u')
                 ->fields('u', array('subdir'))
                 ->condition('user_id', $row->author_id)
                 ->execute()
                 ->fetchField();
    $year = date('Y', $row->post_date);
    $month = date('m', $row->post_date);
    $day = date('d', $row->post_date);
    $row->title_url = "$username/blog/$year/$month/$day/" . $row->title_url;
    $row->tags = array();
    $result = Database::getConnection('default', $this->sourceConnection)
              ->select('tags', 't')
              ->fields('t', array('tag'))
              ->condition('type', 'blog_post')
              ->condition('xref_id', $row->post_id)
              ->execute();
    foreach ($result as $tag_row) {
      $row->tags[] = $tag_row->tag;
    }
  }
}
