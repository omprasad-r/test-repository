<?php
/**
 * @file
 * User migration class for Roadrunner.
 */

/**
 * User migration class for Roadrunner.
 */
class RRUserMigration extends RRMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t("Migrate users from the source database");
    $this->dependencies = array($this->group->getName() . 'Picture');

    $fields = array(
      // From users
      'id' => 'Unique user ID',
      'login' => 'username',
      'permalink' => 'URL alias to user page?',
      'avatar_file_name' => 'Filename of avatar',
      'avatar_content_type' => 'MIME type of avatar',
      'avatar_file_size' => 'Avatar size in bytes',
      'email' => 'User email address',
      'first_name' => 'User first name',
      'crypted_password' => 'User password, encrypted (how?)',
      'salt' => 'Salt for user password',
      'fanclub_id' => 'Reference to fanclub, what does this one mean?',
      'is_admin' => 'Map to administrator role?',
      'updated_at' => 'Account last updated',
      'created_at' => 'Account created',
      'comments_count' => 'Number of comment',
      'ratings_count' => 'Number of ratings',
      'flags_count' => 'Number of flags',
      'board_posts_count' => 'Number of board posts',
      'sitewide_score' => 'Sitewide score?',
      'topics_count' => 'Number of topic posts',
      'is_suspended' => 'User should be blocked',
      'wants_notified' => '?',
      'last_name' => 'User last name',
      'push_user_id' => '?',
      'name' => 'Full name',
      'legacy_id' => 'From previous CMS?',
      'legacy_password' => 'From previous CMS?',
      'legacy_password_salt' => 'From previous CMS?',
      'referral_id' => '?',
      'phone' => 'Phone number',
      'shirt_size' => 'Shirt size',
      'ship_to_address1' => 'Address line 1',
      'ship_to_address2' => 'Address line 2',
      'ship_to_city' => 'City',
      'ship_to_state' => 'State',
      'ship_to_zip' => 'Zip code',
      'ship_to_country' => 'Country',
      'rr_id' => '?',
      'email_opt_in' => '1 or 0',
      'ip_id' => '?',
      'previous_login' => 'Previous time logged in',
      'signature' => 'Signature',
      'login_time' => 'Last time logged in',
      'banned_by' => 'Account that banned this account',
      'ban_time' => 'When account banned',
      'ban_reason' => 'Reason account banned',
      'ban_times' => 'How many times banned',
      'location' => 'Country (sometimes)',
      'website' => 'Website',
      'rank_id' => '?',
      'date_display' => 'Date display format',
      'time_display' => 'Time display format',
      'time_zone' => 'Timezone',
      'auto_subscribe' => 'Auto-subscribe to what?',
      'identifier' => '?',
      'email_verified' => '1 or 0',
      'legacy_source' => '?',
      'facebook_uid' => 'Facebook id',
      'facebook_access_token' => 'Facebook access token',
      'rr_user_id' => '?',
      'changed_email' => '?',
      'rr_access_token' => '?',
      'receive_admin_summary' => '?',
      'has_whitelist' => '?',
      // From memberships
      'validated_at' => 'Membership validated',
      'member_created_at' => 'Membership created time',
      'member_updated_at' => 'Membership updated time',
      'is_publisher' => '1 or 0',
      'is_moderator' => '1 or 0',
      'is_band_member' => '1 or 0',
      'next_renewal_at' => '?',
      'user_key' => '?',
      'member_email_opt_in' => '?',
      'is_tour_manager' => '1 or 0',
      'member_facebook_access_token' => '?',
      'remember_token' => '?',
      'remember_token_expires_at' => '?',
      'receive_publisher_summary' => '?',
      // Generated fields
      'computed_status' => 'The Drupal status determined from the source data',
      'roles' => 'List of roles based on booleans',
    );

    $query = $this->connection
      ->select('users', 'u')
      ->fields('u');
    $query->innerJoin('memberships', 'm', 'u.id=m.user_id');
    $query->fields('m', array('validated_at', 'is_publisher', 'is_moderator',
        'is_band_member', 'next_renewal_at', 'user_key', 'is_tour_manager',
        'remember_token', 'remember_token_expires_at', 'receive_publisher_summary'));
    $query->addField('m', 'created_at', 'member_created_at');
    $query->addField('m', 'updated_at', 'member_updated_at');
    $query->addField('m', 'email_opt_in', 'member_email_opt_in');
    $query->addField('m', 'facebook_access_token', 'member_facebook_access_token');
    $query->condition('m.fanclub_id', $arguments['artist_id']);
    $query->orderBy('updated_at');

    $this->source = new MigrateSourceSQL($query, $fields, NULL,
      array('map_joinable' => FALSE));

    $this->destination = new MigrateDestinationUser();

    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'id' => array('type' => 'int',
                             'unsigned' => TRUE,
                             'not null' => TRUE,
                             'description' => 'User ID',
                            )
        ),
        MigrateDestinationUser::getKeySchema()
      );

    $this->highwaterField = array(
      'name' => 'updated_at',
      'alias' => 'u',
    );

    $this->addFieldMapping('name', 'login')
         ->dedupe('users', 'name');
    $this->addFieldMapping('mail', 'email');
    $this->addFieldMapping('created', 'member_created_at');
    $this->addFieldMapping('login', 'login_time');
    $this->addFieldMapping('access', 'login_time');
    $this->addFieldMapping('status', 'computed_status');
    $this->addFieldMapping('signature', 'signature');
    $this->addFieldMapping('signature_format')
         ->defaultValue('1');
    $this->addFieldMapping('picture', 'id')
         ->sourceMigration($this->group->getName() . 'Picture');
    $this->addFieldMapping('role_names', 'roles');
    $this->addFieldMapping('timezone', 'time_zone')
         ->callbacks(array($this, 'handleTimezone'));

    $this->addFieldMapping('field_member_name', 'name');
    $this->addFieldMapping('field_nickname', 'name');
    $this->addFieldMapping('field_city', 'ship_to_city');
    $this->addFieldMapping('field_state', 'ship_to_state');
    $this->addFieldMapping('field_postal_code', 'ship_to_zip');
    $this->addFieldMapping('field_profile_updated', 'member_updated_at');
    $this->addFieldMapping('field_country', 'ship_to_country')
         ->callbacks(array($this, 'handleCountry'));

    $this->addFieldMapping(NULL, 'permalink')
         ->description('Map to path or migrate_redirects?')
         ->issueGroup('Acquia')
         ->issuePriority(MigrateFieldMapping::ISSUE_PRIORITY_MEDIUM);

    $this->addUnmigratedDestinations(array(
      'data',
      'field_about_me', 'field_about_me:language',
      'field_birthdate',
        'field_city:language',
      'field_comment',
      'field_eos_id',
      'field_favorite_bands', 'field_favorite_bands:language',
      'field_gender',
      'field_hobbies', 'field_hobbies:language',
        'field_member_name:language',
        'field_nickname:language',
        'field_postal_code:language',
      'field_profile_14', 'field_profile_14:language',
      'field_provider', 'field_provider:language',
      'field_site_id', 'field_site_id:language',
        'field_state:language',
      'field_twitter_name', 'field_twitter_name:language',
      'field_type',
      'field_usage_type',
      'init',
      'is_new',
      'language',
      'pass',
      'path',
      'pathauto',
      'roles',
      'theme',
    ));

    $this->addUnmigratedSources(array(
      'auto_subscribe',
      'avatar_content_type',
      'avatar_file_size',
      'ban_reason',
      'ban_time',
      'ban_times',
      'banned_by',
      'board_posts_count',
      'changed_email',
      'comments_count',
      'created_at',
      'crypted_password',
      'date_display',
      'email_opt_in',
      'email_verified',
      'facebook_access_token',
      'facebook_uid',
      'fanclub_id',
      'first_name',
      'flags_count',
      'has_whitelist',
      'identifier',
      'ip_id',
      'is_admin',
      'is_band_member',
      'is_moderator',
      'is_publisher',
      'is_suspended',
      'is_tour_manager',
      'last_name',
      'legacy_id',
      'legacy_password',
      'legacy_password_salt',
      'legacy_source',
      'location',
      'member_email_opt_in',
      'member_facebook_access_token',
      'next_renewal_at',
      'phone',
      'previous_login',
      'push_user_id',
      'rank_id',
      'ratings_count',
      'receive_admin_summary',
      'receive_publisher_summary',
      'referral_id',
      'remember_token',
      'remember_token_expires_at',
      'rr_access_token',
      'rr_id',
      'rr_user_id',
      'salt',
      'ship_to_address1',
      'ship_to_address2',
      'shirt_size',
      'sitewide_score',
      'time_display',
      'topics_count',
      'updated_at',
      'validated_at',
      'user_key',
      'wants_notified',
      'website',
    ));
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

    // Is the user blocked?
    if ($row->is_suspended == 1) {
      $row->computed_status = 0;
    }
    else {
      $row->computed_status = 1;
    }

    // If the full name isn't populated, fill it in from first/last
    if (empty($row->name)) {
      $row->name = trim($row->first_name . ' ' . $row->last_name);
    }

    // Assign roles
    $row->roles = array('fan club member');
    if ($row->is_admin) {
      $row->roles[] = 'administrator';
    }
    if ($row->is_publisher) {
      $row->roles[] = 'site maintainer';
    }
    if ($row->is_moderator) {
      $row->roles[] = 'site maintainer';
    }
    if ($row->is_band_member) {
      $row->roles[] = 'professional member';
    }
    if ($row->is_tour_manager) {
      $row->roles[] = 'developer';
    }
  }

  /**
   * Callback to translate from various timezone representations (mostly city
   * names) to IANA form.
   *
   * @param $value
   *
   * @return mixed
   */
  protected function handleTimezone($value) {
    if (!empty($this->timezones[$value])) {
      $value = $this->timezones[$value];
    }
    elseif (!empty($value)) {
      $this->queueMessage(t('Couldn\'t identify timezone !timezone',
        array('!timezone' => $value)), Migration::MESSAGE_INFORMATIONAL);
    }
    return $value;
  }

  protected function handleCountry($value) {
    // Translate from two-letter codes to field_country allowed_values
    $value = trim($value);
    $upper_value = strtoupper($value);
    if (!empty($this->countries[$upper_value])) {
      return $this->countries[$upper_value];
    }
    else {
      if ($value && (in_array(strtolower($value), array_map('strtolower', $this->countries)) === FALSE)) {
        $this->queueMessage(t('Couldn\'t identify country !country',
          array('!country' => $value)), Migration::MESSAGE_INFORMATIONAL);
      }
      return '';
    }
  }

  protected $timezones = array(
    '-9.5' => 'Pacific/Marquesas',
    'Abu Dhabi' => 'Asia/Dubai',
    'Adelaide' => 'Australia/Adelaide',
    'AEST' => 'Australia/Melbourne',
    'Alaska' => 'America/Anchorage',
    'Almaty' => 'Asia/Almaty',
    'Amsterdam' => 'Europe/Amsterdam',
    'Arizona' => 'America/Pheonix',
    'AST' => '',  // Ambiguous
    'Astana' => 'Asia/Almaty',
    'Athens' => 'Europe/Athens',
    'Atlantic Time (Canada)' => 'America/Halifax',
    'Auckland' => 'Pacific/Auckland',
    'AWST' => 'Australia/Perth',
    'Azores' => 'Atlantic/Azores',
    'Bangkok' => 'Asia/Bangkok',
    'Beijing' => 'Asia/Shanghai',
    'Belgrade' => 'Europe/Belgrade',
    'Berlin' => 'Europe/Berlin',
    'Bern' => 'Europe/Zurich',
    'BGT' => '',  // Not a known timezone
    'Bogota' => 'America/Bogota',
    'Brasilia' => 'America/Sao_Paulo',
    'Bratislava' => 'Europe/Bratislava',
    'Brisbane' => 'Australia/Brisbane',
    'Brussels' => 'Europe/Brussels',
    'Bucharest' => 'Europe/Bucharest',
    'Budapest' => 'Europe/Budapest',
    'Buenos Aires' => 'America/Argentina/Buenos_Aires',
    'BZT' => 'America/Sao_Paulo',
    'Cairo' => 'Africa/Cairo',
    'Canberra' => 'Australia/Canberra',
    'Caracas' => 'America/Caracas',
    'Casablanca' => 'Africa/Casablanca',
    'Central America' => 'America/Panama',
    'Central Time (US & Canada)' => 'America/Chicago',
    'Chihuahua' => 'America/Chihuahua',
    'Copenhagen' => 'Europe/Copenhagen',
    'CST' => 'America/Chicago',
    'Darwin' => 'Australia/Darwin',
    'Dhaka' => 'Asia/Dhaka',
    'Dublin' => 'Europe/Dublin',
    'Eastern Time (US & Canada)' => 'America/New_York',
    'Edinburgh' => 'UTC',
    'EET' => 'Europe/Bucharest',
    'Ekaterinburg' => 'Asia/Yekaterinburg',
    'EST' => 'America/New_York',
    'Fiji' => 'Pacific/Fiji',
    'Georgetown' => 'America/Guyana',
    'GMT' => 'UTC',
    'Greenland' => 'America/Scoresbysund',
    'Guadalajara' => 'America/Mexico_City',
    'Hanoi' => 'Asia/Ho_Chi_Minh',
    'Hawaii' => 'Pacific/Honolulu',
    'Helsinki' => 'Europe/Helsinki',
    'Hobart' => 'Australia/Hobart',
    'Hong Kong' => 'Asia/Hong_Kong',
    'Indiana (East)' => 'America/Indiana/Indianapolis',
    'International Date Line West' => 'Pacific/Midway',
    'Islamabad' => 'Asia/Karachi',
    'Istanbul' => 'Asia/Istanbul',
    'Jakarta' => 'Asia/Jakarta',
    'Jerusalem' => 'Asia/Jerusalem',
    'JST' => 'Asia/Tokyo',
    'JVT' => 'Asia/Jakarta',
    'Kabul' => 'Asia/Kabul',
    'Krasnoyarsk' => 'Asia/Krasnoyarsk',
    'Kuala Lumpur' => 'Asia/Kuala_Lumpur',
    'Kuwait' => 'Asia/Kuwait',
    'Kyev' => 'Europe/Kiev',
    'La Paz' => 'America/La_Paz',
    'Lima' => 'America/Lima',
    'Lisbon' => 'Europe/Lisbon',
    'Ljubljana' => 'Europe/Belgrade',
    'London' => 'Europe/London',
    'Madrid' => 'Europe/Madrid',
    'Magadan' => 'Asia/Magadan',
    'Mazatlan' => 'America/Mazatlan',
    'Melbourne' => 'Australia/Melbourne',
    'MET' => 'Europe/Brussels',
    'Mexico City' => 'America/Mexico_City',
    'Mid-Atlantic' => 'Atlantic/South_Georgia',
    'Midway Island' => 'Pacific/Midway',
    'Minsk' => 'Europe/Minsk',
    'Monrovia' => 'Africa/Monrovia',
    'Monterrey' => 'America/Monterrey',
    'Moscow' => 'Europe/Moscow',
    'Mountain Time (US & Canada)' => 'America/Denver',
    'MST' => 'America/Denver',
    'Mumbai' => 'Asia/Kolkata',
    'Nairobi' => 'Africa/Nairobi',
    'New Delhi' => 'Asia/Kolkata',
    'Newfoundland' => 'America/St_Johns',
    'Novosibirsk' => 'Asia/Novosibirsk',
    'Nuku\'alofa' => 'Pacific/Tongatapu',
    'NZT' => 'Pacific/Auckland',
    'Osaka' => 'Asia/Tokyo',
    'Pacific Time (US & Canada)' => 'America/Los_Angeles',
    'Paris' => 'Europe/Paris',
    'Perth' => 'Australia/Perth',
    'Prague' => 'Europe/Prague',
    'Pretoria' => 'Africa/Johannesburg',
    'PST' => 'America/Los_Angeles',
    'Quito' => 'America/Guayaquil',
    'R3T' => 'Europe/Kaliningrad',
    'R4T' => 'Europe/Moscow',
    'R5T' => 'Asia/Oral',
    'Riga' => 'Europe/Riga',
    'Riyadh' => 'Asia/Riyadh',
    'Rome' => 'Europe/Rome',
    'Samoa' => 'Pacific/Samoa',
    'Santiago' => 'America/Santiago',
    'Sapporo' => 'Asia/Tokyo',
    'Sarajevo' => 'Europe/Sarajevo',
    'Saskatchewan' => 'America/Regina',
    'Seoul' => 'Asia/Seoul',
    'Singapore' => 'Asia/Singapore',
    'Skopje' => 'Europe/Skopje',
    'Sofia' => 'Europe/Sofia',
    'Solomon Is.' => 'Pacific/Guadalcanal',
    'St. Petersburg' => 'Europe/Moscow',
    'Stockholm' => 'Europe/Stockholm',
    'Sydney' => 'Australia/Sydney',
    'Taipei' => 'Asia/Taipei',
    'Tallinn' => 'Europe/Tallinn',
    'Tashkent' => 'Asia/Tashkent',
    'Tehran' => 'Asia/Tehran',
    'Tijuana' => 'America/Tijuana',
    'Tokyo' => 'Asio/Tokyo',
    'UTC' => 'UTC',
    'Vienna' => 'Europe/Vienna',
    'Vilnius' => 'Europe/Vilnius',
    'Volgograd' => 'Europe/Volgograd',
    'Warsaw' => 'Europe/Warsaw',
    'WAT' => 'Africa/Kinshasa',
    'Wellington' => 'Pacific/Auckland',
    'YST' => 'America/Yakutat',
    'Zagreb' => 'Europe/Zagreb',
  );

  protected $countries = array(
    'AF' => 'Afghanistan',
    'AL' => 'Albania',
    'DZ' => 'Algeria',
    'AS' => 'American Samoa',
    'AD' => 'Andorra',
    'AO' => 'Angola',
    'AI' => 'Anguilla',
    'AQ' => 'Antarctica',
    'AG' => 'Antigua And Barbuda',
    'AR' => 'Argentina',
    'AM' => 'Armenia',
    'AW' => 'Aruba',
    'AU' => 'Australia',
    'AUSTRAILA' => 'Australia',
    'AT' => 'Austria',
    'VIENNA' => 'Austria',
    'AZ' => 'Azerbaijan',
    'BS' => 'Bahamas',
    'BH' => 'Bahrain',
    'BD' => 'Bangladesh',
    'BB' => 'Barbados',
    'BY' => 'Belarus',
    'BE' => 'Belgium',
    'BELGIQUE' => 'Belgium',
    'BELGIË' => 'Belgium',
    'BZ' => 'Belize',
    'BJ' => 'Benin',
    'BM' => 'Bermuda',
    'BT' => 'Bhutan',
    'BO' => 'Bolivia',
    'LA PAZ' => 'Bolivia',
    'BA' => 'Bosnia & Herzegovina',
    'BOSNIA AND HERZEGOVINA' => 'Bosnia & Herzegovina',
    'REBULIKA SRPSKA' => 'Bosnia & Herzegovina',
    'BW' => 'Botswana',
    'BV' => 'Bouvet Island',
    'BR' => 'Brazil',
    'BRASIL' => 'Brazil',
    'BRASIL/BRAZIL!' => 'Brazil',
    'SÃO PAULO' => 'Brazil',
    'IO' => 'British Ind Ocean Ter',
    'BN' => 'Brunei Darussalam',
    'BRUNEI' => 'Brunei Darussalam',
    'BG' => 'Bulgaria',
    'BF' => 'Burkina Faso',
    'BI' => 'Burundi',
    'KH' => 'Cambodia',
    'CM' => 'Cameroon',
    'CAMAROON' => 'Cameroon',
    'CA' => 'Canada',
    'ONTARIO' => 'Canada',
    'CANDA' => 'Canada',
    'CV' => 'Cape Verde',
    'CAPE VERE ISLANDS' => 'Cape Verde',
    'KY' => 'Cayman Islands',
    'CF' => 'Central African Republic',
    'CENTRAL AFRICAN REP' => 'Central African Republic',
    'TD' => 'Chad',
    'CL' => 'Chile',
    'SANTIAGO' => 'Chile',
    'CN' => 'China',
    'CHINA, PEOPLE\'S REPUBLIC OF' => 'China',
    'CX' => 'Christmas Island',
    'CC' => 'Cocos (Keeling Is)',
    'CO' => 'Columbia',
    'COLOMBIA' => 'Columbia',
    'KM' => 'Comoros',
    'CG' => 'Congo',
    'CK' => 'Cook Islands',
    'CR' => 'Costa Rica',
    'CI' => 'Cote D Ivoire',
    'IVORY COAST' => 'Cote D Ivoire',
    'HR' => 'Croatia (Hrvatska)',
    'CROATIA' => 'Croatia (Hrvatska)',
    'CU' => 'Cuba',
    'CY' => 'Cyprus',
    'CZ' => 'Czech Republic',
    'SEVERNÍ TERASA' => 'Czech Republic',
    'MORAVIA AND SLEZIA' => 'Czech Republic',
    'CZECH REP.' => 'Czech Republic',
    'KARLOVARSKÝ KRAJ' => 'Czech Republic',
    'CD' => 'Democratic Republic Of Congo (Zaire)',
    'CONGO, DEMOCRATIC REPUBLIC OF' => 'Democratic Republic Of Congo (Zaire)',
    'DK' => 'Denmark',
    'DJ' => 'Djibouti',
    'DM' => 'Dominica',
    'DO' => 'Dominican Republic',
    'TP' => 'East Timor',
    'TL' => 'East Timor',
    'EC' => 'Ecuador',
    'EG' => 'Egypt',
    'SV' => 'El Salvador',
    'GQ' => 'Equatorial Guinea',
    'ER' => 'Eritrea',
    'EE' => 'Estonia',
    'ET' => 'Ethiopia',
    'FK' => 'Falkland Islands (Malvinas)',
    'FO' => 'Faroe Islands',
    'FJ' => 'Fiji',
    'FI' => 'Finland',
    'FR' => 'France',
    'REUNION ISLAND' => 'France',
    'FX' => 'France, Metro',
    'GF' => 'French Guiana',
    'PF' => 'French Polynesia',
    'TF' => 'French Southern Ter',
    'GA' => 'Gabon',
    'GM' => 'Gambia',
    'GE' => 'Georgia',
    'DE' => 'Germany',
    'ECHZELL' => 'Germany',
    'BAVARIA' => 'Germany',
    'DEUTSCHLAND' => 'Geermany',
    'SINZIG' => 'Germany',
    'SACHSEN-ANHALT' => 'Germany',
    'GH' => 'Ghana',
    'GI' => 'Gibraltar',
    'GR' => 'Greece',
    'GL' => 'Greenland',
    'GD' => 'Grenada',
    'GP' => 'Guadeloupe',
    'GU' => 'Guam',
    'GT' => 'Guatemala',
    'GN' => 'Guinea',
    'GW' => 'Guinea-Bissau',
    'GY' => 'Guyana',
    'HT' => 'Haiti',
    'HM' => 'Heard & McDonald Is',
    'HN' => 'Honduras',
    'HK' => 'Hong Kong',
    'HU' => 'Hungary',
    'IS' => 'Iceland',
    'IN' => 'India',
    'ID' => 'Indonesia',
    'JAKARTA' => 'Indonesia',
    'CIMAHI' => 'Indonesia',
    'DKI Jakarta' => 'Indonesia',
    'IR' => 'Iran',
    'IRAN, ISLAMIC REPUBLIC OF' => 'Iran',
    'IQ' => 'Iraq',
    'IE' => 'Ireland',
    'IL' => 'Israel',
    'IT' => 'Italy',
    'ITALIA' => 'Italy',
    'LECCO' => 'Italy',
    'BOLOGNA' => 'Italy',
    'TOSCANA' => 'Italy',
    'SAN CIPRIANO D\'AVERSA' => 'Italy',
    'CIAMPINO' => 'Italy',
    'JM' => 'Jamaica',
    'JP' => 'Japan',
    'JO' => 'Jordan',
    'KZ' => 'Kazakhstan',
    'KAZAKSTAN' => 'Kazakhstan',
    'KE' => 'Kenya',
    'KI' => 'Kiribati',
    'KW' => 'Kuwait',
    'KG' => 'Kyrgyzstan',
    'LA' => 'Laos',
    'LV' => 'Latvia',
    'LB' => 'Lebanon',
    'LS' => 'Lesotho',
    'LR' => 'Liberia',
    'LY' => 'Libya',
    'LI' => 'Liechtenstein',
    'LT' => 'Lithuania',
    'LU' => 'Luxembourg',
    'MO' => 'Macau',
    'MK' => 'Macedonia',
    'MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF' => 'Macedonia',
    'MG' => 'Madagascar',
    'MW' => 'Malawi',
    'MY' => 'Malaysia',
    'MV' => 'Maldives',
    'ML' => 'Mali',
    'MT' => 'Malta',
    'MH' => 'Marshall Islands',
    'MQ' => 'Martinique',
    'MR' => 'Mauritania',
    'MU' => 'Mauritius',
    'YT' => 'Mayotte',
    'MX' => 'Mexico',
    'MÉXICO' => 'Mexico',
    'HIDALGO' => 'Mexico',
    'FM' => 'Micronesia',
    'MD' => 'Moldova',
    'MC' => 'Monaco',
    'MN' => 'Mongolia',
    'MS' => 'Montserrat',
    'MA' => 'Morocco',
    'MAROCCO' => 'Morocco',
    'MZ' => 'Mozambique',
    'MM' => 'Myanmar (Burma)',
    'NA' => 'Namibia',
    'NR' => 'Nauru',
    'NP' => 'Nepal',
    'NL' => 'Netherlands',
    'MAASTRICHT' => 'Netherlands',
    'THE NETHERLANDS' => 'Netherlands',
    'HOLLAND' => 'Netherlands',
    'NEDERLAND' => 'Netherlands',
    'UTRECHT' => 'Netherlands',
    'NDERLAND' => 'Netherlands',
    'AN' => 'Netherlands Antilles',
    'NC' => 'New Caledonia',
    'NZ' => 'New Zealand',
    'NORTH ISLAND' => 'New Zealand',
    'NI' => 'Nicaragua',
    'NE' => 'Niger',
    'NG' => 'Nigeria',
    'NU' => 'Niue',
    'NF' => 'Norfolk Island',
    'KP' => 'Korea (North)',
    'MP' => 'Northern Mariana Is',
    'NO' => 'Norway',
    'OM' => 'Oman',
    'PK' => 'Pakistan',
    'PW' => 'Palau',
    'PA' => 'Panama',
    'PG' => 'Papua New Guinea',
    'PY' => 'Paraguay',
    'PE' => 'Peru',
    'PH' => 'Philippines',
    'QUEZON CITY' => 'Philippines',
    'PN' => 'Pitcairn',
    'PL' => 'Poland',
    'POLAND OO' => 'Poland',
    'PT' => 'Portugal',
    'PORTUGAL/EUROPE' => 'Portugal',
    'PR' => 'Puerto Rico',
    'QA' => 'Qatar',
    'RE' => 'Reunion',
    'RO' => 'Romania',
    'RU' => 'Russian Federation',
    'RUSSIA' => 'Russian Federation',
    'SU' => 'Russian Federation',
    'РОССИЯ' => 'Russian Federation',
    'RW' => 'Rwanda',
    'SH' => 'St. Helena',
    'KN' => 'Saint Kitts and Nevis',
    'LC' => 'Saint Lucia',
    'PM' => 'St. Pierre and Miquelo',
    'VC' => 'St. Vincent/Grenadines',
    'SM' => 'San Marino',
    'ST' => 'Sao Tome and Principe',
    'SA' => 'Saudi Arabia',
    'SN' => 'Senegal',
    'SC' => 'Seychelles',
    'SL' => 'Sierra Leone',
    'SG' => 'Singapore',
    'SK' => 'Slovakia (Slovak Rep)',
    'SLOVAKIA' => 'Slovakia (Slovak Rep)',
    'SI' => 'Slovenia',
    'SB' => 'Solomon Islands',
    'SO' => 'Somalia',
    'ZA' => 'South Africa',
    'GS' => 'Georgia and S. Sand Is',
    'SOUTH SANDWICH ISLANDS' => 'Georgia and S. Sand Is',
    'KR' => 'Korea (South)',
    'KOREA' => 'Korea (South)',
    'ES' => 'Spain',
    'PALAFRUGELL' => 'Spain',
    'ESPAÑA' => 'Spain',
    'CANARY ISLANDS' => 'Spain',
    'LK' => 'Sri Lanka',
    'SD' => 'Sudan',
    'SR' => 'Suriname',
    'SJ' => 'Svalbard And Jan Mayen',
    'SZ' => 'Swaziland',
    'SE' => 'Sweden',
    'SVERIGE' => 'Sweden',
    'CH' => 'Switzerland',
    'SCHWEIZ' => 'Switzerland',
    'SWIZERLAND' => 'Switzerland',
    'SY' => 'Syria',
    'TW' => 'Taiwan',
    'TJ' => 'Tajikistan',
    'TZ' => 'Tanzania',
    'TH' => 'Thailand',
    'TG' => 'Togo',
    'TK' => 'Tokelau',
    'TO' => 'Tonga',
    'TT' => 'Trinidad And Tobago',
    'TN' => 'Tunisia',
    'TR' => 'Turkey',
    'TM' => 'Turkmenistan',
    'TC' => 'Turks and Caicos Is',
    'TV' => 'Tuvalu',
    'UG' => 'Uganda',
    'UA' => 'Ukraine',
    'AE' => 'United Arab Emirates',
    'GB' => 'United Kingdom',
    'UK' => 'United Kingdom',
    'BRITIAN' => 'United Kingdom',
    'ENGLAND' => 'United Kingdom',
    'ENGLAND UK' => 'United Kingdom',
    'NORTHERN IRELAND' => 'United Kingdom',
    'NORTHUMBERLAND' => 'United Kingdom',
    'SCOTLAND' => 'United Kingdom',
    'SCOTLAND, UK' => 'United Kingdom',
    'UNITED KINGDON' => 'United Kingdom',
    'MERSEYSIDE' => 'United Kingdom',
    'ENGLAND (UNITED KINGDOM)' => 'United Kingdom',
    'UNIDED KINGDOM' => 'United Kingdom',
    'UNTIED KINGDOM' => 'United Kingdom',
    'DUMFRIES AND GALLOWAY' => 'United Kingdom',
    'N.IRELAND' => 'United Kingdom',
    'GREAT BRITAIN' => 'United Kingdom',
    'LONDON' => 'United Kingdom',
    'US' => 'United States',
    'U.S.' => 'United States',
    'U.S' => 'United States',
    'USA' => 'United States',
    'U.S.A' => 'United States',
    'U.S.A.' => 'United States',
    'UNITED STATED' => 'United States',
    'UNITED STATES OF AMERICA' => 'United States',
    'THE UNITED STATES OF AMERICA' => 'United States',
    'UNITEDSTATES' => 'United States',
    'CALIFORNIA' => 'United States',
    'AMERICA' => 'United States',
    'U.S.FUCKINGA.' => 'United States',
    'USOFA' => 'United States',
    'DEWITT' => 'United States',
    'FLOYD' => 'United States',
    'U S' => 'United States',
    'JEFFERSON' => 'United States',
    'PIMA' => 'United States',
    'UNITED STAES' => 'United States',
    'U.S. (AT THE MOMENT)' => 'United States',
    'DENVER' => 'United States',
    'UNITES STATES' => 'United States',
    'USA!!' => 'United States',
    'UNINTED STATES' => 'United States',
    'UNITED STATE OF AMERICA' => 'United States',
    'NEW HAVEN' => 'United States',
    'SPARTANBURG' => 'United States',
    'TUCSON' => 'United States',
    'UM' => 'US Minor Outlying Is',
    'UY' => 'Uruguay',
    'UZ' => 'Uzbekistan',
    'VU' => 'Vanuatu',
    'VA' => 'Vatican City State',
    'VE' => 'Venezuela',
    'VN' => 'Viet Nam',
    'VIETNAM' => 'Viet Nam',
    'VG' => 'Virgin Islands (Brit)',
    'Virgin Islands (United Kingdom)' => 'Virgin Islands (Brit)',
    'VI' => 'Virgin Islands (US)',
    'WF' => 'Wallis and Futuna Is',
    'EH' => 'Western Sahara',
    'WS' => 'Western Samoa',
    'YE' => 'Yemen',
    'YU' => 'Yugoslavia',
    'ZM' => 'Zambia',
    'ZW' => 'Zimbabwe'
  );
}
