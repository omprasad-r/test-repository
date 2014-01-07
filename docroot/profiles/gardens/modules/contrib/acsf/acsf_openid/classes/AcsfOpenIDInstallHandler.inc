<?php

/**
 * @file
 * This class handles OpenID related site installation tasks.
 */

class AcsfOpenIDInstallHandler extends AcsfEventHandler {

  /**
   * Implements AcsfEventHandler::handle().
   */
  public function handle() {
    if (isset($this->event->context['acsf_site_install_info']['openid']) && isset($this->event->context['acsf_site_install_info']['account_mail'])) {
      $this->setAccountOpenID($this->event->context['acsf_site_install_info']['account_mail'], $this->event->context['acsf_site_install_info']['openid']);
    }
    if (!empty($this->event->context['acsf_site_install_info']['site_has_central_admin']) && isset($this->event->context['acsf_site_install_info']['admin_openid']) && isset($this->event->context['acsf_site_install_info']['admin_mail'])) {
      $this->setAccountOpenID($this->event->context['acsf_site_install_info']['admin_mail'], $this->event->context['acsf_site_install_info']['admin_openid']);
    }

    if (acsf_openid_force_openid()) {
      // When the users are required to log in via the factory then some of the
      // email notification texts need tweaking to make them consistent with the
      // setup.
      $variables = acsf_openid_setup_user_mail_variables();
      foreach ($variables as $variable => $content) {
        variable_set($variable, $content);
      }
    }
  }

  /**
   * Sets up a user for OpenID.
   */
  private function setAccountOpenID($mail, $openid) {
    // Find the account.
    $account = user_load_by_mail($mail);
    if ($account) {
      user_set_authmaps($account, array('authname_openid' => $openid));
    }
    else {
      watchdog('acsf_openid', "Could not set up OpenID for @mail because there is no account with that mail.", array('@mail' => $mail), WATCHDOG_CRITICAL);
    }
  }

}
