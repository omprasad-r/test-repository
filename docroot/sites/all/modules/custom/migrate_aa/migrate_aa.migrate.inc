<?php

/**
 * @file
 * General support for Artist Arena migrations. Here we define wrapper classes
 * for each kind of D6 and D7 migration, where we can add common mappings and
 * methods.
 */

class AAFile6Migration extends DrupalFile6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->addUnmigratedDestinations(array('path', 'urlencode'));
  }
}
class AAFile7Migration extends DrupalFile7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->addUnmigratedDestinations(array('migrate_redirects'));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $row->uri = str_replace('public://', '', $row->uri);
  }
}

class AAYoutube7Migration extends DrupalYoutube7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->addUnmigratedDestinations(array('migrate_redirects'));
  }
}

class AAPicture6Migration extends DrupalPicture6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->addUnmigratedDestinations(array('migrate_redirects'));
  }
}
class AAPicture7Migration extends DrupalPicture7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->addUnmigratedDestinations(array('migrate_redirects'));
  }
}

class AARole6Migration extends DrupalRole6Migration {}
class AARole7Migration extends DrupalRole7Migration {}

class AAUser6Migration extends DrupalUser6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('created', 'created', FALSE)
         ->callbacks(array($this, 'adjustTimestamp'));
    $this->addFieldMapping('access', 'access', FALSE)
         ->callbacks(array($this, 'adjustTimestamp'));
    $this->addFieldMapping('login', 'login', FALSE)
         ->callbacks(array($this, 'adjustTimestamp'));
    // Rather than the default behavior of copying paths directly, we
    // want to generate new paths, and redirect old paths to them
    $this->removeFieldMapping('path');
    $this->addFieldMapping('pathauto', NULL, FALSE)
         ->defaultValue(1);
    $this->addFieldMapping('migrate_redirects', 'path');

    $this->addUnmigratedDestinations(array('field_comment'));
  }

  protected function adjustTimestamp($value) {
    // AA was Eastern time, Gardens is UTC, so adjust timestamps by 5 hours.
    $difference = 5 * 60 * 60;
    return $value - $difference;
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

class AAUser7Migration extends DrupalUser7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('created', 'created', FALSE)
         ->callbacks(array($this, 'adjustTimestamp'));
    $this->addFieldMapping('access', 'access', FALSE)
         ->callbacks(array($this, 'adjustTimestamp'));
    $this->addFieldMapping('login', 'login', FALSE)
         ->callbacks(array($this, 'adjustTimestamp'));
    // Rather than the default behavior of copying paths directly, we
    // want to generate new paths, and redirect old paths to them
    $this->removeFieldMapping('path');
    $this->addFieldMapping('pathauto', NULL, FALSE)
         ->defaultValue(1);
    $this->addFieldMapping('migrate_redirects', 'path');

    $this->addUnmigratedDestinations(array('field_comment'));
  }

  protected function adjustTimestamp($value) {
    // AA was Eastern time, Gardens is UTC, so adjust timestamps by 5 hours.
    $difference = 5 * 60 * 60;
    return $value - $difference;
  }
}

class AANode6Migration extends DrupalNode6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->addUnmigratedDestinations(array('field_is_pro'));
    $this->addFieldMapping('path', NULL, FALSE);
    $this->addFieldMapping(NULL, 'path');
    $this->addFieldMapping('pathauto', NULL, FALSE)
         ->defaultValue(1);
    if (module_exists('redirect')) {
      $this->addFieldMapping('migrate_redirects', 'path');
    }
    $this->addFieldMapping('created', 'created', FALSE)
         ->callbacks(array($this, 'adjustTimestamp'));
    $this->addFieldMapping('changed', 'changed', FALSE)
         ->callbacks(array($this, 'adjustTimestamp'));
    if (module_exists('statistics')) {
      $this->addFieldMapping('daycount', NULL, FALSE)
           ->defaultValue(0);
      $this->addFieldMapping('totalcount', NULL, FALSE)
           ->defaultValue(0);
    }
    $this->addUnmigratedSources(array(
      'log',
      'revision',
      'revision_uid',
      'upload', 'upload:description', 'upload:list', 'upload:weight',
    ));
  }

  protected function adjustTimestamp($value) {
    // AA was Eastern time, Gardens is UTC, so adjust timestamps by 5 hours.
    $difference = 5 * 60 * 60;
    return $value - $difference;
  }

  /**
   * AA location fields generally look like "Boston, MA", or "Paris, France".
   * Parse them out to distinct city/state/country fields.
   *
   * @param $row
   * @param $location_field
   */
  protected function parseLocation($row, $location_field) {
    // Separate the pieces - the first goes back in $location_field, which
    // is mapped to the city.
    $location = explode(',', $row->$location_field);
    if (count($location) == 2) {
      $row->$location_field = $location[0];
      // Is it a state or country?
      $location = trim($location[1]);
      if (strlen($location) == 2 && $location != 'GB' && $location != 'IE') {
        $row->state = $location;
        $row->country = 'USA';
      }
      else {
        $row->country = $location;
        $row->state = '';
      }
    }
    else {
      $row->country = $row->$location_field;
      $row->state = '';
      $row->$location_field = '';
    }
  }

  /**
   * Buy links come from link fields with a title subfield, we're storing the
   * <a> tag in a text field, so construct the link.
   *
   * @param $row
   * @param $link_field
   *
   * @return string
   */
  protected function buildBuyLink($row, $link_field) {
    if (!empty($row->$link_field)) {
      if (!is_array($row->$link_field)) {
        $row->$link_field = array($row->$link_field);
      }
      $title_field = $link_field . ':title';
      if (empty($row->$title_field)) {
        $row->$title_field = array();
      }
      elseif (!is_array($row->$title_field)) {
        $row->$title_field = array($row->$title_field);
      }
      if (empty($row->{$title_field}[0])) {
        $title = t('Buy');
      }
      else {
        $title = $row->{$title_field}[0];
      }
      $row->$link_field = '<a href="' . $row->{$link_field}[0] . '">' . $title . '</a>';
    }
  }

  public function prepare($node, $row) {
    if ($node->uid < 2 && !empty($this->defaultUid)) {
      $node->uid = $this->defaultUid;
    }
  }
}

class AANode7Migration extends DrupalNode7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->removeFieldMapping('path');
    $this->addFieldMapping('pathauto', NULL, FALSE)
         ->defaultValue(1);
    $this->addFieldMapping('migrate_redirects', 'path');
    $this->addFieldMapping('created', 'created', FALSE)
         ->callbacks(array($this, 'adjustTimestamp'));
    $this->addFieldMapping('changed', 'changed', FALSE)
         ->callbacks(array($this, 'adjustTimestamp'));

    $this->addUnmigratedDestinations(array('field_is_pro', 'path'));
  }

  protected function adjustTimestamp($value) {
    // AA was Eastern time, Gardens is UTC, so adjust timestamps by 5 hours.
    $difference = 5 * 60 * 60;
    return $value - $difference;
  }

  /**
   * AA location fields generally look like "Boston, MA", or "Paris, France".
   * Parse them out to distinct city/state/country fields.
   *
   * @param $row
   * @param $location_field
   */
  protected function parseLocation($row, $location_field) {
    // Separate the pieces - the first goes back in $location_field, which
    // is mapped to the city.
    if (is_array($row->$location_field)) {
      $row->$location_field = reset($row->$location_field);
    }
    $row->$location_field = trim($row->$location_field);
    if ($row->$location_field) {
      $location = explode(',', $row->$location_field);
      $row->$location_field = trim($location[0]);
      // Is it a state or country?
      if (isset($location[1])) {
        $location = trim($location[1]);
        if (strlen($location) == 2 && $location != 'GB' && $location != 'IE') {
          $row->state = $location;
          $row->country = '';
        }
        else {
          $row->country = $location;
          $row->state = '';
        }
      }
    }
  }

  /**
   * Buy links come from link fields with a title subfield, we're storing the
   * <a> tag in a text field, so construct the link.
   *
   * @param $row
   * @param $link_field
   *
   * @return string
   */
  protected function buildBuyLink($row, $link_field) {
    if (!empty($row->$link_field)) {
      $title_field = $link_field . ':title';
      if (empty($row->{$title_field}[0])) {
        $title = t('Buy');
      }
      else {
        $title = $row->{$title_field}[0];
      }
      $row->$link_field = '<a href="' . $row->{$link_field}[0] . '">' . $title . '</a>';
    }
  }

  public function prepare($node, $row) {
    if ($node->uid < 2 && !empty($this->defaultUid)) {
      $node->uid = $this->defaultUid;
    }
  }
}

/**
 * Gallery migration is special, because we're converting nodes into terms -
 * the standard migrate_d2d classes won't do.
 */
class AAGallery6Migration extends DrupalMigration {
  /**
   * The machine name of the node type we're migrating from.
   *
   * @var string
   */
  protected $sourceType;

  /**
   * The machine name of the Drupal 7 vocabulary we're migrating into.
   *
   * @var string
   */
  protected $destinationVocabulary;

  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->sourceType = $arguments['source_type'];
    $this->destinationVocabulary = $arguments['destination_vocabulary'];

    // Create our three main objects - source, destination, and map
    $this->source = new MigrateSourceSQL($this->query(), $this->sourceFields,
      NULL, $this->sourceOptions);

    $this->destination = new MigrateDestinationTerm($this->destinationVocabulary);

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'nid' => array('type' => 'int',
                       'unsigned' => TRUE,
                       'not null' => TRUE,
                       'description' => 'Source node ID',
                      ),
      ),
      MigrateDestinationTerm::getKeySchema()
    );

    $this->highwaterField = array(
      'name' => 'changed',
      'alias' => 'n',
      'type' => 'int',
    );

    $this->addFieldMapping('name', 'title');
    $this->addFieldMapping('field_is_pro')
         ->defaultValue(1);

    $this->addUnmigratedDestinations(array(
      'description',
      'format',
      'migrate_redirects',
      'parent',
      'parent_name',
      'path',
      'pathauto',
      'tac_redirect_403', 'tac_redirect_403:title', 'tac_redirect_403:attributes',
      'weight',
    ));

    $this->addUnmigratedSources(array('changed'));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    $original = $row->title;
    // Replace the characters that mess up gallery aliases with spaces
    $row->title = strtr($row->title, '-/\\', '   ');
  }

  /**
   * Query the gallery nodes - all we need is the title.
   *
   * @return QueryConditionInterface
   */
  protected function query() {
    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('node', 'n')
             ->fields('n', array('nid', 'title', 'changed'))
             ->condition('type', $this->sourceType);
    return $query;
  }
}

class AAGallery7Migration extends DrupalMigration {
  /**
   * The machine name of the node type we're migrating from.
   *
   * @var string
   */
  protected $sourceType;

  /**
   * The machine name of the Drupal 7 vocabulary we're migrating into.
   *
   * @var string
   */
  protected $destinationVocabulary;

  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->sourceType = $arguments['source_type'];
    $this->destinationVocabulary = $arguments['destination_vocabulary'];

    // Create our three main objects - source, destination, and map
    $this->source = new MigrateSourceSQL($this->query(), $this->sourceFields,
      NULL, $this->sourceOptions);

    $this->destination = new MigrateDestinationTerm($this->destinationVocabulary);

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'nid' => array('type' => 'int',
                       'unsigned' => TRUE,
                       'not null' => TRUE,
                       'description' => 'Source node ID',
                      ),
      ),
      MigrateDestinationTerm::getKeySchema()
    );

    $this->highwaterField = array(
      'name' => 'changed',
      'alias' => 'n',
      'type' => 'int',
    );

    $this->addFieldMapping('name', 'title');
    $this->addFieldMapping('field_is_pro')
         ->defaultValue(1);

    $this->addUnmigratedDestinations(array(
      'description',
      'format',
      'migrate_redirects',
      'parent',
      'parent_name',
      'path',
      'pathauto',
      'tac_redirect_403',
      'weight',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    // Replace the characters that mess up gallery aliases with spaces
    $row->title = strtr($row->title, '-/\\', '   ');
  }

  /**
   * Query the gallery nodes - all we need is the title.
   *
   * @return QueryConditionInterface
   */
  protected function query() {
    $query = Database::getConnection('default', $this->sourceConnection)
             ->select('node', 'n')
             ->fields('n', array('nid', 'title', 'changed'))
             ->condition('type', $this->sourceType);
    return $query;
  }
}

class AAComment6Migration extends DrupalComment6Migration {
  public function __construct(array $arguments) {
    // Make certain comment_notify does nothing
    global $conf;
    $conf['comment_notify_node_types'] = array();
    parent::__construct($arguments);
    $this->addUnmigratedDestinations(array('migrate_redirects', 'path'));
  }

  /**
   * Implements DrupalCommentMigration::query().
   *
   * @return SelectQueryInterface
   */
  public function query() {
    $query = parent::query();
    // We've run into instances where comments have uids not in
    // the user table - since we know anonymous comments are not enabled on
    // any of the sites, make sure every uid is valid.
    $query->innerJoin('users', 'u', 'c.uid=u.uid');
    return $query;
  }
}

class AAComment7Migration extends DrupalComment7Migration {
  public function __construct(array $arguments) {
    // Make certain comment_notify does nothing
    global $conf;
    $conf['comment_notify_node_types'] = array();
    parent::__construct($arguments);

    $this->addUnmigratedDestinations(array(
      'comment_body:language',
      'migrate_redirects',
    ));
    $this->addUnmigratedSources(array(
      'comment_body:format',
    ));
  }

  /**
   * Implements DrupalCommentMigration::query().
   *
   * @return SelectQueryInterface
   */
  public function query() {
    $query = parent::query();
    // We've run into instances where comments have uids not in
    // the user table - since we know anonymous comments are not enabled on
    // any of the sites, make sure every uid is valid.
    $query->innerJoin('users', 'u', 'c.uid=u.uid');
    return $query;
  }
}

class AATerm6Migration extends DrupalTerm6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->addUnmigratedDestinations(array(
      'migrate_redirects',
      'tac_redirect_403', 'tac_redirect_403:title', 'tac_redirect_403:attributes',
    ));
  }
}

class AATerm7Migration extends DrupalTerm7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->addUnmigratedDestinations(array('migrate_redirects'));
  }
}

/**
 * From link module git repo - there's no official release with this support yet,
 * so we don't have this in Gardens.
 */
class MigrateAALinkFieldHandler extends MigrateFieldHandler {
  public function __construct() {
    $this->registerTypes(array('link_field'));
  }

  static function arguments($title = NULL, $attributes = NULL, $language = NULL) {
    $arguments = array();
    if (!is_null($title)) {
      $arguments['title'] = $title;
    }
    if (!is_null($attributes)) {
      $arguments['attributes'] = $attributes;
    }
    if (!is_null($language)) {
      $arguments['language'] = $language;
    }
    return $arguments;
  }

  public function fields() {
    return array(
      'title' => t('Link: Title'),
      'attributes' => t('Link: Attributes'),
    );
  }

  public function prepare($entity, array $field_info, array $instance, array $values) {
    if (isset($values['arguments'])) {
      $arguments = $values['arguments'];
      unset($values['arguments']);
    }
    else {
      $arguments = array();
    }

    $language = $this->getFieldLanguage($entity, $field_info, $arguments);

    foreach($values as $delta => $value) {
      $item = array();
      if (isset($arguments['title'])) {
        if (!is_array($arguments['title'])) {
          $item['title'] = $arguments['title'];
        }
        else if (isset($arguments['title'][$delta])) {
          $item['title'] = $arguments['title'][$delta];
        }
      }
      if (isset($arguments['attributes'])) {
        $item['attributes'] = $arguments['attributes'];
      }
      $item['url'] = $value;
      $return[$language][$delta] = $item;
      $delta++;
    }

    return isset($return) ? $return : NULL;
  }
}

/**
 * Implements hook_migrate_api().
 */
function migrate_aa_migrate_api() {
  $api = array(
    'api' => 2,
  );
  return $api;
}
