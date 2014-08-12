<?php

class CeeloShowMigration extends CeeloMigration {
  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->destination = new MigrateDestinationNode('event');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'VenueEventID' =>
        array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'PK, you know?',
        ),
    ), MigrateDestinationNode::getKeySchema());

    $this->source = new MigrateSourceCSV(dirname(__FILE__) . '/../data/shows.txt', array(
      array('VenueEventID' => 'PK'),
      array('VenueID' => 'Secondary PK - not needed'),
      array('Name' => 'Name of the Event'),
      array('StartDate' => 'Start Date'),
      array('TimeStart' => 'Start time'),
      array('TimeEnd' => 'End time'),
      array('Artists' => 'Relevant Artists'),
      array('Info' => 'Event info'),
      array('TicketLink' => 'Link to purchase tickets'),
      array('Active' => 'Status'),
      array('Price' => 'Price'),
      array('PriceText' => 'Price plaintext'),
      array('FullInfo' => 'More info!'),
      array('url' => 'canonical URL'),
      array('VenueName' => 'Venue name'),
      array('Town' => 'Town location'),
      array('County' => 'County'),
      array('Country' => 'Country'),
    ), array(
      'delimiter'   => "\t",
      'header_rows' => 1,
    ));

    $this->addFieldMapping('title', 'Name');
    $this->addFieldMapping('category')->defaultValue('Show');
    $this->addFieldMapping('category:create_term')->defaultValue(TRUE);

    $this->addFieldMapping('date', 'StartDate');

    $this->addFieldMapping('field_event_city', 'Town');
    $this->addFieldMapping('field_event_country', 'Country');

    $this->addFieldMapping('field_event_venue', 'VenueName');

    $this->addFieldMapping('field_event_buylink', 'TicketLink');

    $this->addFieldMapping('status', 'Active');

  }
}