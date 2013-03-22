<?php
/**
 * @file
 * Support for migrating Ning events to Drupal nodes.
 */

class NingEventMigration extends NingNodeMigration {
  public function __construct($arguments) {
    $arguments['source_feature'] = 'events';

    $this->sourceFields += array(
      'id' => 'Unique page ID',
      'contributorName' => 'Internal username of author',
      'title' => 'Page title',
      'description' => 'Page body',
      'createdDate' => 'Page creation date',
      'updatedDate' => 'Page updated date',
      'approved' => 'Y/N',
      'startDate' => 'Event start date/time',
      'endDate' => 'Event end date/time',
      'showGuestList' => '1/0',
      'location' => 'Event city, state - or venue if "city" is populated',
      'city' => 'Event city, state',
      'street' => 'Street address of event',
      'organizedBy' => 'Internal username of organizer',
      'state' => 'Event state, populated dynamically from location or city',
    );

    parent::__construct($arguments);

    $this->addUnmigratedSources(array(
      'approved',
      'city',
      'endDate',
      'location',
      'organizedBy',
      'showGuestList',
      'startDate',
      'street',
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

    if (isset($row->comments)) {
      unset($row->comments);
    }
    if (isset($row->attendees)) {
      unset($row->attendees);
    }

    // There are two possibilities - both location and city are populated (and
    // location is the venue), or only location is populated (and contains
    // city, state).
    if (!empty($row->city)) {
      $components = explode(',', $row->city);
    }
    elseif (!empty($row->location)) {
      $components = explode(',', $row->location);
      unset($row->location);
    }
    if (is_array($components)) {
      $row->city = $components[0];
      if (isset($components[1])) {
        $row->state = $components[1];
      }
    }
  }
}

/**
 * Migrate comments on events to Drupal.
 */
class NingEventCommentMigration extends NingCommentMigration {
  public function __construct($arguments) {
    $arguments['source_feature'] = 'events';
    $arguments['parent_id_field'] = 'id';

    parent::__construct($arguments);
  }
}
