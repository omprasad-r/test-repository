<?php
/**
 * @file
 * Migration implementations specific to overcastkids.
 */

/**
 * Registration of this site's migrations.
 */
class overcastkidsRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    $default_uid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'OCK Admin')
                   ->execute()
                   ->fetchField();
    parent::__construct($site_name, 0, $default_uid);
    $this->sourceDirectory = 'http://www.overcastkids.com/sites/overcastkids.com/files/';
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
    $this->registerMigration('Picture');
    $this->registerMigration('User');
    $this->registerMigration('News');
    $this->registerMigration('Contest');
    $this->registerMigration('Event');
    $this->registerMigration('TourDate');
    $this->registerMigration('Photo');
  }
}

class overcastkidsMigration extends DynamicMigration {
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
                        ->condition('name', 'OCK Admin')
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

class overcastkidsPictureMigration extends overcastkidsMigration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('image_img', 'i')
             ->fields('i');
    $query->innerJoin('users', 'u', 'i.user_id=u.id AND u.active=1');

    $this->source = new MigrateSourceSQL($query, array(), NULL, $this->sourceOptions);

    $this->destination = new MigrateDestinationFile('file', 'MigrateFileUri');

    $this->map = new MigrateSQLMap($arguments['machine_name'],
      array(
        'id_img' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'Source ID of image',
        ),
      ),
      MigrateDestinationFile::getKeySchema()
    );

    $this->addFieldMapping('value', 'filename_img');
    $this->addFieldMapping('source_dir')
         ->defaultValue('http://www.overcastkids.com/v2/v2/admin/uploadedimages3/');
    $this->addFieldMapping('destination_dir')
         ->defaultValue('public://pictures');
    $this->addFieldMapping('destination_file', 'filename_img');
    $this->addFieldMapping('file_replace')
         ->defaultValue(FILE_EXISTS_RENAME);
    $this->addFieldMapping('timestamp', 'date_img');
    $this->addFieldMapping('uid', 'user_id');

    $this->addUnmigratedDestinations(array(
      'migrate_redirects',
      'preserve_files',
    ));
    $this->addUnmigratedSources(array(
      'def_img',          // No source data
      'description_img',  // No source data
      'idalb_img',        // No source data
      'order_set',        // No source data
    ));
  }
}

class overcastkidsUserMigration extends overcastkidsMigration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'Picture';

    $fields = array(
      'full_name' => 'Constructed from first_name and last_name',
    );

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('users', 'u')
             ->fields('u');
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
    // Note this will be determined by both access and active
    $this->addFieldMapping('status', 'active');
    $this->addFieldMapping('created', 'signup_date');
    $this->addFieldMapping('role_names')
         ->defaultValue('fan club member');
    $this->addFieldMapping('picture', 'def_img')
         ->sourceMigration('Picture');
    $this->addFieldMapping('field_profile_updated', 'signup_date');
    $this->addFieldMapping('field_member_name', 'full_name');
    $this->addFieldMapping('field_nickname', 'first_name');
    $this->addFieldMapping('field_city', 'city');
    $this->addFieldMapping('field_state', 'state');
    $this->addFieldMapping('field_postal_code', 'zip');
    $this->addFieldMapping('field_country', 'country');
    $this->addFieldMapping('field_gender', 'gender');
    $this->addFieldMapping('field_about_me', 'aboutme');
    $this->addFieldMapping('field_hobbies', 'thingsiminto');
    $this->addFieldMapping('field_favorite_song', 'favsong');

    $this->addUnmigratedDestinations(array(
      'access',
      'data',
        'field_about_me:language',
      'field_birthdate',
        'field_city:language',
      'field_comment',
      'field_eos_id',
      'field_favorite_bands', 'field_favorite_bands:language',
        'field_hobbies:language',
        'field_member_name:language',
        'field_nickname:language',
        'field_postal_code:language',
      'field_profile_14', 'field_profile_14:language',
      'field_provider', 'field_provider:language',
      'field_site_id', 'field_site_id:language',
        'field_state:language',
      'field_twitter_name', 'field_twitter_name:language',
      'field_type',
      'field_usage_type',
      'is_new',
      'language',
      'login',
      'migrate_redirects',
      'path',
      'pathauto',
      'roles',
      'signature', 'signature_format',
      'theme',
      'timezone',
    ));

    $this->addUnmigratedSources(array(
      'access',
      'address_1',
      'address_2',
      'code',
      'dob',
      'groups',
      'im',
      'im_type',
      'last_name',
      'level',
      'mb_id',
      'phone',
      'quote',
      'regcode',
      'showim',
      'showmail',
      'upcomingevents',
      'wishlist',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    if (!$row->username) {
      $row->username = 'generated' . $row->id;
    }

    if ($row->first_name && $row->last_name) {
      $row->full_name = $row->first_name . ' ' . $row->last_name;
    }
    elseif ($row->first_name) {
      $row->full_name = $row->first_name;
    }
    elseif ($row->last_name) {
      $row->full_name = $row->last_name;
    }
    else {
      $row->full_name = $row->username;
    }

    // Ignore bogus signup dates
    if ($row->signup_date == '0000-00-00') {
      $row->signup_date = NULL;
    }

    // Accounts are blocked iff both active and access are 0
    if ($row->active || $row->access) {
      $row->active = 1;
    }
    else {
      $row->active = 0;
    }
  }
}

class overcastkidsBlogMigration extends overcastkidsMigration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('body', 'description');
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
}

class overcastkidsContestMigration extends overcastkidsBlogMigration {
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

class overcastkidsNewsMigration extends overcastkidsBlogMigration {
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

class overcastkidsEventBaseMigration extends overcastkidsMigration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('uid')
         ->defaultValue($this->defaultUid);
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);


    $this->addUnmigratedDestinations(array(
        'body:summary', 'body:language',
      'changed',
      'comment',
      'created',
      'daycount',
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
}

class overcastkidsEventMigration extends overcastkidsEventBaseMigration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('events', 'e')
             ->fields('e');
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
                      'description' => 'Source event ID',
                      'alias' => 'e',
                     ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('body', 'description');
    $this->addFieldMapping('field_event_date', 'date');
    $this->addFieldMapping('field_event_venue', 'location');
    $this->addFieldMapping('field_event_maplink')
         ->defaultValue('');
    $this->addFieldMapping('field_event_buylink')
         ->defaultValue('');
    $this->addFieldMapping('field_event_category')
         ->defaultValue('Events');
    $this->addFieldMapping('field_event_category:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_event_city', 'city');
    $this->addFieldMapping('field_event_state', 'state');
    $this->addFieldMapping('field_event_country')
         ->defaultValue('US');

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
      'cost',   // Always 0
      'order',  // Always 0
      'time',   // Empty
      'venue',  // location actually holds the venue
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $location = explode(',', $row->title);
    $row->city = trim($location[0]);
    $row->state = trim($location[1]);
  }
}

class overcastkidsTourDateMigration extends overcastkidsEventBaseMigration {
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

    $this->addFieldMapping('title', 'citystate');
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
      'body',
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
      'link',   // Always empty
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $location = explode(',', $row->citystate);
    $row->city = trim($location[0]);
    $row->state = trim($location[1]);
  }
}

class overcastkidsPhotoMigration extends XMLMigration {
  public function __construct($arguments) {
    $this->arguments = $arguments;
    parent::__construct(MigrateGroup::getInstance($arguments['group']));

    $this->defaultUid = db_select('users', 'u')
                        ->fields('u', array('uid'))
                        ->condition('name', 'OCK Admin')
                        ->execute()
                        ->fetchField();

    $fields = array(
      'src' => 'Filename, relative to site prefix and lgPath',
      'caption' => 'Photo caption',
      'gallery_title' => 'Title of the gallery',
      'lgPath' => 'Relative path to the file',
    );
    $items_url = DRUPAL_ROOT . '/' . drupal_get_path('module', 'migrate_aa') .
                  '/overcastkids.xml';
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
        'field_photo_asset:language',
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
    $row->gallery_title = $this->applyXpath($row, '../@title');
    if (!$row->caption) {
      $row->caption = $row->src;
    }
    // Replace the characters that mess up gallery aliases with spaces
    $row->gallery_title = strtr($row->gallery_title, '-/\\', '   ');
    $row->src = 'http://www.overcastkids.com/newsite/' . $row->lgPath . $row->src;
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
