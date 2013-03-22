<?php

class FieldMigration extends MigrationBase {
  protected $fields = array();
  public function getFields() {
    return $this->fields;
  }

  protected $profileFields = array();
  public function getProfileFields() {
    return $this->profileFields;
  }

  public function __construct() {
    parent::__construct(MigrateGroup::getInstance('warner'));
    $this->description = t('Member fields');
    $this->team = array(
      new MigrateTeamMember('Mike Ryan', 'mike.ryan@acquia.com', 'Acquia'),
      new MigrateTeamMember('Arun De Joe', 'DeJoe.Arun@wmg.com', 'Warner'),
    );

    // Get the header row from the CSV
    $stream = new MigrateWarnerStreamWrapper();
    $localpath = $stream->getRealPath('warner://') . '/' . variable_get('warner_private_directory', '');;
    foreach (glob("$localpath/*.csv") as $source_file) {
      break;
    }
    $handle = fopen($source_file, 'r');
    for ($i = 0; $i < 17; $i++) {
      fgets($handle);
    }
    $header = fgetcsv($handle);
    fclose($handle);

    // See https://wead2c.basecamphq.com/projects/7412194-acquia-wmg-gardens-oh-my/posts/49122636/comments#comment_125455416

    // The first six columns are standard
    $this->fields = array(
      array('member_name', 'Member Name'),
      array('email', 'Member Email Address'),
      array('type', 'Type'),
      array('groups', 'Groups'),
      array('registered', 'Date Registered'),
      array('logged_in', 'Last Sign-In'),
    );

    // OK, gather all the fields from here to the Facebook stuff
    $index = 6;
    $this->profileFields = array();
    // Certain fields are common to most or all sites - we will have built-in
    // fields for them. Keyed by the CSV header (lower-cased), value will be used
    // as the source field name and (with field_ prepended) the destination field name.
    $known_fields = array(
      'postal code' => 'postal_code',
      'about me' => 'about_me',
      'city' => 'city',
      'state' => 'state',
      'gender' => 'gender',
      'name' => 'nickname',
      'hobbies' => 'hobbies',
      'favorite bands' => 'favorite_bands',
      'twitter name' => 'twitter_name',
      'country' => 'country',
    );
    while ($index < count($header)) {
      $column = trim($header[$index]);
      $lower_column = strtolower($column);
      if ($lower_column == 'facebook connect enabled') break;
      // There are certain columns we know, and have hard-coded field names for
      if (!empty($known_fields[$lower_column])) {
        $field_name = $known_fields[$lower_column];
        $custom = FALSE;
      }
      else {
        $field_name = "profile_$index";
        $custom = TRUE;
      }
      $this->fields[] = array($field_name, $column);
      $this->profileFields[] = array(
        'custom' => $custom,
        'field_name' => $field_name,
        'label' => $column,
      );
      $index++;
    }

    // The rest of the columns are standard
    $postfix_fields = array(
      array('fbconnect_enabled', 'Facebook Connect Enabled'),
      array('fb_comments', 'Publish My Comments on My Facebook Wall'),
      array('fb_upload', 'Post on My Facebook Wall When I Upload Something'),
      array('fb_discussion', 'Post on My Facebook Wall When I Create a Discussion Post'),
      array('fb_blog', 'Post on My Facebook Wall When I Post a New Blog'),
      array('comment_notify', 'Notify Me When Someone Comments On'),
      array('allowed_apps', 'Allowed Applications'),
      array('profile_updated', 'Profile Updated'),
      array('mobile_email', 'Mobile Upload Email Address'),
      array('visits_this_month', 'Visits This Month'),
      array('time_this_month', 'Time on Site This Month'),
      array('ppv_this_month', 'Pages Per Visit This Month'),
      array('visits_past_year', 'Visits Last 12 Months'),
      array('time_past_year', 'Time on Site Last 12 Months'),
      array('ppv_past_year', 'Pages Per Visit Last 12 Months'),
      array('visits_all_time', 'Visits All Time'),
      array('time_all_time', 'Time on Site All Time'),
      array('ppv_all_time', 'Pages Per Visit All Time'),
      array('pro_video_views', 'Pro Video Plays/Views'),
      array('pro_audio_views', 'Pro Audio Plays/Views'),
      array('pro_album_views', 'Pro Album Plays/Views'),
      array('pro_photo_views', 'Pro Photo Plays/Views'),
      array('pro_blog_views', 'Pro Blog Plays/Views'),
      array('pro_video_comments', 'Pro Video Comments'),
      array('pro_audio_comments', 'Pro Audio Comments'),
      array('pro_album_comments', 'Pro Album Comments'),
      array('pro_photo_comments', 'Pro Photo Comments'),
      array('pro_blog_comments', 'Pro Blog Comments'),
      array('pro_video_shares', 'Pro Video Shares'),
      array('pro_audio_shares', 'Pro Audio Shares'),
      array('pro_photo_shares', 'Pro Photo Shares'),
      array('pro_blog_shares', 'Pro Blog Shares'),
      array('pro_video_rated', 'Pro Video Rated'),
      array('pro_audio_rated', 'Pro Audio Rated'),
      array('pro_album_rated', 'Pro Album Rated'),
      array('pro_photo_rated', 'Pro Photo Rated'),
      array('pro_audio_buys', 'Pro Audio Buys'),
      array('pro_album_buys', 'Pro Album Buys'),
      array('discussions_started', 'Discussions Started'),
      array('discussion_comments', 'Discussions Comments'),
      array('profile_comments', 'Profile Comments'),
      array('videos_uploaded', 'Videos Uploaded'),
      array('photos_uploaded', 'Photos Uploaded'),
      array('ugc_video_views', 'UGC Videos Plays/Views'),
      array('ugc_photo_views', 'UGC Photos Plays/Views'),
      array('ugc_discussion_views', 'UGC Discussions Views'),
      array('ugc_video_comments', 'UGC Videos Comments'),
      array('ugc_photo_comments', 'UGC Photos Comments'),
      array('ugc_discussion_comments', 'UGC Discussions Comments'),
      array('ugc_video_shares', 'UGC Videos Shares'),
      array('ugc_photo_shares', 'UGC Photos Shares'),
      array('ugc_discussion_shares', 'UGC Discussions Shares'),
      array('ugc_videos_rated', 'UGC Videos Rated'),
      array('ugc_photos_rated', 'UGC Photos Rated'),
      array('ugc_videos_flagged', 'UGC Videos Flagged'),
      array('ugc_photos_flagged', 'UGC Photos Flagged'),
      array('ugc_discussions_flagged', 'UGC Discussions Flagged'),
      array('ugc_comments_flagged', 'UGC Comments Flagged'),
      array('usage_level', 'Member Usage Level'),
      array('usage_type', 'Member Usage Type'),
      array('influence_score', 'Influence Score'),
      array('birthdate', 'Birthdate'),
      array('eos_id', 'Eos-wide ID'),
      array('site_id', 'Site-level ID'),
    );

    $this->fields = array_merge($this->fields, $postfix_fields);
  }

  public function import() {
    // The first time we're run on the site, delete all existing content
    if (variable_get('warner_content_wiped', 'no') != 'yes') {
      $result = db_select('node', 'n')
                ->fields('n', array('nid', 'type', 'title'))
                ->condition('type', array('album', 'audio', 'blog',
                                    'forum', 'photo', 'video'), 'IN')
                ->execute();
      foreach ($result as $row) {
        node_delete($row->nid);
      }
      variable_set('warner_content_wiped', 'yes');
    }

    // Create the fields. Note that we may also need to create some of the common
    // fields, in sites cloned before they were added to the template.
    foreach ($this->profileFields as $field) {
      $field_name = 'field_' . $field['field_name'];
      $label = $field['label'];
      if (!field_read_instance('user', $field_name, 'user')) {
        switch ($field_name) {
          case 'field_postal_code':
          case 'field_city':
          case 'field_state':
          case 'field_nickname':
          case 'field_twitter_name':
            $type = 'text';
            break;
          case 'field_gender':
          case 'field_country':
            $type = 'list_text';
            break;
          case 'field_about_me':
          case 'field_favorite_bands':
          case 'field_hobbies':
          default:
            $type = 'text_long';
            break;
        }

        $field_record = array(
          'field_name' => $field_name,
          'type' => $type,
        );
        $field_record = field_create_field($field_record);
        if ($field_record) {
          $instance = array(
            'field_name' => $field_name,
            'entity_type' => 'user',
            'bundle' => 'user',
            'label' => $label,
          );
          $instance = field_create_instance($instance);
        }
      }
    }

    // Let's make sure missing node fields also get added
    $node_fields = array(
      array(
        'node_type' => 'audio',
        'field_name' => 'field_lyrics',
        'label' => 'Lyrics',
        'field_type' => 'text_long',
      ),
      array(
        'node_type' => 'audio',
        'field_name' => 'field_download_url',
        'label' => 'Download URL',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'audio',
        'field_name' => 'field_secondary_title',
        'label' => 'Secondary Title',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'audio',
        'field_name' => 'field_video_url',
        'label' => 'Song video URL',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'audio',
        'field_name' => 'field_copyright_name',
        'label' => 'Copyright name',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'audio',
        'field_name' => 'field_copyright_url',
        'label' => 'Copyright URL',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'album',
        'field_name' => 'field_secondary_title',
        'label' => 'Secondary Title',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'album',
        'field_name' => 'field_copyright_name',
        'label' => 'Copyright name',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'album',
        'field_name' => 'field_copyright_url',
        'label' => 'Copyright URL',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'photo',
        'field_name' => 'field_secondary_title',
        'label' => 'Secondary Title',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'photo',
        'field_name' => 'field_copyright_name',
        'label' => 'Copyright name',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'photo',
        'field_name' => 'field_copyright_url',
        'label' => 'Copyright URL',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'video',
        'field_name' => 'field_secondary_title',
        'label' => 'Secondary Title',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'video',
        'field_name' => 'field_copyright_name',
        'label' => 'Copyright name',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'video',
        'field_name' => 'field_copyright_url',
        'label' => 'Copyright URL',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'audio',
        'field_name' => 'field_legacy_url',
        'label' => 'Legacy asset URL',
        'field_type' => 'text',
      ),
      array(
        'node_type' => 'video',
        'field_name' => 'field_legacy_url',
        'label' => 'Legacy asset URL',
        'field_type' => 'text',
      ),
    );
    foreach ($node_fields as $field) {
      $field_record = field_read_field($field['field_name']);
      if (!$field_record) {
        $field_record = array(
          'field_name' => $field['field_name'],
          'type' => $field['field_type'],
        );
        $field_record = field_create_field($field_record);
      }
      if (!field_read_instance('node', $field['field_name'], $field['node_type'])) {
        // Hide the legacy URL fields
        if ($field['field_name'] == 'field_legacy_url') {
          $instance = array(
            'field_name' => $field['field_name'],
            'entity_type' => 'node',
            'bundle' => $field['node_type'],
            'label' => $field['label'],
            'display' => array(
              'default' => array(
                'type' => 'hidden',
              )
            ),
          );
        }
        else {
          $instance = array(
            'field_name' => $field['field_name'],
            'entity_type' => 'node',
            'bundle' => $field['node_type'],
            'label' => $field['label'],
          );
        }
        $instance = field_create_instance($instance);
      }
    }

    // Remove any thumbnail fields
    if ($instance = field_read_instance('node', 'field_thumbnail', 'album')) {
      field_delete_instance($instance);
    }
    if ($instance = field_read_instance('node', 'field_thumbnail', 'audio')) {
      field_delete_instance($instance);
    }
    if ($instance = field_read_instance('node', 'field_thumbnail', 'photo')) {
      field_delete_instance($instance);
    }
    if ($instance = field_read_instance('node', 'field_thumbnail', 'video')) {
      field_delete_instance($instance);
    }

    // Disable media_youtube if enabled
    if (module_exists('media_youtube')) {
      module_disable(array('media_youtube'));
    }

    return MigrationBase::RESULT_COMPLETED;
  }

  public function rollback() {
    foreach ($this->profileFields as $field) {
      $field_name = 'field_' . $field['field_name'];
      if (!empty($field['custom']) && $instance = field_read_instance('user', $field_name, 'user')) {
        field_delete_instance($instance);
      }
    }
    return MigrationBase::RESULT_COMPLETED;
  }

  public function sourceCount($refresh = FALSE) {
    $count = 0;
    foreach ($this->profileFields as $field) {
      if (!empty($field['custom'])) {
        $count++;
      }
    }
    return $count;
  }

  public function importedCount() {
    $count = 0;
    foreach ($this->profileFields as $field) {
      $field_name = 'field_' . $field['field_name'];
      if (!empty($field['custom']) && field_read_instance('user', $field_name, 'user')) {
        $count++;
      }
    }
    return $count;
  }

  public function processedCount() {
    return $this->importedCount();
  }
  public function isComplete() {
    return TRUE;
  }
}
