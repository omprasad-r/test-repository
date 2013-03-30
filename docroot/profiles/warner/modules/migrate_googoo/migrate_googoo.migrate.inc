<?php
/**
 * @file
 * Summary
 */

/**
 * Common base class for all googoodolls migrations.
 */
abstract class GooGooMigration extends Migration {
  /**
   * User ID of the account (Goo Goo Dolls) to use as the author of content
   * without an identifiable author.
   *
   * @var int
   */
  protected $defaultAuthor = 21626;

  public function __construct() {
    parent::__construct(MigrateGroup::getInstance('GooGooDolls'));

    // Setup connections for all four source databases.
    $connectionInfo = array(
      'driver' => 'mysql',
      'username' => 'legacy',
      'password' => variable_get('migrate_googoo_password', ''),
      'host' => '23.23.120.51',
      'prefix' => '',
    );
    foreach (array('site', 'moblog', 'smf', 'agilebill') as $database_suffix) {
      $connectionInfo['database'] = 'googoo_' . $database_suffix;
      Database::addConnectionInfo($connectionInfo['database'], 'default', $connectionInfo);
    }
  }

  /**
   * Build a full timestamp out of the separate date and time columns (if present)
   *
   * @param $row
   */
  protected function handleDate($row) {
    if (isset($row->time)) {
      $timestamp = Migration::timestamp($row->date . ' ' . $row->time);
      // Adjust for timezone (minus 4 hours)
      $row->date = $timestamp - 4 * 60 * 60;
    }
  }
}

/**
 * Implements hook_migrate_api().
 */
function migrate_googoo_migrate_api() {
  $api = array(
    'api' => 2,
    'migrations' => array(
      'GooGooBlogTour' => array('class_name' => 'GooGooBlogTourMigration'),
      'GooGooBlogRoadie' => array('class_name' => 'GooGooBlogRoadieMigration'),
      'GooGooBlogLLI' => array('class_name' => 'GooGooBlogLLIMigration'),
      'GooGooBlogStudio' => array('class_name' => 'GooGooBlogStudioMigration'),
      'GooGooBlogGutterFlower' => array('class_name' => 'GooGooBlogGutterFlowerMigration'),
      'GooGooBlogDailyGoo' => array('class_name' => 'GooGooBlogDailyGooMigration'),
      'GooGooBlogNews' => array('class_name' => 'GooGooBlogNewsMigration'),
      'GooGooTourDate' => array('class_name' => 'GooGooTourDateMigration'),
      'GooGooForumTopic' => array('class_name' => 'GooGooForumTopicMigration'),
      'GooGooForumComment' => array('class_name' => 'GooGooForumCommentMigration'),
      'GooGooPhotoMoblog' => array('class_name' => 'GooGooPhotoMoblogMigration'),
      'GooGooPhotoMoblogComment' => array('class_name' => 'GooGooPhotoMoblogCommentMigration'),
      'GooGooPhoto' => array('class_name' => 'GooGooPhotoMigration'),
      'GooGooPhotoComment' => array('class_name' => 'GooGooPhotoCommentMigration'),
      'GooGooPicture' => array('class_name' => 'GooGooPictureMigration'),
      'GooGooUser' => array('class_name' => 'GooGooUserMigration'),
    ),
  );
  return $api;
}
