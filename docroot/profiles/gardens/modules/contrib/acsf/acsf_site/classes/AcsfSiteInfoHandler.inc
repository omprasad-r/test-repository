<?php

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
    $nid = $this->event->context['nid'];
    $site = new AcsfSite($nid);
    $site->refresh();
  }

}
