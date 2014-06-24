<?php

/**
 * @file
 * Contains GardensDuplicationScrubAcquiaLiftHandler.
 */

/**
 * Scrubs the Acquia Lift configuration.
 *
 * Scrub the Acquia Lift-related variables, so the new site doesn't pollute a
 * live experiment, and so that we don't end up with Acquia Lift campaigns
 * linked to content that was scrubbed.
 */
class GardensDuplicationScrubAcquiaLiftHandler extends \Acquia\Acsf\AcsfEventHandler {

  /**
   * Implements \Acquia\Acsf\AcsfEventHandler::handle().
   */
  public function handle() {
    drush_print(dt('Entered @class', array('@class' => get_class($this))));

    $options = $this->event->context['scrub_options'];

    // In case the site uses Acquia Lift and this is not an exact copy or the
    // config has been specified to be retained, remove any configuration here.
    if (empty($options['retain-acquia-lift-config'])) {
      $tables = array(
        'visitor_actions_actions',
        'personalize_agent',
        'personalize_campaign_goals',
        'personalize_fields_option_sets',
        'personalize_multivariate_test',
        'personalize_option_sets',
      );
      foreach ($tables as $table) {
        // $tables is fixed in code, so no need to escape the table names here.
        if (db_table_exists($table)) {
          db_query("TRUNCATE TABLE {$table}")->execute();
        }
      }
    }
    elseif (db_table_exists('personalize_agent')) {
      // If we are keeping the campaigns and keys, pause all campaigns. (status
      // 4 == PERSONALIZE_STATUS_PAUSED).
      db_query('UPDATE {personalize_agent} SET status = 4')->execute();
    }
  }

}

