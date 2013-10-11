<?php
/**
 * @file
 * Mappings specific to bieberfever.
 */

class bieberfeverRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    parent::__construct($site_name, 6);
    $this->commonArguments['format_mappings'] =  array(
      '1' => '1',
      '2' => '6',
      '3' => 'script',
      '11' => '2',
    );
    $this->roleArguments['role_mappings'] = array(
      'Fan Club' => 'fan club member',
      'Customer Service' => 'administrator',
      'Product Manager' => 'administrator',
      '13 and younger' => 'authenticated user',
      '14 and older' => 'authenticated user',
      'monthly' => 'Fan Club without Presale',
      'Newsletter' => 'authenticated user',
    );
    $this->sourceDirectory = 'http://bieberfever.com/sites/bieberfever.com/files/';
  }

  protected function register() {
    $this->registerRole();
    $this->registerPicture();
    $this->registerUser();
    $this->registerFile();

    // Forums vocabulary
    $this->registerTerm('1', 'forums');
    // Product Vocabulary vocabulary - can probably hard-code assignments
    // Content Type vocabulary (vid=5) - separate media_file nodes into audio/video

    // @todo: Lots of gallery comments in the legacy site
    $this->registerGallery('admin_gallery', 'photo_category');
    $this->registerGallery('fan_gallery', 'fans_photo_category');
//    $this->registerGallery('fan_with_artist_gallery', 'photo_category');
    $this->registerNode('fan_with_artist_gallery', 'media_gallery');
    $this->registerComment('fan_with_artist_gallery', 'media_gallery');
    $this->registerNode('meet_greet', 'media_gallery');
    $this->registerNode('admin_photo', 'photo');
    $this->registerComment('admin_photo', 'photo');
    $this->registerNode('contest_media', 'photo');
    $this->registerNode('event', 'event');
    $this->registerComment('event', 'event');
//    $this->registerNode('fan_with_artist_photo', 'photo');
//    $this->registerComment('fan_with_artist_photo', 'photo');
    $this->registerNode('forum', 'forum');
    $this->registerComment('forum', 'forum');
    $this->registerNode('image', 'photo');
    $this->registerNode('photo', 'fan_photo');
    $this->registerComment('photo', 'fan_photo');
    // @todo: product, product_kit
    $this->registerNode('story', 'blog');
    $this->registerComment('story', 'blog');
    // Special comment registration, to avoid depending on non-existent
    // uprofile migration
    $arguments = $this->commonArguments + $this->commentArguments + array(
      'source_type' => 'uprofile',
      'destination_type' => 'comment_on_anything',
    );
    $this->registerMigration('Commentuprofile', $arguments);
    $this->registerMigration('PrivateMessage', $this->commonArguments);
  }
}

class bieberfeverRoleMigration extends AARole6Migration {
  protected function preImport() {
    parent::preImport();
    // Make sure the new role Fan Club without Presale exists
    $role_name = 'Fan Club without Presale';
    $rid = db_select('role', 'r')
           ->fields('r', array('rid'))
           ->condition('name', $role_name)
           ->execute()
           ->fetchField();
    if (!$rid) {
      db_insert('role')
        ->fields(array('name' => $role_name))
        ->execute();
    }
  }
}

class bieberfeverPictureMigration extends AAPicture6Migration {
  public function prepareRow($row) {
    $row->filepath = str_replace('sites/bieberfever.com/files/', '', $row->filepath);
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
  }
}

class bieberfeverUserMigration extends AAUser6Migration {
  public function __construct(array $arguments) {
    $this->sourceFields['friend'] = 'List of friend uids';
    parent::__construct($arguments);

    // Known mappings
    $this->addFieldMapping('field_member_name', 'field_name');
    $this->addFieldMapping('field_nickname', 'field_name');
    $this->addFieldMapping('field_about_me', 'field_about_me');
    $this->addFieldMapping('field_gender', 'field_gender');
    $this->addFieldMapping('field_hobbies', 'field_interests');
    $this->addFieldMapping('field_country', 'profile_country');
    $this->addFieldMapping('field_birthdate', 'profile_user_birthday');
    $this->addFieldMapping('friend', 'friend')
         ->sourceMigration($this->machineName);
    $this->addFieldMapping('name', 'name', FALSE);

    // Prevent flag from setting cookies
    global $user;
    if ($user->uid == 0) {
      $user = user_load(1);
    }

    // Provide a blank slate for all these fields.
    $this->addUnmigratedDestinations(array(
        'field_about_me:language',
      'field_city', 'field_city:language',
      'field_eos_id',
      'field_favorite_bands', 'field_favorite_bands:language',
        'field_hobbies:language',
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

    // No actual profile data in the source
    $this->addUnmigratedSources(array(
        'field_about_me:format',
      'field_favorite_movies',
        'field_interests:format',
      'field_location',
      'field_promo',
      'profile_affiliate_opt_in',
      'profile_fan_club_email_opt_in',
      'profile_parent_email',
      'profile_parent_name',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Do the same manipulation as the picture migration, so we match the
    // path in the map table.
    $row->picture = str_replace('sites/bieberfever.com/files/', '', $row->picture);

    // Birthdays are (sometimes) serialized arrays, convert to a datetime string
    if (!empty($row->profile_user_birthday) &&
        substr($row->profile_user_birthday, 0, 1) == '{') {
      static $threshold_date;
      if (!isset($threshold_date)) {
        $threshold_date = strftime('%Y-%m-%d', strtotime('13 years ago'));
      }
      $birthday = unserialize($row->profile_user_birthday);
      $row->profile_user_birthday = sprintf('%d-%02d-%02d',
        $birthday['year'], $birthday['month'], $birthday['day']);
      // Skip anyone with a birthdate since 13 years ago
      if ($row->profile_user_birthday > $threshold_date) {
        return FALSE;
      }
    }

    $query = Database::getConnection('default', $this->sourceConnection)
           ->select('user_relationships', 'ur')
           ->fields('ur', array('requestee_id'))
           ->condition('requester_id', $row->uid)
           ->condition('approved', 1);
    // Make sure the friend actually exists.
    $query->innerJoin('users', 'u', 'ur.requestee_id = u.uid');
    $row->friend = $query->execute()->fetchCol();
  }

  protected function createStub($migration, $source_key) {
    migrate_instrument_start('bieberfeverUserMigration::createStub');
    $account = new stdClass;
    $name = 'Stub_' . $source_key[0];
    $account->name = $name;
    $account->mail = $name . '@example.com';
    $account->status = 0;
    $account->signature = '';
    user_save($account, array());
    migrate_instrument_stop('bieberfeverUserMigration::createStub');
    if (isset($account->uid)) {
      return array($account->uid);
    }
    else {
      return FALSE;
    }
  }

  public function prepare($account, $row) {
    if (!empty($account->friend)) {
      if (is_array($account->friend)) {
        $this->friend = $account->friend;
      }
      else {
        $this->friend = array($account->friend);
      }
    }
    else {
      $this->friend = array();
    }
  }

  public function complete($account, $row) {
    // comment_on_anything doesn't create its nodes automatically, we must do
    // the work here. Stolen from _comment_on_anything_queue_worker_process_entities().
    migrate_instrument_start('User complete');
    // Reload to make sure picture is properly populated
    $account = user_load($account->uid);
    $account->field_comment[LANGUAGE_NONE] = array(
      array(
        'nid' => comment_on_anything_create_and_cache(
          $account, 'user', $account
        )
      ),
    );
    entity_save('user', $account);

    // Overrides broken migrate_extras handler.
    foreach ($this->friend as $friend) {
      try {
        // Make it two-way.
        flag('flag', 'friend', $friend, $account);
      }
      catch (Exception $e) {
        // Ignore "Cannot modify header information" based on setcookie
        if (strstr($e->getMessage(), 'Cannot modify header information') === FALSE) {
          throw $e;
        }
      }
    }

    migrate_instrument_stop('User complete');
  }
}

class bieberfeverFileMigration extends AAFile6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addUnmigratedSources(array(
      'origname',
    ));
  }

  public function prepareRow($row) {
    $row->filepath = str_replace('sites/bieberfever.com/files/', '', $row->filepath);
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
  }
}

class bieberfever1TermMigration extends AATerm6Migration {}

class bieberfeveradmin_galleryMigration extends AAGallery6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
  }
}

class bieberfeverfan_galleryMigration extends AAGallery6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->addFieldMapping('field_is_pro', NULL, FALSE)
         ->defaultValue(0);
  }
}

class bieberfeverNodeMigration extends AANode6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    // Override the uid mapping - rather than leaving it to sourceMigration()
    // at mapping time, and setting a default value, we want to ignore any
    // content with unresolved uid references, so we need to check in prepareRow().
    $this->addFieldMapping('uid', 'uid', FALSE);
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    // We want to ignore any content authored by unmigrated users, so we check
    // it here.
    $row->uid = db_select('migrate_map_user', 'u')
                ->fields('u', array('destid1'))
                ->condition('sourceid1', $row->uid)
                ->execute()
                ->fetchField();
    if (!$row->uid) {
      return FALSE;
    }
  }
}

class bieberfeveradmin_photoMigration extends bieberfeverNodeMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->dependencies[] = 'admin_gallery';

    $this->addFieldMapping('field_photo_asset', 'field_admin_photo')
         ->sourceMigration('File');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_photo_asset:preserve_files')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_photo_asset:title', 'field_admin_photo:title');
    $this->addFieldMapping('field_photo_category', 'field_admin_gallery')
         ->sourceMigration('admin_gallery');
    $this->addFieldMapping('field_photo_category:source_type')
         ->defaultValue('tid');

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:data',
        'field_photo_category:create_term', 'field_photo_category:ignore_case',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
    ));

    $this->addUnmigratedSources(array(
        'field_admin_photo:list', 'field_admin_photo:title',
    ));
  }
}

class bieberfeverfan_with_artist_galleryMigration extends bieberfeverNodeMigration {
  public function __construct($arguments) {
    $this->sourceFields['fwa_photos'] = 'Fan With Artist photos';
    parent::__construct($arguments);
    $this->dependencies[] = 'File';

    $this->removeFieldMapping('field_is_pro');
    $this->removeFieldMapping('migrate_redirects');
    $this->addFieldMapping('media_gallery_description', 'body');
    $this->addFieldMapping('media_gallery_description:format', 'format');
    $this->addFieldMapping('media_gallery_media', 'fwa_photos')
         ->sourceMigration('File');
    $this->addFieldMapping('media_gallery_media:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('media_gallery_media:preserve_files')
         ->defaultValue(1);
    $this->addFieldMapping('media_gallery_lightbox_extras')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'media_gallery_allow_download',
      'media_gallery_block_columns',
      'media_gallery_block_rows',
      'media_gallery_collection', 'media_gallery_collection:source_type',
        'media_gallery_collection:create_term', 'media_gallery_collection:ignore_case',
      'media_gallery_columns',
        'media_gallery_description:language',
      'media_gallery_expose_block',
      'media_gallery_format',
      'media_gallery_image_info_where',
        'media_gallery_media:language', 'media_gallery_media:data',
        'media_gallery_media:title',
      'media_gallery_rows',
    ));

    $this->addUnmigratedSources(array(
      'field_display_image', 'field_display_image:list',
      'log',
      'revision',
      'revision_uid',
      'upload', 'upload:description', 'upload:list', 'upload:weight',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $row->fwa_photos = Database::getConnection('default', $this->sourceConnection)
      ->select('content_type_fan_with_artist_photo', 'p')
      ->fields('p', array('field_fan_with_artist_photo_fid'))
      ->condition('p.field_fan_with_artist_gallery_nid', $row->nid)
      ->execute()
      ->fetchCol();
  }
}

class bieberfevermeet_greetMigration extends bieberfeverNodeMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';

    $this->removeFieldMapping('field_is_pro');
    $this->removeFieldMapping('migrate_redirects');
    $this->addFieldMapping('media_gallery_description', 'body');
    $this->addFieldMapping('media_gallery_description:format', 'format');
    $this->addFieldMapping('media_gallery_media', 'field_mg_photo')
         ->sourceMigration('File');
    $this->addFieldMapping('media_gallery_media:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('media_gallery_media:preserve_files')
         ->defaultValue(1);
    $this->addFieldMapping('media_gallery_lightbox_extras')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'media_gallery_allow_download',
      'media_gallery_block_columns',
      'media_gallery_block_rows',
      'media_gallery_collection', 'media_gallery_collection:source_type',
        'media_gallery_collection:create_term', 'media_gallery_collection:ignore_case',
      'media_gallery_columns',
        'media_gallery_description:language',
      'media_gallery_expose_block',
      'media_gallery_format',
      'media_gallery_image_info_where',
        'media_gallery_media:language', 'media_gallery_media:data',
        'media_gallery_media:title',
      'media_gallery_rows',
    ));

    $this->addUnmigratedSources(array(
        'field_mg_photo:list',
      'log',
      'revision',
      'revision_uid',
      'upload', 'upload:description', 'upload:list', 'upload:weight',
    ));
  }
}

class bieberfevercontest_mediaMigration extends bieberfeverNodeMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';

    $this->addFieldMapping('field_photo_asset', 'field_media_upload')
         ->sourceMigration('File');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_photo_asset:preserve_files')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_photo_category')
         ->defaultValue('Never Say Never Movie Poster');
    $this->addFieldMapping('field_photo_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:data',
          'field_photo_asset:title',
        'field_photo_category:source_type', 'field_photo_category:ignore_case',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
    ));

    $this->addUnmigratedSources(array(
      '11',
        'field_media_upload:list',
    ));
  }
}

abstract class bieberfeverflagMigration extends AANode6Migration {
  protected $flagName;
  public function __construct(array $arguments) {
    $this->flagName = $arguments['flag_name'];
    parent::__construct($arguments);
    $this->addFieldMapping($this->flagName)
         ->description('Populated in complete()');
  }

  public function complete($node, $row) {
    $query = Database::getConnection('default', $this->sourceConnection)
           ->select('flag_content', 'fc')
           ->fields('fc', array('uid'))
           ->condition('fc.content_type', 'node')
           ->condition('fc.content_id', $row->nid);
    $query->innerJoin('flags', 'f', "fc.fid=f.fid AND f.name='{$this->flagName}'");
    $uids = $query->execute()->fetchCol();
    foreach ($uids as $legacy_uid) {
      $uid = $this->handleSourceMigration('User', array($legacy_uid));
      if ($uid) {
        $account = user_load($uid);
        flag('flag', $this->flagName, $node->nid, $account);
      }
    }
  }
}

class bieberfevereventMigration extends bieberfeverflagMigration {
  public function __construct($arguments) {
    $this->sourceFields['state'] = 'State extracted from location';
    $this->sourceFields['country'] = 'Country extracted from location';
    $arguments['flag_name'] = 'rsvp';
    parent::__construct($arguments);

    $this->addFieldMapping('field_event_date', 'field_event_date_time');
    $this->addFieldMapping('field_event_venue', 'field_event_venue');
    $this->addFieldMapping('field_event_availability', 'field_mng_window');
    $this->addFieldMapping('field_event_city', 'field_event_location');
    $this->addFieldMapping('field_event_state', 'state');
    $this->addFieldMapping('field_event_country', 'country');
    $this->addFieldMapping('field_event_buylink', 'field_tour_buy')
         ->defaultValue('');
    $this->addFieldMapping('field_event_buylink:format')
         ->defaultValue(1);
    $this->addFieldMapping('field_event_maplink')
         ->defaultValue('');

    $this->removeFieldMapping('body:format');
    $this->removeFieldMapping('field_is_pro');

    $this->addUnmigratedDestinations(array(
      'field_event_address', 'field_event_address:language',
        'field_event_availability:language',
        'field_event_buylink:language',
      'field_event_category', 'field_event_category:source_type', 'field_event_category:create_term', 'field_event_category:ignore_case',
        'field_event_city:language',
        'field_event_country:language',
        'field_event_maplink:format', 'field_event_maplink:language',
        'field_event_state:language',
        'field_event_venue:language',
    ));

    $this->addUnmigratedSources(array(
      '3',
      'field_mng_enabled',
        'field_mng_window:value2',
      'field_region',
      'field_sale_date', 'field_sale_date:value2',
        'field_tour_buy:title', 'field_tour_buy:attributes',
      'format',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $this->parseLocation($row, 'field_event_location');
  }
}

class bieberfeverfan_photoMigration extends bieberfeverNodeMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';

    $this->addFieldMapping('field_photo_asset', 'field_fan_photo')
         ->sourceMigration('File');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_photo_category', 'field_fan_gallery_reference')
         ->description('Map to photo category')
         ->issueGroup(MigrateFieldMapping::ISSUE_PRIORITY_MEDIUM);

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:title', 'field_photo_asset:data',
        'field_photo_category:source_type', 'field_photo_category:create_term', 'field_photo_category:ignore_case',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
    ));

    $this->addUnmigratedSources(array(
        'field_fan_photo:list', 'field_fan_photo:data',
    ));
  }
}

class bieberfeverforumMigration extends bieberfeverflagMigration {
  public function __construct($arguments) {
    $arguments['flag_name'] = 'bookmarks';
    parent::__construct($arguments);
    $this->dependencies[] = '1Term';

    $this->addFieldMapping('taxonomy_forums', '1')
         ->sourceMigration('1Term');
    $this->addFieldMapping('taxonomy_forums:source_type')
         ->defaultValue('tid');

    $this->addUnmigratedDestinations(array(
        'taxonomy_forums:create_term', 'taxonomy_forums:ignore_case',
    ));
  }
}

class bieberfeverimageMigration extends bieberfeverNodeMigration {
  public function __construct($arguments) {
    $this->sourceFields['image_fid'] = 'Source image fid';
    parent::__construct($arguments);
    $this->dependencies[] = 'File';

    $this->addFieldMapping('field_photo_asset', 'image_fid')
         ->sourceMigration('File');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_photo_asset:preserve_files')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_photo_category')
         ->defaultValue('Official');

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language','field_photo_asset:title', 'field_photo_asset:data',
        'field_photo_category:source_type', 'field_photo_category:create_term', 'field_photo_category:ignore_case',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
    ));

    $this->addUnmigratedSources(array(
      '2',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $row->image_fid = Database::getConnection('default', $this->sourceConnection)
                      ->select('image', 'i')
                      ->fields('i', array('fid'))
                      ->condition('nid', $row->nid)
                      ->condition('image_size', '_original')
                      ->execute()
                      ->fetchField();
  }
}

class bieberfeverphotoMigration extends bieberfeverNodeMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->dependencies[] = 'fan_gallery';

    $this->addFieldMapping('field_photo_asset', 'field_image')
         ->sourceMigration('File');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_photo_asset:preserve_files')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_fan_photo_category', 'field_gallery')
         ->sourceMigration('fan_gallery');
    $this->addFieldMapping('field_fan_photo_category:source_type')
         ->defaultValue('tid');
    $this->removeFieldMapping('field_is_pro');

    $this->addUnmigratedDestinations(array(
        'field_photo_asset:language', 'field_photo_asset:title', 'field_photo_asset:data',
        'field_fan_photo_category:create_term', 'field_fan_photo_category:ignore_case',
      'field_fan_rating',
    ));

    $this->addUnmigratedSources(array(
        'field_image:list',
      'log',
      'revision',
      'revision_uid',
      'upload', 'upload:description', 'upload:list', 'upload:weight',
    ));
  }
}

class bieberfeverstoryMigration extends bieberfeverflagMigration {
  public function __construct(array $arguments) {
    $arguments['flag_name'] = 'bookmarks';
    parent::__construct($arguments);
    $this->dependencies[] = 'File';

    $this->addFieldMapping('field_blog_category', '3');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_blog_media', 'field_story_image')
         ->sourceMigration('File');
    $this->addFieldMapping('field_blog_media:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_blog_media:preserve_files')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'field_blog_category:source_type', 'field_blog_category:ignore_case',
      'field_blog_media:language', 'field_blog_media:title', 'field_blog_media:data',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
    ));

    $this->addUnmigratedSources(array(
        'field_story_image:list',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $term_field_name = '3';
    switch (reset($row->$term_field_name)) {
      case 1:
        $row->$term_field_name = 'Contests';
        break;
      case 2:
        $row->$term_field_name = 'News';
        break;
      case 7:
        $row->$term_field_name = 'General';
        break;
      default:
        $row->$term_field_name = '';
        break;
    }
    return TRUE;
    if ($row->field_story_image) {
      $new_fid = db_select('migrate_map_file', 'map')
                 ->fields('map', array('destid1'))
                 ->condition('sourceid1', $row->field_story_image)
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
        if (is_array($row->body)) {
          $row->body = $media_tag . $row->body[0];
        }
        else {
          $row->body = $media_tag . $row->body;
        }
        if (is_array($row->teaser)) {
          $row->teaser = $media_tag . $row->teaser[0];
        }
        else {
          $row->teaser = $media_tag . $row->teaser;
        }
      }
    }
  }
}

class bieberfeverCommentMigration extends AAComment6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    // Override the uid mapping - rather than leaving it to sourceMigration()
    // at mapping time, and setting a default value, we want to ignore any
    // comments with unresolved uid references, so we need to check in prepareRow().
    $this->addFieldMapping('uid', 'uid', FALSE);

    // Override to prevent attempt to look up sourceMigration, we'll get the
    // right nid in prepareRow().
    $this->addFieldMapping('nid', 'nid', FALSE);
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    // We want to ignore any comments authored by unmigrated users, so we check
    // it here.
    $row->uid = db_select('migrate_map_user', 'u')
                ->fields('u', array('destid1'))
                ->condition('sourceid1', $row->uid)
                ->execute()
                ->fetchField();
    if (!$row->uid) {
      return FALSE;
    }

    // Also ignore any comments attached to unimported nodes
    $row->nid = db_select('migrate_map_' . strtolower($this->nodeMigration), 'n')
                ->fields('n', array('destid1'))
                ->condition('sourceid1', $row->nid)
                ->execute()
                ->fetchField();
    if (!$row->nid) {
      return FALSE;
    }
  }
}

class bieberfeverCommentadmin_photoMigration extends bieberfeverCommentMigration {}
class bieberfeverCommenteventMigration extends bieberfeverCommentMigration {}
class bieberfeverCommentfan_with_artist_galleryMigration extends bieberfeverCommentMigration {}
class bieberfeverCommentforumMigration extends bieberfeverCommentMigration {}
class bieberfeverCommentphotoMigration extends bieberfeverCommentMigration {}
class bieberfeverCommentstoryMigration extends bieberfeverCommentMigration {}

class bieberfeverCommentuprofileMigration extends AAComment6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Find the comment_on_anything node for the target user. First, find the
    // author of the original uprofile node, and translate that uid to the target
    // uid.
    $uid = Database::getConnection('default', $this->sourceConnection)
           ->select('node', 'n')
           ->fields('n', array('uid'))
           ->condition('nid', $row->nid)
           ->execute()
           ->fetchField();
    $uid = db_select('migrate_map_user', 'map')
           ->fields('map', array('destid1'))
           ->condition('sourceid1', $uid)
           ->execute()
           ->fetchField();
    if ($uid) {
      $row->nid = db_select('field_data_field_comment', 'cp')
             ->fields('cp', array('field_comment_nid'))
             ->condition('entity_type', 'user')
             ->condition('entity_id', $uid)
             ->execute()
             ->fetchField();
      if (!$row->nid) {
        // Unresolved stubs have no comment_on_anything node, ignore.
        return FALSE;
      }
    }
    // We have orphaned comments - target uid has been deleted
    else {
      return FALSE;
    }
  }
}

class bieberfeverPrivateMessageMigration extends DrupalMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'User';

    $fields = array(
      'mid' => '',
      'author' => '',
      'subject' => '',
      'body' => '',
      'format' => '',
      'timestamp' => '',
      'recipients' => '',
      'is_new' => '',
    );
    $this->source = new MigrateSourceSQL($this->query(), $fields, NULL,
      array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationPrivateMsg();
    $this->map = new MigrateSQLMap($this->machineName,
       array(
         'mid' => array('type' => 'int',
                        'unsigned' => TRUE,
                        'not null' => TRUE,
                        'description' => 'Message ID',
                        'alias' => 's',
                       ),
       ),
       MigrateDestinationPrivateMsg::getKeySchema()
    );

    $this->addSimpleMappings(array('subject', 'body', 'timestamp',
                             'is_new'));
    $this->addFieldMapping('author', 'author')
         ->sourceMigration('User');
    $this->addFieldMapping('recipients', 'recipients')
         ->sourceMigration('User');
    $this->addFieldMapping('format')
         ->defaultValue('1');
    $this->addUnmigratedDestinations(array('path'));
    $this->addUnmigratedSources(array('format'));
  }

  protected function query() {
    $query = Database::getConnection('default', $this->sourceConnection)
               ->select('pm_message', 'm')
               ->fields('m');
    $query->innerJoin('users', 'u', 'm.author=u.uid');
    return $query;
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $row->body = strip_tags($row->body);

    $result = Database::getConnection('default', $this->sourceConnection)
                   ->select('pm_index', 'i')
                   ->fields('i', array('recipient', 'is_new'))
                   ->condition('i.mid', $row->mid)
                   ->condition('recipient', $row->author, '<>')
                   ->execute();
    $row->recipients = array();
    foreach ($result as $pm_row) {
      $row->recipients[] = $pm_row->recipient;
      // Plugin only supports one is_new value, not one per recipient, so for
      // the few multi-recipient cases we'll settle for the last value.
      $row->is_new = $pm_row->is_new;
    }
    if (empty($row->recipients)) {
      return FALSE;
    }
    return TRUE;
  }
}
