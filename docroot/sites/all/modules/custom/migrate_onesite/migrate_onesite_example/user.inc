<?php
/**
 * @file
 * Migration class for importing users from ONEsite to Drupal Gardens.
 */

class MigrateOnesiteExampleUser extends OnesiteUserMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->addFieldMapping('role_names', NULL, FALSE)
         ->defaultValue('fan club member');
    $this->addFieldMapping('field_member_name', 'display_name');
    $this->addFieldMapping('field_profile_updated', 'date_updated');
    $this->addFieldMapping('field_birthdate', 'dob');
    $this->addFieldMapping('field_gender', 'gender')
         ->callbacks(array($this, 'handleGender'));
    $this->addFieldMapping('field_city', 'loc_city');
    $this->addFieldMapping('field_state', 'loc_state');
    $this->addFieldMapping('field_postal_code', 'loc_zip');

    $this->addUnmigratedDestinations(array(
        'field_city:language',
      'field_eos_id',
        'field_member_name:language',
      'field_nickname', 'field_nickname:language',
        'field_postal_code:language',
      'field_provider', 'field_provider:language',
      'field_site_id', 'field_site_id:language',
        'field_state:language',
    ));

    $this->addUnmigratedSources(array(
      'aim_id',
      'blog_id',
      'body_type',
      'children_number',
      'children_status',
      'deleted',
      'dob_day',
      'dob_display',
      'dob_month',
      'dob_year',
      'drink',
      'ethnicity',
      'favorite_diggy_songs',
      'first_name',
      'for_dating',
      'for_friendships',
      'for_hookups',
      'for_networking',
      'for_relationships',
      'gender1',
      'gmail_id',
      'height',
      'here_for',
      'hidden',
      'home_city',
      'home_country',
      'home_state',
      'home_zip',
      'last_name',
      'loc_country',
      'loc_custom',
      'loc_latitude',
      'loc_longitude',
      'loc_zip2',
      'marital_status1',
      'msn_id',
      'orientation',
      'personal_quote',
      'portrait',
      'profile_height',
      'profile_width',
      'relationship_status',
      'religion',
      'school_city',
      'school_major',
      'school_name',
      'school_state',
      'skype_id',
      'smoke',
      'student_classification',
      'student_email',
      'subscribed_platforms',
      'yahoo_id',
    ));
  }

  protected function handleGender($value) {
    switch ($value) {
      case 0:
        return 'Male';
      case 1:
        return 'Female';
      default:
        return '';
    }
  }
}
