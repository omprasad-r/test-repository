<?php
/**
 * @file
 * Migration implementations specific to thewilliambeckettsociety.
 */

/**
 * Registration of this site's migrations.
 */
class thewilliambeckettsocietyRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    parent::__construct($site_name, 7, 0);
    $this->commonArguments['format_mappings'] =  array(
      'filtered_html' => '1',
      'plain_text' => '11',
      'full_html' => '6',
      'unfiiltered' => '11',
      'php_code' => 'script',
    );
    $this->roleArguments['role_mappings'] = array(
      'administrator' => 'administrator',
      'Fan Club' => 'fan club member',
      'Product Manager' => 'administrator',
    );
    $this->sourceDirectory = 'http://www.thewilliambeckettsociety.com/sites/santislittlehelpers.com/files/';
  }

  /**
   * Register our migrations.
   */
  protected function register() {
    $this->registerRole();
    $this->registerPicture();
    $this->registerUser();
    $this->registerFile();
    $this->registerTerm('forums', 'forums');
    $this->registerNode('article', 'blog');
    $this->registerComment('article', 'blog');
    $this->registerNode('contests', 'blog');
    $this->registerNode('event', 'blog');
    $this->registerNode('forum', 'forum');
    $this->registerComment('forum', 'forum');
    $this->registerNode('tour_date', 'event');
    $this->registerComment('tour_date', 'event');
    $this->registerNode('image', 'photo');
    $this->registerMigration('webform', $this->commonArguments);
  }
}

class thewilliambeckettsocietyRoleMigration extends AARole7Migration {}
class thewilliambeckettsocietyPictureMigration extends AAPicture7Migration {}

class thewilliambeckettsocietyUserMigration extends AAUser7Migration {
  public function __construct(array $arguments) {
    $arguments['picture_migration'] = 'Picture';
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
      'field_phpbb_id',
      'field_user_location',
      'uuid'
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
}

class thewilliambeckettsocietyforumsTermMigration extends AATerm7Migration {}

class thewilliambeckettsocietyFileMigration extends AAFile7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
  }
}

class thewilliambeckettsocietyarticleMigration extends AANode7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';

    $this->defaultUid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'SLH Admin')
                   ->execute()
                   ->fetchField();
    $this->addFieldMapping('field_blog_category')
         ->defaultValue('General');

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:create_term', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'field_image', 'field_image:alt', 'field_image:title', 'field_image:width',
        'field_image:height',
      'field_tags',   // No source data
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    if (!empty($row->field_image)) {
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
        $summary_field = 'body:summary';
        $row->$summary_field = $media_tag . $row->{$summary_field}[0];
      }
    }
    $format_field = 'body:format';
    if (empty($row->$format_field)) {
      $row->$format_field = 'full_html';
    }
  }
}

class thewilliambeckettsocietycontestsMigration extends AANode7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->defaultUid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'SLH Admin')
                   ->execute()
                   ->fetchField();
    $this->addFieldMapping('field_blog_category')
         ->defaultValue('Contests');

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:create_term', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case', // No source data
    ));
  }
}

class thewilliambeckettsocietyeventMigration extends AANode7Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->defaultUid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'SLH Admin')
                   ->execute()
                   ->fetchField();
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

class thewilliambeckettsocietytour_dateMigration extends AANode7Migration {
  public function __construct($arguments) {
    $this->sourceFields['state'] = 'State extracted from location';
    $this->sourceFields['country'] = 'Country extracted from location';
    parent::__construct($arguments);
    $this->defaultUid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'SLH Admin')
                   ->execute()
                   ->fetchField();
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
    $this->addFieldMapping('field_event_date', 'field_tourdatenew');
    $this->addFieldMapping('field_event_category')
         ->defaultValue('Tour Date');
    $this->addFieldMapping('field_event_category:create_term')
         ->defaultValue(TRUE);

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
      'field_presale_dat',
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

class thewilliambeckettsocietyforumMigration extends AANode7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'forumsTerm';

    $this->defaultUid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'SLH Admin')
                   ->execute()
                   ->fetchField();
    $this->addFieldMapping('taxonomy_forums', 'taxonomy_forums', FALSE)
         ->sourceMigration('forumsTerm');
    $this->addFieldMapping('taxonomy_forums:source_type')
         ->defaultValue('tid');

    $this->addUnmigratedDestinations(array(
        'taxonomy_forums:create_term', 'taxonomy_forums:ignore_case',
    ));
  }
}

class thewilliambeckettsocietyimageMigration extends AANode7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->defaultUid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'SLH Admin')
                   ->execute()
                   ->fetchField();
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
    $this->addFieldMapping('field_photo_asset:title', 'field_photo:title');
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
      'field_photo', 'field_photo:alt', 'field_photo:width', 'field_photo:height',
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

class thewilliambeckettsocietywebformMigration extends DrupalMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'User';

    $fields = array(
      'username' => 'username',
      'first_name' => 'first_name',
      'last_name' => 'last_name',
      'email_address' => 'email_address',
      'date_of_event' => 'date_of_event',
      'venue_city_and_state' => 'venue_city_and_state',
      'have_you_ever_met_tai_before' => 'have_you_ever_met_tai_before',
      'confirm_that_you_have_read_all_of_the_above_before_filling_out_this_form_and_that_you_have_purchased_your_tickets_thru_pre_sale' => 'confirm_that_you_have_read_all_of_the_above_before_filling_out_this_form_and_that_you_have_purchased_your_tickets_thru_pre_sale',
    );
    $this->source = new MigrateSourceSQL($this->query(), $fields, NULL,
      array('map_joinable' => FALSE));
    $nodes = node_load_multiple(array(), array('type' => 'webform',
      'title' => 'Mingle & Tingle'));
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
    $this->addFieldMapping('data_new_1346344054157', 'username');
    $this->addFieldMapping('data_new_1346344123627', 'first_name');
    $this->addFieldMapping('data_new_1346344162796', 'last_name');
    $this->addFieldMapping('data_new_1346344197483', 'email_address');
    $this->addFieldMapping('data_new_1346344247930', 'date_of_event');
    $this->addFieldMapping('data_new_1346344300331', 'venue_city_and_state');
    $this->addFieldMapping('data_new_1346344338182', 'have_you_ever_met_tai_before');
    $this->addFieldMapping('data_new_1346344402679', 'confirm_that_you_have_read_all_of_the_above_before_filling_out_this_form_and_that_you_have_purchased_your_tickets_thru_pre_sale');

    $this->addUnmigratedDestinations(array(
      'data_new_1346343987773',
      'sid',
    ));
  }

  protected function query() {
    // We're only interested in one webform, with the known legacy nid 25971
    $query = Database::getConnection('default', 'legacy')
             ->select('webform_submissions', 's')
             ->fields('s', array('sid', 'uid', 'is_draft', 'submitted', 'remote_addr'))
             ->condition('nid', 2528);
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

class thewilliambeckettsocietyCommentarticleMigration extends AAComment7Migration {}
class thewilliambeckettsocietyCommentforumMigration extends AAComment7Migration {}
class thewilliambeckettsocietyCommenttour_dateMigration extends AAComment7Migration {}
