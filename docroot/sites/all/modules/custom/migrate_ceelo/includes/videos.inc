<?php

class CeeloVideoMigration extends CeeloMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->destination = new MigrateDestinationNode('video');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'PhotoID' =>
        array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'Photo ID from TSV',
        ),
    ), MigrateDestinationNode::getKeySchema());

    $this->source = new MigrateSourceCSV('data/videos.txt', array(
      'VideoID',
      'Name',
      'Code',
      'Keywords',
      'Description',
      'Active',
      'Embed',
      'URL',
    ), array(
      'delimiter'   => "\t",
      'header_rows' => 1,
    ));

    $this->addFieldMapping('title', 'Name');
    $this->addFieldMapping('field_video_asset', 'Embed');

    // TODO: Figure out the url grabbiness (has to parse....).
  }

  public function prepareRow($row) {
    $row->embed = $this->getEmbedUrl($row->embed);
  }

  protected function getEmbedUrl($value) {
    if (strpos($value, 'object') === FALSE) {
      return $value;
    }

    $embed_regex = '#<param name="movie" value=["\'](.*?)["\']\s*>#';

    preg_match($embed_regex, $value, $match);

    if (!empty($matches[1])) {
      return $matches[1];
    }

    return $value;
  }
}