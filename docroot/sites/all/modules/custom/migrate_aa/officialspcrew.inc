<?php
/**
 * @file
 * Migration implementations specific to officialspcrew.
 */

/**
 * Registration of this site's migrations.
 */
class officialspcrewRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    $this->defaultUid = 232296;    // SPCrew Admin
    parent::__construct($site_name, 7, $this->defaultUid);

    $this->commonArguments['format_mappings'] =  array(
      'filtered_html' => '1',
      'plain_text' => '11',
      'full_html' => '6',
      'unfiltered' => '6',
      'php_code' => 'script',
    );
    $this->roleArguments['role_mappings'] = array(
      'band member' => 'professional member',
      'Fan Club' => 'fan club member',
      'Product Manager' => 'administrator',
    );
    $this->sourceDirectory = 'http://www.officialspcrew.com/sites/officialspcrew.com/files/';
  }

  /**
   * Register our migrations.
   */
  protected function register() {
    $this->registerRole();
    $this->registerUser();
    $this->registerFile();
    $this->registerNode('article', 'blog');
    $this->registerComment('article', 'blog');
    $this->registerNode('blog', 'forum');
    $this->registerComment('blog', 'forum');
    $this->registerNode('contests', 'blog');
    $this->registerComment('contests', 'blog');
    $this->registerNode('event', 'blog');
    $this->registerComment('event', 'blog');
    $this->registerNode('exclusive_download', 'audio');
    $this->registercomment('exclusive_download', 'audio');
    $this->registerNode('forum', 'forum');
    $this->registerComment('forum', 'forum');
    $this->registerNode('image', 'photo');
    // Only 64 comments total assigned to image nodes, and we have no
    // basis on which to attach them to specific photo nodes.
//    $this->registerComment('image', 'photo');
    $this->registerNode('image_single', 'photo');
    $this->registerComment('image_single', 'photo');
    $this->registerNode('tour_date', 'event');
    $this->registerComment('tour_date', 'event');
    $this->registerMigration('webform', $this->commonArguments);
  }
}

class officialspcrewRoleMigration extends AARole7Migration {}

class officialspcrewUserMigration extends AAUser7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    // Known mappings
    $this->addFieldMapping('field_member_name', 'field_name');
    $this->addFieldMapping('field_nickname', 'field_name');
    $this->addFieldMapping('field_about_me', 'field_bio');
    $this->addFieldMapping('field_favorite_song', 'field_fav_song');

    $this->addUnmigratedDestinations(array(
        'field_about_me:language',
      'field_birthdate',
      'field_city', 'field_city:language',
      'field_country',
      'field_eos_id',
      'field_favorite_bands', 'field_favorite_bands:language',
        'field_favorite_song:language',
      'field_gender',
      'field_hobbies', 'field_hobbies:language',
        'field_member_name:language',
        'field_nickname:language',
      'field_postal_code', 'field_postal_code:language',
      'field_profile_14', 'field_profile_14:language',
      'field_profile_updated',
      'field_provider', 'field_provider:language',
      'field_site_id', 'field_site_id:language',
      'field_state', 'field_state:language',
      'field_twitter_name', 'field_twitter_name:language',
      'field_type',
      'field_usage_type',
    ));

    $this->addUnmigratedSources(array(
        'field_bio:format', 'field_bio:summary',
      'field_user_location',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    if (empty($row->field_name)) {
      $row->field_name = $row->name;
    }
  }

  protected function query() {
    // Skip SPCrew Admin, it already exists
    $query = parent::query();
    $query->condition('name', 'SPCrew Admin', '<>');
    return $query;
  }
}

class officialspcrewFileMigration extends AAFile7Migration {}

class officialspcrewarticleMigration extends AANode7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->defaultUid = 232296;    // SPCrew Admin
    $this->addFieldMapping('field_blog_category')
         ->defaultValue('General');

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:create_term', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'field_image', 'field_image:alt', 'field_image:title', 'field_image:width', 'field_image:height',
      'field_tags', // No source data
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Rewrite any img tags referencing local files to point to a media
    // tag for the migrated file
    $row->body[0] = preg_replace_callback('|<img +(.*?)>|i',
      array($this, 'replaceImgs'), $row->body[0]);

    // If the former image field was populated, prepend the image
    if (isset($row->field_image)) {
      $new_fid = db_select('migrate_map_file', 'map')
                 ->fields('map', array('destid1'))
                 ->condition('sourceid1', $row->field_image)
                 ->execute()
                 ->fetchField();
      if ($new_fid) {
        $img_info = array(
          'type' => 'media',
          'view_mode' => 'media_large',
          'fid' => $new_fid,
          'attributes' => array(),
        );
        $media_tag = '[[' . drupal_json_encode($img_info) . ']]' . '<br />';
        $row->body = $media_tag . $row->body[0];
      }
    }
  }

  /**
   * If we have an image reference, replace it with media tags if we can.
   *
   * @param array $matches
   */
  protected function replaceImgs(array $matches) {
    // Default to the original <img> tag.
    $result = $matches[0];

    // The src parameter is required
    if (preg_match('|src=[\'"]/sites/officialspcrew.com/files/(.*?)[\'"]|i', $result, $src_matches)) {
      $src = $src_matches[1];
    }
    else {
      return $result;
    }

    // Get the fid, if any. If none, let the img tag stand
    $fid = db_select('file_managed', 'f')
           ->fields('f', array('fid'))
           ->condition('filename', basename($src))
           ->execute()
           ->fetchField();
    if (!$fid) {
      return $result;
    }

    $attributes = array(
      'class' => 'media-image',
      'typeof' => 'foaf:Image',
      'style' => '',
    );

    // Build the media tag
    $img_info = array(
      'type' => 'media',
      'view_mode' => 'media_large',
      'fid' => $fid,
      'attributes' => array($attributes),
    );
    $result = '[[' . drupal_json_encode($img_info) . ']]';

    return $result;
  }
}

class officialspcrewblogMigration extends AANode7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->defaultUid = 232296;    // SPCrew Admin
    $this->addFieldMapping('taxonomy_forums')
         ->defaultValue('Blog Posts');
    $this->addFieldMapping('taxonomy_forums:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
        'taxonomy_forums:source_type', 'taxonomy_forums:ignore_case',
    ));
  }
}

class officialspcrewcontestsMigration extends AANode7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->defaultUid = 232296;    // SPCrew Admin
    $this->addFieldMapping('field_blog_category')
         ->defaultValue('Contests');

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:create_term', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
    ));
  }
}

class officialspcreweventMigration extends AANode7Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->defaultUid = 232296;    // SPCrew Admin
    $this->addFieldMapping('field_blog_category')
         ->defaultValue('Events');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'field_event_date',
      'field_event_location',
      'field_event_venue',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    if (is_array($row->field_event_date)) {
      // Date is yyyy-mm-dd hh:mm:ss, truncate to just date portion
      $row->field_event_date = substr(reset($row->field_event_date), 0, 10);
    }
    if (is_array($row->field_event_venue)) {
      $row->field_event_venue = reset($row->field_event_venue);
    }
    if (is_array($row->field_event_location)) {
      $row->field_event_location = reset($row->field_event_location);
    }
    if (is_array($row->body)) {
      $row->body = reset($row->body);
    }
    $row->body = "<strong>Event date:</strong> {$row->field_event_date}<br />\n"
      . "<strong>Venue:</strong> {$row->field_event_venue}<br />\n"
      . "<strong>Location:</strong> {$row->field_event_location}<br />\n"
      . $row->body;
  }
}

class officialspcrewexclusive_downloadMigration extends AANode7Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->defaultUid = 232296;    // SPCrew Admin
    $this->addFieldMapping('field_audio_asset', 'field_priv_file')
         ->sourceMigration('File');
    $this->addFieldMapping('field_audio_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_audio_asset:title', 'field_priv_file:description');
    $this->addFieldMapping('field_audio_category')
         ->defaultValue('Exclusive Downloads');
    $this->addFieldMapping('field_audio_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
        'field_audio_asset:language', 'field_audio_asset:data',
        'field_audio_category:source_type', 'field_audio_category:ignore_case',
      'field_audio_duration', 'field_audio_duration:language',
      'field_audio_embed', 'field_audio_embed:format', 'field_audio_embed:language',
      'field_audio_image', 'field_audio_image:file_class', 'field_audio_image:language',
        'field_audio_image:destination_dir', 'field_audio_image:destination_file',
        'field_audio_image:file_replace', 'field_audio_image:preserve_files',
        'field_audio_image:source_dir', 'field_audio_image:alt', 'field_audio_image:title',
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
      'field_download_url', 'field_download_url:language',
      'field_legacy_url', 'field_legacy_url:language',
      'field_lyrics', 'field_lyrics:format', 'field_lyrics:language',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
      'field_video_url', 'field_video_url:language',
    ));

    $this->addUnmigratedSources(array(
        'field_priv_file:display',
    ));
  }
}

class officialspcrewforumMigration extends AANode7Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->defaultUid = 232296;    // SPCrew Admin
    $this->addFieldMapping('taxonomy_forums')
         ->defaultValue('News');

    $this->addUnmigratedDestinations(array(
        'taxonomy_forums:source_type', 'taxonomy_forums:create_term',
        'taxonomy_forums:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'taxonomy_forums',
    ));
  }
}

class officialspcrewtour_dateMigration extends AANode7Migration {
  public function __construct($arguments) {
    $this->sourceFields['state'] = 'State extracted from location';
    $this->sourceFields['country'] = 'Country extracted from location';
    parent::__construct($arguments);
    $this->defaultUid = 232296;    // SPCrew Admin
    $this->addFieldMapping('field_event_buylink', 'field_buy_link')
         ->defaultValue('');
    $this->addFieldMapping('field_event_buylink:format')
         ->defaultValue('1');
    $this->addFieldMapping('field_event_maplink')
         ->defaultValue('');
    $this->addFieldMapping('field_event_venue', 'field_venue');
    $this->addFieldMapping('field_event_city', 'field_location');
    $this->addFieldMapping('field_event_state', 'state');
    $this->addFieldMapping('field_event_country', 'country');
    $this->addFieldMapping('field_event_date', 'field_tourdate');
    $this->addFieldMapping('field_event_category')
         ->defaultValue('Tour Date');
    $this->addFieldMapping('field_event_category:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_presale_date', 'field_presale_dat');

    $this->removeFieldMapping('field_is_pro');
    $this->removeFieldMapping('body:format');

    $this->addUnmigratedDestinations(array(
      'field_event_address', 'field_event_address:language',
      'field_event_availability', 'field_event_availability:language',
        'field_event_buylink:language',
        'field_event_category:source_type', 'field_event_category:ignore_case',
        'field_event_city:language',
        'field_event_country:language',
        'field_event_maplink:format', 'field_event_maplink:language',
        'field_event_state:language',
        'field_event_venue:language',
    ));

    $this->addUnmigratedSources(array(
      'body:format',
        'field_buy_link:title', 'field_buy_link:attributes',
      'field_vip_ticket',   // No source data
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $this->parseLocation($row, 'field_location');
    $this->buildBuyLink($row, 'field_buy_link');
  }
}

class officialspcrewimageMigration extends AANode7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->defaultUid = 232296;    // SPCrew Admin
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'field_photo_fid' => array('type' => 'int',
                       'unsigned' => TRUE,
                       'not null' => TRUE,
                       'description' => 'Source file ID',
                       'alias' => 'p',
                      ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('field_photo_asset', 'field_photo_fid')
         ->sourceMigration('File');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_photo_asset:title', 'title');
    $this->addFieldMapping('field_photo_category', 'title');
    $this->addFieldMapping('field_photo_category:create_term')
         ->defaultValue(TRUE);

    $this->removeFieldMapping('body');
    $this->removeFieldMapping('body:summary');
    $this->removeFieldMapping('body:format');

    $this->addUnmigratedDestinations(array(
      'body', 'body:summary', 'body:format',
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:data',
        'field_photo_category:ignore_case', 'field_photo_category:source_type',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
    ));

    $this->addUnmigratedSources(array(
      'field_photo', 'field_photo:title', 'field_photo:alt', 'field_photo:width', 'field_photo:height',
      'nid',
    ));
  }

  protected function query() {
    $query = parent::query();
    $query->innerJoin('field_data_field_photo', 'p', 'n.nid=p.entity_id');
    $query->fields('p', array('field_photo_fid'));
    return $query;
  }
}

class officialspcrewimage_singleMigration extends AANode7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->defaultUid = 232296;    // SPCrew Admin
    $this->addFieldMapping('field_photo_asset', 'field_img')
         ->sourceMigration('File');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_photo_asset:title', 'field_img:title');
    $this->addFieldMapping('field_photo_category')
         ->defaultValue('Official');

    $this->removeFieldMapping('body');
    $this->removeFieldMapping('body:summary');
    $this->removeFieldMapping('body:format');

    $this->addUnmigratedDestinations(array(
      'body', 'body:summary', 'body:format',
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:data',
        'field_photo_category:create_term', 'field_photo_category:ignore_case', 'field_photo_category:source_type',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
    ));

    $this->addUnmigratedSources(array(
        'field_img:alt', 'field_img:width', 'field_img:height',
    ));
  }
}

class officialspcrewwebformMigration extends DrupalMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'User';

    $fields = array(
      'username' => 'username',
      'first_name' => 'first_name',
      'last_name' => 'last_name',
      'email_address' => 'email_address',
      'date_of_event' => 'date_of_event',
      'venue_city_and_state_' => 'venue_city_and_state_',
      'have_you_ever_met_sp_before' => 'have_you_ever_met_sp_before',
      'confirm_you_have_purchased_your_ticket_to_attend_this_date' => 'confirm_you_have_purchased_your_ticket_to_attend_this_date',
      'do_you_require_a_parent_to_escort' => 'do_you_require_a_parent_to_escort',
    );
    $this->source = new MigrateSourceSQL($this->query(), $fields, NULL,
      array('map_joinable' => FALSE));
    $nodes = node_load_multiple(array(), array('type' => 'webform',
      'title' => 'Soundcheck Party | Early Entry'));
    $node = reset($nodes);
    $this->destination = new MigrateDestinationWebformSubmission($node);
    $this->map = new MigrateSQLMap($this->machineName,
       array(
         'sid' => array('type' => 'int',
                        'unsigned' => TRUE,
                        'not null' => TRUE,
                        'description' => 'Webform submission ID',
                        'alias' => 's',
                       ),
       ),
       MigrateDestinationWebformSubmission::getKeySchema()
    );

    $this->addSimpleMappings(array('is_draft', 'submitted', 'remote_addr'));
    $this->addFieldMapping('uid', 'uid')
         ->sourceMigration('User');
    // On the Gardens side we get these unintuitive form keys
    $this->addFieldMapping('data_new_1346161126381', 'username');
    $this->addFieldMapping('data_new_1346161183040', 'first_name');
    $this->addFieldMapping('data_new_1346161207005', 'last_name');
    $this->addFieldMapping('data_new_1346161225929', 'email_address');
    $this->addFieldMapping('data_new_1346162125707', 'date_of_event');
    $this->addFieldMapping('data_new_1346161317986', 'venue_city_and_state_');
    $this->addFieldMapping('data_new_1346161872556', 'do_you_require_a_parent_to_escort');
    $this->addFieldMapping('data_new_1346161575447', 'have_you_ever_met_sp_before');
    $this->addFieldMapping('data_new_1346161821586', 'confirm_you_have_purchased_your_ticket_to_attend_this_date');

    $this->addUnmigratedDestinations(array(
      'sid',
    ));
  }

  protected function query() {
    // We're only interested in one webform, with the known legacy nid 25971
    $query = Database::getConnection('default', 'legacy')
             ->select('webform_submissions', 's')
             ->fields('s', array('sid', 'uid', 'is_draft', 'submitted', 'remote_addr'))
             ->condition('nid', 25971);
    return $query;
  }

  /**
   * Use the explicit machine_name argument as this migration's machine name.
   */
  protected function generateMachineName($class_name = NULL) {
    return $this->arguments['machine_name'];
  }

  public function prepareRow($row) {
    $query = Database::getConnection('default', 'legacy')
             ->select('webform_submitted_data', 'd')
             ->fields('d', array('data'))
             ->condition('sid', $row->sid);
    $query->innerJoin('webform_component', 'c', 'd.nid=c.nid AND d.cid=c.cid');
    $query->fields('c', array('form_key'));
    $result = $query->execute();
    foreach ($result as $data_row) {
      $row->{$data_row->form_key} = $data_row->data;
    }
  }
}

class officialspcrewCommentarticleMigration extends AAComment7Migration {}
class officialspcrewCommentblogMigration extends AAComment7Migration {}
class officialspcrewCommentcontestsMigration extends AAComment7Migration {}
class officialspcrewCommenteventMigration extends AAComment7Migration {}
class officialspcrewCommentexclusive_downloadMigration extends AAComment7Migration {}
class officialspcrewCommentforumMigration extends AAComment7Migration {}
class officialspcrewCommentimage_singleMigration extends AAComment7Migration {}
class officialspcrewCommenttour_dateMigration extends AAComment7Migration {}
