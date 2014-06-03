<?php

/**
 * @file
 * Contains GardensDuplicationScrubJanrainHandler.
 */

/**
 * Scrubs the Janrain configuration.
 *
 * Scrub the Janrain-related variables, so the new site doesn't try reusing the
 * same Janrain app.
 */
class GardensDuplicationScrubJanrainHandler extends \Acquia\Acsf\AcsfEventHandler {

  /**
   * Implements \Acquia\Acsf\AcsfEventHandler::handle().
   */
  public function handle() {
    drush_print(dt('Entered @class', array('@class' => get_class($this))));

    $options = $this->event->context['scrub_options'];

    if ($options['retain_janrain_settings']) {
      return;
    }

    db_delete('variable')->condition('name', 'rpx_%', 'LIKE')->execute();
  }

}

