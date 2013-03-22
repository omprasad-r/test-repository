<?php

class DiscussionMigration extends WarnerMigration {
  public function __construct($group, $live = TRUE) {
    // Fields specific to the discussion type
    $specific_fields = array(
      'category' => '',
    );
    parent::__construct($specific_fields, 'discussion', 'forum', $live);
    $this->description = t('Import discussion entities from XML into Drupal');

    $this->addFieldMapping("taxonomy_forums", 'category')
         ->xpath('category')
         ->arguments(array('create_term' => TRUE));
  }
}

class DiscussionNonLiveMigration extends DiscussionMigration {
  public function __construct($group) {
    parent::__construct($group, FALSE);
    $this->description = t('Import non-live discussion entities from XML into Drupal');
  }
}