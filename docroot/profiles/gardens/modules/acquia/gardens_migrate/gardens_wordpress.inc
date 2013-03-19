<?php

class GardensWordPressBlog extends WordPressBlog {
  /**
   * The implemented WordPress migrations, in the order they should be run.
   */
  public function migrationClasses() {
    return array(
      'WordPressAuthor' => 'WordPressAuthor',
      'WordPressCategory' => 'WordPressCategory',
      'WordPressTag' => 'WordPressTag',
      'WordPressBlogEntry' => 'GardensWordPressBlogEntry',
      'WordPressPage' => 'GardensWordPressPage',
      'WordPressAttachment' => 'WordPressAttachment',
      'WordPressComment' => 'WordPressComment',
    );
  }
}
