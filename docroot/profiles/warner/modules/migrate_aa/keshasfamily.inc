<?php
/**
 * @file
 * Migration implementations specific to keshasfamily.
 */

/**
 * Registration of this site's migrations.
 */
class keshasfamilyRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    parent::__construct($site_name, 6);
    $this->commonArguments['format_mappings'] =  array(
      '1' => '1',       // Filtered HTML
      '2' => '6',       // Full HTML
      '3' => 'script',  // PHP
      '4' => 'script',  // js and css
    );
    $this->roleArguments['role_mappings'] = array(
      'Fan Club' => 'fan club member',
      'Product Manager' => 'administrator',
      // @todo: Convert, Disciple, Missionary, News Posting?
    );
    $this->sourceDirectory = 'http://keshasfamily.com/sites/keshasfamily.com/files/';
  }

  /**
   * Register our migrations.
   */
  protected function register() {
    $this->registerRole();
    $this->registerPicture();
    $this->registerUser();
    $this->registerFile();
    $this->registerTerm('1', 'photo_category');
    $this->registerGallery('gallery', 'photo_category');
    $this->registerNode('gallery_image', 'photo');
    $this->registerComment('gallery_image', 'photo');
    $this->registerNode('link', 'link');
    $this->registerComment('link', 'link');
    $this->registerNode('official_video', 'video');
    $this->registerComment('official_video', 'video');
    $this->registerNode('photo', 'photo');
    $this->registerComment('photo', 'photo');
    $this->registerNode('release', 'album');
    $this->registerNode('release_track', 'audio');
    $this->registerNode('status', 'status');
    $this->registerComment('status', 'status');
    $this->registerNode('tour_date_vip', 'event');
    $this->registerNode('video', 'video');
    $this->registerComment('video', 'video');
    // Special handling because the legacy story type is being split into
    // two parts here - blog for admin-generated stories, and forum for
    // UGC stories. We register the UGC normally, but give the admin content
    // a distinct machine name while making sure we get the source_type right.
    $this->registerNode('story', 'forum');
    $this->registerComment('story', 'forum');
    $arguments = array(
      'source_type' => 'story',
    );
    $this->registerNode('blog', 'blog', $arguments);
  }
}

class keshasfamilyRoleMigration extends AARole6Migration {}

class keshasfamilyPictureMigration extends AAPicture6Migration {
  public function prepareRow($row) {
    $row->filepath = str_replace('sites/keshasfamily.com/files/', '', $row->filepath);
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
  }
}

class keshasfamilyUserMigration extends AAUser6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    // Known mappings
    $this->addFieldMapping('field_member_name', 'field_realname');
    $this->addFieldMapping('field_real_name', 'field_realname');
    $this->addFieldMapping('field_website', 'field_website');
    $this->addFieldMapping('field_website:title', 'field_website:title');
    $this->addFieldMapping('field_website:attributes', 'field_website:attributes');

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
      'field_nickname', 'field_nickname:summary', 'field_nickname:format', 'field_nickname:language',
      'field_postal_code', 'field_postal_code:language',
      'field_profile_14', 'field_profile_14:language',
      'field_profile_updated',
      'field_provider', 'field_provider:language',
        'field_real_name:language',
      'field_site_id', 'field_site_id:language',
      'field_state', 'field_state:language',
      'field_twitter_name', 'field_twitter_name:language',
      'field_type',
      'field_usage_type',
    ));

    $this->addUnmigratedSources(array(
      'field_user_location',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    // Do the same manipulation as the picture migration, so we match the
    // path in the map table.
    $row->picture = str_replace('sites/keshasfamily.com/files/', '', $row->picture);
  }

  public function prepare($account, $row) {
    // Make sure anyone with the Convert (6), Disciple (8), or Missionary (7)
    // legacy role gets destination role fan club member (26).
    if (is_array($row->roles)) {
      if (in_array(6, $row->roles) || in_array(7, $row->roles) || in_array(8, $row->roles)) {
        $account->roles[26] = 26;
      }
    }
  }
}

class keshasfamilyFileMigration extends AAFile6Migration {
  public function prepareRow($row) {
    $row->filepath = str_replace('sites/keshasfamily.com/files/', '', $row->filepath);
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
  }
}

class keshasfamily1TermMigration extends AATerm6Migration {}

class keshasfamilygalleryMigration extends AAGallery6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
  }
}

class keshasfamilygallery_imageMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->dependencies[] = 'gallery';

    $this->addFieldMapping('field_photo_asset', 'field_gallery_image')
         ->sourceMigration('File');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_photo_category', 'field_parent_gallery')
         ->sourceMigration('gallery');
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
        'field_gallery_image:list', 'field_gallery_image:data',
    ));
  }
}

class keshasfamilyphotoMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->dependencies[] = '1Term';

    $this->addFieldMapping('field_photo_asset', 'field_photo')
         ->sourceMigration('File');
    $this->addFieldMapping('field_photo_asset:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_photo_category', '1')
         ->sourceMigration('1Term');
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
        'field_photo:list', 'field_photo:data',
    ));
  }

}

class keshasfamilylinkMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_community_link', 'field_link');
    $this->addFieldMapping('field_community_link:title', 'field_link:title');
    $this->addFieldMapping('field_community_link:attributes', 'field_link:attributes');

    $this->removeFieldMapping('field_is_pro');
  }
}

class keshasfamilyrelease_trackMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_audio_category', 'field_release_track_parent');
    $this->addFieldMapping('field_audio_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'field_audio_asset', 'field_audio_asset:file_class', 'field_audio_asset:language',
        'field_audio_asset:destination_dir', 'field_audio_asset:destination_file',
        'field_audio_asset:file_replace', 'field_audio_asset:preserve_files',
        'field_audio_asset:source_dir', 'field_audio_asset:title', 'field_audio_asset:data',
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
      'field_audio', // No source data
      'field_buy_link', 'field_buy_link:title', 'field_buy_link:attributes', // No source data
      'field_track_number',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Use the album name as the audio category
    $row->field_release_track_parent =
      Database::getConnection('default', $this->sourceConnection)
        ->select('node', 'n')
        ->fields('n', array('title'))
        ->condition('nid', $row->field_release_track_parent)
        ->execute()
        ->fetchField();
  }
}

/**
 * Migration of release nodes to album nodes.
 */
class keshasfamilyreleaseMigration extends AANode6Migration {
  public function __construct($arguments) {
    $this->sourceFields['tracks'] = 'Track listing populated from body';
    $this->sourceFields['tracks_num'] = 'Track count populated from body';
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->dependencies[] = 'release_track';

    $this->addFieldMapping('field_recording_artist')
         ->defaultValue('Ke$ha');
    $this->addFieldMapping('field_tracks', 'tracks');
    $this->addFieldMapping('field_tracks:format')
         ->defaultValue('1'); // Filtered HTML
    $this->addFieldMapping('field_album_tracks_num', 'tracks_num');
    $this->addFieldMapping('field_album_date', 'field_release_date');
    $this->addFieldMapping('field_album_image', 'field_cover_art')
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
        'field_cover_art:list', 'field_cover_art:data',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    $result = Database::getConnection('default', $this->sourceConnection)
              ->select('content_type_release_track', 't')
              ->fields('t', array('nid'))
              ->condition('field_release_track_parent_nid', $row->nid)
              ->orderBy('field_track_number_value')
              ->execute();
    $row->tracks_num = 0;
    foreach ($result as $track_row) {
      $query = db_select('migrate_map_release_track', 'map');
      $query->innerJoin('node', 'n', 'map.destid1=n.nid');
      $audio_result = $query->fields('n', array('nid', 'title'))
                      ->condition('sourceid1', $track_row->nid)
                      ->execute()
                      ->fetchObject();
      if ($audio_result) {
        $title = trim($audio_result->title);
        $row->tracks[] =
          "<a href=\"/node/{$audio_result->nid}\">$title</a>";
        $row->tracks_num++;
      }
    }
  }
}

class keshasfamilystatusMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->removeFieldMapping('field_is_pro');
  }
}

/**
 * Migration of UGC story nodes to forum nodes.
 */
class keshasfamilystoryMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('body:summary', 'field_teaser', FALSE);
    $this->addFieldMapping('taxonomy_forums')
         ->defaultValue('Stories');
    $this->addFieldMapping('taxonomy_forums:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
        'taxonomy_forums:source_type', 'taxonomy_forums:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'field_story_image', 'field_story_image:list', 'field_story_image:data',
        'field_teaser:format',
      'teaser',
    ));
  }

  /**
   * Implements DrupalNodeMigration::query().
   *
   * @return QueryConditionInterface
   */
  protected function query() {
    $query = parent::query();
    // We exclude content authored by Fan Club Admin (uid=16) to arrive at
    // the UGC story content.
    $query->condition('n.uid', 16, '<>');
    return $query;
  }
}

/**
 * Migration of admin-authored story nodes to blog nodes.
 */
class keshasfamilyblogMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('body:summary', 'field_teaser', FALSE);
    $this->addFieldMapping('field_blog_category')
         ->defaultValue('Stories');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'field_story_image', 'field_story_image:list', 'field_story_image:data',
        'field_teaser:format',
      'teaser',
    ));
  }

  /**
   * Implements DrupalNodeMigration::query().
   *
   * @return QueryConditionInterface
   */
  protected function query() {
    $query = parent::query();
    // We take only content authored by Fan Club Admin (uid=16) to arrive at
    // the admin story content.
    $query->condition('n.uid', 16);
    return $query;
  }
}

/**
 * Migration of tour_date_vip nodes to event nodes.
 */
class keshasfamilytour_date_vipMigration extends AANode6Migration {
  public function __construct($arguments) {
    $this->sourceFields['state'] = 'State extracted from location';
    $this->sourceFields['country'] = 'Country extracted from location';
    parent::__construct($arguments);

    $this->addFieldMapping('field_event_venue', 'field_vip_venue');
    $this->addFieldMapping('field_event_date', 'field_vip_tourdate');
    $this->addFieldMapping('field_event_city', 'field_vip_location');
    $this->addFieldMapping('field_event_state', 'state');
    $this->addFieldMapping('field_event_country', 'country');
    $this->addFieldMapping('field_event_buylink', 'field_vip_buy_tickets')
         ->defaultValue('');
    $this->addFieldMapping('field_event_buylink:format')
         ->defaultValue('1');
    $this->addFieldMapping('field_event_maplink')
         ->defaultValue('');
    $this->addFieldMapping('field_event_category')
         ->defaultValue('VIP');
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
        'field_vip_buy_tickets:title', 'field_vip_buy_tickets:attributes',
      'field_vip_buy_vip', 'field_vip_buy_vip:title', 'field_vip_buy_vip:attributes',
      'format',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $this->parseLocation($row, 'field_vip_location');
    $row->field_tour_buy = $this->buildBuyLink($row, 'field_vip_buy_tickets');
  }
}

class keshasfamilyofficial_videoMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_video_asset', 'field_band_video');
    $this->addFieldMapping('field_video_asset:file_class')
         ->defaultValue('MigrateExtrasFileYoutube');
    $this->addFieldMapping('field_video_asset:title', 'field_band_video:title');
    $this->addFieldMapping('field_video_asset:data', 'field_band_video:data');
    $this->addFieldMapping('field_legacy_url', 'field_band_video');
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
        'field_video_asset:language',
        'field_video_category:source_type', 'field_video_category:ignore_case',
      'field_video_duration', 'field_video_duration:language',
    ));

    $this->addUnmigratedSources(array(
        'field_band_video:value', 'field_band_video:provider',
        'field_band_video:status', 'field_band_video:version',
        'field_band_video:description', 'field_band_video:duration',
    ));
  }
}

class keshasfamilyvideoMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_video_asset', 'field_video');
    $this->addFieldMapping('field_video_asset:file_class')
         ->defaultValue('MigrateExtrasFileYoutube');
    $this->addFieldMapping('field_video_asset:title', 'field_video:title');
    $this->addFieldMapping('field_video_asset:data', 'field_video:data');
    $this->addFieldMapping('field_legacy_url', 'field_video');
    $this->addFieldMapping('field_video_category')
         ->defaultValue('Fan Videos');
    $this->addFieldMapping('field_video_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
      'field_download_url', 'field_download_url:language',
        'field_legacy_url:language',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
        'field_video_asset:language',
        'field_video_category:source_type','field_video_category:ignore_case',
      'field_video_duration', 'field_video_duration:language',
    ));

    $this->addUnmigratedSources(array(
        'field_video:value', 'field_video:provider',
        'field_video:status', 'field_video:version',
        'field_video:description', 'field_video:duration',
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

class keshasfamilyCommentgallery_imageMigration extends AAComment6Migration {}
class keshasfamilyCommentlinkMigration extends AAComment6Migration {}
class keshasfamilyCommentofficial_videoMigration extends AAComment6Migration {}
class keshasfamilyCommentphotoMigration extends AAComment6Migration {}
class keshasfamilyCommentstatusMigration extends AAComment6Migration {}
class keshasfamilyCommentvideoMigration extends AAComment6Migration {}

class keshasfamilyCommentstoryMigration extends AAComment6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $dependencies[] = 'blog';
    // We cheat and use one comment migration for two node migrations, because
    // we've split the legacy story content type into two parts.
    $this->addFieldMapping('nid', 'nid', FALSE)
         ->sourceMigration(array('blog', 'story'));
  }
}

