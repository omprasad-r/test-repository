<?php

/**
 * @file
 * Contains AcsfDuplicationScrubInitializeHandler.
 */

/**
 * Handles final operations for the scrub.
 */
class AcsfDuplicationScrubInitializeHandler extends AcsfEventHandler {

  /**
   * Implements AcsfEventHandler::handle().
   */
  public function handle() {
    drush_print(dt('Entered @class', array('@class' => get_class($this))));
    if (!$this->isComplete()) {
      $site = AcsfSite::load();
      $site->clean();
      variable_del('acsf_duplication_scrub_status');
      variable_set('site_name', $this->event->context['site_name']);
      variable_set('install_time', time());
      $this->setComplete();
    }
  }

  public function isComplete() {
    acsf_vget('acsf_site_duplication_step_initialize_complete', FALSE);
  }

  protected function setComplete() {
    acsf_vset('acsf_site_duplication_step_initialize_complete', TRUE, 'acsf_duplication_scrub');
  }

}

