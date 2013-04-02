<?php

define('GARDENS_HEALTH_STATUS_NONE', -1);
define('GARDENS_HEALTH_STATUS_PASS', 0);
define('GARDENS_HEALTH_STATUS_FAIL', 1);

/**
 * Defines the interface for creating health reports.
 */
abstract class GardensHealthReport {

  // An implementing class can define their own messages to be more verbose or use
  // the $messages array to provide additional information to the administrator.
  public $message_error = 'NO STATUS';
  public $message_pass = 'PASSED';
  public $message_fail = 'FAILED';
  
  // The messages array can be populated internally to send arbitrary messages
  // back to the dashboard. These messages can be translated strings or html.
  public $messages = array();
  
  /**
   * Constructor.
   *
   * @param String $name
   *  The human-readable name of this check.
   */
  public function __construct($name) {
    $this->name = $name;
  }

  /**
   * Getter for the health check name.
   */
  public function getName() {
    return $this->name;
  }

  /**
   * Setter for the health check messages array.
   *
   * @param String $message
   */
  public function addMessage($message) {
    $this->messages[] = $message;
  }

  /**
   * Getter for the health check messages array.
   */
  public function getMessages() {
    return $this->messages;
  }

  /**
   * Perform a health check and report back a status that is one of the class
   * constants self::STATUS_x. This check should populate $this->messages if
   * additional information needs to be provided to the administrator.
   *
   * @return Integer
   *  GARDENS_HEALTH_STATUS_NONE, GARDENS_HEALTH_STATUS_PASS, GARDENS_HEALTH_STATUS_FAIL
   */
  public abstract function checkStatus();
}
