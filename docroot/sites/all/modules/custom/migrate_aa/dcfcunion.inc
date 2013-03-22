<?php
/**
 * @file
 * Mappings specific to dcfcunion.
 */

class dcfcunionRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    $default_uid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'Fan Club Admin')
                   ->execute()
                   ->fetchField();
    parent::__construct($site_name, 6, $default_uid);
    $this->commonArguments['format_mappings'] =  array(
      '1' => '1',
      '5' => '6',
      '2' => '6',
      '3' => 'script',
      '6' => '11',
    );
    $this->roleArguments['role_mappings'] = array(
      'Fan Club' => 'fan club member',
      'Fan Club Admin' => 'administrator',
      'user admin' => 'administrator',
      'reminder--fill_out_user_profile' => 'authenticated user',
    );
    $this->sourceDirectory = 'http://dcfcunion.com/sites/dcfcunion.com/files/';
  }

  protected function register() {
    $this->registerRole();
    $this->registerPicture();
    $this->registerUser();
    $this->registerFile();

    // Forums vocabulary
    $this->registerTerm('2', 'forums');
    $this->registerGallery('admin_gallery', 'photo_category');
    $this->registerNode('admin_photo', 'photo');
    $this->registerNode('event', 'event');
    $this->registerComment('event', 'event');
    $this->registerGallery('fan_gallery', 'photo_category');
    $this->registerNode('fan_photo', 'photo');
    $this->registerNode('forum', 'forum');
    $this->registerComment('forum', 'forum');

//    $this->registerNode('poll', 'poll');
    $this->registerNode('story', 'blog');
    $this->registerComment('story', 'blog');
  }
}

class dcfcunionRoleMigration extends AARole6Migration {}
class dcfcunionadmin_galleryMigration extends AAGallery6Migration {}

class dcfcunionadmin_photoMigration extends AANode6Migration {
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

class dcfcunioneventMigration extends AANode6Migration {
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
         ->defaultValue('1');
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
      'field_additional_acts',
      'field_mng_enabled',
        'field_mng_window:value2',
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

class dcfcunionfan_galleryMigration extends AAGallery6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->addFieldMapping('field_is_pro', NULL, FALSE)
         ->defaultValue(0);
  }
}

class dcfcunionfan_photoMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->dependencies[] = 'fan_gallery';

    $this->addFieldMapping('field_photo_asset', 'field_fan_photo')
         ->sourceMigration('File');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_photo_category', 'field_fan_gallery_reference')
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
        'field_fan_photo:list', 'field_fan_photo:data',
    ));
  }
}

class dcfcunionforumMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = '2Term';

    $this->addFieldMapping('taxonomy_forums', '2')
         ->sourceMigration('2Term');
    $this->addFieldMapping('taxonomy_forums:source_type')
         ->defaultValue('tid');

    $this->addUnmigratedDestinations(array(
      'taxonomy_forums:create_term', 'taxonomy_forums:ignore_case',
    ));
  }
}

class dcfcunionpageMigration extends AANode6Migration {}
class dcfcunionpollMigration extends AANode6Migration {}
class dcfcunionFileMigration extends AAFile6Migration {}
class dcfcunionPictureMigration extends AAPicture6Migration {}

class dcfcunionUserMigration extends AAUser6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    // Known mappings
    $this->addFieldMapping('field_member_name', 'field_name');
    $this->addFieldMapping('field_nickname', 'field_name');
    $this->addFieldMapping('field_about_me', 'field_about_me');
    $this->addFieldMapping('field_favorite_bands', 'field_favorite_bands');
    $this->addFieldMapping('field_gender', 'field_gender');
    $this->addFieldMapping('field_hobbies', 'field_interests');
    $this->addFieldMapping('field_twitter_name', 'field_twitter');

    // Provide a blank slate for all these fields.
    $this->addUnmigratedDestinations(array(
        'field_about_me:language',
      'field_birthdate',
      'field_city', 'field_city:language',
      'field_country',
      'field_eos_id',
        'field_favorite_bands:language',
        'field_hobbies:language',
        'field_member_name:language',
        'field_nickname:language',
      'field_postal_code', 'field_postal_code:language',
      'field_profile_14', 'field_profile_14:language',
      'field_profile_updated',
      'field_provider', 'field_provider:language',
      'field_site_id', 'field_site_id:language',
      'field_state', 'field_state:language',
        'field_twitter_name:language',
      'field_type',
      'field_usage_type',
    ));

    // No actual profile data in the source
    $this->addUnmigratedSources(array(
        'field_about_me:format',
      'field_aim',
      'field_blog', 'field_blog:title', 'field_blog:attributes',
      'field_children',
      'field_ethnicity',
      'field_facebook', 'field_facebook:title', 'field_facebook:attributes',
      'field_favorite_dc_album',
      'field_favorite_dc_moment',
      'field_favorite_dc_song',
      'field_google',
      'field_homepage', 'field_homepage:title', 'field_homepage:attributes',
      'field_icq',
       'field_interests:format',
      'field_linkedin', 'field_linkedin:title', 'field_linkedin:attributes',
      'field_location',
      'field_msn',
      'field_myspace', 'field_myspace:title', 'field_myspace:attributes',
      'field_occupation',
      'field_orientation',
      'field_politcal_leaning',
      'field_religion',
      'field_skype',
      'field_status',
        'field_twitter:title', 'field_twitter:attributes',
      'field_yahoo',
      'field_zodiac_sign',
      'user_aim',
      'user_from',
      'user_icq',
      'user_interests',
      'user_occ',
      'user_website',
      'user_yim',
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

class dcfcunionstoryMigration extends AANode6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->addUnmigratedDestinations(array(
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
      'field_blog_category', 'field_blog_category:source_type', 'field_blog_category:create_term', 'field_blog_category:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'field_full_embed_code', 'field_full_embed_code:format',
      'field_teaser_embed_code', 'field_teaser_embed_code:format',
      '3'));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    if (!empty($row->field_full_embed_code)) {
      $row->body = $row->field_full_embed_code . "\n<br>\n" . $row->body;
    }
  }
}

class dcfcunion2TermMigration extends AATerm6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
  }
}

class dcfcunionCommenteventMigration extends AAComment6Migration {}
class dcfcunionCommentforumMigration extends AAComment6Migration {}
class dcfcunionCommentstoryMigration extends AAComment6Migration {}
