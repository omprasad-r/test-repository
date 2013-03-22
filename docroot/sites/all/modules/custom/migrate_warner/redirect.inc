<?php

/**
 * Base class implementing shared behavior among all the content (XML-based)
 * migrations.
 */
class RedirectMigration extends XMLMigration {
  public function __construct() {
    // Always call the parent constructor first for basic setup
    parent::__construct(MigrateGroup::getInstance('warner'));
    $this->softDependencies = array('CommentNonLive');

    $this->team = array(
      new MigrateTeamMember('Mike Ryan', 'mike.ryan@acquia.com', 'Acquia'),
      new MigrateTeamMember('Pete Jelliffe', 'Pete.Jelliffe@wmg.com', 'Warner'),
    );

    $this->systemOfRecord = Migration::DESTINATION;

    // Fields common to all content types
    $fields = array(
      'EosId' => 'Eos-wide ID',
      'live_site_url' => 'Old URL',
    );

    // The source ID here is the one retrieved from each data item in the XML file, and
    // used to identify specific items
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'EosId' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $stream = new MigrateWarnerStreamWrapper();
    $localpath = $stream->getRealPath('warner://') . '/' . variable_get('warner_private_directory', '');
    $filespec = "$localpath/site_content_live*.xml";
    foreach (glob($filespec) as $this->items_url) {
      break;
    }
    $item_xpath = "/file/article";  // Select all article elements
    $item_ID_xpath = "@EosId";
    $this->source = new MigrateSourceXML($this->items_url, $item_xpath, $item_ID_xpath,
      $fields, array('cache_counts' => TRUE));
    // Arbitrary node type - will actually apply to all node types
    $this->destination = new MigrateDestinationNode('blog');

    $this->addFieldMapping('nid', 'EosId')
         ->sourceMigration(array('Photo', 'PhotoNonLive', 'Video', 'VideoNonLive',
            'Audio', 'AudioNonLive', 'Blog', 'BlogNonLive', 'Discussion',
            'DiscussionNonLive', 'Event', 'EventNonLive', 'Album', 'AlbumNonLive'));
    $this->addFieldMapping('migrate_redirects', 'live_site_url')
         ->xpath('live_site_url')
         ->callbacks(array($this, 'stripDomain'));
  }

  public function stripDomain($url) {
    $url = trim($url, '"');
    preg_match('|^https?://[^/]+(.*)|', $url, $matches);
    return $matches[1];
  }
}
