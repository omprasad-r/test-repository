<?php

/**
 * Base class for registration of Artist Arena site migrations.
 */
abstract class AAMigrationRegistry {
  /**
   * Gardens site name (e.g., greenday, pattygriffinnew, etc.)
   * @var string
   */
  protected $siteName;

  /**
   * Drupal version of the source site (6 or 7).
   * @var string
   */
  protected $sourceVersion;

  /**
   * Arguments common to all migrations for a given site.
   * @var array
   */
  protected $commonArguments = array();

  /**
   * Arguments common to all node migrations for a given site.
   * @var array
   */
  protected $nodeArguments = array();

  /**
   * Arguments common to all comment migrations for a given site.
   * @var array
   */
  protected $commentArguments = array();

  /**
   * Arguments specific to the role migration for a given site.
   * @var array
   */
  protected $roleArguments = array();

  /**
   * The source directory to prepend to files for copying from the source site.
   * D7 sources will typically look something like http://greenday.com/sites/greenday.com/files/,
   * D6 sources a simple http://www.pattygriffin.com/
   * @var
   */
  protected $sourceDirectory;

  /**
   * Initialize the defaults for a given site migration.
   *
   * @param $site_name
   * @param $source_version
   */
  public function __construct($site_name, $source_version, $default_uid = 636) {
    $this->siteName = $site_name;
    $this->sourceVersion = $source_version;
    $this->defaultUid = $default_uid;    // wmgd2c
    $this->commonArguments = array(
      'source_version' => $this->sourceVersion,
      'group' => $this->siteName,
      'source_connection' => 'legacy',
      'source_database' => array(
        'driver' => 'mysql',
        'database' => $this->siteName . '_legacy',
        'username' => 'legacy',
        'password' => variable_get('migrate_aa_password', ''),
        'host' => '23.23.120.51',
        'prefix' => '',
      ),
    );
    $this->nodeArguments = array(
      'user_migration' => 'User',
      'default_uid' => $this->defaultUid,
    );
    $this->commentArguments = array(
      'user_migration' => 'User',
      'default_uid' => $this->defaultUid,
    );
  }

  /**
   * @static
   * Instantiate the registration object for a given site, and perform
   * registration of all migrations for that site.
   */
  static public function registerAll() {
    $site_name = variable_get('migrate_aa_sitename', '');
    $registry_class = $site_name . 'Registry';
    $registry = new $registry_class($site_name);
    $registry->register();
  }

  /**
   * Register one migration.
   *
   * @param $type
   * @param $arguments
   */
  protected function registerMigration($type, $arguments = array()) {
    $arguments['class_name'] = $this->siteName . $type . 'Migration';
    $arguments['machine_name'] = $type;
    $arguments['description'] = t('!type migration from Drupal !version',
      array('!type' => $type, '!version' => $this->sourceVersion));
    if (class_exists($arguments['class_name'])) {
      MigrationBase::registerMigration($arguments['class_name'],
        $arguments['machine_name'], $arguments);
    }
    else {
      MigrationBase::displayMessage(
        t('Could not register !machine_name migration, class !class_name does not exist.',
          array('!machine_name' => $arguments['machine_name'],
                '!class_name' => $arguments['class_name'])));
    }
  }

  /**
   * Each site will implement register(), to register all their site-specific
   * migrations.
   */
  abstract protected function register();

  /**
   * Register the role migration.
   */
  protected function registerRole() {
    $arguments = $this->commonArguments + $this->roleArguments;
    $this->registerMigration('Role', $arguments);
  }

  /**
   * Register the picture migration.
   */
  protected function registerPicture() {
    $arguments = $this->commonArguments + array(
      'default_uid' => $this->defaultUid,
      'source_dir' => $this->sourceDirectory,
    );
    $this->registerMigration('Picture', $arguments);
  }

  /**
   * Register the user migration.
   */
  protected function registerUser() {
    $arguments = $this->commonArguments;
    if (class_exists($this->siteName . 'PictureMigration')) {
      $arguments['picture_migration'] = 'Picture';
    }
    if (class_exists($this->siteName . 'RoleMigration')) {
      $arguments['role_migration'] = 'Role';
    }
    $this->registerMigration('User', $arguments);
  }

  /**
   * Register the file migration.
   */
  protected function registerFile() {
    $arguments = $this->commonArguments + array(
      'default_uid' => $this->defaultUid,
      'skip_pictures' => TRUE,  // Only relevant to D7 sources
      'source_dir' => $this->sourceDirectory,
    );
    if (class_exists($this->siteName . 'UserMigration')) {
      $arguments['user_migration'] = 'User';
    }
    $this->registerMigration('File', $arguments);
  }

  /**
   * Register a taxonomy term migration.
   *
   * @param $source_vocab
   *  Machine name (or, for D6, vid) of the source vocabulary.
   * @param $destination_vocab
   *  Machine name of the destination vocabulary.
   * @param array $overrides
   *  Any arguments to be overridden for this migration.
   */
  protected function registerTerm($source_vocab, $destination_vocab, $overrides = array()) {
    $arguments = $this->commonArguments + array(
      'source_vocabulary' => $source_vocab,
      'destination_vocabulary' => $destination_vocab,
    );
    $arguments = array_merge_recursive($arguments, $overrides);
    $this->registerMigration($source_vocab . 'Term', $arguments);
  }

  /**
   * Register a node migration.
   *
   * @param $source_type
   *  Machine name of the source node type.
   * @param $destination_type
   *  Machine name of the destination node type.
   * @param array $overrides
   *  Any arguments to be overridden for this migration.
   */
  protected function registerNode($source_type, $destination_type, $overrides = array()) {
    $arguments = $this->commonArguments + $this->nodeArguments + array(
      'source_type' => $source_type,
      'destination_type' => $destination_type,
    );
    $arguments = array_merge_recursive($arguments, $overrides);
    $this->registerMigration($source_type, $arguments);
  }

  /**
   * Register a migration from gallery nodes to media category terms.
   *
   * @param $source_type
   *  Machine name of the source gallery node type.
   * @param $destination_vocab
   *  Machine name of the destination category vocabulary.
   * @param array $overrides
   *  Any arguments to be overridden for this migration.
   */
  protected function registerGallery($source_type, $destination_vocab, $overrides = array()) {
    $arguments = $this->commonArguments + array(
      'source_type' => $source_type,
      'destination_vocabulary' => $destination_vocab,
    );
    $arguments = array_merge_recursive($arguments, $overrides);
    $this->registerMigration($source_type, $arguments);
  }

  /**
   * Register a comment migration.
   *
   * @param $source_type
   *  Machine name of the source node type parenting these comments.
   * @param $destination_type
   *  Machine name of the destination node type parenting these comments.
   * @param array $overrides
   *  Any arguments to be overridden for this migration.
   */
  protected function registerComment($source_type, $destination_type, $overrides = array()) {
    $arguments = $this->commonArguments + $this->commentArguments + array(
      'source_type' => $source_type,
      'destination_type' => $destination_type,
      'node_migration' => $source_type,
    );
    $arguments = array_merge_recursive($arguments, $overrides);
    $this->registerMigration("Comment$source_type", $arguments);
  }
}
