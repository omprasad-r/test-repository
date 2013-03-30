<?php

/**
 * Base class implementing shared behavior among all the content (XML-based)
 * migrations.
 */
abstract class WarnerMigration extends XMLMigration {
  protected $nodeType;

  public function __construct($specific_fields, $xml_type, $node_type, $live) {
    // Always call the parent constructor first for basic setup
    parent::__construct(MigrateGroup::getInstance('warner'));
    $this->softDependencies = array('Member');

    $this->team = array(
      new MigrateTeamMember('Mike Ryan', 'mike.ryan@acquia.com', 'Acquia'),
      new MigrateTeamMember('Pete Jelliffe', 'Pete.Jelliffe@wmg.com', 'Warner'),
    );

    $this->nodeType = $node_type;

    // Fields common to all content types
    $common_fields = array(
      'EosId' => 'Eos-wide ID',
      'isPro' => '',
      'addedDate' => '',
      'profile' => 'Email of user creating the content',
      'title' => 'Title of the content',
      'description' => 'Description of the content',
      'live_site_url' => 'Old URL',
    );
    $fields = array_merge($common_fields, $specific_fields);

    if ($live) {
      $source_id = 'EosId';
    }
    else {
      $source_id = 'mediaId';
    }
    // The source ID here is the one retrieved from each data item in the XML file, and
    // used to identify specific items
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        $source_id => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $stream = new MigrateWarnerStreamWrapper();
    $localpath = $stream->getRealPath('warner://') . '/' . variable_get('warner_private_directory', '');
    if ($live) {
      $filespec = "$localpath/site_content_live*.xml";
    }
    else {
      $filespec = "$localpath/site_content_non_live*.xml";
    }
    foreach (glob($filespec) as $this->items_url) {
      break;
    }
    $item_xpath = "/file/article[@type='$xml_type']";  // Select elements of the requested type
    $item_ID_xpath = "@$source_id";
    $this->source = new MigrateSourceXML($this->items_url, $item_xpath, $item_ID_xpath,
      $fields, array('cache_counts' => TRUE));
    $this->destination = new MigrateDestinationNode($node_type);

    // Get the right format identifier for Safe HTML on this system
    $this->safe_html = db_select('filter_format', 'f')
                       ->fields('f', array('format'))
                       ->condition('name', 'Filtered HTML')
                       ->execute()
                       ->fetchField();
    // Also plain text
    $this->plain_text = db_select('filter_format', 'f')
                       ->fields('f', array('format'))
                       ->condition('name', 'Plain text')
                       ->execute()
                       ->fetchField();

    // Common mappings
    $this->addFieldMapping('created', 'addedDate')
         ->xpath('addedDate');
    $this->addFieldMapping('changed', 'addedDate')
         ->xpath('addedDate');
    $this->addFieldMapping('uid', 'profile')
         ->xpath('profile')
         ->sourceMigration('Member')
         ->defaultValue(1);
    $this->addFieldMapping('field_is_pro', 'isPro')
         ->xpath('@isPro')
         ->callbacks(array($this, 'handleBoolean'));

    $this->addFieldMapping('title', 'title')
         ->xpath('title');
    $this->addFieldMapping('body', 'description')
         ->xpath('description')
         ->arguments(array('format' => $this->safe_html));
    $this->addFieldMapping('status')
         ->defaultValue($live);
    $this->addFieldMapping('migrate_redirects')
         ->description('Do it in separate migration');

    $this->addUnmigratedDestinations(array('is_new', 'revision_uid',
      'promote', 'sticky', 'revision', 'language', 'path', 'pathauto', 'comment',
      'log', 'tnid'));
  }

  public function prepareRow($row) {
    // Bogus if we don't have EosId
    if (empty($row->EosId)) {
      // Give a real integer value so writing map table doesn't fail
      $row->EosId = 0;
      $row->mediaId = 0;
      return FALSE;
    }
  }

  protected function handleBoolean($value) {
    if (strtolower($value) == 'true') {
      return 1;
    }
    else {
      return 0;
    }
  }

  public function handleRating($node, $row) {
    foreach ($row->xml->rating as $rating) {
      $rating_value = $rating->rating_value;
      $profile = reset($rating->xpath('@profile'));
      $uid = db_select('users', 'u')
             ->fields('u', array('uid'))
             ->condition('mail', $profile)
             ->execute()
             ->fetchField();
      if ($uid && function_exists('_fivestar_cast_vote')) {
        _fivestar_cast_vote('node', $node->nid, 20 * $rating_value, 'rating', $uid, FALSE, TRUE);
      }
    }
  }

  public function stripDomain($url) {
    $url = trim($url, '"');
    preg_match('|^https?://[^/]+(.*)|', $url, $matches);
    return $matches[1];
  }
}

abstract class WarnerMediaMigration extends WarnerMigration {
  public function __construct($specific_fields, $xml_type, $node_type, $live) {
    $media_fields = array(
      'category' => '',
      'media_group_name' => '',
      'media_folder_name' => '',
      'asset' => '',
      'secondary_title' => '',
      'copyright_name' => '',
      'copyright_url' => '',
    );
    $fields = array_merge($specific_fields, $media_fields);
    parent::__construct($fields, $xml_type, $node_type, $live);

    $this->addFieldMapping("field_{$node_type}_category", 'category')
         ->xpath('category')
         ->arguments(array('create_term' => TRUE));
    if ($node_type == 'photo' || $node_type == 'video') {
      $this->addFieldMapping("field_fan_{$node_type}_category", 'category')
           ->xpath('category')
           ->arguments(array('create_term' => TRUE));
    }
    $this->addFieldMapping('field_secondary_title', 'secondary_title')
         ->xpath('secondary_title');
    $this->addFieldMapping('field_copyright_name', 'copyright_name')
         ->xpath('copyright_name');
    $this->addFieldMapping('field_copyright_url', 'copyright_url')
         ->xpath('copyright_url');
    $this->addFieldMapping(NULL, 'media_group_name')
         ->issueGroup('DNM');
    $this->addFieldMapping(NULL, 'media_folder_name')
         ->issueGroup('DNM');
  }

  /**
   * Handle the odd duplicated value.
   *
   * @param $value
   * @return string
   */
  protected function applyUrlDecode($value) {
    if (is_array($value)) {
      $value = reset($value);
    }
    $value = urldecode($value);
    return $value;
  }

  /**
   * Override applyMappings to pick-and-choose what kind of category to import.
   * @return void
   */
  protected function applyMappings() {
    parent::applyMappings();
    // For photo and video nodes, we have both pro and fan categories.
    // Both are mapped up fron - here we delete the one we don't want
    // on this particular node.
    if ($this->nodeType == 'photo' || $this->nodeType == 'video') {
      $pro_category = "field_{$this->nodeType}_category";
      $fan_category = "field_fan_{$this->nodeType}_category";
      $is_combined = variable_get('warner_combined_taxonomy', FALSE);
      if ($this->destinationValues->field_is_pro || $is_combined) {
        unset($this->destinationValues->$fan_category);
      }
      else {
        unset($this->destinationValues->$pro_category);
      }
    }
  }
}
