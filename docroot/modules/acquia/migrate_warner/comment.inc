<?php

class CommentMigration extends XMLMigration {
  public function __construct($group, $live = TRUE) {
    // Always call the parent constructor first for basic setup
    parent::__construct(MigrateGroup::getInstance('warner'));
    $this->softDependencies = array('Album', 'Member');

    $this->team = array(
      new MigrateTeamMember('Mike Ryan', 'mikeryan@cyrve.com', 'Cyrve'),
      new MigrateTeamMember('Pete Jelliffe', 'Pete.Jelliffe@wmg.com', 'Warner'),
      new MigrateTeamMember('Nik Gregory', 'Nik.Gregory@acquia.com', 'Acquia'),
    );

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
      'content_id' => 'Eos ID of the content being commented upon',
      'content_type' => 'Type of content being commented upon',
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
    if ($live) {
      $filespec = "$localpath/site_content_live*.xml";
    }
    else {
      $filespec = "$localpath/site_content_non_live*.xml";
    }
    foreach (glob($filespec) as $items_url) {
      break;
    }
    $item_xpath = "/file/article/comment";  // Select elements of the requested type
    $item_ID_xpath = '@EosId';
    $this->source = new MigrateSourceXML($items_url, $item_xpath, $item_ID_xpath,
      $fields, array('reader_class' => 'CommentXMLReader', 'cache_counts' => TRUE));

    // Audio is an arbitrary selecion - we'll fill in the correct type in prepare()
    $this->destination = new MigrateDestinationComment('comment_node_audio');

    // Common mappings
    $this->addFieldMapping('nid', 'content_id')
         ->sourceMigration(array('Album', 'Audio', 'Blog', 'Discussion', 'Photo', 'Video'));
    $this->addFieldMapping(NULL, 'content_type')
         ->xpath('../@type');
    $this->addFieldMapping('uid', 'profile')
         ->xpath('@profile')
         ->sourceMigration('Member');
    $this->addFieldMapping('created', 'addedDate')
         ->xpath('@addedDate');
    $this->addFieldMapping('changed', 'addedDate')
         ->xpath('@addedDate');
    // '16' is Safe HTML in Gardens
    $this->addFieldMapping('comment_body', 'comment')
         ->xpath('.')
         ->arguments(array('format' => $this->safe_html));
    $this->addFieldMapping('status')
         ->defaultValue($live);

    $this->addUnmigratedDestinations(array('pid', 'subject', 'hostname', 'thread', 'name',
    	'homepage', 'language', 'path', 'mail'));
  }

  public function prepareRow($row) {
    $row->content_id = $row->xml->parentEosId;
    $row->content_type = $row->xml->parentType;
  }
  
  public function prepare($comment, $row) {
    switch ($row->content_type) {
      case 'album':
        $comment->node_type = 'comment_node_album';
        break;
      case 'audio':
        $comment->node_type = 'comment_node_audio';
        break;
      case 'blog':
        $comment->node_type = 'comment_node_blog';
        break;
      case 'discussion':
        $comment->node_type = 'comment_node_forum';
        break;
      case 'photo':
        $comment->node_type = 'comment_node_photo';
        break;
      case 'video':
        $comment->node_type = 'comment_node_video';
        break;
    }
  }
}

class CommentNonLiveMigration extends CommentMigration {
  public function __construct($group) {
    parent::__construct($group, FALSE);
    $this->description = t('Import non-live comments from XML into Drupal');
  }
}

class CommentXMLReader extends MigrateXMLReader {
  protected $eosidPath = array();
  protected $typePath = array();

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
        $this->eosidPath[$this->reader->depth] = $this->reader->getAttribute('EosId');
        $this->typePath[$this->reader->depth] = $this->reader->getAttribute('EosId');
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
            // Remember the parent's ID
            $this->currentElement->parentEosId = $this->eosidPath[$this->reader->depth-1];
            $this->currentElement->parentType = $this->typePath[$this->reader->depth-1];
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
