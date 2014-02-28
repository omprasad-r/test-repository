<?php

/**
 * @file
 * Contains GardensDuplicationScrubConfigurationHandler.
 */

/**
 * Scrubs non-core configuration.
 */
class GardensDuplicationScrubConfigurationHandler extends AcsfEventHandler {

  /**
   * Implements AcsfEventHandler::handle().
   */
  public function handle() {
    drush_print(dt('Entered @class', array('@class' => get_class($this))));

    $options = $this->event->context['scrub_options'];

    // The Google Analytics "Web Property Id" is not a private key, but it needs
    // to be unique per website, so we delete it here. A site administrator will
    // need to get a new Id from Google and enter it into the Google Analytics
    // module settings page.
    variable_del('googleanalytics_account');
    // Enterprise needed a quick solution to stop including our GA code. That
    // quick solution has been extended to be a long term solution by now,
    // except that it happens on the gardener instead of the gardens site. To be
    // able to preserve the previous setup we did not removed variable
    // responsible for the quick approach, but we definitely do not want it to
    // be copied over during site duplication.
    variable_del('gardens_misc_include_google_analytics');

    if (!empty($options['mollom_public_key']) && !empty($options['mollom_private_key'])) {
      gardens_misc_update_mollom_keys_if_necessary($options['mollom_public_key'], $options['mollom_private_key']);
    }

    // We must disable domain 301 redirection to be able to use the new site.
    if (variable_get('domain_301_redirect_enabled', 0)) {
      variable_set('domain_301_redirect_enabled', 0);
    }

    // If for some reason a site had its canonical domain set, it should not
    // carry over to the duplicated site as it could cause unwanted redirecting.
    variable_del('gardens_misc_canonical_domain');

    // Remove the old xmlsitemap base URL.
    variable_del('xmlsitemap_base_url');
  }

}

