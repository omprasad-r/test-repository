<?php

/**
 * @file
 * This class handles ACSF variable related site installation tasks.
 */

class AcsfVariableInitInstallHandler extends AcsfEventHandler {

  /**
   * Implements AcsfEventHandler::handle().
   */
  public function handle() {
    acsf_variables_cache_refresh();
  }

}
