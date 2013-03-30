<?php
/**
 * @file
 * Mappings specific to kolfanclub.
 */

class kolfanclubRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    parent::__construct($site_name, 6);
    $this->commonArguments['format_mappings'] =  array(
      '1' => '1', // Filtered HTML
      '2' => '6', // Full HTML
      '3' => 'script',  // PHP code
    );
    $this->roleArguments['role_mappings'] = array(
      'Fan Club' => 'fan club member',
      'Project Manager' => 'administrator',
      'Developer' => 'administrator',
    );
    $this->sourceDirectory = 'http://kolfanclub.com/sites/kolfanclub.com/files/';
  }

  protected function register() {
    $this->registerRole();
    $this->registerPicture();
    $this->registerUser();

    // Forums vocabulary
    $this->registerTerm('1', 'forums');

    $this->registerNode('forum', 'forum');
    $this->registerComment('forum', 'forum');
    $this->registerNode('story', 'blog');
    $this->registerComment('story', 'blog');
    $this->registerNode('tour_date', 'event');
    $this->registerComment('tour_date', 'event');
  }
}

class kolfanclubRoleMigration extends AARole6Migration {}

class kolfanclubPictureMigration extends AAPicture6Migration {
  public function prepareRow($row) {
    $row->filepath = str_replace('sites/kolfanclub.com/files/', '', $row->filepath);
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
  }
}

class kolfanclubUserMigration extends AAUser6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    // Known mappings
    $this->addFieldMapping('field_member_name', 'profile_name');
    $this->addFieldMapping('field_nickname', 'profile_name');
    $this->addFieldMapping('field_about_me', 'profile_bio');
    $this->addFieldMapping('field_favorite_song', 'profile_favorite_song');

    // Provide a blank slate for all these fields.
    $this->addUnmigratedDestinations(array(
        'field_about_me:language',
      'field_birthdate',
      'field_city', 'field_city:language',
      'field_country',
      'field_eos_id',
      'field_favorite_bands', 'field_favorite_bands:language',
        'field_favorite_song:language',
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

    $this->addUnmigratedSources(array(
      'profile_location',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Do the same manipulation as the picture migration, so we match the
    // path in the map table.
    $row->picture = str_replace('sites/kolfanclub.com/files/', '', $row->picture);

    // Birthdays are serialized arrays, convert to a datetime string
    if (!empty($row->profile_user_birthday)) {
      $birthday = unserialize($row->profile_user_birthday);
      $row->profile_user_birthday = $birthday['year'] . '-' . $birthday['month']
        . '-' . $birthday['day'];
    }
  }
}

class kolfanclub1TermMigration extends AATerm6Migration {}

class kolfanclubforumMigration extends AANode6Migration {
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

class kolfanclubstoryMigration extends AANode6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_blog_category')
         ->defaultValue('Story');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'field_tags', 'field_tags:source_type', 'field_tags:create_term', 'field_tags:ignore_case',
        'field_blog_category:source_type', 'field_blog_category:ignore_case',
    ));

    $this->addUnmigratedSources(array(
      'field_image', 'field_image:list', 'field_image:data', // No data
    ));
  }
}

class kolfanclubtour_dateMigration extends AANode6Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_event_date', 'field_tourdate');
    $this->addFieldMapping('field_event_venue', 'field_venue');
    $this->addFieldMapping('field_event_buylink', 'field_buy_link')
         ->defaultValue('');
    $this->addFieldMapping('field_event_buylink:format')
         ->defaultValue(1);
    $this->addFieldMapping('field_event_maplink')
         ->defaultValue('');
    $this->addFieldMapping('field_event_category')
         ->defaultValue('Tour Date');
    $this->addFieldMapping('field_event_category:create_term')
         ->defaultvalue(TRUE);

    $this->removeFieldMapping('body:format');
    $this->removeFieldMapping('field_is_pro');

    $this->addUnmigratedDestinations(array(
      'field_event_address', 'field_event_address:language',
      'field_event_availability', 'field_event_availability:language',
        'field_event_buylink:language',
        'field_event_category:source_type', 'field_event_category:ignore_case',
      'field_event_city', 'field_event_city:language',
      'field_event_country', 'field_event_country:language',
        'field_event_maplink:format', 'field_event_maplink:language',
      'field_event_state', 'field_event_state:language',
       'field_event_venue:language',
    ));

    $this->addUnmigratedSources(array(
        'field_buy_link:title', 'field_buy_link:attributes',
      'field_presale',
      'field_vip_ticket', 'field_vip_ticket:title', 'field_vip_ticket:attributes',
      'format',
    ));
  }
}

class kolfanclubCommentforumMigration extends AAComment6Migration {}
class kolfanclubCommentstoryMigration extends AAComment6Migration {}
class kolfanclubCommenttour_dateMigration extends AAComment6Migration {}
