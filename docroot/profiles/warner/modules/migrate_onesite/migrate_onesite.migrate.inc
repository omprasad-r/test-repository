<?php
/**
 * @file
 * Base classes for migration from ONEsite to Drupal.
 */

/**
 * Base class for all migrations from ONEsite.
 */
abstract class OnesiteMigration extends DynamicMigration {
  /**
   * Arguments to the migration - see the constructor for details.
   * @var array
   */
  protected $arguments = array();

  /**
   * Array of source fields from the ONEsite tables, keyed by column name,
   * with description as the value.
   *
   * @var array
   */
  protected $sourceFields = array();

  /**
   * Constructor - handles generally-applicable arguments.
   * @param array $arguments
   *  source_connection (required) - Name of the database connection holding
   *    the source data. If source_database is not defined, it is assumed to
   *    already be defined (most likely via $databases in settings.php).
   *  source_database (optional) - If present, a database array as in
   *    $databases used to defined a source connection. For security, do not
   *    define the password directly in this array, assign it instead to the
   *    variable migrate_onesite_password (e.g., in $conf in settings.php).
   *  group_name (optional) - Migration group to hold the migrations. Defaults
   *    to 'ONEsite'.
   */
  public function __construct(array $arguments) {
    $this->arguments = $arguments;
    $this->sourceConnection = $arguments['source_connection'];
    if (!empty($arguments['source_database'])) {
      $arguments['source_database']['password'] = variable_get(
        'migrate_onesite_password', '');
      Database::addConnectionInfo($this->sourceConnection, 'default', $arguments['source_database']);
    }
    if (!empty($arguments['group_name'])) {
      $group_name = $arguments['group_name'];
    }
    else {
      $group_name = 'ONEsite';
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
   * Utility callback for node and comment bodies. Converts the custom [quote]
   * tags to HTML <blockquote>.
   *
   * @param string $body
   *
   * @return string
   */
  protected function handleQuotes($body) {
    $body = preg_replace('|\[quote[^\]]*\]|', '<blockquote>', $body);
    $body = str_replace('[/quote]', '</blockquote>', $body);
    return $body;
  }
}

/**
 * Base class for all ONEsite node migrations.
 */
abstract class OnesiteNodeMigration extends OnesiteMigration {
  /**
   * Constructor - Defines default mappings appropriate for most, if not all,
   * nodes, as well as creating the destination class.
   * @param array $arguments
   *  bundle (required) - The destination content type (bundle).
   *  user_migration (required) - Machine name of the migration importing users.
   *  default_format (optional) - Machine name of the default format for bodies.
   */
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->dependencies[] = $arguments['user_migration'];

    $this->destination = new MigrateDestinationNode($arguments['bundle']);

    $this->addUnmigratedDestinations(array(
        'body:summary', 'body:language',
      'comment',
      'is_new',
      'language',
      'log',
      'promote',
      'revision',
      'revision_uid',
      'sticky',
      'tnid',
      'translate'
    ));

    // Mark some fields provided by optional core and contrib modules.
    if (module_exists('path')) {
      $this->addUnmigratedDestinations(array('path'));
      if (module_exists('pathauto')) {
        $this->addUnmigratedDestinations(array('pathauto'));
      }
    }
    if (module_exists('pathauto')) {
      $this->addUnmigratedDestinations(array('migrate_redirects'));
    }
    if (module_exists('statistics')) {
      $this->addUnmigratedDestinations(array('daycount', 'timestamp', 'totalcount'));
    }
  }

  /**
   * Translate the ONEsite string status codes into Drupal node statuses.
   * @param $value
   *
   * @return int
   */
  protected function handleStatus($value) {
    if ($value == 'public' || $value == 'active') {
      return NODE_PUBLISHED;
    }
    else {
      return NODE_NOT_PUBLISHED;
    }
  }
}
/**
 * Base class for all ONEsite comment migrations.
 */
abstract class OnesiteCommentMigration extends OnesiteMigration {
  /**
   * Constructor - Defines default mappings appropriate for most, if not all,
   * comments, as well as creating the destination class.
   * @param array $arguments
   *  parent_migration (required) - Machine name of the migration that imports
   *    the parent type.
   *  comment_bundle (required) - Machine name of the comment bundle destination
   *    (e.g., 'comment_node_blog').
   *  user_migration (required) - Machine name of the migration importing users.
   *  default_format (optional) - Machine name of the default format for bodies.
   */
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Migration of !type comments',
      array('!type' => $arguments['parent_type']));
    $this->dependencies[] = $arguments['parent_migration'];
    $this->dependencies[] = $arguments['user_migration'];

    $this->destination = new MigrateDestinationComment($arguments['comment_bundle']);

    $this->addUnmigratedDestinations(array(
      'homepage',
      'hostname',
      'language',
      'mail',
      'migrate_redirects',
      'name',
      'pid',
      'thread',
    ));
  }

  /**
   * Translate the ONEsite string status codes into Drupal comment statuses.
   * @param $value
   *
   * @return int
   */
  protected function handleStatus($value) {
    if ($value == 'public' || $value == 'active') {
      return COMMENT_PUBLISHED;
    }
    else {
      return COMMENT_NOT_PUBLISHED;
    }
  }
}

/**
 * Implementation of hook_migrate_api.
 */
function migrate_onesite_migrate_api() {
  $api = array(
    'api' => 2,
  );
  return $api;
}
