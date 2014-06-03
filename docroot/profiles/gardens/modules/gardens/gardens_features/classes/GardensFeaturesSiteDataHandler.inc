<?php

/**
 * @file
 * Handle the Factory sync so that we can capture the responsive enabled variable.
 */

class GardensFeaturesSiteDataHandler extends \Acquia\Acsf\AcsfEventHandler {

  /**
   * Implements \Acquia\Acsf\AcsfEventHandler::handle().
   */
  public function handle() {
    $data = $this->event->context;
    $enabled = !empty($data['sf_site']['responsive_enabled']) ? TRUE : FALSE;
    variable_set('gardens_features_responsive_enabled', $enabled);
  }

}
