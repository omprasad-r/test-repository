<?php
/**
 * @file
 * Photo migration classes for googoodolls import.
 */

class GooGooTourDateMigration extends GooGooMigration {
  public function __construct() {
    parent::__construct();

    $this->description = 'Import of tour dates from googoo_site';

    $fields = array(
      // tour table
      'id' => 'Unique ID',
      'tourdate' => 'Date of event',
      'tourcity' => 'City, ST',
      'tourvenue' => 'Venue',
      'onsalelink' => 'Link to buy tickets',
      'onsaledate' => 'Date tickets went on sale',
      'presale' => 'C/N',
      'presaledate' => 'Date of presale',
      'mg' => 'Meet & Greet',
      // Generated
      'tourstate' => 'Event state (extracted from tourcity)',
      'tourcountry' => 'Event country (extracted from tourcity)',
    );

    $query = Database::getConnection('default', 'googoo_site')
             ->select('tour', 't')
             ->fields('t');
    $this->source = new MigrateSourceSQL($query, $fields, NULL,
          array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationNode('event');
    $this->map = new MigrateSQLMap(
      $this->machineName,
      array(
        'id' =>
          array('type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Event ID',
               ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('title', 'tourvenue');
    $this->addFieldMapping('uid')
         ->defaultValue($this->defaultAuthor);
    $this->addFieldMapping('created', 'tourdate');
    $this->addFieldMapping('changed', 'tourdate');
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);
    $this->addFieldMapping('field_event_date', 'tourdate');
    $this->addFieldMapping('field_event_venue', 'tourvenue');
    $this->addFieldMapping('field_event_state', 'tourstate');
    $this->addFieldMapping('field_event_city', 'tourcity');
    $this->addFieldMapping('field_event_country', 'tourcountry');
    $this->addFieldMapping('field_event_category')
         ->defaultValue('Tour Date');
    $this->addFieldMapping('field_event_category:create_term')
         ->defaultValue(TRUE);

    $this->addUnmigratedDestinations(array(
      'body', 'body:summary', 'body:language',
      'comment',
      'daycount',
      'field_event_address', 'field_event_address:language',
      'field_event_availability', 'field_event_availability:language',
      'field_event_buylink', 'field_event_buylink:format', 'field_event_buylink:language',
        'field_event_category:source_type', 'field_event_category:ignore_case',
        'field_event_city:language',
        'field_event_country:language',
      'field_event_maplink', 'field_event_maplink:format', 'field_event_maplink:language',
        'field_event_state:language',
        'field_event_venue:language',
      'is_new',
      'language',
      'log',
      'migrate_redirects',
      'path',
      'pathauto',
      'promote',
      'revision',
      'revision_uid',
      'sticky',
      'timestamp',
      'tnid',
      'totalcount',
      'translate',
    ));

    $this->addUnmigratedSources(array(
      'mg',
      'onsaledate',
      'onsalelink',
      'presale',
      'presaledate',
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

    // Break the location out into city/state/country.
    $city_bits = explode(',', $row->tourcity);
    if (count($city_bits) == 2) {
      $row->tourcity = trim($city_bits[0]);
      $state = trim($city_bits[1]);
      switch ($state) {
        case 'UK':
          $row->tourstate = '';
          $row->tourcountry = 'United Kingdom';
          break;
        case 'SK':
        case 'NL':
        case 'NS':
        case 'PE':
        case 'QC':
        case 'NB':
        case 'ON':
        case 'MB':
        case 'AB':
          $row->tourstate = $state;
          $row->tourcountry = 'Canada';
          break;
        default:
          $row->tourstate = $state;
          $row->tourcountry = 'United States';
          break;
      }
    }
  }

  /**
   * Implements Migration::prepare().
   *
   * @param $node
   * @param $row
   */
  public function prepare($node, $row) {
    // Build the title from venue and date
    $timestamp = Migration::timestamp($row->tourdate);
    $node->title = $row->tourvenue . ' - ' . format_date($timestamp);
  }
}
