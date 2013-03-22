<?php
/**
 * @file
 *
 */

class NingWarnerMemberMigration extends NingMemberMigration {
  public function __construct($arguments) {
    $site = basename($this->sourceDirectory);

    $this->sourceFields += array(
      'age' => 'Actually, birthday, e.g. "March 14"',
    );
    if ($site == 'GucciMane') {
      $this->sourceFields += array(
        'mobile_join_gucci_mane_039_s_official_mobile_list' =>
          'Contains unpunctuated phone numbers',
      );
    }
    elseif ($site == 'TheNewBoyz') {
      $this->sourceFields += array(
        'aim_so_the_new_boyz_can_aim_with_you' => 'Contents are pretty random',
        'mobile_join_the_mobile_list' => 'Contains unpunctuated phone numbers',
      );
    }
    parent::__construct($arguments);

    $this->addFieldMapping('name', 'fullName', FALSE)
         ->dedupe('users', 'name');
    $this->addFieldMapping('field_profile_updated', 'createdDate');
    $this->addFieldMapping('field_member_name', 'fullName');
    $this->addFieldMapping('field_nickname', 'fullName');
    $this->addFieldMapping('field_gender', 'gender')
         ->callbacks(array($this, 'handleGender'));
    $this->addFieldMapping('field_birthdate', 'birthdate');
    $this->addFieldMapping('field_country', 'country')
         ->callbacks(array($this, 'handleCountry'));
    $this->addFieldMapping('field_postal_code', 'zip');

    $this->addUnmigratedDestinations(array(
      'field_about_me', 'field_about_me:language',
      'field_city', 'field_city:language',
      'field_comment',
      'field_eos_id',
      'field_favorite_bands', 'field_favorite_bands:language',
      'field_hobbies', 'field_hobbies:language',
        'field_member_name:language',
        'field_nickname:language',
        'field_postal_code:language',
      'field_profile_14', 'field_profile_14:language',
      'field_provider', 'field_provider:language',
      'field_site_id', 'field_site_id:language',
      'field_state', 'field_state:language',
      'field_twitter_name', 'field_twitter_name:language',
      'field_type',
      'field_usage_type',
    ));

    $this->addUnmigratedSources(array(
      'age',
    ));

    if ($site == 'GucciMane') {
      $this->addUnmigratedDestinations(array('field_fav_feed', 'field_fav_feed:language'));
      $this->addUnmigratedSources(array('mobile_join_gucci_mane_039_s_official_mobile_list'));
    }
    elseif ($site == 'TheNewBoyz') {
      $this->addUnmigratedSources(array(
        'aim_so_the_new_boyz_can_aim_with_you',
        'mobile_join_the_mobile_list',
      ));
    }
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

  protected function handleGender($value) {
    switch (drupal_strtolower($value)) {
      case 'm':
        $value = 'Male';
        break;
      case 'f':
        $value = 'Female';
        break;
      default:
        $value = '';
        break;
    }
    return $value;
  }

  protected function handleCountry($value) {
    $value = drupal_strtoupper($value);
    // Translate from two-letter codes to field_country allowed_values
    if (!empty($this->countries[$value])) {
      return $this->countries[$value];
    }
    else {
      return '';
    }
  }

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
    'AT' => 'Austria',
    'AZ' => 'Azerbaijan',
    'BS' => 'Bahamas',
    'BH' => 'Bahrain',
    'BD' => 'Bangladesh',
    'BB' => 'Barbados',
    'BY' => 'Belarus',
    'BE' => 'Belgium',
    'BZ' => 'Belize',
    'BJ' => 'Benin',
    'BM' => 'Bermuda',
    'BT' => 'Bhutan',
    'BO' => 'Bolivia',
    'BA' => 'Bosnia & Herzegovina',
    'BW' => 'Botswana',
    'BV' => 'Bouvet Island',
    'BR' => 'Brazil',
    'IO' => 'British Ind Ocean Ter',
    'BN' => 'Brunei Darussalam',
    'BG' => 'Bulgaria',
    'BF' => 'Burkina Faso',
    'BI' => 'Burundi',
    'KH' => 'Cambodia',
    'CM' => 'Cameroon',
    'CA' => 'Canada',
    'CV' => 'Cape Verde',
    'KY' => 'Cayman Islands',
    'CF' => 'Central African Republic',
    'TD' => 'Chad',
    'CL' => 'Chile',
    'CN' => 'China',
    'CX' => 'Christmas Island',
    'CC' => 'Cocos (Keeling Is)',
    'CO' => 'Columbia',
    'KM' => 'Comoros',
    'CG' => 'Congo',
    'CK' => 'Cook Islands',
    'CR' => 'Costa Rica',
    'CI' => 'Cote D Ivoire',
    'HR' => 'Croatia (Hrvatska)',
    'CU' => 'Cuba',
    'CY' => 'Cyprus',
    'CZ' => 'Czech Republic',
    'CD' => 'Democratic Republic Of Congo (Zaire)',
    'DK' => 'Denmark',
    'DJ' => 'Djibouti',
    'DM' => 'Dominica',
    'DO' => 'Dominican Republic',
    'TP' => 'East Timor',
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
    'FX' => 'France, Metro',
    'GF' => 'French Guiana',
    'PF' => 'French Polynesia',
    'TF' => 'French Southern Ter',
    'GA' => 'Gabon',
    'GM' => 'Gambia',
    'GE' => 'Georgia',
    'DE' => 'Germany',
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
    'IR' => 'Iran',
    'IQ' => 'Iraq',
    'IE' => 'Ireland',
    'IL' => 'Israel',
    'IT' => 'Italy',
    'JM' => 'Jamaica',
    'JP' => 'Japan',
    'JO' => 'Jordan',
    'KZ' => 'Kazakhstan',
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
    'FM' => 'Micronesia',
    'MD' => 'Moldova',
    'MC' => 'Monaco',
    'MN' => 'Mongolia',
    'MS' => 'Montserrat',
    'MA' => 'Morocco',
    'MZ' => 'Mozambique',
    'MM' => 'Myanmar (Burma)',
    'NA' => 'Namibia',
    'NR' => 'Nauru',
    'NP' => 'Nepal',
    'NL' => 'Netherlands',
    'AN' => 'Netherlands Antilles',
    'NC' => 'New Caledonia',
    'NZ' => 'New Zealand',
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
    'PN' => 'Pitcairn',
    'PL' => 'Poland',
    'PT' => 'Portugal',
    'PR' => 'Puerto Rico',
    'QA' => 'Qatar',
    'RE' => 'Reunion',
    'RO' => 'Romania',
    'RU' => 'Russian Federation',
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
    'SI' => 'Slovenia',
    'SB' => 'Solomon Islands',
    'SO' => 'Somalia',
    'ZA' => 'South Africa',
    'GS' => 'Georgia and S. Sand Is',
    'KR' => 'Korea (South)',
    'ES' => 'Spain',
    'LK' => 'Sri Lanka',
    'SD' => 'Sudan',
    'SR' => 'Suriname',
    'SJ' => 'Svalbard And Jan Mayen',
    'SZ' => 'Swaziland',
    'SE' => 'Sweden',
    'CH' => 'Switzerland',
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
    'US' => 'United States',
    'UM' => 'US Minor Outlying Is',
    'UY' => 'Uruguay',
    'UZ' => 'Uzbekistan',
    'VU' => 'Vanuatu',
    'VA' => 'Vatican City State',
    'VE' => 'Venezuela',
    'VN' => 'Viet Nam',
    'VG' => 'Virgin Islands (Brit)',
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
