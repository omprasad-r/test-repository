<?php

/**
 * Handle an AcsfSite data save event.
 */

class GardensMiscAcsfSiteDataHandler extends \Acquia\Acsf\AcsfEventHandler {

  /**
   * Implements \Acquia\Acsf\AcsfEventHandler::handle().
   */
  public function handle() {
    // Acquire a lock, since multiple phone home requests may be running at once.
    if (lock_acquire('gardens_misc_gardens_client_factory_sync')) {
      // Save the embed.ly API key in the appropriate place.
      $key = $this->event->context['gardener_base']['embedly_api_key'];
      variable_set('oembedembedly_api_key', $key);

      // The site's domain may have changed, so adjust SSL settings if necessary.
      gardens_misc_adjust_secure_domain_settings_if_necessary();

      // Release the lock.
      lock_release('gardens_misc_gardens_client_factory_sync');
    }
  }

}
