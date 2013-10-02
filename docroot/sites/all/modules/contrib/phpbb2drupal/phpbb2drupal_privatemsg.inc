<?php

/**
 * Import Private Messages to the privatemsg module
 *
 * @author DarkSnow.net
 * @file
 * Import PHPBB private messages into the Drupal privatemsg system
 */
class PhpbbPrivateMessageMigration extends PhpbbMigration {
  static $phpbb_type = 'PrivateMessage';
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Migrate private messages to the privatemsg module.');
    $this->dependencies = array($this->getMigrationName('User'));

    $this->source = new MigrateSourceSQL($this->query(), array(), NULL,
      array('map_joinable' => FALSE));

    $this->destination = new MigrateDestinationPrivateMsg(
      array('text_format' => $this->importFormat)
    );

    if ($this->phpbbVersion == 2) {
      $source_key = 'privmsgs_id';
    }
    else {
      $source_key = 'msg_id';
    }
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        $source_key => array('type' => 'int', 'unsigned' => TRUE, 'not null' => TRUE)
      ),
      MigrateDestinationPrivateMsg::getKeySchema()
    );

    $this->addFieldMapping('subject', 'message_subject');
    $this->addFieldMapping('body', 'message_text');
    $this->addFieldMapping('format')->defaultValue($this->importFormat);
    $this->addFieldMapping('recipients', 'to_address')
      ->separator(PHPBB_LIST_SEPARATOR)
      ->sourceMigration($this->getMigrationName('User'));
    $this->addFieldMapping('timestamp', 'message_time');
    $this->addFieldMapping('author', 'author_id')
      ->sourceMigration($this->getMigrationName('User'))->defaultValue(1);
    $this->addFieldMapping('is_new')->defaultValue(1);

    $this->addFieldMapping(NULL, 'bbcode_uid')->issueGroup(t('DNM')); // Used to sanitise body text

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('path'));
  }

  /**
   * Method to return the query used in the import. 
   * Can be called by sub classes and extended if needed.
   */
  protected function query() {
    $query =
      Database::getConnection('default', $this->migration_settings['machine_name'])
        ->select('privmsgs', 'p');
    if ($this->phpbbVersion == 2) {
      $query->condition('p.privmsgs_type', 0);
      $query->innerJoin('privmsgs_text', 't', 'p.privmsgs_id=t.privmsgs_text_id');
      $query->addField('p', 'privmsgs_id');
      $query->addField('p', 'privmsgs_from_userid', 'author_id');
      $query->addField('p', 'privmsgs_date', 'message_time');
      $query->addField('p', 'privmsgs_subject', 'message_subject');
      $query->addField('t', 'privmsgs_text', 'message_text');
      $query->addField('t', 'privmsgs_bbcode_uid', 'bbcode_uid');
      $query->addField('p', 'privmsgs_to_userid', 'to_address');
      $query->orderBy('p.privmsgs_date');
    }
    else {
      $query->fields('p', array(
        'msg_id', 'author_id', 'message_time',
        //'message_edit_time', 'message_edit_user',
        'message_subject', 'message_text',
        'bbcode_uid', 'to_address'
      ))
      ->orderBy('message_time');
    }
    return $query;
  }
  
  function prepareRow($message) {
    if (parent::prepareRow($message) === FALSE) {
      return FALSE;
    }

    if ($this->phpbbVersion == 3) {
      // Populate to_address with all recipients of this message
      $recipients = array();
      $query = Database::getConnection('default', $this->migration_settings['machine_name'])
        ->select('privmsgs_to', 'pt')
        ->fields('pt', array('user_id'))
        ->condition('msg_id', $message->msg_id);

      foreach ($query->execute() as $to) {
        if ($to->user_id != $message->author_id) {
          array_push($recipients, $to->user_id);
        }
      }

      if (count($recipients) == 0 && !empty($message->to_address)) {
        // If there's no entry in the pm_to table, use the to_address field instead
        array_push($recipients, drupal_substr($message->to_address, 2));
      }

      if (count($recipients) == 0 || $recipients[0] == NULL) {
        // If there's still nobody to send it to, it's an invalid message
        return FALSE;
      }
      $message->to_address = implode(PHPBB_LIST_SEPARATOR, $recipients);
    }

    // remove the :bbcode_uid from message_text
    if (!empty($message->bbcode_uid)) {
      $message->message_text = preg_replace("/:$message->bbcode_uid/", '', $message->message_text);
    }
    $message->message_text = PhpbbContentHelpers::stripBbcode($message->message_text);
    $message->message_text = PhpbbContentHelpers::textSanitise($message->message_text);
    $message->message_subject = PhpbbContentHelpers::textSanitise($message->message_subject);
  }
  
  function prepare($message, $row) {
    // If all recipents are deleted then send to anon user
    if (!sizeof($message->recipients)) {
      $message->recipients[] = 0;
    }
    // Just won't work with uid 1
    elseif ($message->recipients == 1) {
      throw new MigrateException(t('No valid recipients'));
    }
  }
}
