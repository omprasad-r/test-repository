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
    $site->refresh($this);
    $site->getSiteInstallInfo($this);

    $this->postSiteInstallTasks();
  }

  /**
   * Executes the site's post install tasks.
   */
  public function postSiteInstallTasks() {
    // Create a role for the site maintainer.
    $maintainer_role = new stdClass();
    $maintainer_role->name = 'site maintainer';
    user_role_save($maintainer_role);
    variable_set('gardens_site_owner_role', $maintainer_role->rid);

    // Create a role for the administrator.
    $admin_role = new stdClass();
    $admin_role->name = 'administrator';
    $admin_role->weight = 2;
    user_role_save($admin_role);
    variable_set('user_admin_role', $admin_role->rid);

    // When installing a site with drush si we create one user which might be
    // either a central admin or an account for the site owner. If it's the
    // central admin account then we still have to create a user for the site
    // owner.
    $account_name = !empty($this->event->context['acsf_site_install_info']['account_name']) ? $this->event->context['acsf_site_install_info']['account_name'] : '';
    $account_mail = !empty($this->event->context['acsf_site_install_info']['account_mail']) ? $this->event->context['acsf_site_install_info']['account_mail'] : '';
    if ($account_name && $account_mail && !user_load_by_name($account_name) && !user_load_by_mail($account_mail)) {
      $site_owner_account = new stdClass();
      $site_owner_account->is_new = TRUE;
      $password = user_password();
      $edit = array(
        'name' => $this->event->context['acsf_site_install_info']['account_name'],
        'mail' => $this->event->context['acsf_site_install_info']['account_mail'],
        'pass' => array(
          'pass1' => $password,
          'pass2' => $password,
        ),
        'status' => 1,
        'roles' => array(
          $admin_role => 1,
          $maintainer_role->rid => 1,
        ),
        // Set login to non-zero to avoid e-mail verification needed error.
        'login' => 1,
        'openid' => $this->event->context['acsf_site_install_info']['openid'],
      );
      user_save($site_owner_account, $edit);
    }

    // Set the mollom keys if they are provisioned.
    if (!empty($this->event->context['acsf_site_install_info']['mollom'])) {
      variable_set('mollom_public_key', $this->event->context['acsf_site_install_info']['mollom']['public_key']);
      variable_set('mollom_private_key', $this->event->context['acsf_site_install_info']['mollom']['private_key']);
      if (module_exists('mollom')) {
        // Refresh mollom.
        _mollom_status(TRUE);
      }
    }
  }

}
