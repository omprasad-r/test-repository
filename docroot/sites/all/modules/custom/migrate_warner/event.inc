<?php

class EventMigration extends WarnerMigration {
  public function __construct($group, $live = TRUE) {
    // Fields specific to the event type
    $specific_fields = array(
      'category' => '',
      'region' => '',
      'city' => '',
      'address' => '',
      'map_link' => '',
      'venue' => '',
      'availability' => '',
      'buy_link_url' => '',
      'buy_link_name' => '',
      'date_time' => '',
    );
    parent::__construct($specific_fields, 'event', 'event', $live);
    $this->description = t('Import event entities from XML into Drupal');

    $this->addFieldMapping('field_event_category', 'category')
         ->xpath('category')
         ->arguments(array('create_term' => TRUE));
    $this->addFieldMapping('field_event_date', 'date_time')
         ->xpath('location/date_time')
         ->callbacks(array($this, 'stripQuotes'))
         ->arguments(array('timezone' => variable_get('date_default_timezone', 'UTC')));
    $this->addFieldMapping('field_event_country', 'region')
         ->xpath('location/region')
         ->callbacks(array($this, 'stripQuotes'));
    $this->addFieldMapping('field_event_state', 'region')
             ->xpath('location/region')
             ->callbacks(array($this, 'stripQuotes'));
    $this->addFieldMapping('field_event_city', 'city')
         ->xpath('location/city')
         ->callbacks(array($this, 'stripQuotes'));
    $this->addFieldMapping('field_event_address', 'address')
         ->xpath('location/address')
         ->callbacks(array($this, 'stripQuotes'));
    $this->addFieldMapping('field_event_maplink', 'map_link')
         ->xpath('location/map_link')
         ->callbacks(array($this, 'stripQuotes'));
    $this->addFieldMapping('field_event_buylink', 'buy_link_url')
         ->xpath('location/buy_link_url')
         ->callbacks(array($this, 'stripQuotes'));
    $this->addFieldMapping(NULL, 'buy_link_name')
         ->xpath('location/buy_link_name')
         ->callbacks(array($this, 'stripQuotes'));
    $this->addFieldMapping('field_event_venue', 'venue')
         ->xpath('location/venue')
         ->callbacks(array($this, 'stripQuotes'));
    $this->addFieldMapping('field_event_availability', 'availability')
         ->xpath('location/availability')
         ->callbacks(array($this, 'stripQuotes'));
    $this->removeFieldMapping('field_is_pro');
    $this->removeFieldMapping(NULL, 'isPro');
    $this->addFieldMapping(NULL, 'isPro')
         ->description('No is_pro field on events in Drupal');
  }

  /**
   * Strip enclosing quotes from a value.
   * @param $value
   * @return string
   */
  protected function stripQuotes($value) {
    return trim($value, '"');
  }

  public function prepare($node, $row) {
    // Construct the Buy url if present
    if (!empty($row->buy_link_url)) {
      $link = '<a href="' . $this->stripQuotes($row->buy_link_url) . '">' .
              $this->stripQuotes($row->buy_link_name) . '</a>';
      $node->field_event_buylink[LANGUAGE_NONE][0] = array(
          'format' => $this->safe_html,
          'value_format' => $this->safe_html,
          'value' => $link,
      );
    }
    unset($row->xml);
    // If the region didn't map to a state, treat it as a country
    $states = ARRAY('AL'=>"ALABAMA",
                    'AK'=>"ALASKA",
                    'AZ'=>"ARIZONA",
                    'AR'=>"ARKANSAS",
                    'CA'=>"CALIFORNIA",
                    'CO'=>"COLORADO",
                    'CT'=>"CONNECTICUT",
                    'DE'=>"DELAWARE",
                    'DC'=>"DISTRICT OF COLUMBIA",
                    'FL'=>"FLORIDA",
                    'GA'=>"GEORGIA",
                    'HI'=>"HAWAII",
                    'ID'=>"IDAHO",
                    'IL'=>"ILLINOIS",
                    'IN'=>"INDIANA",
                    'IA'=>"IOWA",
                    'KS'=>"KANSAS",
                    'KY'=>"KENTUCKY",
                    'LA'=>"LOUISIANA",
                    'ME'=>"MAINE",
                    'MD'=>"MARYLAND",
                    'MA'=>"MASSACHUSETTS",
                    'MI'=>"MICHIGAN",
                    'MN'=>"MINNESOTA",
                    'MS'=>"MISSISSIPPI",
                    'MO'=>"MISSOURI",
                    'MT'=>"MONTANA",
                    'NE'=>"NEBRASKA",
                    'NV'=>"NEVADA",
                    'NH'=>"NEW HAMPSHIRE",
                    'NJ'=>"NEW JERSEY",
                    'NM'=>"NEW MEXICO",
                    'NY'=>"NEW YORK",
                    'NC'=>"NORTH CAROLINA",
                    'ND'=>"NORTH DAKOTA",
                    'OH'=>"OHIO",
                    'OK'=>"OKLAHOMA",
                    'OR'=>"OREGON",
                    'PA'=>"PENNSYLVANIA",
                    'RI'=>"RHODE ISLAND",
                    'SC'=>"SOUTH CAROLINA",
                    'SD'=>"SOUTH DAKOTA",
                    'TN'=>"TENNESSEE",
                    'TX'=>"TEXAS",
                    'UT'=>"UTAH",
                    'VT'=>"VERMONT",
                    'VA'=>"VIRGINIA",
                    'WA'=>"WASHINGTON",
                    'WV'=>"WEST VIRGINIA",
                    'WI'=>"WISCONSIN",
                    'WY'=>"WYOMING");
    if (!empty($row->region)) {
      $region = drupal_strtoupper($row->region);
      if (in_array($region, $states) || in_array($region, array_keys($states))) {
        unset($node->field_event_country);
      }
      else {
        unset($node->field_event_state);
      }
    }
  }
}

class EventNonLiveMigration extends EventMigration {
  public function __construct($group) {
    parent::__construct($group, FALSE);
    $this->description = t('Import non-live event entities from XML into Drupal');
  }
}
