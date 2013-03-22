<?php
/**
 * @file
 * Mappings specific to atlhustlerclub.
 */

class atlhustlerclubRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    parent::__construct($site_name, 6, 21521);
    $this->commonArguments['format_mappings'] =  array(
      '1' => '1',
      '11' => '6',
      '2' => '6',
      '3' => 'script',
      '4' => '11',
    );
    // Let First Year Membership Complete, Forum Moderator, Forum Moderator 2
    // migrate as-is
    $this->roleArguments['role_mappings'] = array(
      'Fan Club' => 'fan club member',
      'Product Manager' => 'administrator',
      'Site Admin' => 'administrator',
    );
    $this->sourceDirectory =
      'http://alltimelowhustlers.com/sites/atlhustlerclub.com/files/';
  }

  protected function register() {
    $this->registerRole();
    $this->registerPicture();
    $this->registerUser();
    $this->registerFile();

    $this->registerTerm('2', 'forums');
    $this->registerTerm('4', 'blog_category');
    // Note that there are comments on the gallery
    $this->registerGallery('admin_gallery', 'photo_category');
    $this->registerNode('admin_photo', 'photo');
    $this->registerComment('admin_photo', 'photo');
    $this->registerNode('blog', 'forum');
    $this->registerComment('blog', 'forum');
    // Special handling because the legacy event type is being split into
    // two parts here - event nodes for tour dates, and blog nodes for
    // other events. We register the other events normally, but give the tour
    // dates a distinct machine name while making sure we get the source_type right.
    $this->registerNode('event', 'blog');
    $this->registerComment('event', 'blog');
    $this->registerNode('tourdate', 'event');
    $this->registerComment('tourdate', 'event');

    $this->registerNode('forum', 'forum');
    $this->registerComment('forum', 'forum');
    $this->registerNode('story', 'blog');
    $this->registerComment('story', 'blog');
    $this->registerMigration('webform', $this->commonArguments);
  }
}

class atlhustlerclubRoleMigration extends AARole6Migration {}

class atlhustlerclubadmin_galleryMigration extends AAGallery6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_is_pro', NULL, FALSE)
         ->defaultValue(TRUE);
  }
}

class atlhustlerclubeventMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->defaultUid = 21521;

    $this->addFieldMapping('field_blog_category')
         ->defaultValue('Events');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:ignore_case',
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      '4',
      'field_event_date_time',
      'field_event_location',
      'field_event_venue',
      'field_mng_enabled',
      'field_mng_window', 'field_mng_window:value2',
      'field_supporting_act',
       'field_tour_buy', 'field_tour_buy:title', 'field_tour_buy:attributes',
      'format',
    ));
  }

  protected function query() {
    $query = parent::query();
    // tid 2 is Event
    $query->innerJoin('term_node', 'tn', 'n.vid=tn.vid AND tn.tid=2');
    return $query;
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

class atlhustlerclubtourdateMigration extends AANode6Migration {
  public function __construct($arguments) {
    $arguments['source_type'] = 'event';
    $this->sourceFields['state'] = 'State extracted from location';
    $this->sourceFields['country'] = 'Country extracted from location';

    parent::__construct($arguments);
    $this->defaultUid = 21521;

    $this->addFieldMapping('field_event_category')
         ->defaultValue('Tour Dates');
    $this->addFieldMapping('field_event_category:create_term')
         ->defaultValue(TRUE);
    $this->addFieldMapping('field_event_date', 'field_event_date_time');
    $this->addFieldMapping('field_event_venue', 'field_event_venue');
    $this->addFieldMapping('field_event_city', 'field_event_location');
    $this->addFieldMapping('field_event_state', 'state');
    $this->addFieldMapping('field_event_country', 'country');
    $this->addFieldMapping('field_event_buylink', 'field_tour_buy')
         ->defaultValue('');
    $this->addFieldMapping('field_event_buylink:format')
         ->defaultValue(1);
    $this->addFieldMapping('field_event_maplink')
         ->defaultValue('');

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
      '4',
      'field_mng_enabled',
      'field_mng_window', 'field_mng_window:value2',
      'field_supporting_act',
        'field_tour_buy:title', 'field_tour_buy:attributes',
      'format',
    ));
  }

  protected function query() {
    // tid 2 is Event
    $query = parent::query();
    $query->leftJoin('term_node', 'tn', 'n.vid=tn.vid AND tn.tid=2');
    $query->isNull('tn.tid');
    return $query;
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    if (is_array($row->field_event_date_time)) {
      // Date is yyyy-mm-dd hh:mm:ss, truncate to just date portion
      $row->field_event_date_time = reset($row->field_event_date_time);
    }
    $row->field_event_date_time = substr($row->field_event_date_time, 0, 10);
    if (is_array($row->field_event_venue)) {
      $row->field_event_venue = reset($row->field_event_venue);
    }
    $this->parseLocation($row, 'field_event_location');
    if (is_array($row->field_event_location)) {
      $row->field_event_location = reset($row->field_event_location);
    }
  }
}

class atlhustlerclubadmin_photoMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->dependencies[] = 'admin_gallery';
    $this->defaultUid = 21521;
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

class atlhustlerclubforumMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = '2Term';
    $this->defaultUid = 21521;
    $this->addFieldMapping('taxonomy_forums', '2')
         ->sourceMigration('2Term');
    $this->addFieldMapping('taxonomy_forums:source_type')
         ->defaultValue('tid');

    $this->addUnmigratedDestinations(array(
      'taxonomy_forums:create_term', 'taxonomy_forums:ignore_case',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
  }
}

class atlhustlerclubFileMigration extends AAFile6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->addUnmigratedSources(array(
      'origname',
    ));
  }
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
  }
}

class atlhustlerclubPictureMigration extends AAPicture6Migration {}

class atlhustlerclubUserMigration extends AAUser6Migration {
  public function __construct(array $arguments) {
    $this->sourceFields['birthday_day'] = 'Day of user birthday';
    $this->sourceFields['birthday_month'] = 'Month of use birthday';
    parent::__construct($arguments);

    // Known mappings
    $this->addFieldMapping('field_member_name', 'name');
    $this->addFieldMapping('field_nickname', 'name');
    $this->addFieldMapping('field_about_me', 'profile_fun_facts');
    $this->addFieldMapping('field_birthdate', 'profile_birthday');
    $this->addFieldMapping('field_city', 'profile_location');
    $this->addFieldMapping('field_birthday_day', 'birthday_day');
    $this->addFieldMapping('field_birthday_month', 'birthday_month');
    $this->addFieldMapping('field_website', 'profile_web_site');

    // Override default signature handling
    $this->addFieldMapping('signature', 'signature2', FALSE);
    $this->addFieldMapping('signature_format', NULL, FALSE)
         ->defaultValue('1');

    // Provide a blank slate for all these fields.
    $this->addUnmigratedDestinations(array(
        'field_about_me:language',
        'field_city:language',
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
        'field_website:language',
      'path'
    ));

    // No actual profile data in the source
    $this->addUnmigratedSources(array(
      'field_promo',
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
    if (!empty($row->signature2)) {
      $row->signature2 = substr($row->signature2, 0, 255);
    }
    if (isset($row->profile_birthday) && $row->profile_birthday == 'N;') {
      $row->profile_birthday = '';
    }
    elseif (!empty($row->profile_birthday)) {
      $birthday = unserialize($row->profile_birthday);
      if (is_array($birthday)) {
        $row->profile_birthday = $birthday['year'] . '/' . $birthday['month'] .
                                 '/' . $birthday['day'];
        $row->birthday_day = $birthday['day'];
        $row->birthday_month = $birthday['month'];
      }
    }
  }

  protected function query() {
    // Skip the existing admin account, there's already one
    $query = parent::query();
    $query->condition('u.name', 'THC Admin', '<>');
    // And pick up the signature
    $query->leftJoin('users_signature', 'us', 'u.uid=us.uid');
    $query->addField('us', 'signature', 'signature2');
    return $query;
  }


  public function complete($account, $row) {
    // comment_on_anything doesn't create its nodes automatically, we must do
    // the work here. Stolen from _comment_on_anything_queue_worker_process_entities().
    migrate_instrument_start('User complete');
    // Reload to make sure picture is properly populated
    $account = user_load($account->uid);
    if (empty($account->field_comment)) {
      $account->field_comment[LANGUAGE_NONE] = array(
        array(
          'nid' => comment_on_anything_create_and_cache(
            $account, 'user', $account
          )
        ),
      );
      entity_save('user', $account);
    }
    migrate_instrument_stop('User complete');
  }
}

class atlhustlerclubblogMigration extends AANode6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'File';
    $this->defaultUid = 21521;
    $this->addFieldMapping('taxonomy_forums')
         ->defaultValue('Blogs');
    $this->addFieldMapping('taxonomy_forums:create_term')
         ->defaultValue(TRUE);
    $this->addUnmigratedDestinations(array(
        'taxonomy_forums:source_type', 'taxonomy_forums:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      '6',
      'field_blog_image', 'field_blog_image:list', 'field_blog_image:data',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    // If the former image field was populated, prepend the image
    if (isset($row->field_blog_image)) {
      $new_fid = db_select('migrate_map_file', 'map')
                 ->fields('map', array('destid1'))
                 ->condition('sourceid1', $row->field_blog_image)
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
          $row->body = $media_tag;
        }
      }
    }
  }
}

class atlhustlerclubstoryMigration extends AANode6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = '4Term';
    $this->dependencies[] = 'File';
    $this->defaultUid = 21521;
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

    // Rewrite any img tags referencing local files to point to a media
    // tag for the migrated file
    $row->body = preg_replace_callback('|<img +(.*?)>|i',
      array($this, 'replaceImgs'), $row->body);
    $row->teaser = preg_replace_callback('|<img +(.*?)>|i',
      array($this, 'replaceImgs'), $row->teaser);

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
        $row->body = $media_tag . $row->body;
      }
    }
  }

  /**
   * If we have an image reference, replace it with media tags if we can.
   *
   * @param array $matches
   */
  protected function replaceImgs(array $matches) {
    // Default to the original <img> tag.
    $result = $matches[0];

    // The src parameter is required
    if (preg_match('|src=[\'"]/sites/atlhustlerclub.com/files/(.*?)[\'"]|i', $result, $src_matches)) {
      $src = $src_matches[1];
    }
    else {
      return $result;
    }

    // Get the fid, if any. If none, let the img tag stand
    $fid = db_select('file_managed', 'f')
           ->fields('f', array('fid'))
           ->condition('filename', urldecode(basename($src)))
           ->execute()
           ->fetchField();
    if (!$fid) {
      return $result;
    }

    $attributes = array(
      'class' => 'media-image',
      'typeof' => 'foaf:Image',
      'style' => '',
    );

    // Build the media tag
    $img_info = array(
      'type' => 'media',
      'view_mode' => 'media_large',
      'fid' => $fid,
      'attributes' => array($attributes),
    );
    $result = '[[' . drupal_json_encode($img_info) . ']]';

    return $result;
  }
}

class atlhustlerclubwebformMigration extends DrupalMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = 'User';

    $fields = array(
      'username' => 'username',
      'first_name' => 'first_name',
      'last_name' => 'last_name',
      'email_address' => 'email_address',
      'date_of_event' => 'date_of_event',
      'city__state' => 'city__state',
      'number_of_tickets_purchased' => 'number_of_tickets_purchased',
      'guests' => 'guests',
      'mg_or_ee' => 'mg_or_ee',
      'have_you_ever_met_the_atl_boys_before' => 'have_you_ever_met_the_atl_boys_before',
      'confirm_that_you_have_read_all_of_the_above_before_filling_out_this_form' => 'confirm_that_you_have_read_all_of_the_above_before_filling_out_this_form',
      'what_piece_of_hustler_merch_would_you_like_to_see_made' => 'what_piece_of_hustler_merch_would_you_like_to_see_made',
    );
    $this->source = new MigrateSourceSQL($this->query(), $fields, NULL,
      array('map_joinable' => FALSE));
    $nodes = node_load_multiple(array(), array('type' => 'webform',
      'title' => 'Select M&G Sign Up'));
    $node = reset($nodes);
    $this->destination = new MigrateDestinationWebformSubmission($node);
    $this->map = new MigrateSQLMap($this->machineName,
       array(
         'sid' => array('type' => 'int',
                        'unsigned' => TRUE,
                        'not null' => TRUE,
                        'description' => 'Webform submission ID',
                        'alias' => 's',
                       ),
       ),
       MigrateDestinationWebformSubmission::getKeySchema()
    );

    $this->addSimpleMappings(array('submitted', 'remote_addr'));
    $this->addFieldMapping('uid', 'uid')
         ->sourceMigration('User');
    $this->addFieldMapping('is_draft')
         ->defaultValue(0);
    // On the Gardens side we get these unintuitive form keys
    $this->addFieldMapping('data_new_1346960272729', 'username');
    $this->addFieldMapping('data_new_1346960598775', 'first_name');
    $this->addFieldMapping('data_new_1346960634561', 'last_name');
    $this->addFieldMapping('data_new_1346960673212', 'email_address');
    $this->addFieldMapping('data_new_1346960718925', 'date_of_event');
    $this->addFieldMapping('data_new_1346961121117', 'city__state');
    $this->addFieldMapping('data_new_1346961266027', 'number_of_tickets_purchased');
    $this->addFieldMapping('data_new_1346961351618', 'guests');
    $this->addFieldMapping('data_new_1346961403890', 'mg_or_ee');
    $this->addFieldMapping('data_new_1346961468463', 'have_you_ever_met_the_atl_boys_before');
    $this->addFieldMapping('data_new_1346961561600', 'confirm_that_you_have_read_all_of_the_above_before_filling_out_this_form');
    $this->addFieldMapping('data_new_1346961638378', 'what_piece_of_hustler_merch_would_you_like_to_see_made');

    $this->addUnmigratedDestinations(array(
      'data_new_1346959782966',
      'sid',
    ));
  }

  protected function query() {
    // We're only interested in one webform, with the known legacy nid 8346
    $query = Database::getConnection('default', 'legacy')
             ->select('webform_submissions', 's')
             ->fields('s', array('sid', 'uid', 'submitted', 'remote_addr'))
             ->condition('nid', 8346);
    return $query;
  }

  /**
   * Use the explicit machine_name argument as this migration's machine name.
   */
  protected function generateMachineName($class_name = NULL) {
    return $this->arguments['machine_name'];
  }

  public function prepareRow($row) {
    $query = Database::getConnection('default', 'legacy')
             ->select('webform_submitted_data', 'd')
             ->fields('d', array('data'))
             ->condition('sid', $row->sid);
    $query->innerJoin('webform_component', 'c', 'd.nid=c.nid AND d.cid=c.cid');
    $query->fields('c', array('form_key'));
    $result = $query->execute();
    foreach ($result as $data_row) {
      $row->{$data_row->form_key} = $data_row->data;
    }
  }
}

class atlhustlerclub2TermMigration extends AATerm6Migration {}
class atlhustlerclub4TermMigration extends AATerm6Migration {}

class atlhustlerclubCommentadmin_photoMigration extends AAComment6Migration {}
class atlhustlerclubCommentblogMigration extends AAComment6Migration {}
class atlhustlerclubCommenteventMigration extends AAComment6Migration {
  public function query() {
    $query = parent::query();
    // tid 2 is Event
    $query->innerJoin('term_node', 'tn', 'n.vid=tn.vid AND tn.tid=2');
    return $query;
  }
}
class atlhustlerclubCommenttourdateMigration extends AAComment6Migration {
  public function __construct($arguments) {
    $arguments['source_type'] = 'event';
    $arguments['node_migration'] = 'tourdate';
    parent::__construct($arguments);
  }

  public function query() {
    // tid 2 is Event
    $query = parent::query();
    $query->leftJoin('term_node', 'tn', 'n.vid=tn.vid AND tn.tid=2');
    $query->isNull('tn.tid');
    return $query;
  }

}
class atlhustlerclubCommentforumMigration extends AAComment6Migration {}
class atlhustlerclubCommentstoryMigration extends AAComment6Migration {}
