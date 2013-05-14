<?php
/**
 * @file
 * Registration and shared base class for Roadrunner data migrations.
 */

/**
 * Common base class for all migration classes - primarily manages the source
 * database connection.
 */
abstract class RRMigration extends Migration {
  /**
   * Array of arguments from the class registration.
   *
   * @var array
   */
  protected $arguments = array();

  /**
   * Connection object for the source database for the given site.
   *
   * @var DatabaseConnection
   */
  protected $connection;

  /**
   * Base URL for the original site being migrated.
   */
  protected $artistSiteUrl;

  public function __construct($arguments) {
    $this->arguments = $arguments;
    parent::__construct(MigrateGroup::getInstance($arguments['group_name']));
    Database::addConnectionInfo('rr_legacy', 'default', migrate_rr_connection());
    $this->connection = Database::getConnection('default', 'rr_legacy');
  }

  protected function generateMachineName() {
    return $this->arguments['machine_name'];
  }
}

/**
 * Implements hook_migrate_api().
 *
 * @return array
 */
function migrate_rr_migrate_api() {
  $api = array(
    'api' => 2,
  );
  return $api;
}
