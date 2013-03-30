<?php
/**
 * @file
 * Mappings specific to davidbowie.
 */

class davidbowieRegistry extends AAMigrationRegistry {
  public function __construct($site_name) {
    parent::__construct($site_name, 7);
    $this->commonArguments['format_mappings'] =  array(
      'filtered_html' => '1',
      'full_html' => '6',
      'unfiltered' => '6',
      'plain_text' => '11',
    );

    $this->sourceDirectory =
      'http://aa:letMeIn!@davidbowie.bkstg.artistarena.com/sites/davidbowie.com/files/';
  }

  protected function register() {
    $this->registerUser();
    $arguments = $this->commonArguments + array(
      'default_uid' => $this->defaultUid,
      'skip_pictures' => TRUE,  // Only relevant to D7 sources
      'source_dir' => $this->sourceDirectory,
      'user_migration' => 'User',
      'file_class' => 'MigrateAAFileUri',
    );
    $this->registerMigration('File', $arguments);
    $this->registerTerm('tags', 'tags');
    $this->registerNode('article', 'blog');
  }
}

/**
 * Just migrating one admin account.
 */
class davidbowieUserMigration extends AAUser7Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_member_name', 'name');
    $this->addFieldMapping('field_nickname', 'name');
    $this->addFieldMapping('field_about_me', 'field_bio');
    $this->addFieldMapping('roles', NULL, FALSE);
    $this->addFieldMapping('role_names', NULL, FALSE)
         ->defaultValue('administrator');

    $this->addUnmigratedDestinations(array(
        'field_about_me:language',
      'field_birthdate',
      'field_city', 'field_city:language',
      'field_country',
      'field_eos_id',
      'field_favorite_bands', 'field_favorite_bands:language',
      'field_gender',
      'field_hobbies', 'field_hobbies:language',
        'field_member_name:language',
        'field_nickname:language',
      'field_postal_code', 'field_postal_code:language',
      'field_profile_14', 'field_profile_14:language',
      'field_profile_updated',
      'field_provider', 'field_provider:language',
      'field_site_id', 'field_site_id:language',
      'field_state', 'field_state:language',
      'field_twitter_name', 'field_twitter_name:language',
      'field_type',
      'field_usage_type',
      'path',
    ));

    $this->addUnmigratedSources(array(
      'field_fav_song',
      'field_user_location',
      'uuid',
    ));
  }

  protected function query() {
    $query = parent::query();
    $query->condition('name', 'Total Blam Blam');
    return $query;
  }
}

class davidbowieFileMigration extends AAFile7Migration {}


/**
 * Override to disable url encoding.
 */
class MigrateAAFileUri extends MigrateFileUri {
  protected function copyFile($destination) {
    // Perform the copy operation
    if (!@copy($this->sourcePath, $destination)) {
      throw new MigrateException(t('The specified file %file could not be copied to ' .
        '%destination.',
        array('%file' => $this->sourcePath, '%destination' => $destination)));
    }
    else {
      return TRUE;
    }
  }
}

class davidbowietagsTermMigration extends AATerm7Migration {}

class davidbowiearticleMigration extends AANode7Migration {
  /**
   * List of image fids for the current node.
   *
   * @var array
   */
  protected $fids = array();

  protected $authorUid;

  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $this->dependencies = array('File', 'User', 'tagsTerm');

    $this->addFieldMapping('field_tags', 'field_tags', FALSE)
         ->sourceMigration('tagsTerm');
    $this->addFieldMapping('field_tags:source_type', NULL, FALSE)
         ->defaultValue('tid');

    $this->addFieldMapping('field_blog_category')
         ->defaultValue('News');
    $this->addFieldMapping('field_blog_category:create_term')
         ->defaultValue(TRUE);
    $this->authorUid = db_select('users', 'u')
                  ->fields('u', array('uid'))
                  ->condition('name', 'Total Blam Blam')
                  ->execute()
                  ->fetchField();
    $this->addFieldMapping('uid', NULL, FALSE)
         ->defaultValue($this->authorUid);

    $this->addFieldMapping('field_blog_image', 'field_image');
    $this->addFieldMapping('field_blog_image:file_class')
         ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('field_blog_image:alt', 'field_image:alt');
    $this->addFieldMapping('field_blog_image:title', 'field_image:title');
    $this->addFieldMapping('status', NULL, FALSE)
         ->defaultValue(NODE_PUBLISHED);

    $this->addUnmigratedDestinations(array(
        'field_blog_category:source_type', 'field_blog_category:ignore_case',
        'field_blog_image:language',
        'field_tags:create_term', 'field_tags:ignore_case',
    ));

    $this->addUnmigratedSources(array(
        'field_image:height', 'field_image:width',
      'uid',
    ));
  }

  public function prepareRow($row) {
    if (parent::prepareRow($row) === FALSE) {
      return FALSE;
    }

    // Change title from all upper to capitalized
    $row->title = ucwords(strtolower(trim($row->title)));

    // Strip dates from the beginning of titles
    if (preg_match('|[0-9][0-9]\.[0-9][0-9]\.[0-9][0-9][0-9][0-9] |', $row->title)) {
      $row->title = substr($row->title, 11);
    }
    elseif (preg_match('|[0-9][0-9]/[0-9][0-9]/[0-9][0-9]/|', $row->title)) {
          $row->title = substr($row->title, 9);
    }

    $this->fids = array();
    // If the former image field was populated, prepend the image
    if (isset($row->field_image)) {
      if (is_array($row->field_image)) {
        $row->field_image = reset($row->field_image);
      }
      $new_fid = db_select('migrate_map_file', 'map')
                 ->fields('map', array('destid1'))
                 ->condition('sourceid1', $row->field_image)
                 ->execute()
                 ->fetchField();
      if ($new_fid) {
        $row->field_image = $new_fid;
        $this->fids[] = $new_fid;
      }
    }

    // Look for embedded local image references, create file entities as
    // necessary, and rewrite the references to media tags, also adding the
    // entities to the image field.
    $row->body = preg_replace_callback('|<img +(.*?)>|i',
      array($this, 'replaceImgs'), $row->body);
    $row->field_image = $this->fids;

    // Remove <strong> tags
    $row->body = preg_replace('|</?strong>|', '', $row->body);
  }

  /**
   * If we have an image reference, replace it with media tags if we can.
   *
   * @param array $matches
   */
  protected function replaceImgs(array $matches) {
    // Default to the original <img> tag.
    $result = $matches[0];

    // The src parameter is required
    if (preg_match('|src=[\'"]/sites/davidbowie.com/files/(.*?)[\'"]|i', $result, $src_matches)) {
      $src = $src_matches[1];
    }
    else {
      return $result;
    }

    // Get the fid, if any. If none, let the img tag stand
    $fid = db_select('file_managed', 'f')
           ->fields('f', array('fid'))
           ->condition('filename', urldecode(basename($src)))
           ->execute()
           ->fetchField();
    if (!$fid) {
      // Copy the file and create the entity
      $file = new MigrateAAFileUri();
      $src = 'http://aa:letMeIn!@davidbowie.bkstg.artistarena.com/sites/davidbowie.com/files/'
             . $src;
      try {
        $file = $file->processFile($src, $this->authorUid);
      }
      catch (Exception $e) {
        $this->saveMessage($e->getMessage(), MigrationBase::MESSAGE_INFORMATIONAL);
        return $result;
      }
      $fid = $file->fid;
    }
    $this->fids[$fid] = $fid;
    // Remove from the body - it's enough to have it in the field.
    return '';
  }
}
