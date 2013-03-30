<?php
/**
 * @file
 * General support for migrating Ning comments to Drupal.
 */

class NingCommentMigration extends NingMigration {
  /**
   * Machine name of the related member migration.
   *
   * @var string
   */
  protected $memberMigration = '';

  /**
   * Machine name of the related entity migration.
   *
   * @var string
   */
  protected $entityMigration = '';

  /**
   * Constructor - handles generally-applicable arguments.
   * @param array $arguments
   *  member_migration (required) - Machine name of the related member
   *    migration.
   *  entity_migration (required) - Machine name of the related entity
   *    migration.
   *  parent_id_field (required) - Name of the unique ID field for the
   *    comment parent.
   */
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->memberMigration = $arguments['member_migration'];
    $this->entityMigration = $arguments['entity_migration'];
    $this->softDependencies[] = $this->memberMigration;
    $this->softDependencies[] = $this->entityMigration;

    $this->sourceFields += array(
      'id' => 'Unique page ID',
      'contributorName' => 'Internal username of author',
      'description' => 'Page body',
      'createdDate' => 'Page creation date',
      'parentId' => 'ID of the parent user/node',
    );

    // Note that we plug in our own JSON reader class, because the comments
    // are embedded within the top-level objects the base class handles.
    $this->source = new MigrateSourceJSON($this->sourceFile,
      $arguments['parent_id_field'], $this->sourceFields,
      array('cache_counts' => TRUE, 'reader_class' => 'NingJSONReader',
        'cache_key' => $this->machineName));

    $this->destination = new MigrateDestinationComment($arguments['bundle']);

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'id' =>
          array('type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'description' => 'Unique content ID',
               ),
        ),
        MigrateDestinationComment::getKeySchema()
    );

    $this->addFieldMapping('uid', 'contributorName')
         ->sourceMigration($this->memberMigration);
    $this->addFieldMapping('nid', 'parentId')
         ->sourceMigration($this->entityMigration);
    $this->addFieldMapping('comment_body', 'description');
    $this->addFieldMapping('created', 'createdDate');
    $this->addFieldMapping('changed', 'createdDate');
    $this->addFieldMapping('status')
         ->defaultValue(COMMENT_PUBLISHED);
    $this->addFieldMapping('comment_body:format')
         ->defaultValue($arguments['default_format']);

    $this->addUnmigratedDestinations(array(
        'comment_body:language',
      'homepage',
      'hostname',
      'language',
      'mail',
      'name',
      'pid',
      'subject',
      'thread'
    ));

    if (module_exists('redirect')) {
      $this->addUnmigratedDestinations(array('migrate_redirects'));
    }
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

    if (!empty($row->description)) {
      // Rewrite any img tags referencing local files to point to a media
      // tag for the migrated file. Ignore any a tag referencing the image
      // wrapped around it (well, we assume an api.ning.com ref is a self-ref).
      $row->description = preg_replace_callback(
        '|(?:<a +.*?href="http://api\.ning\.com.*?>)?<img +(.*?)>(?:</a>)?|i',
        array($this, 'replaceImgs'), $row->description);
    }
  }

}

/**
 * A JSON reader class that knows how to find the Ning comments.
 */
class NingJSONReader extends MigrateJSONReader {
  /**
   * The comments we are currently iterating over.
   *
   * @var array
   */
  protected $cachedComments = array();

  /**
   * Unique ID of the member or node the comments are replying to.
   *
   * @var string
   */
  protected $parentId = '';

  /**
   * Implements MigrateJSONReader::next().
   *
   * Each object in the JSON file has an option comments array, and we want
   * to iterate over that. The strategy is to call the base implementation, which
   * returns the parent object, repeatedly until we find one with comments.
   * Then we save that comment array and iterate over that until its empty, at
   * which point we'll look for another parent object with comments.
   */
  public function next() {
    $this->currentElement = $this->currentId = NULL;
    if (empty($this->cachedComments)) {
      // We have no comments to work with, find the next parent object with
      // comments.
      parent::next();
      while (!empty($this->currentElement)) {
        if (isset($this->currentElement->comments)) {
          // We found some comments - remember them and escape to process them.
          $this->cachedComments = $this->currentElement->comments;
          $this->parentId = $this->currentId;
          break;
        }
        // Keep looking for a parent object with comments.
        parent::next();
      }
    }
    // If we have comments to work with, pull the first one off the queue.
    if (!empty($this->cachedComments)) {
      $this->currentElement = array_shift($this->cachedComments);
      $this->currentElement->parentId = $this->parentId;
      $this->currentID = $this->currentElement->id;
    }
  }
}
