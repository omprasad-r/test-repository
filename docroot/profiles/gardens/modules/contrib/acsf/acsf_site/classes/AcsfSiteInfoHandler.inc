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
    $site = AcsfSite::load();
    $site->refresh();

    // ACSF required variables.
    if ($db_role = variable_get('gardens_db_name', FALSE)) {
      variable_set("file_public_path", 'sites/g/files/' . $site->site_db . '/f');
    }
    else {
      watchdog('acsf', 'Unable to set file_public_path as gardens_db_name is not populated.', array(), WATCHDOG_CRITICAL);
    }

    // Notify the user to verify their email address.
    $site->verification_status["last_updated"] = time();

// Gardens profile required variables.
// @todo deprecate these in the gardens profile modules in favor of the AcsfSite API.
//  variable_set("gardens_client_name", $site->gardens_client_name);
//  variable_set("gardens_misc_standard_domain", $site->domains[0]);
//  variable_set("gardens_site_id", $site->site_id);
//  variable_set("acquia_gardens_gardener_url", strtr($site->gardener_url, array('http://' => 'https://')));
//  variable_set("gardens_service_name_long", $site->gardens_service_name_long);
//  variable_set("gardens_features_responsive_enabled", TRUE);
//  variable_set("gardens_client_site_verification_status", $verification_status);

    $site->save();
  }

}
