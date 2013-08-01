<?php

/**
 * @defgroup gardens_signup_site_statuses Gardens signup site statuses.
 * @{
 * Constants representing the status of a Gardens site, from provisioning
 * through full installation. These must remain synchronized with the
 * equivalent constants defined in gardens_signup.inc in the Gardener codebase.
 */

/**
 * Status constant indicating that something went wrong while creating a
 * Gardens site.
 */
define('GARDENS_SIGNUP_SITE_STATUS_FAILED', 'failed');

/**
 * Status constant indicating that the site should be considered deleted from
 * the system.
 */
define('GARDENS_SIGNUP_SITE_STATUS_DELETED', 'deleted');

/**
 * Status constant indicating that a Gardens site has been provisioned but not
 * yet installed on Acquia Hosting.
 */
define('GARDENS_SIGNUP_SITE_STATUS_PROVISIONED', 'provisioned');

/**
 * Status constant indicating that a Gardens site is in the process of being
 * installed on Acquia Hosting.
 */
define('GARDENS_SIGNUP_SITE_STATUS_INSTALLING', 'installing');

/**
 * Status constant indicating that a Gardens site has been installed, but not
 * yet assigned to a particular customer.
 * This status was deprecated as part of DG-8020.
 */
define('GARDENS_SIGNUP_SITE_STATUS_INSTALLED', 'installed');

/**
 * Status constant indicating that a Gardens site is in the process of being
 * duplicated from another site.
 */
define('GARDENS_SIGNUP_SITE_STATUS_DUPLICATING', 'duplicating');

/**
 * Status constant indicating that a Gardens site has been reserved for a
 * particular customer, but not yet assigned to them on Acquia Hosting.
 */
define('GARDENS_SIGNUP_SITE_STATUS_RESERVED', 'reserved');

/**
 * Status constant indicating that a Gardens site is in the process of being
 * assigned to a particular customer on Acquia Hosting.
 */
define('GARDENS_SIGNUP_SITE_STATUS_ASSIGNING', 'assigning');

/**
 * Status constant indicating that a Gardens site is completely operational and
 * belongs to a particular customer.
 */
define('GARDENS_SIGNUP_SITE_STATUS_COMPLETED', 'completed');

/**
 * @} End of "defgroup gardens_signup_site_statuses".
 */

/**
 * @defgroup gardens_signup_site_operations Gardens signup site operations.
 * @{
 * Constants representing operations that can be performed on a Gardens site.
 * These must remain synchronized with the equivalent constants defined in
 * gardens_signup.inc in the Gardener codebase.
 */

/**
 * Operation constant indicating that a Gardens site should be installed but
 * not configured for a particular customer.
 * This operation was deprecated as part of DG-8020.
 */
define('GARDENS_SIGNUP_SITE_OPERATION_INSTALL', 'install');

/**
 * Operation constant indicating that a Gardens site should be installed and
 * configured for a particular customer at the same time.
 */
define('GARDENS_SIGNUP_SITE_OPERATION_INSTALL_AND_CONFIGURE', 'install and configure');

/**
 * Operation constant indicating that a pre-installed Gardens site should be
 * configured for a particular customer.
 */
define('GARDENS_SIGNUP_SITE_OPERATION_CONFIGURE', 'configure');

/**
 * Operation constant indicating that a Gardens site should be duplicated from
 * another site.
 */
define('GARDENS_SIGNUP_SITE_OPERATION_DUPLICATE', 'duplicate');

/**
 * @} End of "defgroup gardens_signup_site_operations".
 */

require_once('library/acquia_gardens_xmlrpc.inc');

/**
 * Initialize a Hosting site to be ready to install Gardens sites.
 *
 * This is expected to run when a new Hosting site is provisioned to serve as
 * a multisite installation of Gardens, or when a new web node comes up for an
 * existing Hosting site of this kind.
 *
 * The function ensures that a directory is in place on the shared filesystem
 * in which this Hosting site's Gardens site directories can be stored (these
 * are the directories in which the settings.php file for each site lives). It
 * then creates symlinks on the web node's local disk for any Gardens sites
 * that might already be assigned to it, pointing to the appropriate location
 * within that directory; for example, a typical symlink might point from
 * sites/mysite.drupalgardens.com (in the local docroot) to something like
 * gardens-sites/g12446 (on the shared filesystem), thereby tying that domain
 * to that particular Gardens site.
 *
 * Throws an exception on error.
 *
 * @param $hosting_site_name
 *   The machine-readable name (from Acquia Hosting) of the Hosting site to
 *   initialize for Gardens.
 * @param $hosting_site_environment
 *   The machine-readable environment (from Acquia Hosting) of the
 *   Hosting site to initialize for Gardens.
 * @param $update_settings
 *   If TRUE, all settings.php files will be re-written.
 * @param $performance_logging
 *   If TRUE, extra performance data will be logged to syslog() while this
 *   Hosting site is being initialized.
 */
function install_gardens_initialize($hosting_site_name, $hosting_site_environment, $update_settings = FALSE, $performance_logging = FALSE) {
  return install_gardens_initialize_push($hosting_site_name, $hosting_site_environment, $update_settings, $performance_logging);
}

function install_gardens_initialize_push($hosting_site_name, $hosting_site_environment, $update_settings = FALSE, $performance_logging = FALSE) {
  if ($performance_logging) {
    $microtime = microtime(TRUE);
    syslog(LOG_INFO, "install_gardens_initialize (performance log) $hosting_site_name $hosting_site_environment $microtime: Starting to initialize Gardens sites");
  }
  $sites_directory = acquia_gardens_sites_directory($hosting_site_name, $hosting_site_environment);
  acquia_gardens_ensure_directory($sites_directory);

  if ($update_settings) {
    // Retrieve a list of currently active domains.
    $site_domains = acquia_gardens_call('get.active.domains.by.site', array($hosting_site_name, $hosting_site_environment));
    foreach ($site_domains as $gardens_site_id => $domains) {
      $site_directory = acquia_gardens_site_directory($hosting_site_name, $hosting_site_environment, $gardens_site_id);
      acquia_gardens_ensure_settings_file($site_directory, $hosting_site_name, $hosting_site_environment, $gardens_site_id);
    }
  }

  // AN-15310 only run this check once in about 20k times to minimize load on gluster.
  // This script runs once per 5 seconds normally.
  $num = mt_rand(0, 1000000);

  // We want this to happen about 8 times per 24 hours.
  if ($num <= 400) {
    acquia_gardens_full_domain_sync(NULL, $hosting_site_environment);
  }
}

function acquia_gardens_full_domain_sync($cleanup_deleted = TRUE, $hosting_site_environment = NULL) {
  $script_dir = dirname(__FILE__);
  if (empty($hosting_site_environment)) {
    try {
      $hosting_site_environment = acquia_gardens_get_site_environment();
    }
    catch (Exception $e) {
      syslog(LOG_ERR, 'Failed to get hosting site environment in acquia_gardens_full_domain_sync');
      exit();
    }
  }
  exec(sprintf('cd %s/docroot && AH_SITE_ENVIRONMENT=%s ../drush/drush --include=../hosting-drush domain-sync --cleanup-deleted', $script_dir, $hosting_site_environment), $out, $ret);
  syslog(LOG_INFO, "Ran sync command with output: " . implode("\n", $out));
  if ($ret != 0) {
    throw new Exception ("Unable to run domain sync command");
  }
}

/**
 * Helper function to remove deleted domains for Gardens sites.
 *
 * Deletion is only performed if the domain is still actually pointing to its
 * associated Gardens site ID in the filesystem.
 *
 * @param $deleted_domains
 *   An array whose keys are the Gardens site IDs with domains to delete, and
 *   whose values are arrays of domains that should be deleted.
 * @param $hosting_site_name
 *   The name of the Hosting site which these Gardens sites belong to.
 */
function acquia_gardens_remove_site_domains($deleted_domains, $hosting_site_name) {
  foreach ($deleted_domains as $gardens_site_id => $domains) {
    $site_directory = acquia_gardens_site_directory_legacy($hosting_site_name, $gardens_site_id);
    foreach ($domains as $domain) {
      $site_directory_symlink = acquia_gardens_site_directory_symlink($domain);
      if (file_exists($site_directory_symlink) && is_link($site_directory_symlink)) {
        $target = readlink($site_directory_symlink);
        if ($target && $target == $site_directory) {
          unlink($site_directory_symlink);
          syslog(LOG_INFO, "Hosting site $hosting_site_name removed symlink for deleted domain $site_directory_symlink");
        }
      }
    }
  }
}

/**
 * Install or configure an Acquia Gardens site on the current server by
 * contacting the Gardener and obtaining information about a site and the
 * operation that it needs performed on it (if there is one) and then
 * contacting the Gardener again to update the site's status.
 *
 * Only one site that currently needs an operation performed is dealt with by
 * this function (the site is chosen at random by the Gardener), to avoid the
 * potential for Drupal caching problems. Subsequent calls to the function (in
 * different PHP threads) will pick up the remaining sites, if there are any.
 *
 * This function logs appropriate status messages to syslog as it proceeds, and
 * throws an exception on error.
 *
 * @param $hosting_site_name
 *   The machine-readable name (from Acquia Hosting) of the Hosting site whose
 *   Gardens sites will be installed or configured.
 * @param $hosting_site_environment
 *   The machine-readable name (from Acquia Hosting) of the Hosting
 *   environment whose Gardens sites will be installed or configured.
 * @param $data
 *   Base64 encoded serialized data array.
 *
 * @throws Exception
 */
function install_gardens($hosting_site_name, $hosting_site_environment, $data = NULL) {
  if (empty($parent_hosting_site_name)) {
    $parent_hosting_site_name = $hosting_site_name;
  }
  // Prepare a Gardens site to process.
  $gardens_site_info = unserialize(base64_decode($data));
  if (empty($gardens_site_info)) {
    // No site to process, so nothing to do here.
    return;
  }

  try {
    $current_directory = dirname(__FILE__);
    $gardens_site_id = $gardens_site_info['site_id'];
    $op = $gardens_site_info['operation'];
    $domains = $gardens_site_info['domains'];
    $site_directory = acquia_gardens_site_directory($hosting_site_name, $hosting_site_environment, $gardens_site_id);
    // Store the install operation in a global variable so the Drupal install
    // code can find out about it if necessary.
    $GLOBALS['gardens_install_op'] = $op;

    syslog(LOG_NOTICE, "Attempting to perform the operation \"$op\" for Gardens site $gardens_site_id on Hosting site $hosting_site_name.");
    // Bail out in the event that the Gardener thinks we should do something
    // for this site but we have no idea how to do it.
    $expected_operations = array(
      GARDENS_SIGNUP_SITE_OPERATION_INSTALL_AND_CONFIGURE,
      GARDENS_SIGNUP_SITE_OPERATION_DUPLICATE,
    );
    if (!in_array($op, $expected_operations)) {
      throw new Exception("GardensError: AN-22794 - A Gardens site requested an invalid operation and could not be processed GardensError: site id: $gardens_site_id; hosting account: $hosting_site_name; original site status \"{$gardens_site_info['original_status']}\"; requested operation was \"$op\").");
    }

    if ($op == GARDENS_SIGNUP_SITE_OPERATION_INSTALL_AND_CONFIGURE) {
      try {
      // Prepare the installation directories and files.
      acquia_gardens_prepare_for_installation($site_directory, $hosting_site_name, $hosting_site_environment, $gardens_site_id, $domains, FALSE, $gardens_site_info['nid']);

      // Store the PID of this installer process so that gardener can check it
      // later if needed.
      file_put_contents("$site_directory/installer.pid", getmypid());

      // Install Drupal with the appropriate installation settings for the
      // Gardens site.
      $user_password1 = _acquia_gardens_user_password();
      $user_password2 = _acquia_gardens_user_password();
      $settings = array(
        'parameters' => array(
          'profile' => 'gardens',
          'locale' => 'en',
        ),
        'server' => array(
          // If the site is being pre-installed it doesn't have a public
          // facing URL yet, but a fake url from the gardener allows us to
          // install.
          'url' => $gardens_site_info['url'] . '/install.php',
        ),
        'forms' => array(
          'install_configure_form' => array(
            'site_name' => $gardens_site_info['site_name'],
            'site_mail' => $gardens_site_info['site_mail'],
            'account' => array(
              'name' => $gardens_site_info['admin_name'],
              'mail' => $gardens_site_info['admin_mail'],
              'pass' => array(
                'pass1' => $user_password1,
                'pass2' => $user_password1,
              ),
            ),
            'openid' => $gardens_site_info['admin_openid'],
            'owner_account' => array(
              'openid' => $gardens_site_info['openid'],
              'account' => array(
                'name' => $gardens_site_info['account_name'],
                'mail' => $gardens_site_info['account_mail'],
                'pass' => array(
                  'pass1' => $user_password2,
                  'pass2' => $user_password2,
                ),
              ),
            ),
            // Do not enable the update status module, since we don't need it
            // for Gardens - and if drupal.org is ever unavailable, the failed
            // attempt to connect might slow down the Gardens site creation
            // considerably. Finally, we want to make sure that Gardens users
            // don't get any of the "update manager" (a.k.a. plugin manager)
            // links which this module provides.
            'update_status_module' => array(1 => NULL),
            'clean_url' => TRUE,
            // Make sure the gardener URL is always https.
            'acquia_gardens_gardener_url' => strtr($gardens_site_info['gardener_url'], array('http://' => 'https://')),
            'gardens_service_name_long' => $gardens_site_info['gardens_service_name_long'],
            'site_template' => $gardens_site_info['template'],
            'features' => !empty($gardens_site_info['features']) ? implode(',', $gardens_site_info['features']) : '',
            'mollom_public_key' => !empty($gardens_site_info['mollom']['public_key']) ? $gardens_site_info['mollom']['public_key'] : '',
            'mollom_private_key' => !empty($gardens_site_info['mollom']['private_key']) ? $gardens_site_info['mollom']['private_key'] : '',
            // We always want the themebuilder screenshot service turned on
            // when we are installing in a Hosting environment.
            'install_themebuilder_screenshot_keys' => TRUE,
            // Allow local user accounts if the method was set to 'local'.
            'acquia_gardens_local_user_accounts' => ($gardens_site_info['account_method'] == 'local'),
            'gardens_client_name' => $gardens_site_info['gardens_client_name'],
          ),
        ),
      );

      chdir("$current_directory/docroot");
      define('DRUPAL_ROOT', getcwd());
      define('MAINTENANCE_MODE', 'install');
      include_once('docroot/includes/install.core.inc');
      install_drupal($settings);

      // Ensure that cron does not get to run untill we are finished. This is
      // similar to lock_acquire() except that it is not using _lock_id as
      // that would set an exit function to kill the semaphore.
      $lock_id = uniqid(mt_rand(), TRUE);
      db_merge('semaphore')
        ->key(array('name' => 'cron'))
        ->fields(array(
          'value' => $lock_id,
          'expire' => microtime(TRUE) + 3600.0,
        ))->execute();

      // Have to make the directory writable again or the touch fails.
      chmod($site_directory, 0755);
      touch("$site_directory/installed");

      // Save the site's original (standard *.drupalgardens.com) domain.
      $standard_domain = reset($domains);
      drupal_override_server_variables(array('url' => $gardens_site_info['url'] . '/index.php'));

      variable_set('gardens_misc_standard_domain', $standard_domain);
      // Normally we only have one domain during configure, so just set the last value.
      // @see: AN-19653  this is a quick fix for imagecache problems.
      $domain = end($domains);
      variable_set('file_public_path', "sites/{$domain}/files");
      variable_set('site_name', $gardens_site_info['site_name']);
      variable_set('site_mail', $gardens_site_info['site_mail']);

      if ($gardens_site_info['account_method'] == 'local') {
        scarecrow_allow_local_user_logins();
      }

      // The site owner is the last user account created. We'll replace that
      // one with the provided information about the person who will actually
      // own the site.
      $uid = db_query("SELECT MAX(uid) FROM {users}")->fetchField();
      $account = user_load($uid);
      variable_set('acquia_gardens_site_owner', $uid);
      $edit['name'] = $gardens_site_info['account_name'];
      $edit['mail'] = $gardens_site_info['account_mail'];
      $edit['init'] = $gardens_site_info['account_mail'];

      // Let the Gardens site know whether to nag the site owner about
      // verifying their email address.
      $verification_status = $gardens_site_info['email_verified'];
      $verification_status['last_updated'] = time();
      variable_set('gardens_client_site_verification_status', $verification_status);

      // Enable the chosen site template and finalize the template selection
      // (unless the site already has a template installed).
      if (!variable_get('site_template_current_template') && !empty($gardens_site_info['template']) && in_array($gardens_site_info['template'], array_keys(site_template_get_all_templates()))) {
        $template = $gardens_site_info['template'];
        if (!empty($gardens_site_info['features'])) {
          // Special string to indicate an empty list of features should be
          // passed in to the site template.
          if ($gardens_site_info['features'] == array('NONE')) {
            $features = array();
          }
          else {
            $features = $gardens_site_info['features'];
          }
        }
        else {
          $features = NULL;
        }
        site_template_install_features($template, $features);
      }
      else {
        // Do any "reverse scrubbing" necessary in the case where this site was
        // preinstalled with a site template before we had all the personalized
        // information that was needed to make it work.
        if (db_table_exists('contact')) {
          db_update('contact')->fields(array('recipients' => variable_get('site_mail')))->execute();
        }
      }
      site_template_finalize_template_selection();

      // Reset the OpenID record to point to the new Gardener account that owns
      // this site.
      user_set_authmaps($account, array('authname_openid' => ''));
      gardens_client_add_authmaps($account, array('authname_openid' => $gardens_site_info['openid']));

      // If the gardener passed an enterprise client name, we switch to and install
      // a profile of the same name, if it exists.  All custom configurations for
      // the client and  additional modules can be handled from there.
      $profile_name = empty($gardens_site_info['gardens_client_name']) ? '' : $gardens_site_info['gardens_client_name'];
      if ($profile_name && file_exists(DRUPAL_ROOT . "/profiles/{$profile_name}/{$profile_name}.info")) {
        $profile_name = $gardens_site_info['gardens_client_name'];
        // Save the value for future use.
        variable_set('gardens_client_name', $profile_name);
        $previous_profile = variable_get('install_profile', 'gardens');
        // First set the profile to make sure the profile "module" is found when we ...
        variable_set('install_profile', $profile_name);
        // ... rebuild module data (making sure it's not cached).
        system_list_reset();
        system_rebuild_module_data();
        // If the profile "module" is not found, reset to the previous profile.
        if (db_query("SELECT 1 FROM {system} WHERE type = 'module' AND name = :name", array(':name' => $profile_name))->fetchField()) {
          // Pass TRUE to module_enable() so that dependent modules are also enabled.
          // Normally, for performance, we would list dependencies here explicitly and
          // pass FALSE, but in this case, we've already incurred the penalty of rebuilding
          // the module data, so can allow module_enable() to benefit from that.
          module_enable(array($profile_name), TRUE);
        }
        else {
          // If the profile "module" is not found, reset to the previous profile. This
          // should not happen.
          variable_set('install_profile', $previous_profile);
          system_list_reset();
          system_rebuild_module_data();
        }
      }

      db_delete('semaphore')->condition('value', $lock_id)->execute();

      }
      catch (Exception $e) {
        // Configuring a site happens live (while the end user is waiting), so
        // we always want to receive alerts when something goes wrong and it
        // needs to be tried again. We therefore re-throw any regular exception
        // (e.g., those triggered by Drupal) as an exception that triggers
        // Gardens alerts. Some excpetions are expected temporarily during site
        // installation, so in those cases the exception should be silent to
        // avoid flooding with alerts.
        $silent_exception = FALSE;
        if (strpos($e->getMessage(), 'This message is expected') !== NULL) {
          $file_change_time = is_dir($site_directory) ? filectime($site_directory) : 0;
          // Silent exception period - tentative 5 minutes.
          $silent_exception = $file_change_time + 300 >= time();
        }
        if (strpos($e->getMessage(), 'Gardens') === 0 || $silent_exception) {
          // The Exception is already a GardensError or GardensWarning.
          syslog(LOG_ERR, 'Gardens Site Install Error: ' . $e->getMessage());
          throw $e;
        }
        else {
          // Turn the message into a GardensError.
          syslog(LOG_ERR, 'Gardens Site Install Error: ' . $e->getMessage());
          throw new Exception("GardensError: AN-22470 - Error occurred during site installation and configuration (install_gardens function) GardensError: " . $e->getMessage() . $e->getTraceAsString());
        }
      }
    }

    if ($op == GARDENS_SIGNUP_SITE_OPERATION_DUPLICATE) {
      $start_time = time();
      // Create the site directory on the shared filesystem.
      acquia_gardens_ensure_directory($site_directory);
      // Create symlinks in the docroot that point to the site directory.
      if (!empty($domains)) {
        foreach ($domains as $domain) {
          acquia_gardens_ensure_symlink($site_directory, acquia_gardens_site_directory_symlink($domain));
        }
      }
      // Create a settings.php file.
      acquia_gardens_ensure_settings_file($site_directory, $hosting_site_name, $hosting_site_environment, $gardens_site_id, TRUE);
      // Copy the files from the parent site.
      chdir("$current_directory/docroot");
      $site_domain = reset($domains);
      $source_site = escapeshellarg($gardens_site_info['parent_site_domain']);
      $dest_site = escapeshellarg($site_domain);
      // Define the timeout for Drush commands. We can set a different timeout
      // by putting a specially-named file in the destination site's site
      // directory.
      $timeout = @file_get_contents("sites/{$site_domain}/site.duplication.drush.command.timeout.txt");
      if (empty($timeout) || (!$timeout = trim($timeout)) || !is_numeric($timeout)) {
        $timeout = 3600;
      }
      acquia_gardens_drush_shell_exec("rsync --exclude-conf sites/$source_site/ sites/$dest_site", $timeout);
      // Copy the database from the parent site.
      acquia_gardens_drush_shell_exec("sql-sync --structure-tables-list=cache,cache_bootstrap,cache_form,cache_menu,cache_page $source_site $dest_site", $timeout);
      // Bootstrap Drupal and give the new site its own identity.
      define('DRUPAL_ROOT', getcwd());
      require_once DRUPAL_ROOT . '/includes/bootstrap.inc';
      drupal_override_server_variables(array('url' => $gardens_site_info['url'] . '/index.php'));
      drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
      variable_set('site_name', $gardens_site_info['site_name']);
      $mollom_public_key = !empty($gardens_site_info['mollom']['public_key']) ? $gardens_site_info['mollom']['public_key'] : '';
      $mollom_private_key = !empty($gardens_site_info['mollom']['private_key']) ? $gardens_site_info['mollom']['private_key'] : '';
      gardens_misc_update_mollom_keys_if_necessary($mollom_public_key, $mollom_private_key);
      // Ensure Drupal filesystem related configuration variables are correct
      // for the new site. Consider the following variables:
      // - file_directory_path
      // - file_directory_temp
      // - file_public_path
      // - file_private_path
      // - file_temporary_path
      // Given the AH environment for Gardens, we want to leave the temp paths
      // alone, and we want to delete the other variables, to ensure they reset
      // to their defaults (because of scarecrow, these shouldn't exist in the
      // {variable} table anyway).
      foreach (array('file_directory_path', 'file_public_path', 'file_private_path') as $var) {
        variable_del($var);
      }
      // Save the site's original (standard *.drupalgardens.com) domain.
      $standard_domain = reset($domains);
      variable_set('gardens_misc_standard_domain', $standard_domain);
      // Normally we only have one domain during configure, so just set it.
      // @see: AN-19653  this is a quick fix for imagecache problems.
      $domain = end($domains);
      variable_set('file_public_path', "sites/{$domain}/files");
      module_enable(array('gardens_duplication'), FALSE);
      $scrub_options = array();
      $scrub_options['retain_janrain_settings'] = $gardens_site_info['exact_copy'] || $gardens_site_info['retain_janrain_settings'];
      if ($gardens_site_info['exact_copy']) {
        $scrub_options['retain_users_and_content'] = TRUE;
        $scrub_options['retain_webform_submissions'] = TRUE;
        $scrub_options['retain_voting_api_data'] = TRUE;
      }
      gardens_duplication_scrub($scrub_options);
      module_disable(array('gardens_duplication'), FALSE);
      // Before mailing site administrators and marking the site duplication
      // complete, make sure at least a fair amount of time has passed. This is
      // for the following reason: When we duplicate a site, it is newly
      // created by the Gardener, not based on a preinstalled site. Therefore,
      // all Hosting web nodes in the tangle must create a database connection
      // include file for it in "real time" before the site is ready to be
      // used. When we call acquia_gardens_ensure_settings_file() above to
      // create the settings.php file for the site, that ensures that the
      // database connection include file (/var/www/site-php/...[file]) already
      // exists before we proceed; however, that only guarantees that the file
      // exists on *this* web node (the one performing the duplication), not
      // all web nodes. So, we need to give a reasonable amount of time for the
      // other web nodes to catch up. Since the Hosting scripts run once per
      // minute on cron, we can base the amount of time we wait on that.
      $end_time = time();
      $time_elapsed = $end_time - $start_time;
      $min_duplication_time = 120; // A couple minutes for now.
      if ($time_elapsed < $min_duplication_time) {
        sleep($min_duplication_time - $time_elapsed);
      }
      gardens_misc_mail_site_admins($gardens_site_info['site_duplication_finished_message']);
    }
  }
  catch (Exception $e) {
    // If something went wrong, ensure that the site is properly "unclaimed"
    // (from the Gardener) before throwing the exception, so that the next PHP
    // process will be able to try the requested operation again.
    acquia_gardens_call('set.install.status', array($gardens_site_info['nid'], $gardens_site_info['original_status']));
    $memory_used = acquia_gardens_peak_memory_usage();
    throw new Exception("Error performing the operation \"$op\" for Gardens site $gardens_site_id on Hosting site $hosting_site_name: " . $e->getMessage() . " ($memory_used)");
  }

  // If we got here, the overall operation was a success.
  acquia_gardens_call('set.install.status', array($gardens_site_info['nid'], $gardens_site_info['desired_status'], TRUE));
  $memory_used = acquia_gardens_peak_memory_usage();
  syslog(LOG_NOTICE, "Successfully performed the operation \"$op\" for Gardens site $gardens_site_id on Hosting site $hosting_site_name. ($memory_used)");
  // For any operation except install, populate data from gardener.
  // This should ALSO happen via task from the gardener.
  if (!gardens_client_phone_home()) {
    syslog(LOG_ERR, 'GardensError: Failed to retrieve Gardener data in install_gardens() for site: '. $gardens_site_id);
  }
}

/**
 * Set up the site directory, symlinks, and settings.php for a duplicated site.
 *
 * @param string $hosting_site_name
 *   The hosting site name ('tangle001').
 * @param string $hosting_site_environment
 *   The hosting site environment ('prod').
 * @param string $gardens_site_id
 *   The unique gardens site id ('mysite').
 * @param string $domains
 *   A json-encoded array of domains to be added to the site.
 * @param int $nid
 *   (Optional) An explicit nid which will override any derived nid.
 *
 * @throws Exception
 */
function acquia_gardens_set_up_directory_for_duplication($hosting_site_name, $hosting_site_environment, $gardens_site_id, $domains, $nid = NULL) {
  foreach (array('hosting_site_name', 'hosting_site_environment', 'gardens_site_id', 'domains', 'nid') as $arg) {
    if (isset($$arg)) {
      $$arg = json_decode($$arg);
    }
  }
  $site_directory = acquia_gardens_site_directory($hosting_site_name, $hosting_site_environment, $gardens_site_id);
  acquia_gardens_prepare_for_installation($site_directory, $hosting_site_name, $hosting_site_environment, $gardens_site_id, $domains, TRUE, $nid);
}

/**
 * Configure a duplicated site.
 *
 * @param string $gardens_site_info
 *   A JSON-encoded array of site information.
 */
function acquia_gardens_configure_duplicated_site($gardens_site_info) {
  $gardens_site_info = json_decode($gardens_site_info);
  define('DRUPAL_ROOT', dirname(__FILE__) . '/docroot');
  require_once DRUPAL_ROOT . '/includes/bootstrap.inc';
  drupal_override_server_variables(array('url' => $gardens_site_info->url . '/index.php'));
  drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  variable_del('gardens_site_duplication_complete');
  variable_set('site_name', $gardens_site_info->site_name);
  $mollom_public_key = !empty($gardens_site_info->mollom->public_key) ? $gardens_site_info->mollom->public_key : '';
  $mollom_private_key = !empty($gardens_site_info->mollom->private_key) ? $gardens_site_info->mollom->private_key : '';
  gardens_misc_update_mollom_keys_if_necessary($mollom_public_key, $mollom_private_key);
  // Ensure Drupal filesystem related configuration variables are correct
  // for the new site. Consider the following variables:
  // - file_directory_path
  // - file_directory_temp
  // - file_public_path
  // - file_private_path
  // - file_temporary_path
  // Given the AH environment for Gardens, we want to leave the temp paths
  // alone, and we want to delete the other variables, to ensure they reset
  // to their defaults (because of scarecrow, these shouldn't exist in the
  // {variable} table anyway).
  foreach (array('file_directory_path', 'file_public_path', 'file_private_path') as $var) {
    variable_del($var);
  }
  // Save the site's original (standard *.drupalgardens.com) domain.
  $domains = $gardens_site_info->domains;
  $standard_domain = reset($domains);
  variable_set('gardens_misc_standard_domain', $standard_domain);
  // Normally we only have one domain during configure, so just set it.
  // @see: AN-19653  this is a quick fix for imagecache problems.
  $domain = end($domains);
  variable_set('file_public_path', "sites/{$domain}/files");
  module_enable(array('gardens_duplication'), FALSE);
  $scrub_options = array();
  $scrub_options['retain_janrain_settings'] = $gardens_site_info->exact_copy || $gardens_site_info->retain_janrain_settings;
  if ($gardens_site_info->exact_copy) {
    $scrub_options['retain_users_and_content'] = TRUE;
    $scrub_options['retain_webform_submissions'] = TRUE;
    $scrub_options['retain_voting_api_data'] = TRUE;
  }
  gardens_duplication_scrub($scrub_options);
  module_disable(array('gardens_duplication'), FALSE);
  variable_set('gardens_site_duplication_complete', 'complete');
}

/**
 * Prepare a Gardens site for installation on the filesystem.
 *
 * Creates directories, settings files, etc., necessary for a Gardens site to
 * be installed in a multisite setup. Throws an exception on failure.
 *
 * @param $site_directory
 *   The site directory where the site will live on the shared filesystem.
 *   Usually obtained by calling acquia_gardens_site_directory().
 * @param $hosting_site_name
 *   The hosting site (tangle) name where the Gardens site will be installed.
 * @param $hosting_site_environment
 *   The hosting environment where the Gardens site will be installed.
 * @param $gardens_site_id
 *   The Gardens site ID that will be installed.
 * @param $domains
 *   An array of domain names at which this site will be accessed.
 * @param $readonly
 *   Whether to make the settings file read-only. Should be set to FALSE for
 *   site installation, and TRUE for site duplication. Defaults to FALSE.
 * @param $nid
 *   (Optional). The site nid can be passed explicitly when install is being
 *   driven from the gardener.  This will override any nid derived from the site
 *   ID.
 *
 * @throws Exception
 */
function acquia_gardens_prepare_for_installation($site_directory, $hosting_site_name, $hosting_site_environment, $gardens_site_id, $domains, $readonly = FALSE, $nid = NULL) {
  // Create the site directory on the shared filesystem.
  acquia_gardens_ensure_directory($site_directory);

  // Create a directory for this site's themebuilder themes.
  acquia_gardens_ensure_directory("$site_directory/themes");
  acquia_gardens_ensure_directory("$site_directory/themes/mythemes");

  // Create symlinks in the docroot that point to the site directory.
  if (!empty($domains)) {
    foreach ($domains as $domain) {
      acquia_gardens_ensure_symlink($site_directory, acquia_gardens_site_directory_symlink($domain));
    }
  }

  // Create a settings.php file.
  acquia_gardens_ensure_settings_file($site_directory, $hosting_site_name, $hosting_site_environment, $gardens_site_id, $readonly, $nid);
  if (!$readonly) {
    // The installer wants the settings file to be writable.
    $settings_file = "$site_directory/settings.php";
    if (acquia_gardens_fileperms($settings_file) != '0600' && !chmod($settings_file, 0600)) {
      throw new Exception("Could not change permissions of the file $settings_file.");
    }
  }
}

/**
 * Update the contents of a file if the contents have changed.
 *
 * @param $path
 *   The file path to update.
 * @param $content
 *   The new content for the file.
 * @return
 *   TRUE if the file was updated, FALSE if it already contained
 *   $content.
 */
function acquia_gardens_update_file($path, $content) {
  $current = file_exists($path) ? file_get_contents($path) : '';
  if ($current === FALSE) {
    throw new Exception("Unable to get contents of file at $path");
  }
  if ($content != $current) {
    if (file_exists($path) && !is_writable($path)) {
      $file_info = lstat($path);
      $mode = $file_info['mode'];
      @chmod($path, $mode | 0664);
    }
    $result = file_put_contents($path, $content);
    if (!empty($mode)) {
      @chmod($path, $mode);
    }
    if ($result === FALSE) {
      throw new Exception("Unable to write to file $path");
    }
    return TRUE;
  }
  return FALSE;
}


/**
 * Returns the directory on the shared filesystem in which a Hosting site will
 * store the sites directories of its Gardens sites.
 */
function acquia_gardens_sites_directory($hosting_site_name, $hosting_site_environment) {
  return "/mnt/files/{$hosting_site_name}.{$hosting_site_environment}/gardens-sites";
}

/**
 * Returns the directory on the shared filesystem that contains the data (e.g.,
 * settings.php file) for an individual Gardens site, belonging to a particular
 * Hosting site.
 */
function acquia_gardens_site_directory($hosting_site_name, $hosting_site_environment, $gardens_site_id) {
  $sites_directory = acquia_gardens_sites_directory($hosting_site_name, $hosting_site_environment);
  return "$sites_directory/$gardens_site_id";
}

/**
 * Legacy version of acquia_gardens_sites_directory that does not use the environment name.
 */
function acquia_gardens_sites_directory_legacy($hosting_site_name) {
  return "/mnt/files/{$hosting_site_name}/gardens-sites";
}

/**
 * Legacy version of acquia_gardens_site_directory that does not use the environment name.
 */
function acquia_gardens_site_directory_legacy($hosting_site_name, $gardens_site_id) {
  $sites_directory = acquia_gardens_sites_directory_legacy($hosting_site_name);
  return "$sites_directory/$gardens_site_id";
}

/**
 * Returns the path (on local disk) for a symlink associated with a particular
 * Gardens site domain. This symlink will typically point to the result of
 * acquia_gardens_site_directory(), which lives on the shared filesystem.
 */
function acquia_gardens_site_directory_symlink($domain) {
  $current_directory = dirname(__FILE__);
  return "$current_directory/docroot/sites/$domain";
}

/**
 * Ensure that a directory exists on the shared Gardens filesystem.
 *
 * The directory will be owned by tangleN:tangleN and be
 * mode 755, making it writable by Gardens PHP and readable by Apache.
 *
 * @param $directory
 *   The directory to be created.
 *
 * @return string
 *   'exists' if the directory already exists, 'created' if this
 *   function creates it.
 *
 * @throws Exception
 */
function acquia_gardens_ensure_directory($directory) {
  if (is_dir($directory)) {
    $return = 'exists';
  }
  else {
    if (!mkdir($directory)) {
      throw new Exception("Failed to create directory $directory.");
    }
    $return = 'created';
  }
  return $return;
}

/**
 * Ensure that a settings file exists on the shared Gardens filesystem.
 *
 * Throws an exception on failure.
 *
 * @param $site_directory
 *   The site directory where the settings.php file should be.
 * @param $hosting_site_name
 *   The hosting site name.
 * @param $hosting_site_environment
 *   The environment where the settings file should be created.
 * @param $gardens_site_id
 *   A gardens site ID.
 * @param $ensure_permissions
 *   (optional) Boolean which determines whether or not to ensure that the
 *   settings file has permissions 0400 (i.e., readable by the web server only,
 *   and not writable). Defaults to FALSE.
 * @param
 *   (Optional) An explicit site NID passed from the gardener.
 *
 * @throws
 *   Exception on failure.
 */
function acquia_gardens_ensure_settings_file($site_directory, $hosting_site_name, $hosting_site_environment, $gardens_site_id, $ensure_permissions = FALSE, $nid = NULL) {
  $matches = array();
  if (!empty($nid)) {
    $site_nid = $nid;
  }
  elseif (preg_match('/(\d+)$/', $gardens_site_id, $matches) == 1) {
    $site_nid = $matches[1];
  }
  else {
    // This should never ever happen.
    throw new Exception(sprintf('Cannot determine the site node id from gardens site id %s', $gardens_site_id));
  }
  // Create a settings.php file.
  //
  // The file_exists() check here prevents command-line installations from
  // proceeding if the settings.inc file (which is required to connect to the
  // database) isn't available yet.
  //
  // The file_exists() check that is actually written into settings.php itself
  // (below) prevents visitors or command-line scripts from ever being
  // redirected to the installer in the case where the settings.inc file goes
  // missing for some reason. For visitors, the installer would give them a
  // chance to try to install Drupal in this case, so instead we log the error
  // and show them a simple message. For command-line scripts, we don't want to
  // redirect to the installer because it would stop the script from running,
  // so instead we throw an exception that can be caught.
  //
  // The glob() below is used to account for a gardener who had suffixed their
  // database names to identify them in a particular environment.
  $settings_inc = array_pop(glob("/var/www/site-php/{$hosting_site_name}.{$hosting_site_environment}/D7-{$hosting_site_environment}-{$gardens_site_id}*settings.inc"));
  if (!file_exists($settings_inc)) {
    syslog(LOG_ERR, "Gardens install unable to glob() $settings_inc for $gardens_site_id");
    // Try the old naming scheme.
    $settings_inc = "/var/www/site-php/{$hosting_site_name}.{$hosting_site_environment}/D7-{$gardens_site_id}-settings.inc";
  }
  if (!file_exists($settings_inc)) {
    throw new Exception("Trying to create settings file but ${settings_inc} hasn't been created by Hosting yet. This message is expected for a couple minutes after a new site starts installing.");
  }
  $source = dirname(__FILE__) . "/docroot/sites/default/default.settings.php";
  $settings_file = "$site_directory/settings.php";
  $extra = <<<EOF

\$conf['cache'] = 1;
\$conf['page_cache_maximum_age'] = 300;
// We can't use an external cache if we are trying to invoke these hooks.
\$conf['page_cache_invoke_hooks'] = FALSE;

if (!class_exists('DrupalFakeCache')) {
  \$conf['cache_backends'][] = 'includes/cache-install.inc';
}
// Rely on the external Varnish cache for page caching.
\$conf['cache_class_cache_page'] = 'DrupalFakeCache';

\$conf['gardens_site_id'] = '{$site_nid}';
\$conf['gardens_db_name'] = '{$gardens_site_id}';

if (file_exists('{$settings_inc}')) {
  include('{$settings_inc}');
}
elseif (!isset(\$_SERVER['SERVER_SOFTWARE']) && (php_sapi_name() == 'cli' || (is_numeric(\$_SERVER['argc']) && \$_SERVER['argc'] > 0))) {
  throw new Exception('No database connection file was found for site {$gardens_site_id}.');
}
else {
  syslog(LOG_ERR, 'GardensError: AN-22471 - No database connection file was found for site GardensError: {$gardens_site_id}.');
  header(\$_SERVER['SERVER_PROTOCOL'] .' 503 Service unavailable');
  print 'The website encountered an unexpected error. Please try again later.';
  exit;
}

EOF;
  $content = file_get_contents($source);
  $content .= $extra;
  if (acquia_gardens_update_file($settings_file, $content)) {
    syslog(LOG_NOTICE, "Wrote or updated settings file $settings_file");
  }
  if ($ensure_permissions) {
    if (acquia_gardens_fileperms($settings_file) != '0400' && !chmod($settings_file, 0400)) {
      throw new Exception("Could not change permissions of the file $settings_file.");
    }
  }
}

/**
 * Ensure that a symlink exists pointing from $link to $target.
 *
 * Throws an exception on error.
 */
function acquia_gardens_ensure_symlink($target, $link) {
  if (is_link($link)) {
    // We use realpath to avoid false positives here as sometimes the path
    // specified by $target does point to the right place via a symlink.
    if (realpath(readlink($link)) == realpath($target)) {
      // Nothing to do.
      return;
    }
    else {
      // This should be rare - like when a custom domain or site name is moved.
      syslog(LOG_ERR, "The symlink at $link already existed but did not point to $target.");
      // Unlink and let the link be created.
      unlink($link);
    }
  }

  $success = symlink($target, $link);
  if (!$success) {
    throw new Exception("Failed to create a symlink pointing from $link to $target.");
  }
}

/**
 * Returns a string, e.g. '0700' or '0644', representing the file permissions
 * of a provided file or directory.
 */
function acquia_gardens_fileperms($target) {
  // See http://php.net/manual/en/function.fileperms.php
  return substr(sprintf('%o', fileperms($target)), -4);
}

/**
 * Executes a Drush command, and throws an exception if the command hangs.
 *
 * This is a wrapper around Drush that allows us to ensure we throw a proper
 * exception for cases where the Drush command hangs, e.g. due to some
 * unforseen code path where it waits for user input, or other timeout errors.
 *
 * @param $command
 *   The Drush command to run. This function will take care of calling the
 *   Drush script itself and adding parameters (--yes and --verbose) that are
 *   necessary for it to be executed properly by this function, as well as
 *   redirect its output to an appropriate temporary file. The caller only
 *   needs to supply the main part of the Drush command itself. For example:
 *   "sql-sync sourcesite.com destinationsite.com"
 * @param $timeout
 *   The amount of time, in seconds, to wait for the command to finish before
 *   assuming that it has hung.
 *
 * @throws
 *   Exception if the command times out.
 */
function acquia_gardens_drush_shell_exec($command, $timeout) {
  // Modify the command to work with Drush in the way that this function calls
  // it. (Note that we need --verbose in order to get the expected string in
  // the output that we will look for below.)
  $command = "../drush/drush --yes --verbose $command";
  // Redirect the command's output to a unique temporary file, so it runs in
  // the background.
  $hash = md5($command);
  $time = microtime(TRUE);
  $tmpfile = "/tmp/drush.$hash.$time";
  $command = "$command > $tmpfile 2>&1 &";
  $start_time = time();
  shell_exec($command);

  do {
    // Every second, read the output file and look for a string that indicates
    // Drush has completed successfully. (This is an ugly hack, of course.)
    $content = @file_get_contents($tmpfile);
    if (strpos($content, 'Command dispatch complete') !== FALSE) {
      $success = TRUE;
      break;
    }
    sleep(1);
  } while (time() - $start_time < $timeout);
  @unlink($tmpfile);
  // If the command did not succeed after the timeout duration has passed,
  // throw an exception; we generally expect the calling code (for example, the
  // site duplication process) to catch the exception and try again later. For
  // now we use a GardensWarning because we want to know that this is happening
  // so we can track, e.g., how often it occurs and whether the site
  // duplication process can recover on its own.
  // @todo: This never actually kills the Drush process itself; can that ever
  //   be dangerous?
  if (empty($success)) {
    throw new Exception("GardensWarning: AN-22472 - A Drush command took too long and will be abandoned so it can be tried again GardensWarning: Time: $timeout seconds; command: $command");
  }
}

/**
 * Returns a string that can be used to append peak memory usage to syslog.
 */
function acquia_gardens_peak_memory_usage() {
  $memory_usage = memory_get_peak_usage(TRUE);
  $memory_usage = round($memory_usage/1048576, 2) . "MB";

  $message = "Peak memory usage: $memory_usage.";

  return $message;
}

/**
 * Generate a random password.
 *
 * This is mostly copied from the user_password() function in user.module,
 * but kept here so we can maintain it separately.
 */
function _acquia_gardens_user_password() {
  $allowable_characters = 'abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789';
  $pass = '';
  for ($i = 0; $i < 10; $i++) {
    $pass .= $allowable_characters[mt_rand(0, strlen($allowable_characters) - 1)];
  }
  return $pass;
}

/**
 * Gets the site environment from the hosting environment variable.
 *
 * The variable is AH_SITE_ENVIRONMENT.
 * @return
 *   A string containing the environment name.  If the environment has
 *   not been set this function throws an exception.
 */
function acquia_gardens_get_site_environment() {
  // Get the site environment from the hosting environment variable.
  $site_environment_variable = 'AH_SITE_ENVIRONMENT';
  $hosting_site_environment = getenv($site_environment_variable);
  if (empty($hosting_site_environment)) {
    if (!empty($_ENV[$site_environment_variable])) {
      $hosting_site_environment = $_ENV[$site_environment_variable];
    }
    else {
      throw new Exception('AH_SITE_ENVIRONMENT variable not set.');
    }
  }
  return $hosting_site_environment;
}
