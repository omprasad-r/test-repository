<?php

/**
 * @file
 * Contains AcsfDuplicationScrubFinalizeHandler.
 */

/**
 * Handles final operations for the scrub.
 */
class AcsfDuplicationScrubFinalizeHandler extends AcsfEventHandler {

  /**
   * Implements AcsfEventHandler::handle().
   */
  public function handle() {
    drush_print(dt('Entered @class', array('@class' => get_class($this))));
    // Clear the theme field so all users will use the default theme.
    db_update('users')
      ->fields(array(
        'theme' => 0,
      ))
      ->execute();

    // Flush all caches, including in-progress multistep forms that are not
    // normally wiped during drupal_flush_all_caches().
    drupal_flush_all_caches();
    cache_clear_all('*', 'cache_form', TRUE);

    module_disable(array('acsf_duplication'), FALSE);

    // Begin the site without any watchdog records.
    if (db_table_exists('watchdog')) {
      db_delete('watchdog')->execute();
    }

    // Clean up ACSF variables.
    acsf_vdel('acsf_site_duplication_step_initialize_complete');
    acsf_vdel('acsf_duplication_enable_for_scrub');

    // Mark the entire scrubbing process as complete.
    variable_set('acsf_duplication_scrub_status', 'complete');
  }

}

