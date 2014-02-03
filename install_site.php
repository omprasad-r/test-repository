<?php

if (count($_SERVER['argv']) < 4) {
  echo "Usage:\n{$_SERVER['argv'][0]} DOMAIN DBROLE BASE64_JSON_SETTINGS\n";
  exit(1);
}

list($script_name, $domain, $db_role, $data) = $_SERVER['argv'];
$gardens_site_info = $data ? json_decode(base64_decode($data), TRUE) : array();
install_site($domain, $db_role, $gardens_site_info);

/**
 * Install and configure an Acquia Gardens site.
 *
 * This function logs appropriate status messages to syslog as it proceeds, and
 * throws an exception on error.
 *
 * @param $domain
 *   Default domain name for this site - e.g. foo.drupalgardens.com
 * @param $db_role
 *   The name of the database "role", like g123456
 * @param $gardens_site_info
 *   Optional, but really needed - all the site info.
 *
 * @throws Exception
 */
function install_site($domain, $db_role, $gardens_site_info = array()) {

  try {
    $current_directory = dirname(__FILE__);
    $gardens_site_id = $gardens_site_info['site_id'];

    // Install Drupal with the appropriate installation settings for the
    // Gardens site.
    $user_password1 = _acquia_gardens_user_password();
    $user_password2 = _acquia_gardens_user_password();
    $settings = array(
      'interactive' => FALSE,
      'parameters' => array(
        'profile' => 'gardens',
        'locale' => 'en',
      ),
      'server' => array(
        'url' => 'http://' . $domain . '/install.php',
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

    drupal_override_server_variables(array('url' => 'http://' . $domain  . '/index.php'));

    variable_set('gardens_misc_standard_domain', $domain);

    variable_set('file_public_path', "sites/g/files/{$db_role}/f");
    variable_set('gardens_site_id', $gardens_site_info['site_id']);
    variable_set('site_name', $gardens_site_info['site_name']);
    variable_set('site_mail', $gardens_site_info['site_mail']);
    variable_set('gardens_client_name', $gardens_site_info['gardens_client_name']);

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

    db_delete('semaphore')->condition('value', $lock_id)->execute();

  }
  catch (Exception $e) {
    // If something went wrong, ensure that the site is properly "unclaimed"
    // (from the Gardener) before throwing the exception, so that the next PHP
    // process will be able to try the requested operation again.

    $memory_used = acquia_gardens_peak_memory_usage();
    // Configuring a site happens live (while the end user is waiting), so
    // we always want to receive alerts when something goes wrong and it
    // needs to be tried again. We therefore re-throw any regular exception
    // (e.g., those triggered by Drupal) as an exception that triggers
    // Gardens alerts.
    if (strpos($e->getMessage(), 'Gardens') === 0) {
      // The Exception is already a GardensError or GardensWarning.
      syslog(LOG_ERR, 'Gardens Site Install Error: ' . $e->getMessage() . " ($memory_used)");
      throw $e;
    }
    else {
      // Turn the message into a GardensError.
      syslog(LOG_ERR, 'Gardens Site Install Error: ' . $e->getMessage());
      throw new Exception("GardensError: AN-22470 - Error occurred during site installation and configuration (install_gardens function) GardensError: " . $e->getMessage() . " ($memory_used)" . $e->getTraceAsString());
    }
  }

  // If we got here, the overall operation was a success.
  $memory_used = acquia_gardens_peak_memory_usage();
  $dir = dirname(__FILE__);
  syslog(LOG_NOTICE, "Successfully installed Gardens site $gardens_site_id in $dir. ($memory_used)");
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

