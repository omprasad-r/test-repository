<?php

/**
 * @file
 * Handle the Factory sync data so that we can still use the old
 * gardens_client_get_data() function.
 */

class GardensClientSiteDataHandler extends AcsfEventHandler {

  /**
   * Implements AcsfEventHandler::handle().
   */
  public function handle() {
    $data = $this->event->context;
    variable_set('gardens_client_gardener_data', $data);
  }

}
