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

  /**
   * Items with these topic_id values should be migrated as news nodes, and NOT
   * migrated as forum nodes.
   *
   * @var array
   */
  protected $newsTopicIds = array(58, 101, 156, 170, 184, 190, 313, 346, 366,
    376, 409, 507, 536, 550, 551, 566, 576);

  public function __construct($arguments) {
    $this->arguments = $arguments;
    parent::__construct($arguments);
    Database::addConnectionInfo('rr_legacy', 'default', migrate_rr_connection());
    $this->connection = Database::getConnection('default', 'rr_legacy');
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
