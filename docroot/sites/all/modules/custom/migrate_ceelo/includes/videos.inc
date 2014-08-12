<?php

class CeeloVideoMigration extends CeeloMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->destination = new MigrateDestinationNode('video');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'VideoID' =>
        array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'Photo ID from TSV',
        ),
    ), MigrateDestinationNode::getKeySchema());

    $this->source = new MigrateSourceCSV(dirname(__FILE__) . '/../data/videos.txt', array(
      array('VideoID', 'PK'),
      array('Name', 'Field Name'),
      array('Code', 'Video Code'),
      array('Keywords', 'Tags'),
      array('Description', 'Description'),
      array('Active', 'Status'),
      array('Embed', 'Embed object'),
      array('URL', 'Canonical URL'),
    ), array(
      'delimiter'   => "\t",
      'header_rows' => 1,
    ));

    $this->addFieldMapping('title', 'Name');
    $this->addFieldMapping('field_video_asset', 'Embed');

    $this->addFieldMapping('migrate_redirects', 'URL');

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

    preg_match($embed_regex, $value, $matches);

    if (!empty($matches[1])) {
      return $matches[1];
    }

    $iframe_regex = '#<iframe .*? src=["\'](.*?)["\']\s*>#';

    preg_match($iframe_regex, $value, $matches);

    if (!empty($matches[1])) {
      return $matches[1];
    }

    return $value;
  }
}