<?php
/**
 * @file
 * Migration implementations specific to pattygriffin.
 */

/**
 * Registration of this site's migrations.
 */
class pattygriffinRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    parent::__construct($site_name, 6);
    $this->commonArguments['format_mappings'] =  array(
      '1' => '1',       // Filtered HTML
      '2' => '6',       // Full HTML
      '3' => 'script',  // PHP
    );
    $this->roleArguments['role_mappings'] = array(
      'Fan Club' => 'fan club member',
      'Product Manager' => 'administrator',
    );
    $this->sourceDirectory = 'http://www.pattygriffin.com/sites/pattygriffin.com/files/';
  }

  /**
   * Register our migrations.
   */
  protected function register() {
    $this->registerRole();
    $this->registerPicture();
    $this->registerUser();
    $this->registerFile();
    $this->registerNode('event', 'event');
    $this->registerComment('event', 'event');
    $this->registerNode('release', 'album');
    $this->registerComment('release', 'album');
    $this->registerNode('story', 'blog');
    $this->registerComment('story', 'blog');
  }
}

/**
 * Migration of release nodes to album nodes.
 */
class pattygriffinreleaseMigration extends AANode6Migration {
  public function __construct($arguments) {
    $this->sourceFields['tracks'] = 'Track listing populated from body';
    $this->sourceFields['tracks_num'] = 'Track count populated from body';
    parent::__construct($arguments);
    $this->dependencies[] = 'File';

    $this->addFieldMapping('field_recording_artist')
         ->defaultValue('Patty Griffin');
    $this->addFieldMapping('field_tracks', 'tracks');
    $this->addFieldMapping('field_tracks:format')
         ->defaultValue('1'); // Filtered HTML
    $this->addFieldMapping('field_album_tracks_num', 'tracks_num');
    $this->addFieldMapping('field_album_date', 'field_release_date');
    $this->addFieldMapping('field_album_image', 'field_release_cover')
         ->sourceMigration('File');
    $this->addFieldMapping('field_album_image:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_buy_links', 'field_buy_link');
    $this->addFieldMapping('field_buy_links:format')
         ->defaultValue('1');

    $this->addUnmigratedDestinations(array(
        'field_album_image:language', 'field_album_image:alt', 'field_album_image:title',
      'field_album_subtitle', 'field_album_subtitle:language',
      'field_audio_embed', 'field_audio_embed:format', 'field_audio_embed:language',
      'field_audio_player', 'field_audio_player:file_class', 'field_audio_player:language',
        'field_audio_player:destination_dir', 'field_audio_player:destination_file',
        'field_audio_player:file_replace', 'field_audio_player:preserve_files',
        'field_audio_player:source_dir', 'field_audio_player:title', 'field_audio_player:data',
        'field_buy_links:language',
      'field_copyright_name', 'field_copyright_name:language',
      'field_copyright_url', 'field_copyright_url:language',
      'field_rating',
        'field_recording_artist:language',
      'field_secondary_title', 'field_secondary_title:language',
        'field_tracks:language',
    ));

    $this->addUnmigratedSources(array(
        'field_release_cover:list', 'field_release_cover:data',
        'field_buy_link:title', 'field_buy_link:attributes',
      'field_track_listing', // No source data
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $row->field_buy_link = $this->buildBuyLink($row, 'field_buy_link');

    // Convert body text into a list of tracks
    $row->tracks_num = preg_match_all('|<li>([^<]+)</li>|i',
      $row->body, $matches);
    foreach ($matches[1] as $match) {
      $row->tracks[] = $match;
    }
    $row->body = '';
  }
}

/**
 * Migration of story nodes to blog nodes.
 */
class pattygriffinstoryMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_blog_category')
         ->defaultValue('General');

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:create_term', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
    ));
    $this->addUnmigratedSources(array('3'));
  }
}

/**
 * Migration of event nodes to event nodes.
 */
class pattygriffineventMigration extends AANode6Migration {
  public function __construct($arguments) {
    $this->sourceFields['state'] = 'State extracted from location';
    $this->sourceFields['country'] = 'Country extracted from location';
    parent::__construct($arguments);

    $this->addSimpleMappings(array('field_event_venue'));
    $this->addFieldMapping('field_event_date', 'field_event_date_time');
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
         ->defaultValue('General');

    $this->removeFieldMapping('field_is_pro');
    $this->removeFieldMapping('body:format');

    $this->addUnmigratedDestinations(array(
      'field_event_address', 'field_event_address:language',
      'field_event_availability', 'field_event_availability:language',
        'field_event_buylink:language',
        'field_event_category:source_type', 'field_event_category:create_term', 'field_event_category:ignore_case',
        'field_event_city:language',
        'field_event_country:language',
        'field_event_maplink:format', 'field_event_maplink:language',
        'field_event_state:language',
        'field_event_venue:language',
    ));
    $this->addUnmigratedSources(array(
      '3',
      'field_mng_enabled',
      'field_mng_window', 'field_mng_window:value2',
        'field_tour_buy:title', 'field_tour_buy:attributes',
      'format',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $this->parseLocation($row, 'field_event_location');
    $row->field_tour_buy = $this->buildBuyLink($row, 'field_tour_buy');
  }
}

class pattygriffinFileMigration extends AAFile6Migration {
  public function prepareRow($row) {
    $row->filepath = str_replace('sites/pattygriffin.com/files/', '', $row->filepath);
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
  }
}

class pattygriffinRoleMigration extends AARole6Migration {}

class pattygriffinPictureMigration extends AAPicture6Migration {
  public function prepareRow($row) {
    $row->filepath = str_replace('sites/pattygriffin.com/files/', '', $row->filepath);
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
  }
}

class pattygriffinUserMigration extends AAUser6Migration {
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

    // All these uprofile fields are empty
    $this->addUnmigratedSources(array(
      'field_about_me', 'field_about_me:format',
      'field_aim',  
      'field_bands',  
      'field_bday', 
      'field_blog', 'field_blog:title', 'field_blog:attributes',
      'field_facebook', 'field_facebook:title', 'field_facebook:attributes',
      'field_favorite', 
      'field_favorite1', 'field_favorite1:title', 'field_favorite1:attributes',
      'field_favorite2', 'field_favorite2:title', 'field_favorite2:attributes',
      'field_favorite3', 'field_favorite3:title', 'field_favorite3:attributes',
      'field_gender', 
      'field_google', 
      'field_homepage', 'field_homepage:title', 'field_homepage:attributes',
      'field_icq',    
      'field_interests', 'field_interests:format',
      'field_linkedin', 'field_linkedin:title', 'field_linkedin:attributes',
      'field_location', 
      'field_meet', 
      'field_msn',  
      'field_myspace', 'field_myspace:title', 'field_myspace:attributes',
      'field_name', 
      'field_occupation', 
      'field_shows',  
      'field_skype',  
      'field_song',
      'field_status', 
      'field_twitter', 'field_twitter:title', 'field_twitter:attributes',
      'field_yahoo',  
      'field_zodiac_sign',  
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    // Do the same manipulation as the picture migration, so we match the
    // path in the map table.
    $row->picture = str_replace('sites/pattygriffin.com/files/', '', $row->picture);
  }
}

class pattygriffinCommenteventMigration extends AAComment6Migration {}
class pattygriffinCommentreleaseMigration extends AAComment6Migration {}
class pattygriffinCommentstoryMigration extends AAComment6Migration {}
