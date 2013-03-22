<?php
/**
 * @file
 *
 */

class NingWarnerEventMigration extends NingEventMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_event_date', 'startDate', FALSE);
    $this->addFieldMapping('field_event_venue', 'location', FALSE);
    $this->addFieldMapping('field_event_city', 'city', FALSE);
    $this->addFieldMapping('field_event_state', 'state', FALSE);
    $this->addFieldMapping('field_event_address', 'street', FALSE);
    $this->addFieldMapping('field_event_category')
         ->defaultValue('Events');
    $this->addFieldMapping('field_event_category:create_term')
         ->defaultValue(TRUE);
    $this->removeFieldMapping('body:format');

    $this->addUnmigratedDestinations(array(
        'field_event_address:language',
      'field_event_availability', 'field_event_availability:language',
      'field_event_buylink', 'field_event_buylink:format', 'field_event_buylink:language',
        'field_event_category:source_type', 'field_event_category:ignore_case',
        'field_event_city:language',
      'field_event_country', 'field_event_country:language',
      'field_event_maplink', 'field_event_maplink:format', 'field_event_maplink:language',
        'field_event_state:language',
        'field_event_venue:language',
    ));
  }
}
