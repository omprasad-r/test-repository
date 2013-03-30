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
    $this->registerGallery('fan_gallery', 'photo_category');
    $this->registerGallery('fan_with_artist_gallery', 'photo_category');
    $this->registerGallery('meet_greet', 'photo_category');
    $this->registerNode('admin_photo', 'photo');
    $this->registerComment('admin_photo', 'photo');
    $this->registerNode('contest_media', 'photo');
    $this->registerNode('event', 'event');
    $this->registerComment('event', 'event');
    $this->registerNode('fan_with_artist_photo', 'photo');
    $this->registerComment('fan_with_artist_photo', 'photo');
    $this->registerNode('forum', 'forum');
    $this->registerComment('forum', 'forum');
    $this->registerNode('image', 'photo');
    $this->registerNode('photo', 'photo');
    $this->registerComment('photo', 'photo');
    // @todo: product, product_kit
    $this->registerNode('story', 'blog');
    $this->registerComment('story', 'blog');
    // Special comment registration, to avoid depending on non-existent
    // uprofile migration
    $arguments = $this->commonArguments + $this->commentArguments + array(
      'source_type' => 'uprofile',
      'destination_type' => 'comment_on_anything',
    );
    $this->registerMigration("Commentuprofile", $arguments);
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
    parent::__construct($arguments);

    // Known mappings
    $this->addFieldMapping('field_member_name', 'field_name');
    $this->addFieldMapping('field_nickname', 'field_name');
    $this->addFieldMapping('field_about_me', 'field_about_me');
    $this->addFieldMapping('field_gender', 'field_gender');
    $this->addFieldMapping('field_hobbies', 'field_interests');
    $this->addFieldMapping('field_country', 'profile_country');
    $this->addFieldMapping('field_birthdate', 'profile_user_birthday');

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

    // Birthdays are serialized arrays, convert to a datetime string
    if (!empty($row->profile_user_birthday)) {
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

class bieberfeverfan_with_artist_galleryMigration extends AAGallery6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->addFieldMapping('field_is_pro', NULL, FALSE)
         ->defaultValue(0);
  }
}

class bieberfevermeet_greetMigration extends AAGallery6Migration {
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
    $this->addFieldMapping('field_photo_category', 'field_admin_gallery')
         ->sourceMigration('admin_gallery');
    $this->addFieldMapping('field_photo_category:source_type')
         ->defaultValue('tid');

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:title', 'field_photo_asset:data',
        'field_photo_category:create_term', 'field_photo_category:ignore_case',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
    ));

    $this->addUnmigratedSources(array(
        'field_admin_photo:list', 'field_admin_photo:data',
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
    $this->addFieldMapping('field_photo_category')
         ->defaultValue('Never Say Never Movie Poster');
    $this->addFieldMapping('field_photo_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:title', 'field_photo_asset:data',
        'field_photo_category:source_type', 'field_photo_category:ignore_case',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
    ));

    $this->addUnmigratedSources(array(
      '11',
        'field_media_upload:list', 'field_media_upload:data',
    ));
  }
}

class bieberfevereventMigration extends bieberfeverNodeMigration {
  public function __construct($arguments) {
    $this->sourceFields['state'] = 'State extracted from location';
    $this->sourceFields['country'] = 'Country extracted from location';
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

class bieberfeverfan_with_artist_photoMigration extends bieberfeverNodeMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->dependencies[] = 'fan_with_artist_gallery';

    $this->addFieldMapping('field_photo_asset', 'field_fan_with_artist_photo')
         ->sourceMigration('File');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_photo_category', 'field_fan_with_artist_gallery')
         ->sourceMigration('fan_with_artist_gallery');
    $this->addFieldMapping('field_photo_category:source_type')
         ->defaultValue('tid');

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:title', 'field_photo_asset:data',
        'field_photo_category:create_term', 'field_photo_category:ignore_case',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
    ));

    $this->addUnmigratedSources(array(
        'field_fan_with_artist_photo:list', 'field_fan_with_artist_photo:data',
    ));
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

class bieberfeverforumMigration extends bieberfeverNodeMigration {
  public function __construct($arguments) {
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
    $this->addFieldMapping('field_photo_category', 'field_gallery')
         ->sourceMigration('fan_gallery');
    $this->addFieldMapping('field_photo_category:source_type')
         ->defaultValue('tid');

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:title', 'field_photo_asset:data',
        'field_photo_category:create_term', 'field_photo_category:ignore_case',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
    ));

    $this->addUnmigratedSources(array(
        'field_image:list', 'field_image:data',
    ));
  }
}

class bieberfeverstoryMigration extends bieberfeverNodeMigration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';

    $this->addFieldMapping('field_blog_category', '3');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
        'field_blog_category:source_type', 'field_blog_category:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'field_story_image', 'field_story_image:list', 'field_story_image:data',
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
class bieberfeverCommentfan_with_artist_photoMigration extends bieberfeverCommentMigration {}
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
    }
    // We have orphaned comments - target uid has been deleted
    else {
      return FALSE;
    }
  }
}
