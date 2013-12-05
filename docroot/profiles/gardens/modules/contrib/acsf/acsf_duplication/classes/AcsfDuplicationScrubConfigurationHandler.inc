<?php

/**
 * @file
 * Contains AcsfDuplicationScrubConfigurationHandler.
 */

/**
 * Handles the scrubbing of Drupal configuration.
 */
class AcsfDuplicationScrubConfigurationHandler extends AcsfEventHandler {

  /**
   * Implements AcsfEventHandler::handle().
   */
  public function handle() {
    drush_print(dt('Entered @class', array('@class' => get_class($this))));
    variable_del('cron_last');
    variable_del('cron_semaphore');

    // Clear site-specific keys that will auto-initialize when first needed.
    // Initialize site-specific keys that do not auto-initialize.
    // Mollom keys are handled by install_gardens.inc instead of here, because
    // that's the code that has access to them.
    variable_del('drupal_private_key');
    variable_set('cron_key', drupal_hash_base64(drupal_random_bytes(55)));

    // The Google Analytics "Web Property Id" is not a private key, but it needs
    // to be unique per website, so we delete it here. A site administrator will
    // need to get a new Id from Google and enter it into the Google Analytics
    // module settings page.
    variable_del('googleanalytics_account');
  }

}

