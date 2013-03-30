<?php

/**
 * @file
 * Mappings specific to meatloaf.
 */

class meatloafRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    parent::__construct($site_name, 6, 21506);
    $this->commonArguments['format_mappings'] =  array(
      'foo' => 'bar',
      '0' => '6',
      '1' => '1',
      '2' => '6',
      '3' => 'script',
    );
    // Let Forum Moderator, Newsletter migrate as-is
    $this->roleArguments['role_mappings'] = array(
      'Fan Club' => 'fan club member',
      'Product Manager' => 'administrator',
    );
    $this->sourceDirectory =
      'http://meatloaf.net/sites/meatloaf.net/files/imagecache/pic_node_full/';
  }

  protected function register() {
    $this->registerRole();
    $this->registerUser();
    $this->registerFile();

    $this->registerTerm('4', 'blog_category');
    $this->registerGallery('admin_gallery', 'photo_category');
    $this->registerNode('admin_photo', 'photo');
    $this->registerNode('event', 'event');
    $this->registerNode('story', 'blog');
    $this->registerNode('youtube_video', 'video');
    $this->registerNode('release', 'album');
  }
}

class meatloafRoleMigration extends AARole6Migration {}

class meatloafadmin_galleryMigration extends AAGallery6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_is_pro', NULL, FALSE)
         ->defaultValue(TRUE);
  }
}

class meatloafeventMigration extends AANode6Migration {
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
    $this->addFieldMapping('field_event_category')
         ->defaultValue('Tour Dates');
    $this->addFieldMapping('field_event_category:create_term')
         ->defaultValue(TRUE);

    $this->removeFieldMapping('body:format');
    $this->removeFieldMapping('field_is_pro');

    $this->addUnmigratedDestinations(array(
      'field_event_address', 'field_event_address:language',
        'field_event_availability:language',
        'field_event_buylink:language',
        'field_event_category:source_type', 'field_event_category:ignore_case',
        'field_event_city:language',
        'field_event_country:language',
        'field_event_maplink:format', 'field_event_maplink:language',
        'field_event_state:language',
        'field_event_venue:language',
    ));

    $this->addUnmigratedSources(array(
      '4',
      'field_mng_enabled',
        'field_mng_window:value2',
      'field_set_list', 'field_set_list:format',
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

class meatloafadmin_photoMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->dependencies[] = 'admin_gallery';
    $this->defaultUid = 21506;
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

class meatloafFileMigration extends AAFile6Migration {}

class meatloafUserMigration extends AAUser6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    // Known mappings
    $this->addFieldMapping('field_member_name', 'name');
    $this->addFieldMapping('field_nickname', 'name');

    // Provide a blank slate for all these fields.
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
      'path'
    ));

    // No actual profile data in the source
    $this->addUnmigratedSources(array(
    ));
  }

  protected function query() {
    $query = parent::query();
    $query->condition('name', 'Site Admin', '<>');
    return $query;
  }
}

class meatloafstoryMigration extends AANode6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = '4Term';
    $this->dependencies[] = 'File';
    $this->defaultUid = 21506;
    $this->addFieldMapping('field_blog_category', '4')
         ->sourceMigration('4Term');
    $this->addFieldMapping('field_blog_category:source_type')
         ->defaultValue('tid');

    $this->addUnmigratedDestinations(array(
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
        'field_blog_category:create_term', 'field_blog_category:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'field_story_image', 'field_story_image:list', 'field_story_image:data',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Fix known funky characters
    if ($row->nid == 182) {
      $row->body = str_replace('“', '"', $row->body);
    }
    // If the former image field was populated, prepend the image
    if (isset($row->field_story_image)) {
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
        $row->body = $media_tag . $row->body[0];
      }
    }
  }
}

class meatloafyoutube_videoMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->defaultUid = 21506;
    $this->addFieldMapping('field_video_asset', 'field_video_url');
    $this->addFieldMapping('field_video_asset:file_class')
         ->defaultValue('MigrateExtrasFileYoutube');
    $this->addFieldMapping('field_legacy_url', 'field_video_url');
    $this->addFieldMapping('field_video_category')
         ->defaultValue('Official Videos');
    $this->addFieldMapping('field_video_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
      'field_download_url', 'field_download_url:language',
        'field_legacy_url:language',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
        'field_video_asset:language', 'field_video_asset:title', 'field_video_asset:data',
        'field_video_category:source_type','field_video_category:ignore_case',
      'field_video_duration', 'field_video_duration:language',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Some of the base values are oembeds or iframes, but the value subfield
    // is always valid when present.
    $value_name = 'field_video:value';
    if (isset($row->$value_name)) {
      $row->field_video = 'http://www.youtube.com/watch?v=' . $row->$value_name;
    }
  }
}

/**
 * Migration of release nodes to album nodes.
 */
class meatloafreleaseMigration extends AANode6Migration {
  public function __construct($arguments) {
    $this->sourceFields['tracks'] = 'Track listing populated from body';
    $this->sourceFields['tracks_num'] = 'Track count populated from body';
    parent::__construct($arguments);
    $this->dependencies[] = 'File';

    $this->addFieldMapping('field_recording_artist')
         ->defaultValue('Meatloaf');
    $this->addFieldMapping('field_tracks', 'tracks');
    $this->addFieldMapping('field_tracks:format')
         ->defaultValue('1'); // Filtered HTML
    $this->addFieldMapping('field_album_tracks_num', 'tracks_num');
    $this->addFieldMapping('field_album_date', 'field_release_date');
    $this->addFieldMapping('field_album_image', 'field_release_cover')
         ->sourceMigration('File');
    $this->addFieldMapping('field_album_image:file_class')
         ->defaultValue('MigrateFileFid');

    $this->addUnmigratedDestinations(array(
        'field_album_image:language', 'field_album_image:alt', 'field_album_image:title',
      'field_album_subtitle', 'field_album_subtitle:language',
      'field_audio_embed', 'field_audio_embed:format', 'field_audio_embed:language',
      'field_audio_player', 'field_audio_player:file_class', 'field_audio_player:language',
        'field_audio_player:destination_dir', 'field_audio_player:destination_file',
        'field_audio_player:file_replace', 'field_audio_player:preserve_files',
        'field_audio_player:source_dir', 'field_audio_player:title', 'field_audio_player:data',
      'field_buy_links', 'field_buy_links:format', 'field_buy_links:language',
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
      'field_rating',
        'field_recording_artist:language',
      'field_secondary_title', 'field_secondary_title:language',
        'field_tracks:language',
    ));

    $this->addUnmigratedSources(array(
      'field_buy_link', 'field_buy_link:title', 'field_buy_link:attributes',
        'field_release_cover:list', 'field_release_cover:data',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Convert body text into a list of tracks
    $row->tracks_num = preg_match_all('|[0-9]+\. ([^\n]+)|i',
      $row->body, $matches);
    foreach ($matches[1] as $match) {
      $row->tracks[] = $match;
    }

    $row->body = '';
  }
}

class meatloaf4TermMigration extends AATerm6Migration {}
