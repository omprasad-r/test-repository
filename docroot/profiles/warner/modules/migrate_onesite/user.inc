<?php
/**
 * @file
 * Migration class for importing users from ONEsite to Drupal. May be extended,
 * particularly to add project-specific field mappings.
 */

class OnesiteUserMigration extends OnesiteMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = 'Migration of users from ONEsite';
    $this->dependencies[] = 'Picture';

    $this->sourceFields = array(
      // user table
      'user_id' => 'Unique user ID',
      'username' => 'Always the same as email',
      'password' => 'Encrypted password',
      'encoded_or_hashed_password' => 'The hashed version of the password',
      'email' => 'Email address',
      'domain' => 'Legacy site domain',
      'subdir' => 'Appears to be username',
      'date_created' => 'Date created',
      'last_login' => 'Date last logged in',
      'last_activity' => 'Date of last activity',
      'account_status' => 'Values good-standing, pending, delinquent, deleted',
      'network_id' => 'Ignore',
      'node_id' => 'Ignore',
      'first_name' => 'First name of user',
      'last_name' => 'Last name of user',
      'display_name' => 'Name to display for user - for forums?',
      'dob_month' => 'Birth month',
      'dob_day' => 'Birth day',
      'dob_year' => 'Birth year',
      'dob' => 'Birthdate as a signed timestamp',
      'dob_display' => 'Always private, ignore',
      'gender' => 'Gender - values from 0-3 (mostly 1-2)',
      'loc_custom' => 'NULL',
      'personal_quote' => 'NULL',
      // profile table (leaving out columns redundant with user)
      'blog_id' => 'ID of user blog?',
      'profile_width' => 'Empty',
      'profile_height' => 'Empty',
      'gender1' => 'Empty',
      'orientation' => 'Empty',
      'loc_city' => 'Empty',
      'loc_state' => 'Two-letter state code',
      'loc_country' => 'Two-letter country codes',
      'loc_zip' => 'Postal code',
      'loc_zip2' => 'Empty',
      'loc_latitude' => 'Empty',
      'loc_longitude' => 'Empty',
      'home_city' => 'Empty',
      'home_state' => 'Empty',
      'home_country' => 'Empty',
      'home_zip' => 'Empty',
      'school_name' => 'Empty',
      'school_state' => 'Empty',
      'school_city' => 'Empty',
      'student_email' => 'Empty',
      'student_classification' => 'Empty',
      'school_major' => 'Empty',
      'for_dating' => 'Empty',
      'for_friendships' => 'Empty',
      'for_relationships' => 'Empty',
      'for_hookups' => 'Empty',
      'for_networking' => 'Empty',
      'religion' => 'Empty',
      'here_for' => 'Empty',
      'relationship_status' => 'Empty',
      'marital_status1' => 'Empty',
      'children_status' => 'Empty',
      'children_number' => 'Empty',
      'ethnicity' => 'Empty',
      'body_type' => 'Empty',
      'height' => 'Empty',
      'smoke' => 'Empty',
      'drink' => 'Empty',
      'skype_id' => 'Empty',
      'aim_id' => 'Empty',
      'yahoo_id' => 'Empty',
      'msn_id' => 'Empty',
      'gmail_id' => 'Empty',
      'hidden' => 'Empty',
      'deleted' => 'Empty',
      'date_updated' => 'Timestamp when profile updated',
      'portrait' => 'Empty',
      // Constructed at runtime from subdir
      'profile_photo' => 'URL of profile picture',
    );
    // Add in any custom profile fields
    $this->sourceFields += $this->profileFields('prof_ext_entry');
    $this->sourceFields += $this->profileFields('prof_ext_fulltext');

    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('user', 'u')
             ->fields('u')
             ->condition('account_status', 'deleted', '<>')
             ->orderBy('date_updated');
    // Add relevant fields from the profile table
    $query->innerJoin('profile', 'p', 'u.user_id=p.user_id');
    $query->fields('p', array('blog_id', 'profile_width', 'profile_height',
      'gender1', 'orientation', 'loc_city', 'loc_state', 'loc_country', 'loc_zip',
      'loc_zip2', 'loc_latitude', 'loc_longitude', 'home_city', 'home_state',
      'home_country', 'home_zip', 'school_name', 'school_state', 'school_city',
      'student_email', 'student_classification', 'school_major', 'for_dating',
      'for_friendships', 'for_relationships', 'for_hookups', 'for_networking',
      'religion', 'here_for', 'relationship_status','marital_status1',
      'children_status', 'children_number', 'ethnicity', 'body_type', 'height',
      'smoke', 'drink', 'skype_id', 'aim_id', 'yahoo_id', 'msn_id', 'gmail_id',
      'hidden', 'deleted', 'date_updated', 'portrait'));

    $this->source = new MigrateSourceSQL($query, $this->sourceFields, NULL,
          array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationUser();
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'user_id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Unique user ID',
                'alias' => 'u',
               ),
        ),
        MigrateDestinationUser::getKeySchema()
    );

    $this->highwaterField = array(
      'name' => 'date_updated',
      'alias' => 'p',
      'type' => 'int',
    );

    $this->addFieldMapping('name', 'subdir');
    $this->addFieldMapping('mail', 'email');
    $this->addFieldMapping('pass', 'password')
         ->description('Probably can\'t preserve');
    $this->addFieldMapping('created', 'date_created');
    $this->addFieldMapping('login', 'last_login');
    $this->addFieldMapping('access', 'last_activity');
    $this->addFieldMapping('status', 'account_status')
         ->callbacks(array($this, 'handleStatus'));
    $this->addFieldMapping('picture', 'user_id')
         ->sourceMigration('Picture');

    $this->addUnmigratedDestinations(array(
      'data',
      'init',
      'is_new',
      'language',
      'migrate_redirects',
      'path',
      'pathauto',
      'roles',
      'role_names',
      'signature', 'signature_format',
      'timezone',
      'theme',
    ));

    // These source fields seem unlikely to be useful - all others should be
    // mapped or explicitly ignored by the implementing class
    $this->addUnmigratedSources(array(
      'domain',
      'encoded_or_hashed_password',
      'network_id',
      'node_id',
      'username',   // Redundant with email address
    ));
  }

  /**
   * Determine what custom fields are available in a database table containing
   * a 'field_name' column.
   *
   * @param $tablename
   *  prof_ext_entry or prof_ext_fulltext
   *
   * @return array
   *  key: Generated machine name representing the field.
   *  value: The original (user-friendly) field name.
   */
  protected function profileFields($tablename) {
    $profile_fields = array();
    $result = Database::getConnection('default', $this->sourceConnection)
              ->select($tablename, 'p')
              ->fields('p', array('field_name'))
              ->distinct()
              ->execute()
              ->fetchCol();
    foreach ($result as $field_name) {
      $profile_fields[$this->keyName($field_name)] = $field_name;
    }
    return $profile_fields;
  }

  /**
   * Convert a user-friendly field name to a lower-case alpha-numeric string
   * more suitable for specifying field mappings.
   *
   * @param $field_name
   *
   * @return string
   */
  protected function keyName($field_name) {
    // Don't turn apostrophes into underscores.
    $key = str_replace("'", '', $field_name);
    $key = strtolower(preg_replace('|[^a-z0-9_]+|i', '_', $key));
    return $key;
  }

  /**
   * Implementation of Migration::prepareRow().
   *
   * @param $row
   *
   * @return bool
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Pick up custom field data from the two prof_ext tables.
    $result = Database::getConnection('default', $this->sourceConnection)
              ->select('prof_ext_entry', 'p')
              ->fields('p', array('int_value', 'char_value', 'field_name'))
              ->condition('user_id', $row->user_id)
              ->execute();
    foreach ($result as $profile_row) {
      $field_key = $this->keyName($profile_row->field_name);
      // NOTE: int_value case untested (only string fields have been seen)
      if ($profile_row->char_value == '' && $profile_row->int_value == 0) {
        $row->$field_key = $profile_row->int_value;
      }
      else {
        $row->$field_key = $profile_row->char_value;
      }
    }
    $result = Database::getConnection('default', $this->sourceConnection)
              ->select('prof_ext_fulltext', 'p')
              ->fields('p', array('text_value', 'field_name'))
              ->condition('user_id', $row->user_id)
              ->execute();
    foreach ($result as $profile_row) {
      $field_key = $this->keyName($profile_row->field_name);
      $row->$field_key = $profile_row->text_value;
    }
  }

  /**
   * Translate the ONEsite string status codes into Drupal user statuses.
   * @param $value
   *
   * @return int
   */
  protected function handleStatus($value) {
    switch ($value) {
      case 'good-standing':
      case 'pending':
        return 1;
      case 'deliquent':
      default:
        return 0;
    }
  }
}
