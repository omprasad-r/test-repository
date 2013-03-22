<?php
/**
 * @file
 * User migration class for googoodolls import.
 */

require_once drupal_get_path('module', 'migrate_googoo') . '/bbcode-filter.inc';

class GooGooUserMigration extends GooGooMigration {
  /**
   * Informational messages detected during prepareRow(), which must be stashed
   * until they can be saved.
   *
   * @var array
   */
  protected $messages = array();

  public function __construct() {
    parent::__construct();

    $this->description = 'Import of users from googoodolls.com';
    $this->dependencies[] = 'GooGooPicture';

    $fields = array(
      // smf_members table
      'ID_MEMBER' => 'Unique member ID',
      'memberName' => 'Username',
      'dateRegistered' => 'UNIX timestamp of registration date',
      'posts' => 'Number of posts',
      'ID_GROUP' => 'Always 15 except for admin account',
      'lngfile' => 'Always blank or english',
      'lastLogin' => 'UNIX timestamp of last login',
      'realName' => 'Always the same as memberName',
      'instantMessages' => 'Number of instant messages',
      'unreadMessages' => 'Number of unread messages',
      'im_ignore_list' => 'Comma-separated list of IDs, only 24 values',
      'passwd' => 'Account password, looks like MD5',
      'emailAddress' => 'Email address',
      'personalText' => 'Info about user?',
      'gender' => 'Values 0, 1, 2',
      'birthdate' => 'Birthdate in yyyy-mm-dd format',
      'websiteTitle' => 'Name of member website',
      'websiteUrl' => 'Url of member website',
      'location' => 'User city/state/country (free-form)',
      'ICQ' => 'Only 6 populated',
      'AIM' => '251 populated',
      'YIM' => '141 populated',
      'MSN' => '223 populated',
      'hideEmail' => 'Boolean, only 28 zeros',
      'showOnline' => 'Always 1',
      'timeFormat' => 'e.g. %B %d, %Y, %H:%M:%S',
      'signature' => 'Forum signature',
      'timeOffset' => 'Timezone offset?',
      'avatar' => 'Values like GGD/icon5.jpg',
      'im_email_notify' => 'Boolean, only 82 zeros',
      'karmaBad' => 'Boolean',
      'karmaGood' => 'Boolean',
      'usertitle' => 'Always blank',
      'notifyAnnouncements' => 'Boolean, only 21 zeros',
      'notifyOnce' => 'Boolean, only 83 zeros',
      'memberIP' => 'Member IP address',
      'secretQuestion' => 'Always blank',
      'secretAnswer' => 'Always blank',
      'ID_THEME' => 'Usually 0',
      'is_activated' => 'Always 1',
      'validation_code' => '189 records with hex values',
      'ID_MSG_LAST_VISIT' => 'Last message viewed?',
      'additionalGroups' => '1 non-blank value',
      'smileySet' => 'Always blank',
      'ID_POST_GROUP' => '?',
      'totalTimeLoggedIn' => 'Seconds logged in?',
      'passwordSalt' => 'Always blank',
      // ab_account table
      'city' => 'Member city',
      'state' => 'Member state',
      'zip' => 'Member zip code',
    );
    $query = Database::getConnection('default', 'googoo_smf')
             ->select('smf_members', 'm')
             ->fields('m');
    $query->leftJoin('googoo_agilebill.ab_account', 'a', 'm.emailAddress=a.email');
    $query->fields('a', array('city', 'state', 'zip'));
    $this->source = new MigrateSourceSQL($query, $fields, NULL,
          array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationUser(array('md5_passwords' => TRUE));
    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'memberName' =>
          array('type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'description' => 'Unique username',
               ),
      ),
      MigrateDestinationUser::getKeySchema()
    );

    $this->addFieldMapping('name', 'memberName')
         ->dedupe('users', 'name');
    $this->addFieldMapping('field_member_name', 'memberName');
    $this->addFieldMapping('field_nickname', 'memberName');
    $this->addFieldMapping('created', 'dateRegistered');
    $this->addFieldMapping('login', 'lastLogin');
    $this->addFieldMapping('access', 'lastLogin');
    $this->addFieldMapping('pass', 'passwd');
    $this->addFieldMapping('mail', 'emailAddress');
    $this->addFieldMapping('signature', 'signature')
         ->callbacks(array($this, 'handleSignature'));
    $this->addFieldMapping('field_about_me', 'personalText');
    $this->addFieldMapping('field_gender', 'gender')
         ->callbacks(array($this, 'handleGender'));
    $this->addFieldMapping('field_birthdate', 'birthdate')
         ->callbacks(array($this, 'handleBirthdate'));
    $this->addFieldMapping('timezone', 'timeOffset')
         ->callbacks(array($this, 'handleTimezone'));
    $this->addFieldMapping('status')
         ->defaultValue(1);
    $this->addFieldMapping('role_names')
         ->defaultValue('fan club member');

    $this->addFieldMapping('picture', 'avatar')
         ->sourceMigration('GooGooPicture');

    $this->addFieldMapping('field_city', 'city');
    $this->addFieldMapping('field_state', 'state');
    $this->addFieldMapping('field_postal_code', 'zip');
    $this->addFieldMapping('field_country', 'country');

    $this->addUnmigratedSources(array(
      'additionalGroups',
      'AIM',
      'hideEmail',
      'ICQ',
      'ID_GROUP',
      'ID_MEMBER',
      'ID_MSG_LAST_VISIT',
      'ID_POST_GROUP',
      'ID_THEME',
      'im_email_notify',
      'im_ignore_list',
      'instantMessages',
      'is_activated',
      'karmaBad',
      'karmaGood',
      'lngfile',
      'location',
      'memberIP',
      'MSN',
      'notifyAnnouncements',
      'notifyOnce',
      'passwordSalt',
      'personalText',
      'posts',
      'realName',
      'secretAnswer',
      'secretQuestion',
      'showOnline',
      'smileySet',
      'timeFormat',
      'totalTimeLoggedIn',
      'unreadMessages',
      'usertitle',
      'validation_code',
      'websiteTitle',
      'websiteUrl',
      'YIM',
    ));

    $this->addUnmigratedDestinations(array(
      'data',
        'field_about_me:language',
        'field_city:language',
      'field_comment',
      'field_eos_id',
      'field_favorite_bands', 'field_favorite_bands:language',
      'field_hobbies', 'field_hobbies:language',
        'field_member_name:language',
        'field_nickname:language',
        'field_postal_code:language',
      'field_profile_14', 'field_profile_14:language',
      'field_profile_updated',
      'field_provider', 'field_provider:language',
      'field_site_id', 'field_site_id:language',
        'field_state:language',
      'field_twitter_name', 'field_twitter_name:language',
      'field_type',
      'field_usage_type',
      'init',
      'is_new',
      'language',
      'migrate_redirects',
      'path',
      'roles',
      'signature_format',
      'theme',
    ));
  }

  /**
   * Translate gender values to Gardens values.
   *
   * @param $value
   *
   * @return string
   */
  protected function handleGender($value) {
    switch ($value) {
      case 1:
        $value = 'Male';
        break;
      case 2:
        $value = 'Female';
        break;
      case 0:
      default:
        $value = 'N/A';
        break;
    }
    return $value;
  }

  /**
   * Ignore bogus birthdates.
   *
   * @param $value
   *
   * @return string
   */
  protected function handleBirthdate($value) {
    $year = substr($value, 0, 4);
    if ($year < '1900') {
      $value = '';
    }
    return $value;
  }

  /**
   * Translate timezone offsets like -8.
   *
   * @param $value
   *
   * @return mixed
   */
  protected function handleTimezone($value) {
    // @todo
    return $value;
  }

  /**
   * Truncate a signature to 255 characters.
   * @param $value
   *
   * @return string
   */
  protected function handleSignature($value) {
    // Translate bbcode to HTML first
    $value = _bbcode_filter_process($value);

    // If too long, truncate and make sure we properly close tags.
    if (strlen($value) > 255) {
      $this->saveMessage(t('Truncating signature, original HTML: !value',
                            array('!value' => $value)), Migration::MESSAGE_INFORMATIONAL);
      $value = substr($value, 0, 235);
      $value = _filter_htmlcorrector($value);
      if (strlen($value) > 255) {
        $value = strip_tags($value);
        if (strlen($value) > 255) {
          $value = substr($value, 0, 255);
        }
      }
    }

    return $value;
  }

  /**
   * Implements Migration::prepareRow().
   *
   * @param $row
   *
   * @return bool
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // If this email address already exists, just map to the existing uid
    // and don't allow it to be deleted.
    $new_uid = db_select('users', 'u')
               ->fields('u', array('uid'))
               ->condition('mail', $row->emailAddress)
               ->execute()
               ->fetchField();
    if ($new_uid) {
      $this->map->saveIDMapping($row, array($new_uid), MigrateMap::STATUS_IGNORED,
        MigrateMap::ROLLBACK_PRESERVE);
      $this->rollbackAction = MigrateMap::ROLLBACK_PRESERVE;
      return FALSE;
    }

    // If the "state" looks like a country, assign it as the country and leave
    // the state blank.
    static $countries;
    if (!isset($countries)) {
      // Get the country list from the field configuration. Lower-case
      $field = field_read_field('field_country');
      $countries = list_allowed_values($field);
      $countries = array_change_key_case($countries);
    }
    if (isset($countries[strtolower($row->state)])) {
      $row->country = $row->state;
      unset($row->state);
    }
    else {
      $row->country = 'United States';
    }
  }
}
