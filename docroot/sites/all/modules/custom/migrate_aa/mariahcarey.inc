<?php

/**
 * @file
 * Mappings specific to mariahcarey.
 */

class mariahcareyRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    parent::__construct($site_name, 6, 21581);
    $this->commonArguments['format_mappings'] =  array(
      '1' => '1',
      '2' => '6',
      '3' => 'script',
      '5' => '1',
      '11' => 'script',
    );
    // Let Forum Moderator, Newsletter migrate as-is
    $this->roleArguments['role_mappings'] = array(
      'administrator' => 'administrator',
      'Customer Service' => 'administrator',
      'Fan Club' => 'fan club member',
      'Product Manager' => 'administrator',
    );
    $this->sourceDirectory =
      'http://cache.mariahcarey.com/sites/mariahcarey.com/files/imagecache/pic_node_full/';
  }

  protected function register() {
    $this->registerRole();
    $this->registerUser();
    $this->registerFile();

    /*
     * Nodes: banner, extras_file, faq, webform
     * Comments: faq
     * Vocabularies: 12 (FAQ), 14 (Extras), 15 (Album Type)
     */
    $this->registerTerm('4', 'video_category');
    $this->registerTerm('5', 'photo_category');
    $this->registerGallery('admin_gallery', 'photo_category');
    $this->registerNode('admin_photo', 'photo');
    $this->registerNode('event', 'blog');
    $this->registerNode('story', 'blog');
    $this->registerComment('story', 'blog');
    $this->registerNode('video', 'video');
    $this->registerNode('release', 'album');
    $this->registerComment('release', 'album');
    $this->registerNode('release_track', 'audio');
    $this->registerComment('release_track', 'audio');
  }
}

class mariahcareyRoleMigration extends AARole6Migration {}

class mariahcareyadmin_galleryMigration extends AAGallery6Migration {}

class mariahcarey4TermMigration extends AATerm6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->addFieldmapping('field_is_pro')
         ->defaultValue(1);
  }
}

class mariahcarey5TermMigration extends AATerm6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->addFieldmapping('field_is_pro')
         ->defaultValue(1);
  }
}

class mariahcareyeventMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->defaultUid = 21581;

    $this->addFieldMapping('field_blog_category')
         ->defaultValue('Events');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'field_event_date_time',
      'field_event_location',
      'field_event_venue',
      'field_fan_club_buy', 'field_fan_club_buy:title', 'field_fan_club_buy:attributes',
      'field_fan_end_date',
      'field_fan_sale_date',
      'field_mng_enabled',
      'field_mng_window', 'field_mng_window:value2',
      'field_sale_date', 'field_sale_date:value2',
       'field_tour_buy', 'field_tour_buy:title', 'field_tour_buy:attributes',
      'field_vip_buy', 'field_vip_buy:title', 'field_vip_buy:attributes',
      'format',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    if (is_array($row->body)) {
      $row->body = reset($row->body);
    }
    $row->body = "<strong>Event date:</strong> {$row->field_event_date_time}<br />\n"
      . "<strong>Venue:</strong> {$row->field_event_venue}<br />\n"
      . "<strong>Location:</strong> {$row->field_event_location}<br />\n"
      . $row->body;
  }
}

class mariahcareyadmin_photoMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->dependencies[] = 'admin_gallery';
    $this->defaultUid = 21581;
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
      'field_custom_thumbnail', 'field_custom_thumbnail:list', 'field_custom_thumbnail:data',
    ));
  }
}

class mariahcareyFileMigration extends AAFile6Migration {}

class mariahcareyUserMigration extends AAUser6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    // Known mappings
    $this->addFieldMapping('field_member_name', 'name');
    $this->addFieldMapping('field_nickname', 'name');
    $this->addFieldMapping('field_country', 'profile_country')
         ->callbacks(array($this, 'handleCountry'));
    $this->addFieldMapping('field_state', 'profile_state_province');
    $this->addFieldMapping('field_birthdate', 'profile_birth_date');

    // Provide a blank slate for all these fields.
    $this->addUnmigratedDestinations(array(
      'field_about_me', 'field_about_me:language',
      'field_city', 'field_city:language',
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
        'field_state:language',
      'field_twitter_name', 'field_twitter_name:language',
      'field_type',
      'field_usage_type',
      'path'
    ));
  }

  protected function query() {
    $query = parent::query();
    $query->condition('name', 'Fan Club Admin', '<>');
    return $query;
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    if (isset($row->profile_birth_date) && $row->profile_birth_date == 'N;') {
      $row->profile_birth_date = '';
    }
    elseif (!empty($row->profile_birth_date)) {
      $birthday = unserialize($row->profile_birth_date);
      if (is_array($birthday)) {
        $row->profile_birth_date = $birthday['year'] . '/' . $birthday['month'] .
                                 '/' . $birthday['day'];
      }
      // Sometimes the unserialized value is just a string: yyyy-mm-dd.
      else {
        $row->profile_birth_date = $birthday;
      }
    }
  }

}

class mariahcareystoryMigration extends AANode6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->defaultUid = 21581;
    $this->addFieldMapping('field_blog_image', 'field_story_image')
         ->sourceMigration('File');
    $this->addFieldMapping('field_blog_image:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_blog_image:preserve_files')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_blog_category')
         ->defaultValue('News');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(1);
    $this->addFieldMapping('body:summary', 'field_teaser', FALSE);

    $this->addUnmigratedDestinations(array(
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
        'field_blog_category:source_type', 'field_blog_category:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      '11',
      'field_story_image:list',
        'field_teaser:format',
      'teaser',
    ));
  }

}

class mariahcareyvideoMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->defaultUid = 21581;
    $this->dependencies[] = '4Term';
    $this->addFieldMapping('field_video_category', '4')
         ->sourceMigration('4Term');
    $this->addFieldMapping('field_video_category:source_type')
         ->defaultValue('tid');
    $this->addFieldMapping('body:format', NULL, FALSE)
         ->defaultValue('6');

    $this->addUnmigratedDestinations(array(
      'field_video_asset', 'field_video_asset:file_class', 'field_video_asset:destination_dir',
        'field_video_asset:destination_file', 'field_video_asset:file_replace',
        'field_video_asset:preserve_files', 'field_video_asset:source_dir',
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
      'field_download_url', 'field_download_url:language',
      'field_legacy_url', 'field_legacy_url:language',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
        'field_video_asset:language', 'field_video_asset:title', 'field_video_asset:data',
        'field_video_category:create_term','field_video_category:ignore_case',
      'field_video_duration', 'field_video_duration:language',
    ));

    $this->addUnmigratedSources(array(
      'field_caption',
      'field_embedded_video', 'field_embedded_video:value', 'field_embedded_video:provider',
        'field_embedded_video:data', 'field_embedded_video:status',
        'field_embedded_video:version', 'field_embedded_video:title',
        'field_embedded_video:description', 'field_embedded_video:duration',
      'field_vevo_video_embed',
      'field_vevo_video_thumb', 'field_vevo_video_thumb:list', 'field_vevo_video_thumb:data',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Prepend the embed to the body.
    $row->body = $row->field_embedded_video . "<br />\n" . $row->body;
  }
}

class mariahcareyrelease_trackMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_audio_asset', 'field_track_audio');
    $this->addFieldMapping('field_audio_asset:file_class')
         ->defaultValue('MigrateFileUri');
    $this->addFieldMapping('field_audio_asset:title', 'field_track_audio:title');
    $this->addFieldMapping('field_legacy_url', 'field_track_audio');
    $this->addUnmigratedDestinations(array(
        'field_audio_asset:language',
        'field_audio_asset:destination_dir', 'field_audio_asset:destination_file',
        'field_audio_asset:file_replace', 'field_audio_asset:preserve_files',
        'field_audio_asset:source_dir', 'field_audio_asset:data',
      'field_audio_category', 'field_audio_category:create_term', 'field_audio_category:source_type', 'field_audio_category:ignore_case',
      'field_audio_duration', 'field_audio_duration:language',
      'field_audio_embed', 'field_audio_embed:format', 'field_audio_embed:language',
      'field_audio_image', 'field_audio_image:file_class', 'field_audio_image:language',
        'field_audio_image:destination_dir', 'field_audio_image:destination_file',
        'field_audio_image:file_replace', 'field_audio_image:preserve_files',
        'field_audio_image:source_dir', 'field_audio_image:alt', 'field_audio_image:title',
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
      'field_download_url', 'field_download_url:language',
        'field_legacy_url:language',
      'field_lyrics', 'field_lyrics:format', 'field_lyrics:language',
      'field_rating',
      'field_secondary_title', 'field_secondary_title:language',
      'field_video_url', 'field_video_url:language',
    ));

    $this->addUnmigratedSources(array(
      'field_artist_comments',
      'field_buy_link', 'field_buy_link:title', 'field_buy_link:attributes',
      'field_disc_number',
      'field_members_only',
      'field_personnel',
      'field_release_reference',
        'field_track_audio:attributes',
      'field_track_description',
      'field_track_number',
      'field_video_brightcove', 'field_video_brightcove:value',
        'field_video_brightcove:provider', 'field_video_brightcove:data',
        'field_video_brightcove:status', 'field_video_brightcove:provider',
        'field_video_brightcove:data', 'field_video_brightcove:status',
        'field_video_brightcove:version', 'field_video_brightcove:title',
        'field_video_brightcove:description', 'field_video_brightcove:duration',
      'field_video_link', 'field_video_link:title', 'field_video_link:attributes',
    ));
  }
}
/**
 * Migration of release nodes to album nodes.
 */
class mariahcareyreleaseMigration extends AANode6Migration {
  public function __construct($arguments) {
    $this->sourceFields['tracks'] = 'Track listing populated from track relationships';
    $this->sourceFields['tracks_num'] = 'Track count populated from track relationships';
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->dependencies[] = 'release_track';

    $this->addFieldMapping('field_recording_artist')
         ->defaultValue('mariahcarey');
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
      '2',
      'field_buy_link', 'field_buy_link:title', 'field_buy_link:attributes',
      'field_html_buy_link', 'field_html_buy_link:format',
      'field_release_country',
        'field_release_cover:list', 'field_release_cover:data',
      'field_release_format',
      'field_release_label',
      'field_release_type',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Go through the tracks related to this album, updating their
    // category and adding them to the album track listing.
    $audio_tid = db_select('taxonomy_term_data', 'td')
                 ->fields('td', array('tid'))
                 ->condition('vid', 21) // Audio category vocabulary
                 ->condition('name', $row->title)
                 ->execute()
                 ->fetchfield();
    if (!$audio_tid) {
      $term = new stdClass;
      $term->vid = 21;
      $term->name = $row->title;
      taxonomy_term_save($term);
      $audio_tid = $term->tid;
    }
    $track_nids = Database::getConnection('default', $this->sourceConnection)
                  ->select('content_type_release_track', 't')
                  ->fields('t', array('nid'))
                  ->condition('field_release_reference_nid', $row->nid)
                  ->orderBy('field_track_number_value')
                  ->execute()
                  ->fetchCol();
    $row->tracks = array();
    $row->tracks_num = 0;
    foreach ($track_nids as $track_src_nid) {
      $track_dest_nid = db_select('migrate_map_release_track', 'map')
                        ->fields('map', array('destid1'))
                        ->condition('sourceid1', $track_src_nid)
                        ->execute()
                        ->fetchField();
      $track = node_load($track_dest_nid);
      $track->field_audio_category[LANGUAGE_NONE][0]['tid'] = $audio_tid;
      node_save($track);
      $row->tracks[] =
        '<a href="/node/' . $track_dest_nid . '">' . $track->title . '</a>';
      $row->tracks_num++;
    }
  }
}

class mariahcareyCommentstoryMigration extends AAComment6Migration {}
class mariahcareyCommentreleaseMigration extends AAComment6Migration {}
class mariahcareyCommentrelease_trackMigration extends AAComment6Migration {}
