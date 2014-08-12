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
      'VenueEventID' => 'PK',
      'VenueID' => 'Secondary PK - not needed',
      'Name' => 'Name of the Event',
      'StartDate' => 'Start Date',
      'TimeStart' => 'Start time',
      'TimeEnd' => 'End time',
      'Artists' => 'Relevant Artists',
      'Info' => 'Event info',
      'TicketLink' => 'Link to purchase tickets',
      'Active' => 'Status',
      'Price' => 'Price',
      'PriceText' => 'Price plaintext',
      'FullInfo' => 'More info!',
      'url' => 'canonical URL',
      'VenueName' => 'Venue name',
      'Town' => 'Town location',
      'County' => 'County',
      'Country' => 'Country',
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