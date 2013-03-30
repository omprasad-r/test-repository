<?php
/**
 * @file
 * Migration implementations specific to gdgfanclub.
 */

/**
 * Registration of this site's migrations.
 */
class gdgfanclubRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    $default_uid = db_select('migrate_map_user', 'map')
                   ->fields('map', array('destid1'))
                   ->condition('sourceid1', 4)
                   ->execute()
                   ->fetchField();
    parent::__construct($site_name, 7, $default_uid);
    $this->commonArguments['format_mappings'] =  array(
      'filtered_html' => '1',
      'plain_text' => '11',
      'full_html' => '6',
      'no_editor' => '11',
      'php_code' => 'script',
    );
    $this->roleArguments['role_mappings'] = array(
      'administrator' => 'administrator',
      'Fan Club' => 'fan club member',
      'Product Manager' => 'administrator',
    );
    $this->sourceDirectory = 'http://www.gdgfanclub.com/sites/gdgfanclub.com/files/';
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
    $this->registerNode('contests', 'blog');
    $this->registerComment('contests', 'blog');
    $this->registerNode('event', 'event');
    $this->registerNode('tour_date', 'event');
    $this->registerComment('tour_date', 'event');
    $this->registerNode('image', 'photo');
    // Only 10 comments total, assigned to image nodes, and we have no
    // basis on which to attach them to specific photo nodes.
//    $this->registerComment('image', 'photo');
  }
}

class gdgfanclubRoleMigration extends AARole7Migration {}

class gdgfanclubUserMigration extends AAUser7Migration {
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
}

class gdgfanclubFileMigration extends AAFile7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
  }
}

class gdgfanclubarticleMigration extends AANode7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_blog_category')
         ->defaultValue('General');

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:create_term', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'field_image',  // No source data
      'field_tags',   // No source data
    ));
  }
}

class gdgfanclubcontestsMigration extends AANode7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_blog_category')
         ->defaultValue('Contests');

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:create_term', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case', // No source data
    ));
  }
}

class gdgfanclubeventMigration extends AANode7Migration {
  public function __construct($arguments) {
    $this->sourceFields['state'] = 'State extracted from location';
    $this->sourceFields['country'] = 'Country extracted from location';
    parent::__construct($arguments);

    $this->addFieldMapping('field_event_venue', 'field_event_venue');
    $this->addFieldMapping('field_event_city', 'field_event_location');
    $this->addFieldMapping('field_event_state', 'state');
    $this->addFieldMapping('field_event_country', 'country');
    $this->addFieldMapping('field_event_date', 'field_event_date');
    $this->addFieldMapping('field_event_category')
         ->defaultValue('General');
    $this->addFieldMapping('field_event_category:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_event_buylink')
         ->defaultValue('');
    $this->addFieldMapping('field_event_maplink')
         ->defaultValue('');


    $this->removeFieldMapping('field_is_pro');
    $this->removeFieldMapping('body:format');

    $this->addUnmigratedDestinations(array(
      'field_event_address', 'field_event_address:language',
      'field_event_availability', 'field_event_availability:language',
        'field_event_buylink:format', 'field_event_buylink:language',
        'field_event_category:source_type', 'field_event_category:ignore_case',
        'field_event_city:language',
        'field_event_country:language',
        'field_event_maplink:format', 'field_event_maplink:language',
        'field_event_state:language',
        'field_event_venue:language',
    ));

    $this->addUnmigratedSources(array(
      'body:format',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $this->parseLocation($row, 'field_event_location');
  }
}

class gdgfanclubtour_dateMigration extends AANode7Migration {
  public function __construct($arguments) {
    $this->sourceFields['state'] = 'State extracted from location';
    $this->sourceFields['country'] = 'Country extracted from location';
    parent::__construct($arguments);

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

class gdgfanclubimageMigration extends AANode7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';

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

class gdgfanclubCommentarticleMigration extends AAComment7Migration {}
class gdgfanclubCommentcontestsMigration extends AAComment7Migration {}
class gdgfanclubCommenttour_dateMigration extends AAComment7Migration {}
