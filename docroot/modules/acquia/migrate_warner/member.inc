<?php

class MemberMigration extends Migration {
  public function __construct() {
    parent::__construct(MigrateGroup::getInstance('warner'));
    $this->description = t('Members');
    $this->softDependencies = array('Field', 'Picture');
    $this->team = array(
      new MigrateTeamMember('Mike Ryan', 'mike.ryan@acquia.com', 'Acquia'),
      new MigrateTeamMember('Arun De Joe', 'DeJoe.Arun@wmg.com', 'Warner'),
    );

    $this->map = new MigrateSQLMap($this->machineName,
        array('email' => array(
                'type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                )
             ),
        MigrateDestinationUser::getKeySchema()
    );

    $stream = new MigrateWarnerStreamWrapper();
    $localpath = $stream->getRealPath('warner://') . '/' . variable_get('warner_private_directory', '');;
    foreach (glob("$localpath/*.csv") as $source_file) {
      break;
    }

    $csvcolumns = MigrationBase::getInstance('Field')->getFields();
    $this->source = new MigrateSourceCSV($source_file, $csvcolumns,
      array('header_rows' => 18, 'embedded_newlines' => TRUE, 'cache_counts' => TRUE));
    $this->destination = new MigrateDestinationUser();

    // Won't use Drupal username - just dedupe
    $this->addFieldMapping('name', 'member_name')
         ->callbacks(array($this, 'fixUsername'))
         ->dedupe('users', 'name')
         ->description('We need "usernames" to be non-unique, so use field_member_name for them and dedupe here.');
    $this->addFieldMapping('mail', 'email');
    $this->addFieldMapping('created', 'registered')
         ->callbacks(array($this, 'processDateTimestamp'));
    $this->addFieldMapping('login', 'logged_in')
         ->callbacks(array($this, 'processDateTimestamp'));
    // authenticated user
    $this->addFieldMapping('roles')
         ->defaultValue(2);
    $this->addFieldMapping('field_member_name', 'member_name');
    $this->addFieldMapping('field_type', 'type')
         ->description("Will determine roles");
    $this->addFieldMapping('field_profile_updated', 'profile_updated');
    $this->addFieldMapping('field_usage_type', 'usage_type')
         ->description('Basically user badges - archival only');
    $this->addFieldMapping('field_birthdate', 'birthdate')
         ->callbacks(array($this, 'processDate'));
    $this->addFieldMapping('field_eos_id', 'eos_id');
    $this->addFieldMapping('field_site_id', 'site_id');
    $this->addFieldMapping('status')
         ->defaultValue(1);
    $this->addFieldMapping('picture', 'email')
         ->sourceMigration('Picture');
    $this->addFieldMapping(NULL, 'comment_notify')
         ->description('Use comment_notify module. Data is slash-separated list of content types')
         ->issueGroup('Cyrve');

    // Mappings for dynamic fields
    foreach (MigrationBase::getInstance('Field')->getProfileFields() as $field) {
      $field_name = $field['field_name'];
      $this->addFieldMapping('field_' . $field_name, $field_name)
           ->description('Custom field: ' . $field['label']);
    }

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('theme', 'signature', 'signature_format',
      'access', 'init', 'path', 'pathauto', 'timezone', 'language', 'is_new'));
    $this->addFieldMapping('pass')
      ->description('Needs to be reset')
      ->issueGroup('DNM');

    // Unmapped source fields
    $this->addFieldMapping(NULL, 'groups')
         ->description('Indicates enhanced access (roles), out of scope for this phase')
         ->issueGroup('DNM');
    $this->addFieldMapping(NULL, 'fbconnect_enabled')
         ->description('Facebook integration in Gardens? No data in sample site')
         ->issueGroup('Acquia');
    $this->addFieldMapping(NULL, 'fb_comments')
         ->description('Facebook integration in Gardens? No data in sample site')
         ->issueGroup('Acquia');
    $this->addFieldMapping(NULL, 'fb_upload')
         ->description('Facebook integration in Gardens? No data in sample site')
         ->issueGroup('Acquia');
    $this->addFieldMapping(NULL, 'fb_discussion')
         ->description('Facebook integration in Gardens? No data in sample site')
         ->issueGroup('Acquia');
    $this->addFieldMapping(NULL, 'fb_blog')
         ->description('Facebook integration in Gardens? No data in sample site')
         ->issueGroup('Acquia');
    $this->addFieldMapping(NULL, 'allowed_apps')
         ->description('Eos only')
         ->issueGroup('DNM');
    $this->addFieldMapping(NULL, 'mobile_email')
         ->issueGroup('DNM');
    $this->addFieldMapping(NULL, 'visits_this_month')
         ->description('Try to match stats against any tracked in Gardens')
         ->issueGroup('Cyrve');
    $this->addFieldMapping(NULL, 'usage_level')
         ->issueGroup('DNM');
    $this->addFieldMapping(NULL, 'influence_score')
         ->issueGroup('DNM');
  }

  /**
   * Eos allows all sorts of garbage in usernames - clean them up for Drupal.
   * @param $name
   * @return mixed
   */
  protected function fixUsername($name) {
    // Patterns are from user_validate_name
    $name = preg_replace('/[^\x{80}-\x{F7} a-z0-9@_.\'-]/i', '',  $name);
    $name = preg_replace('/[\x{80}-\x{A0}' .         // Non-printable ISO-8859-1 + NBSP
                      '\x{AD}' .                // Soft-hyphen
                      '\x{2000}-\x{200F}' .     // Various space characters
                      '\x{2028}-\x{202F}' .     // Bidirectional text overrides
                      '\x{205F}-\x{206F}' .     // Various text hinting characters
                      '\x{FEFF}' .              // Byte order mark
                      '\x{FF01}-\x{FF60}' .     // Full-width latin
                      '\x{FFF9}-\x{FFFD}' .     // Replacement characters
                      '\x{0}-\x{1F}]/u',        // NULL byte and control characters
                      '', $name);
    $name = preg_replace('/ +/', ' ', $name);
    $name = trim($name);
    return $name;
  }

  /**
   * NULL out any dates that strtotime() can't handle.
   */
  protected function processDate($date) {
    if (!strtotime($date)) {
      $date = NULL;
    }
    return $date;
  }

  /**
   * Same as processDate, but return as a timestamp
   */
  protected function processDateTimestamp($date) {
    $original = $date;
    $date = $this->processDate($date);
    if ($date) {
      $date = MigrationBase::timestamp($date);
    }
    return $date;
  }

  public function prepareRow($row) {
    // Make sure empty birthdate fields don't exist, will generate notices
    if (empty($row->birthdate)) {
      unset($row->birthdate);
    }

    // If an account already exists, skip it on initial insert
    if (empty($row->migrate_map_destid1)) {
      $uid = db_select('users', 'u')
                  ->fields('u', array('uid'))
                  ->condition('mail', $row->email)
                  ->execute()
                  ->fetchField();
      if ($uid) {
        return FALSE;
      }
    }

    // If there's no incoming username, use the name portion of the email
    if (empty($row->member_name)) {
      $split = explode('@', $row->email);
      $row->member_name = $split[0];
    }
  }

  public function prepare($account, $row) {
    // Load the whitelist once
    static $whitelist;
    static $site_maintainer_role;
    static $fan_club_role;
    static $professional_role;
    static $cd_role;
    if (empty($whitelist)) {
      $file = drupal_get_path('module', 'migrate_warner') . '/whitelist.csv';
      $handle = fopen($file, 'r');
      if ($handle) {
        while ($email = strtolower(trim(fgets($handle)))) {
          $whitelist[$email] = $email;
        }
        fclose($handle);
      }
      else {
        drush_print("Could not open whitelist file: $file");
      }
      $site_maintainer_role = db_select('role', 'r')
                              ->fields('r', array('rid'))
                              ->condition('name', 'site maintainer')
                              ->execute()
                              ->fetchField();
      $fan_club_role = db_select('role', 'r')
                       ->fields('r', array('rid'))
                       ->condition('name', 'fan club member')
                       ->execute()
                       ->fetchField();
      $professional_role = db_select('role', 'r')
                       ->fields('r', array('rid'))
                       ->condition('name', 'professional member')
                       ->execute()
                       ->fetchField();
      $cd_role = db_select('role', 'r')
                 ->fields('r', array('rid'))
                 ->condition('name', 'connected CD')
                 ->execute()
                 ->fetchField();
    }

    if (!empty($whitelist[strtolower($account->mail)])) {
      $account->roles[$site_maintainer_role] = $site_maintainer_role;
    }

    // Set the fan club member role if we're loading from a fan club dump
    if (variable_get('warner_fan_club_dump', 0)) {
      $account->roles[$fan_club_role] = $fan_club_role;
    }

    // Also set the fan club or connected CD roles for those in particular groups
    switch (strtolower($row->groups)) {
      case 'bayou club':
      case 'eye of the storm':
      case 'hooligans':
      case 'kid rock congregation':
      case 'shinedown nation':
      case 'sncchasers':
      case 'taylor gang':
      case 'the insiders':
      case 'the panheads':
      case "trey's angels":
      case 'waka fans':
        $account->roles[$fan_club_role] = $fan_club_role;
        break;
      case 'get your heart on!':
      case 'lovestrong':
      case 'team tank':
      case 'valdosta':
        $account->roles[$cd_role] = $cd_role;
        break;
    }

    // Set the professional member role if the type is 'pro'
    if ($row->type == 'pro') {
      $account->roles[$professional_role] = $professional_role;
    }

    // Workaround for http://drupal.org/node/1358318 - prevent re-deduping
    // (assumes usernames are not changed in source)
    if (isset($account->uid)) {
      $account->name = db_select('users', 'u')
                       ->fields('u', array('name'))
                       ->condition('uid', $account->uid)
                       ->execute()
                       ->fetchField();
    }
    // All characters stripped from original, take from email
    else if (!$account->name) {
      $account->name = strtok($account->mail, '@');
      if (!$account->name) {
        $account->name = $account->mail;
      }
      // Dedupe
      $account->name = $this->handleDedupe(array('table' => 'users',
        'column' => 'name'), $account->name, NULL);
    }
  }
}
