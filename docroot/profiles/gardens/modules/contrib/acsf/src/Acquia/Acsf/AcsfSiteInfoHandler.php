<?php

namespace Acquia\Acsf;

/**
 * @file
 * This event handler is responsible for populating the site information after
 * the installation process.
 */

class AcsfSiteInfoHandler extends AcsfEventHandler {

  /**
   * Implements AcsfEventHandler::handle().
   */
  public function handle() {
    $site = acsf_get_acsf_site();
    $site->refresh();

    // Notify the user to verify their email address.
    $site->verification_status["last_updated"] = time();

    $site->save();
  }

}
