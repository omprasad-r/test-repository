<?php
/**
 * @file
 * Migration implementations specific to northerndownpour.
 */

/**
 * Registration of this site's migrations.
 */
class northerndownpourRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    $default_uid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'ND Admin')
                   ->execute()
                   ->fetchField();
    parent::__construct($site_name, 0, $default_uid);
    $this->sourceDirectory = 'http://www.northerndownpour.com/sites/northerndownpour.com/files/';
  }

  /**
   * Register one migration.
   *
   * @param $type
   * @param $arguments
   */
  protected function registerMigration($type, $arguments = array()) {
    $arguments = $this->commonArguments + $arguments;
    $arguments['class_name'] = $this->siteName . $type . 'Migration';
    $arguments['machine_name'] = $type;
    $arguments['description'] = t('!type migration', array('!type' => $type));
    if (class_exists($arguments['class_name'])) {
      MigrationBase::registerMigration($arguments['class_name'],
        $arguments['machine_name'], $arguments);
    }
    else {
      MigrationBase::displayMessage(
        t('Could not register !machine_name migration, class !class_name does not exist.',
          array('!machine_name' => $arguments['machine_name'],
                '!class_name' => $arguments['class_name'])));
    }
  }

  /**
   * Register our migrations.
   */
  protected function register() {
    $this->registerMigration('User');
    $this->registerMigration('News');
    $this->registerMigration('Contest');
    $this->registerMigration('Event');
    $this->registerMigration('TourDate');
    $this->registerMigration('Photo');
  }
}

class northerndownpourMigration extends DynamicMigration {
  /**
   * Connection key for the DatabaseConnection holding the source Drupal
   * installation.
   *
   * @var
   */
  protected $sourceConnection;

  /**
   * Options to be passed to source constructors.
   *
   * @var array
   */
  protected $sourceOptions = array();

  public function __construct($arguments) {
    $this->arguments = $arguments;
    parent::__construct(MigrateGroup::getInstance($arguments['group']));

    $this->sourceConnection = $arguments['source_connection'];
    if (!empty($arguments['source_database'])) {
      Database::addConnectionInfo($this->sourceConnection, 'default', $arguments['source_database']);
    }
    $this->sourceOptions = array('map_joinable' => FALSE, 'cache_counts' => TRUE,
                     'cache_key' => 'migrate_' . $this->machineName);
    $this->defaultUid = db_select('users', 'u')
                        ->fields('u', array('uid'))
                        ->condition('name', 'ND Admin')
                        ->execute()
                        ->fetchField();
  }

  /**
   * Use the explicit machine_name argument as this migration's machine name.
   */
  protected function generateMachineName($class_name = NULL) {
    return $this->arguments['machine_name'];
  }
}

class northerndownpourUserMigration extends northerndownpourMigration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $fields = array('name' => 'Generated from name_f and name_l');
    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('users', 'u')
             ->fields('u', array('id', 'username', 'password', 'access'));
    $query->innerJoin('amember_members', 'm', 'u.username=m.login');
    $query->fields('m', array('email', 'name_f', 'name_l', 'city',
      'state', 'zip', 'country', 'added', 'twitter'));
    $this->source = new MigrateSourceSQL($query, $fields, NULL, $this->sourceOptions);

    $this->destination = new MigrateDestinationUser();

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'id' => array('type' => 'int',
                      'unsigned' => TRUE,
                      'not null' => TRUE,
                      'description' => 'Source user ID',
                      'alias' => 'u',
                     ),
      ),
      DrupalDestinationUser::getKeySchema()
    );

    $this->addFieldMapping('mail', 'email');
    $this->addFieldMapping('init', 'email');
    $this->addFieldMapping('name', 'username')
         ->dedupe('users', 'name');
    $this->addFieldMapping('pass', 'password');
    $this->addFieldMapping('status', 'access');
    $this->addFieldMapping('created', 'added');
    $this->addFieldMapping('role_names')
         ->defaultValue('fan club member');
    $this->addFieldMapping('field_member_name', 'name');
    $this->addFieldMapping('field_nickname', 'name');
    $this->addFieldMapping('field_city', 'city');
    $this->addFieldMapping('field_state', 'state');
    $this->addFieldMapping('field_postal_code', 'zip');
    $this->addFieldMapping('field_country', 'country');
    $this->addFieldMapping('field_twitter_name', 'twitter');

    $this->addUnmigratedDestinations(array(
      'access',
      'data',
      'field_about_me', 'field_about_me:language',
      'field_birthdate',
        'field_city:language',
      'field_comment',
      'field_eos_id',
      'field_favorite_bands', 'field_favorite_bands:language',
      'field_gender',
      'field_hobbies', 'field_hobbies:language',
        'field_member_name:language',
        'field_nickname:language',
        'field_postal_code:language',
      'field_profile_14', 'field_profile_14:language',
      'field_profile_updated',
      'field_provider', 'field_provider:language',
      'field_site_id', 'field_site_id:language',
        'field_state:language',
        'field_twitter_name:language',
      'field_type',
      'field_usage_type',
      'is_new',
      'language',
      'login',
      'migrate_redirects',
      'path',
      'pathauto',
      'picture',
      'roles',
      'signature', 'signature_format',
      'theme',
      'timezone',
    ));

    $this->addUnmigratedSources(array(
      'name_f', 'name_l', // Used to construct full name
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    if (!$row->username) {
      $row->username = 'generated' . $row->id;
    }

    $row->name = trim($row->name_f . ' ' . $row->name_l);

    if (!empty($row->country)) {
      $countries = array(
        'AQ' => 'Antarctica',
        'AR' => 'Argentina',
        'AT' => 'Austria',
        'AU' => 'Australia',
        'BA' => 'Bosnia & Herzegovina',
        'BE' => 'Belgium',
        'BR' => 'Brazil',
        'CA' => 'Canada',
        'CH' => 'Switzerland',
        'CL' => 'Chile',
        'CZ' => 'Czech Republic',
        'DE' => 'Germany',
        'DK' => 'Denmark',
        'ES' => 'Spain',
        'FI' => 'Finland',
        'FR' => 'France',
        'GB' => 'United Kingdom',
        'GT' => 'Guatemala',
        'HK' => 'Hong Kong',
        'ID' => 'Indonesia',
        'IE' => 'Ireland',
        'IL' => 'Israel',
        'IS' => 'Iceland',
        'IT' => 'Italy',
        'JP' => 'Japan',
        'KW' => 'Kuwait',
        'KZ' => 'Kazakhstan',
        'MT' => 'Malta',
        'MX' => 'Mexico',
        'NL' => 'Netherlands',
        'NO' => 'Norway',
        'NZ' => 'New Zealand',
        'PE' => 'Peru',
        'PH' => 'Philippines',
        'PL' => 'Poland',
        'PT' => 'Portugal',
        'RU' => 'Russian Federation',
        'SE' => 'Sweden',
        'SG' => 'Singapore',
        'UA' => 'Ukraine',
        'UK' => 'United Kingdom',
        'UM' => 'United States',
        'US' => 'United States',
        'VE' => 'Venezuela',
        'YU' => 'Serbia',
      );
      if (isset($countries[$row->country])) {
        $row->country = $countries[$row->country];
      }
      else {
        $row->country = '';
      }
    }
  }
}

class northerndownpourBlogMigration extends northerndownpourMigration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('body', 'description')
         ->callbacks(array($this, 'fixQuotes'));
    $this->addFieldMapping('body:format')
         ->defaultValue('1'); // Filtered HTML
    $this->addFieldMapping('uid')
         ->defaultValue($this->defaultUid);
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);
    $this->addFieldMapping('field_is_pro')
         ->defaultvalue(TRUE);

    $this->addUnmigratedDestinations(array(
        'body:summary', 'body:language',
      'changed',
      'comment',
      'created',
      'daycount',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
      'is_new',
      'language',
      'log',
      'migrate_redirects',
      'path',
      'pathauto',
      'promote',
      'revision',
      'revision_uid',
      'sticky',
      'timestamp',
      'tnid',
      'totalcount',
      'translate',
    ));

    $this->addUnmigratedSources(array(
      'order',
    ));
  }

  protected function fixQuotes($body) {
    return str_replace("\'", "'", $body);
  }
}

class northerndownpourContestMigration extends northerndownpourBlogMigration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('contests', 'c')
             ->fields('c');
    $this->source = new MigrateSourceSQL($query, array(), NULL, $this->sourceOptions);

    $this->destination = new MigrateDestinationNode('blog');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'id' => array('type' => 'int',
                      'unsigned' => TRUE,
                      'not null' => TRUE,
                      'description' => 'Source contest ID',
                      'alias' => 'c',
                     ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('field_blog_category')
         ->defaultValue('Contests');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);
    $this->addUnmigratedDestinations(array(
       'field_blog_category:source_type', 'field_blog_category:ignore_case',
    ));
  }
}

class northerndownpourNewsMigration extends northerndownpourBlogMigration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('news', 'n')
             ->fields('n');
    $this->source = new MigrateSourceSQL($query, array(), NULL, $this->sourceOptions);

    $this->destination = new MigrateDestinationNode('blog');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'id' => array('type' => 'int',
                      'unsigned' => TRUE,
                      'not null' => TRUE,
                      'description' => 'Source news ID',
                      'alias' => 'n',
                     ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('field_blog_category')
         ->defaultValue('News');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);
    $this->addUnmigratedDestinations(array(
       'field_blog_category:source_type', 'field_blog_category:ignore_case',
    ));
    $this->addFieldMapping('created', 'date', FALSE);
    $this->addFieldMapping('changed', 'date', FALSE);
  }
}

class northerndownpourEventMigration extends northerndownpourBlogMigration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('events', 'e')
             ->fields('e');
    $this->source = new MigrateSourceSQL($query, array(), NULL, $this->sourceOptions);

    $this->destination = new MigrateDestinationNode('blog');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'id' => array('type' => 'int',
                      'unsigned' => TRUE,
                      'not null' => TRUE,
                      'description' => 'Source event ID',
                      'alias' => 'e',
                     ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('title', 'location', FALSE);
    $this->addFieldMapping('created', 'date', FALSE);
    $this->addFieldMapping('changed', 'date', FALSE);
    $this->addFieldMapping('field_blog_category')
         ->defaultValue('Events');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);
    $this->addUnmigratedDestinations(array(
       'field_blog_category:source_type', 'field_blog_category:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'cost',   // Always 0
      'date',   // Prepended to body
      'order',  // Always 0
      'time',   // Empty
      'title',  // Actually contains location, prepended to body
      'venue',  // location actually holds the venue
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    if (is_array($row->date)) {
      $row->date = reset($row->date);
    }
    if (is_array($row->title)) {
      $row->title = reset($row->title);
    }
    if (is_array($row->description)) {
      $row->description = reset($row->description);
    }
    $row->description = "<strong>Event date:</strong> {$row->date}<br />\n"
      . "<strong>Location:</strong> {$row->title}<br />\n"
      . $row->description;
  }
}

class northerndownpourTourDateMigration extends northerndownpourMigration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('tourdates', 't')
             ->fields('t');
    $fields = array(
      'city' => 'City value extracted from title',
      'state' => 'State value extracted from title',
    );
    $this->source = new MigrateSourceSQL($query, $fields, NULL, $this->sourceOptions);

    $this->destination = new MigrateDestinationNode('event');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'id' => array('type' => 'int',
                      'unsigned' => TRUE,
                      'not null' => TRUE,
                      'description' => 'Source tourdate ID',
                      'alias' => 't',
                     ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('uid')
         ->defaultValue($this->defaultUid);
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);
    $this->addFieldMapping('title', 'citystate');
    $this->addFieldMapping('created', 'date');
    $this->addFieldMapping('changed', 'date');
    $this->addFieldMapping('field_event_date', 'date');
    $this->addFieldMapping('field_event_venue', 'venue');
    $this->addFieldMapping('field_event_maplink')
         ->defaultValue('');
    $this->addFieldMapping('field_event_buylink')
         ->defaultValue('');
    $this->addFieldMapping('field_event_category')
         ->defaultValue('Tour Dates');
    $this->addFieldMapping('field_event_category:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_event_city', 'city');
    $this->addFieldMapping('field_event_state', 'state');
    $this->addFieldMapping('field_event_country')
         ->defaultValue('US');

    $this->addUnmigratedDestinations(array(
      'body', 'body:summary', 'body:language',
      'comment',
      'daycount',
      'field_event_address', 'field_event_address:language',
      'field_event_availability', 'field_event_availability:language',
        'field_event_buylink:format', 'field_event_buylink:language',
        'field_event_category:source_type', 'field_event_category:ignore_case',
        'field_event_city:language',
        'field_event_country:language',
        'field_event_maplink:format', 'field_event_maplink:language',
        'field_event_state:language',
        'field_event_venue:language',
      'is_new',
      'language',
      'log',
      'migrate_redirects',
      'path',
      'pathauto',
      'promote',
      'revision',
      'revision_uid',
      'sticky',
      'timestamp',
      'tnid',
      'totalcount',
      'translate',
    ));

    $this->addUnmigratedSources(array(
      'link',   // Always empty
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $location = explode(',', $row->citystate);
    $row->city = trim($location[0]);
    if (isset($location[1])) {
      $row->state = trim($location[1]);
    }
    else {
      $row->state = '';
    }
  }
}

class northerndownpourPhotoMigration extends XMLMigration {
  public function __construct($arguments) {
    $this->arguments = $arguments;
    parent::__construct(MigrateGroup::getInstance($arguments['group']));

    $this->defaultUid = db_select('users', 'u')
                        ->fields('u', array('uid'))
                        ->condition('name', 'ND Admin')
                        ->execute()
                        ->fetchField();

    $fields = array(
      'src' => 'Filename, relative to site prefix and lgPath',
      'caption' => 'Photo caption',
      'gallery_title' => 'Title of the gallery',
      'lgPath' => 'Relative path to the file',
    );
    $items_url = DRUPAL_ROOT . '/' . drupal_get_path('module', 'migrate_aa') .
                  '/northerndownpour.xml';
    $item_xpath = '/gallery/album/img';
    $item_ID_xpath = '@src';
    $items_class = new MigrateItemsXML($items_url, $item_xpath, $item_ID_xpath);
    $this->source = new MigrateSourceMultiItems($items_class, $fields);
    $this->destination = new MigrateDestinationNode('photo');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'src' => array(
         'type' => 'varchar',
         'length' => 255,
         'not null' => TRUE,
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );
    $this->addFieldMapping(NULL, 'gallery_title');
    $this->addFieldMapping(NULL, 'lgPath');
    $this->addFieldMapping('title', 'caption');
    $this->addFieldMapping('uid')
         ->defaultValue($this->defaultUid);
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);
    $this->addFieldMapping('field_is_pro')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_photo_asset', 'src');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileUri');

    $this->addFieldMapping('field_photo_category', 'gallery_title');
    $this->addFieldMapping('field_photo_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'body', 'body:summary', 'body:format', 'body:language',
      'changed',
      'comment',
      'created',
      'daycount',
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:destination_dir',
          'field_photo_asset:destination_file', 'field_photo_asset:file_replace',
          'field_photo_asset:preserve_files', 'field_photo_asset:source_dir',
          'field_photo_asset:title', 'field_photo_asset:data',
        'field_photo_category:source_type', 'field_photo_category:ignore_case',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
      'is_new',
      'language',
      'log',
      'migrate_redirects',
      'path',
      'pathauto',
      'promote',
      'revision',
      'revision_uid',
      'sticky',
      'timestamp',
      'tnid',
      'totalcount',
      'translate',
    ));
  }

  public function prepareRow($row) {
    $row->lgPath = $this->applyXpath($row, '../@lgPath');
    $row->caption = $this->applyXpath($row, '@caption');
    $row->gallery_title = $this->applyXpath($row, '../@id');
    if (!$row->caption) {
      $row->caption = $row->src;
    }
    // Replace the characters that mess up gallery aliases with spaces
    $row->gallery_title = strtr($row->gallery_title, '-/\\', '   ');
    $row->src = 'http://www.northerndownpour.com/site/' . $row->lgPath . $row->src;
  }

  /**
   * Overrides default of FALSE
   */
  static public function isDynamic() {
    return TRUE;
  }

  /**
   * Use the explicit machine_name argument as this migration's machine name.
   */
  protected function generateMachineName($class_name = NULL) {
    return $this->arguments['machine_name'];
  }
}
