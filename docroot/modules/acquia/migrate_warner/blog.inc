<?php

class BlogMigration extends WarnerMigration {
  public function __construct($group, $live = TRUE) {
    // Fields specific to the blog type
    $specific_fields = array(
      'category' => '',
    );
    parent::__construct($specific_fields, 'blog', 'blog', $live);
    $this->description = t('Import blog entities from XML into Drupal');

    $this->addFieldMapping('field_blog_category', 'category')
         ->xpath('category')
         ->arguments(array('create_term' => TRUE))
         ->description('Currently a vocabulary - would a simple select field make more sense?')
         ->issueGroup('Warner');

    // Get the right format identifier for Safe HTML on this system
    $full_html = db_select('filter_format', 'f')
                       ->fields('f', array('format'))
                       ->condition('name', 'Full HTML')
                       ->execute()
                       ->fetchField();
    $this->removeFieldMapping('body');
    $this->addFieldMapping('body', 'description')
         ->xpath('description')
         ->arguments(array('format' => $full_html));
    $this->addUnmigratedDestinations(array('field_tags'));
  }
}

class BlogNonLiveMigration extends BlogMigration {
  public function __construct($group) {
    parent::__construct($group, FALSE);
    $this->description = t('Import non-live blog entities from XML into Drupal');
  }
}
