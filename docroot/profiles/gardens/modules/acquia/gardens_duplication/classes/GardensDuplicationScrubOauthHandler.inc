<?php

/**
 * @file
 * Contains GardensDuplicationScrubOauthHandler.
 */

/**
 * Scrubs the OAuth keys used by Member API.
 */
class GardensDuplicationScrubOauthHandler extends \Acquia\Acsf\AcsfEventHandler {

  /**
   * Implements \Acquia\Acsf\AcsfEventHandler::handle().
   */
  public function handle() {
    drush_print(dt('Entered @class', array('@class' => get_class($this))));

    $tables = array(
      'oauth_common_consumer',
      'oauth_common_provider_consumer',
    );
    foreach ($tables as $table) {
      if (db_table_exists($table)) {
        $query = db_delete($table);

        if ($preserved = self::getPreservedConsumers()) {
          $query->condition('csid', $preserved, 'NOT IN');
        }

        $query->execute;
      }
    }
  }

  public function getPreservedConsumers() {
    $preserved = array();

    // Preserve the oAuth consumer data required by the MASt mobile app user.
    if (module_exists('mast') && variable_get('mast_app_uid')) {
      if ($mast_csid = db_query('SELECT csid FROM {oauth_common_provider_consumer} WHERE context = :context', array(':context' => 'mast'))->fetchField()) {
        $preserved[] = $mast_csid;
      }
    }

    return $preserved;
  }

}

