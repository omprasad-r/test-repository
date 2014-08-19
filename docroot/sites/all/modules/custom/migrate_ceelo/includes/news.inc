<?php

class CeeloNewsMigration extends CeeloMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->destination = new MigrateDestinationNode('blog');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'NewsID' =>
        array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'News ID from TSV',
        ),
    ), MigrateDestinationNode::getKeySchema());

    $this->source = new MigrateSourceCSV(dirname(__FILE__) . '/../data/news.txt', array(
      array('NewsID', 'PK'),
      array('NewsTypeID', '?'),
      array('Title', 'Title'),
      array('SubTitle', 'Subtitle - Mostly empty'),
      array('Summary', 'Summary field'),
      array('Article', 'Full article text'),
      array('DateArticle', 'createdish'),
      array('DateRelease', 'What we are using for created'),
      array('DateArchive', 'Date archived?'),
      array('Active', 'Status'),
      array('Tags', 'Tags'),
      array('URL', 'url'),
    ), array(
      'delimiter'   => "\t",
      'header_rows' => 1,
    ));

    $this->addFieldMapping('title', 'Title');
    $this->addFieldMapping('body', 'Article');
    $this->addFieldMapping('body:summary', 'Summary');
    $this->addFieldMapping('body:format')->defaultValue(6);
    $this->addFieldMapping('created', 'DateRelease');
    $this->addFieldMapping('status', 'Active');
    $this->addFieldMapping('field_tags', 'Tags')->separator(',');
    $this->addFieldMapping('field_tags:create_term')->defaultValue(TRUE);

    $this->addFieldMapping('migrate_redirects', 'URL');
  }

  public function prepareRow($row) {
    $row->Article = $this->handleInvalidImages($row->Article);
  }

  /**
   * Tries to find any images that don't have an absolute URL.
   *
   * It will grab them from the intertubes and save it down to a local file
   * and drop it in the file_managed table.
   *
   * @todo Figure out if I need to check file_managed for this.
   *
   * @param string $text
   *   Incoming Source text
   *
   * @return string
   *   Text!
   */
  protected function handleInvalidImages($text) {
    $image_regex = '#<img.*?src=["\'](.*?)["\'][^\>]*?>#';

    preg_match_all($image_regex, $text, $matches);

    if (empty($matches)) {
      return $text;
    }

    foreach ($matches[1] as $idx => $match) {
      if (strpos($match, 'http') === 0) {
        continue;
      }

      // youtube in an image source?
      if (strpos($match, 'youtube.com') !== FALSE) {
        continue;
      }

      $image_url = 'http://securecdn.warnerartists.com/ugc/ceelo/' . $match;

      $image_url = str_replace(' ', '%20', $image_url);

      $file = system_retrieve_file($image_url, NULL, TRUE, FILE_EXISTS_REPLACE);

      if (empty($file)) {
        $this->queueMessage('Unable to save file ' . $match, MigrationBase::MESSAGE_WARNING);
        continue;
      }

      $url = file_create_url($file->uri);

      $text = str_replace($match, $url, $text);
    }

    return $text;
  }
}