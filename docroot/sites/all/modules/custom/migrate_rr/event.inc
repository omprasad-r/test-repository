<?php
/**
 * @file
 * Event (tour date) migration classes for Roadrunner.
 */

/**
 * Event (tour date) class for Roadrunner.
 */
class RREventMigration extends RRMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t("Migrate events (tour dates) from the source database");
    $group_name = $this->group->getName();
    $this->dependencies[] = $group_name . 'User';

    $fields = array(
      // From board_posts
      'id' => 'Unique board post ID',
      'perform_at' => 'Event date',
      'ticket_url' => 'URL to get tickets',
      'ticket_description' => 'Text for ticket link',
      'fanclub_id' => 'Which site the event belongs to',
      'venue_name' => 'Name of venue',
      'venue_url' => 'URL for venue',
      'venue_address1' => '',
      'venue_address2' => '',
      'venue_city' => '',
      'venue_state' => '',
      'venue_zip' => '',
      'info' => '',
      'ticket_available_at' => '',
      'created_at' => '',
      'updated_at' => '',
      'venue_country' => '',
      'permalink' => '',
      'publish_at' => '',
      'report_delivered_at' => '',
      'allowed_meet_greet_count' => '',
      'lat' => '',
      'lng' => '',
      'fanclub_ticket_url' => '',
      'disable_plus_one' => '',
      'legacy_id' => '',
      'legacy_source' => '',
      'facebook_event_id' => '',
      'mobile_roadie_id' => '',
    );

    $query = $this->connection
      ->select('tourdates', 't')
      ->fields('t')
      ->condition('fanclub_id', $arguments['artist_id'])
      ->orderBy('updated_at');

    $this->source = new MigrateSourceSQL($query, $fields, NULL,
      array('map_joinable' => FALSE));

    $this->destination = new MigrateDestinationNode('event');

    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'id' => array('type' => 'int',
                        'unsigned' => TRUE,
                        'not null' => TRUE,
                        'description' => 'Tour date ID',
                       )
        ),
        MigrateDestinationNode::getKeySchema()
      );

    $this->highwaterField = array(
      'name' => 'updated_at',
      'alias' => 't',
    );

    $this->addFieldMapping('title', 'venue_name');
    $this->addFieldMapping('body', 'info');
    $this->addFieldMapping('uid')
         ->defaultValue($arguments['default_author_id']);
    $this->addFieldMapping('created', 'created_at');
    $this->addFieldMapping('changed', 'updated_at');
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);
    $this->addFieldMapping('field_event_date', 'perform_at');
    $this->addFieldMapping('field_event_venue', 'venue_name');
    $this->addFieldMapping('field_event_availability', 'ticket_available_at');
    $this->addFieldMapping('field_event_address', 'venue_address1');
    $this->addFieldMapping('field_event_city', 'venue_city');
    $this->addFieldMapping('field_event_state', 'venue_state');
    $this->addFieldMapping('field_event_country', 'venue_country');
    $this->addFieldMapping('field_event_category')
         ->defaultValue('Tour Dates');
    $this->addFieldMapping('field_event_category:create_term')
         ->defaultValue(1);
    $this->addFieldMapping('field_event_buylink', 'ticket_url');
    $this->addFieldMapping('field_event_buylink:format')
         ->defaultValue('6');

    $this->addUnmigratedDestinations(array(
        'body:language', 'body:summary',
      'comment',
      'daycount',
        'field_event_address:language',
        'field_event_availability:language',
        'field_event_buylink:language',
        'field_event_category:source_type', 'field_event_category:ignore_case',
        'field_event_city:language',
        'field_event_country:language',
        'field_event_date:timezone', 'field_event_date:rrule', 'field_event_date:to',
      'field_event_maplink', 'field_event_maplink:format', 'field_event_maplink:language',
        'field_event_state:language',
        'field_event_venue:language',
      'is_new',
      'language',
      'log',
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
      'allowed_meet_greet_count',
      'disable_plus_one',
      'facebook_event_id',
      'fanclub_id',
      'fanclub_ticket_url',
      'lat',
      'legacy_id',
      'legacy_source',
      'lng',
      'mobile_roadie_id',
      'permalink',
      'publish_at',
      'report_delivered_at',
      'ticket_description',
      'venue_address2',
      'venue_url',
      'venue_zip',
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

    if (!empty($row->ticket_url)) {
      if (empty($row->ticket_description)) {
        $row->ticket_description = 'Tickets';
      }
      $row->ticket_url = '<a href="' . $row->ticket_url . '">' .
                         $row->ticket_description . '</a>';
    }
  }
}
