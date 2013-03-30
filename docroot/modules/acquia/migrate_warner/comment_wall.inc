<?php

class CommentWallMigration extends XMLMigration {
  public function __construct($group) {
    // Always call the parent constructor first for basic setup
    parent::__construct(MigrateGroup::getInstance('warner'));
    $this->softDependencies = array('Member');

    // Get the right format identifier for Safe HTML on this system
    $this->safe_html = db_select('filter_format', 'f')
                       ->fields('f', array('format'))
                       ->condition('name', 'Safe HTML')
                       ->execute()
                       ->fetchField();

    // Fields common to all content types
    $fields = array(
      'EosId' => 'Eos-wide ID',
      'addedDate' => '',
      'profile' => 'Email of user creating the comment',
      'proxy_nid' => 'Proxy node for the user being commented upon',
      'comment' => 'Text of the comment',
    );

    // The source ID here is the one retrieved from each data item in the XML file, and
    // used to identify specific items
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'EosId' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
        )
      ),
      MigrateDestinationComment::getKeySchema()
    );

    $stream = new MigrateWarnerStreamWrapper();
    $localpath = $stream->getRealPath('warner://') . '/' . variable_get('warner_private_directory', '');
    foreach (glob("$localpath/profile*.xml") as $items_url) {
      break;
    }

    $item_xpath = "/file/profile/comment";  // Select elements of the requested type
    $item_ID_xpath = '@EosId';
    $this->source = new MigrateSourceXML($items_url, $item_xpath, $item_ID_xpath,
      $fields, array('reader_class' => 'CommentWallXMLReader', 'cache_counts' => TRUE));

    $this->destination = new MigrateDestinationComment('comment_node_comment_on_anything');

    // Common mappings
    $this->addFieldMapping('nid', 'proxy_nid');
    $this->addFieldMapping('uid', 'profile')
         ->xpath('@profile')
         ->sourceMigration('Member');
    $this->addFieldMapping('created', 'addedDate')
         ->xpath('@addedDate');
    $this->addFieldMapping('changed', 'addedDate')
         ->xpath('@addedDate');
    $this->addFieldMapping('comment_body', 'comment')
         ->xpath('.')
         ->arguments(array('format' => $this->safe_html));
    $this->addFieldMapping('status')
         ->defaultValue(1);

    $this->addUnmigratedDestinations(array('pid', 'subject', 'hostname', 'thread', 'name',
    	'homepage', 'language', 'path', 'mail'));
  }

  /**
   * Implementation of XMLMigration::prepareRow().
   *
   * @param $row
   */
  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }
    // We pick up the email address from the parent <profile> element...
    $user_email = $row->xml->parentEmail;
    if (!$user_email) {
      return;
    }
    // ...translate it to a Drupal uid...
    $uid = db_select('migrate_map_member', 'map')
           ->fields('map', array('destid1'))
           ->condition('sourceid1', $user_email)
           ->execute()
           ->fetchField();
    if ($uid) {
      // ...and find the proxy node which will parent the comment.
      $row->proxy_nid = db_select('field_data_field_comment', 'cp')
             ->fields('cp', array('field_comment_nid'))
             ->condition('entity_type', 'user')
             ->condition('entity_id', $uid)
             ->execute()
             ->fetchField();
    }
  }
}

class CommentWallXMLReader extends MigrateXMLReader {
  /**
   * Implementation of Iterator::next(). We need to preserve the EosID of
   * the parent element.
   *
   * @return void
   */
  public function next() {
    migrate_instrument_start('CommentXMLReader::next');
    $this->currentElement = $this->currentId = NULL;

    // Loop over each node in the XML file, looking for elements at a path
    // matching the input query string (represented in $this->elementsToMatch).
    while ($this->reader->read()) {
      if ($this->reader->nodeType == XMLREADER::ELEMENT) {
        $this->currentPath[$this->reader->depth] = $this->reader->localName;
        if ($this->reader->depth == 2 && $this->reader->localName == 'profile') {
          $this->email = $this->reader->readString();
        }
        if ($this->currentPath == $this->elementsToMatch) {
          // We're positioned to the right element path - if filtering on an
          // attribute, check that as well before accepting this element.
          if (empty($this->attributeName) ||
              ($this->reader->getAttribute($this->attributeName) == $this->attributeValue)) {
            // We've found a matching element - get a SimpleXML object representing it.
            // We must associate the DOMNode with a DOMDocument to be able to import
            // it into SimpleXML.
            $node = $this->reader->expand();
            $dom = new DOMDocument();
            $node = $dom->importNode($node, TRUE);
            $dom->appendChild($node);
            $this->currentElement = simplexml_import_dom($node);
            // Remember the parent's email
            $this->currentElement->parentEmail = $this->email;
            $this->currentId = (string)reset($this->currentElement->xpath($this->idQuery));
            break;
          }
        }
      }
      elseif ($this->reader->nodeType == XMLREADER::END_ELEMENT) {
        // Trim currentPath as we exit each element
        unset($this->currentPath[$this->reader->depth]);
      }
    }
    migrate_instrument_stop('CommentXMLReader::next');
  }
}
