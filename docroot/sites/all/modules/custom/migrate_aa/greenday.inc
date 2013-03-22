<?php
/**
 * @file
 * Migration implementations specific to greenday.
 */

/**
 * Registration of this site's migrations.
 */
class greendayRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    parent::__construct($site_name, 7);
    $this->commonArguments['format_mappings'] =  array(
      'filtered_html' => '1',
      'plain_text' => '11',
      'full_html' => '6',
    );
    $this->roleArguments['role_mappings'] = array(
      'administrator' => 'administrator',
      'Product Manager' => 'administrator',
    );
    $this->sourceDirectory = 'http://greenday.com/sites/greenday.com/files/';
  }

  /**
   * Register our migrations.
   */
  protected function register() {
    $this->registerRole();
    $this->registerUser();
    $this->registerFile();
    $this->registerTerm('tags', 'tags');
    $this->registerGallery('gallery', 'photo_category');
    $this->registerGallery('video_gallery', 'video_category');
    $this->registerNode('american_idiot_news', 'blog');
    $this->registerNode('article', 'blog');
    $this->registerNode('broadway_date', 'event');
    $this->registerNode('gallery_image', 'photo');
    $this->registerNode('tour_date', 'event');
    $this->registerNode('video', 'video');
    $arguments = $this->commonArguments + array(
      'user_migration' => 'User',
      'default_uid' => $this->defaultUid,
    );
    $this->registerMigration('Youtube', $arguments);
  }
}

class greendayRoleMigration extends AARole7Migration {}

class greendayUserMigration extends AAUser7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    // Known mappings
    $this->addFieldMapping('field_member_name', 'name');
    $this->addFieldMapping('field_nickname', 'name');

    $this->addUnmigratedDestinations(array(
      'field_about_me', 'field_about_me:language',
      'field_birthdate',
      'field_city', 'field_city:language',
      'field_country',
      'field_eos_id',
      'field_favorite_bands', 'field_favorite_bands:language',
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

    // No actual profile data in the source
    $this->addUnmigratedSources(array('field_bio', 'field_fav_song', 'field_name',
                                'field_user_location', 'uuid'));
  }
}

class greendayFileMigration extends AAFile7Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->addUnmigratedSources(array('uuid'));
  }
}

class greendayYoutubeMigration extends AAYoutube7Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->addUnmigratedSources(array('uuid'));
  }
}

class greendaytagsTermMigration extends AATerm7Migration { }

class greendaygalleryMigration extends AAGallery7Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
  }
}

class greendayvideo_galleryMigration extends AAGallery7Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
  }
}

class greendayamerican_idiot_newsMigration extends AANode7Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_blog_category')
         ->defaultValue('General');
    $default_uid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'Brian')
                   ->execute()
                   ->fetchField();
    $this->addFieldMapping('uid', NULL, FALSE)
         ->defaultValue($default_uid);

    $this->addUnmigratedDestinations(array(
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
        'field_blog_category:source_type', 'field_blog_category:create_term', 'field_blog_category:ignore_case',
    ));
  }
}

class greendayarticleMigration extends AANode7Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'tagsTerm';
    $this->dependencies[] = 'File';

    $this->addFieldMapping('field_tags', 'field_tags')
         ->sourceMigration('tagsTerm');
    $this->addFieldMapping('field_tags:source_type')
         ->defaultValue('tid');
    $this->addFieldMapping('field_blog_category')
         ->defaultValue('General');
    $default_uid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'Brian')
                   ->execute()
                   ->fetchField();
    $this->addFieldMapping('uid', NULL, FALSE)
         ->defaultValue($default_uid);

    $this->addUnmigratedDestinations(array(
        'field_tags:create_term', 'field_tags:ignore_case',
        'field_blog_category:source_type', 'field_blog_category:create_term', 'field_blog_category:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'field_image', 'field_image:alt', 'field_image:title', 'field_image:width',
        'field_image:height',
      'field_image_caption',
      'field_image_credits',
      'uid',
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
  }
}

class greendaybroadway_dateMigration extends AANode7Migration {
  public function __construct($arguments) {
    $this->sourceFields['state'] = 'State extracted from location';
    $this->sourceFields['country'] = 'Country extracted from location';
    parent::__construct($arguments);

    $this->addFieldMapping('field_event_date', 'field_when');
    $this->addFieldMapping('field_event_city', 'field_where');
    $this->addFieldMapping('field_event_state', 'state');
    $this->addFieldMapping('field_event_country', 'country');
    $this->addFieldMapping('field_event_category')
         ->defaultValue('Broadway Date');
    $this->addFieldMapping('field_event_category:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_event_venue', 'field_brdwy_venue');
    $this->addFieldMapping('field_event_buylink', 'field_ticket_info')
         ->defaultValue('');
    $this->addFieldMapping('field_event_buylink:format')
         ->defaultValue('1');
    $this->addFieldMapping('field_event_maplink')
         ->defaultValue('');
    $default_uid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'Brian')
                   ->execute()
                   ->fetchField();
    $this->addFieldMapping('uid', NULL, FALSE)
         ->defaultValue($default_uid);

    $this->removeFieldMapping('field_is_pro');
    $this->removeFieldMapping('body:format');
    $this->addFieldMapping('body', NULL, FALSE);
    $this->addFieldMapping('body:summary', NULL, FALSE);

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
      'field_brdwy_label',  // No source data
        'field_ticket_info:title', 'field_ticket_info:attributes',
        'field_when:value2',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $this->parseLocation($row, 'field_where');
    $this->buildBuyLink($row, 'field_ticket_info');
  }
}

class greendaygallery_imageMigration extends AANode7Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'gallery';
    $this->dependencies[] = 'File';

    $this->addFieldMapping('field_photo_asset', 'field_gallery_image')
         ->sourceMigration('File');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_photo_asset:title', 'field_gallery_image:title');
    $this->addFieldMapping('field_photo_category', 'field_gallery')
         ->sourceMigration('gallery');
    $this->addFieldMapping('field_photo_category:source_type')
         ->defaultValue('tid');
    $this->addFieldMapping('field_secondary_title', 'field_caption');
    $this->addFieldMapping('field_copyright_name', 'field_credits');
    $default_uid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'Brian')
                   ->execute()
                   ->fetchField();
    $this->addFieldMapping('uid', NULL, FALSE)
         ->defaultValue($default_uid);

    $this->addUnmigratedDestinations(array(
        'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
        'field_photo_asset:language', 'field_photo_asset:data',
        'field_photo_category:create_term', 'field_photo_category:ignore_case',
      'field_rating',
        'field_secondary_title:language',
    ));

    $this->addUnmigratedSources(array(
        'field_gallery_image:alt', 'field_gallery_image:width', 'field_gallery_image:height',
    ));
  }
}

class greendaytour_dateMigration extends AANode7Migration {
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
    $default_uid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'Brian')
                   ->execute()
                   ->fetchField();
    $this->addFieldMapping('uid', NULL, FALSE)
         ->defaultValue($default_uid);

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
      'field_presale_dat',  // No source data
      'field_tour_label',   // No source data
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

class greendayvideoMigration extends AANode7Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'video_gallery';
    $this->dependencies[] = 'Youtube';

    $this->addFieldMapping('field_video_asset', 'field_url')
         ->sourceMigration('Youtube');
    $this->addFieldMapping('field_video_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_video_asset:title', 'field_url:title');
    $this->addFieldMapping('field_video_asset:data', 'field_url:data');
    $this->addFieldMapping('field_video_category', 'field_vi_galery')
         ->sourceMigration('video_gallery');
    $this->addFieldMapping('field_video_category:source_type')
         ->defaultValue('tid');
    $this->addFieldMapping('field_video_category:create_term')
         ->defaultValue(TRUE);
    $default_uid = db_select('users', 'u')
                   ->fields('u', array('uid'))
                   ->condition('name', 'Brian')
                   ->execute()
                   ->fetchField();
    $this->addFieldMapping('uid', NULL, FALSE)
         ->defaultValue($default_uid);

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
      'field_download_url', 'field_download_url:language',
      'field_legacy_url', 'field_legacy_url:language',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
        'field_video_asset:language',
        'field_video_category:ignore_case',
      'field_video_duration', 'field_video_duration:language',
    ));
  }
}
