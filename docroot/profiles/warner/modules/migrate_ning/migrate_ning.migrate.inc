<?php
/**
 * @file
 * Base classes for migration from Ning to Drupal.
 */

/**
 * Base class for all migrations from Ning.
 */
abstract class NingMigration extends DynamicMigration {
  /**
   * Arguments to the migration - see the constructor for details.
   * @var array
   */
  protected $arguments = array();

  /**
   * Array of source fields from the JSON files, keyed by field name,
   * with description as the value.
   *
   * @var array
   */
  protected $sourceFields = array();

  /**
   * TRUE to scrape files from the website, FALSE to use a local file dump.
   *
   * @var bool
   */
  protected $useRemoteFiles;

  /**
   * Constructor - handles generally-applicable arguments.
   * @param array $arguments
   *  source_directory (required) - Path to the directory containing Ning
   *    source data (JSON and media files). May be a URL.
   *  source_feature (required) - The source feature (e.g, members, blogs, etc.)
   *    to be imported. Used to construct, for example, the ning-members.json
   *    source file name.
   *  use_remote_files (optional) - FALSE (default) to copy media files from the
   *    local dump, TRUE to scrape from the source site.
   *  group_name (optional) - Migration group to hold the migrations. Defaults
   *    to 'Ning'.
   */
  public function __construct(array $arguments) {
    $this->arguments = $arguments;
    $this->sourceDirectory = rtrim($arguments['source_directory'], '/');
    $this->sourceFeature = $arguments['source_feature'];
    if (isset($arguments['use_remote_files']) && $arguments['use_remote_files']) {
      $suffix = '.json';
      $this->useRemoteFiles = TRUE;
    }
    else {
      $suffix = '-local.json';
      $this->useRemoteFiles = FALSE;
    }
    $this->sourceFile = $this->sourceDirectory . '/ning-' .
                        $this->sourceFeature . $suffix;
    if (!empty($arguments['group_name'])) {
      $group_name = $arguments['group_name'];
    }
    else {
      $group_name = 'Ning';
    }
    parent::__construct(MigrateGroup::getInstance($group_name));
  }

  /**
   * Use the explicit machine_name argument as this migration's machine name.
   */
  protected function generateMachineName($class_name = NULL) {
    return $this->arguments['machine_name'];
  }


  /**
   * If we have an image reference, replace it with media tags if we can.
   *
   * @param array $matches
   */
  protected function replaceImgs(array $matches) {
    // Default to the original <img> tag.
    $result = $matches[0];

    // Only proceed for images served by Ning.
    if (preg_match('|src=[\'"](http://api.ning.com(?::80)?/files/.*?)[\'"]|i', $result, $src_matches)) {
      $src = $src_matches[1];
    }
    else {
      return $result;
    }

    // Assume the basenames are unique.
    $basename = basename($src);
    // Strip any query parameters
    $question_mark = strpos($basename, '?');
    if ($question_mark > 0) {
      $basename = substr($basename, 0, $question_mark);
    }

    // Get the fid, if any. If none, we need to migrate the file directly.
    $fid = db_select('file_managed', 'f')
           ->fields('f', array('fid'))
           ->condition('filename', $basename)
           ->execute()
           ->fetchField();
    if (!$fid) {
      $destination_file = 'public://' . $basename;
      if (!$this->copyFile($src)) {
        return $result;
      }

      // Constriuct and save the file object.
      $file = new stdClass;
      $file->uid = 1;
      $file->filename = $basename;
      $file->uri = $destination_file;
      $file->status = FILE_STATUS_PERMANENT;
      $file->filemime = file_get_mimetype($destination_file);
      file_save($file);
      $fid = $file->fid;
    }

    $attributes = array(
      'class' => 'media-image',
      'typeof' => 'foaf:Image',
      'style' => '',
    );

    // Build the media tag
    $img_info = array(
      'type' => 'media',
      'view_mode' => 'media_large',
      'fid' => $fid,
      'attributes' => array($attributes),
    );
    $result = '[[' . drupal_json_encode($img_info) . ']]';

    return $result;
  }

  /**
   * Copy a file from the specified URL to the Drupal files directory. For some
   * reason, we have found that copy() frequently fails in copying files from
   * api.ning.com, while file_get_contents() always succeeds.
   *
   * @param $url
   *
   * @return int
   */
  protected function copyFile($url, $destination_dir = 'public://') {
    try {
      $basename = basename($url);
      // Strip any query parameters
      $question_mark = strpos($basename, '?');
      if ($question_mark > 0) {
        $basename = substr($basename, 0, $question_mark);
      }
      $contents = file_get_contents($url);
      return file_put_contents($destination_dir . $basename,
        $contents);
    }
    catch (Exception $e) {
      Migration::displayMessage($e->getMessage());
      return FALSE;
    }
  }
}

/**
 * Base class for all Ning node migrations.
 */
class NingNodeMigration extends NingMigration {
  /**
   * Machine name of the related member migration.
   *
   * @var string
   */
  protected $memberMigration = '';

  /**
   * Constructor - handles generally-applicable arguments.
   * @param array $arguments
   *  member_migration (required) - Machine name of the related member
   *    migration.
   */
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->memberMigration = $arguments['member_migration'];
    $this->softDependencies[] = $this->memberMigration;

    $this->source = new MigrateSourceJSON($this->sourceFile, 'id', $this->sourceFields,
          array('cache_counts' => TRUE, 'cache_key' => $this->machineName));

    $this->destination = new MigrateDestinationNode($arguments['bundle']);

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'id' =>
          array('type' => 'varchar',
                'length' => 255,
                'not null' => TRUE,
                'description' => 'Unique content ID',
               ),
        ),
        MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('uid', 'contributorName')
         ->sourceMigration($this->memberMigration);
    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('body', 'description');
    $this->addFieldMapping('created', 'createdDate');
    $this->addFieldMapping('changed', 'updatedDate');
    $this->addFieldMapping('status')
         ->defaultValue(NODE_PUBLISHED);
    $this->addFieldMapping('body:format')
         ->defaultValue($arguments['default_format']);

    $this->addUnmigratedDestinations(array(
        'body:summary', 'body:language',
      'is_new',
      'language',
      'log',
      'promote',
      'revision',
      'revision_uid',
      'sticky',
      'tnid',
      'translate',
    ));

    if (module_exists('comment')) {
      $this->addUnmigratedDestinations(array('comment'));
    }
    if (module_exists('path')) {
      $this->addUnmigratedDestinations(array('path'));
      if (module_exists('pathauto')) {
        $this->addUnmigratedDestinations(array('pathauto'));
      }
    }
    if (module_exists('statistics')) {
      $this->addUnmigratedDestinations(array('daycount', 'totalcount', 'timestamp'));
    }
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

    // Keeping the comments in the row can get in the way, let the comment
    // migrations handle them.
    if (isset($row->comments)) {
      unset($row->comments);
    }

    if (!empty($row->description)) {
      // Rewrite any img tags referencing local files to point to a media
      // tag for the migrated file. Ignore any <a> tag referencing the image
      // wrapped around it (well, we assume an api.ning.com ref is a self-ref).
      $row->description = preg_replace_callback(
        '|(?:<a +.*?href="http://api\.ning\.com.*?>)?<img +(.*?)>(?:</a>)?|i',
        array($this, 'replaceImgs'), $row->description);
    }
  }

  /**
   * Convert Ning status values to Drupal node status.
   *
   * @param $value
   *
   * @return int
   */
  protected function handleStatus($value) {
    if (drupal_strtoupper($value) == 'Y') {
      return NODE_PUBLISHED;
    }
    else {
      return NODE_NOT_PUBLISHED;
    }
  }
}

/**
 * Implementation of hook_migrate_api.
 */
function migrate_ning_migrate_api() {
  $api = array(
    'api' => 2,
  );
  return $api;
}
