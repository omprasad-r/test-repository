<?php

/**
 * This is a very rough command line script that allows developers to
 * automatically install a Gardens site on their local machine. It will
 * likely evolve as the Drupal 7 auto-installer evolves.
 *
 * You can override any of the default installation settings below by entering
 * their key-value pairs on the command line (the array structure is ignored;
 * simply enter the key you want to change, except for the admin password,
 * which is an exception and can be entered in 'pass' directly).
 *
 * For example, running this...
 *
 * php /path/to/directory/install_gardens.php database="my_local_db" username="my_local_db_user" password="my_local_db_pass" site_name="My Gardens Site" name="Site Administrator" pass="my_site_password" url="http://gardens.dev/install.php"
 *
 * ...will try to install a Gardens site named "My Gardens Site" in the local
 * database specified above, with an admin account whose username is "Site
 * Administrator" and password is "my_site_password", and (via Drupal's
 * multisite feature) look for a settings.php file in sites/gardens.dev to use
 * for the installation. You can install a site with an enterprise profile by
 * adding a parameter like: gardens_client_name="pfizer" to the above.
 *
 * IMPORTANT NOTE: Due to a bug in Drupal, you may need to call this script
 * using the full path to it (as in the above example), rather than just
 * calling "php install_gardens.php".
 */
ini_set('memory_limit', '128M');
chdir('docroot');
define('DRUPAL_ROOT', getcwd());
define('MAINTENANCE_MODE', 'install');
include_once('includes/install.core.inc');

print "Starting Gardens installation...\n";
print "(this may take a while)\n";

// Define the default settings that the installer will use. Any of these can
// be overridden via the command line.
$settings = array(
  'parameters' => array(
    'profile' => 'gardens',
    'locale' => 'en',
  ),
  'forms' => array(
    'install_settings_form' => array(
      'driver' => 'mysql',
      'database' => 'gardens',
      'username' => 'root',
      'password' => 'root',
    ),
    'install_configure_form' => array(
      'site_name' => 'My Site',
      'site_mail' => 'admin@example.com',
      'account' => array(
        'name' => 'admin',
        'mail' => 'admin@example.com',
        'pass' => array(
          'pass1' => 'admin',
          'pass2' => 'admin',
        ),
      ),
      'openid' => '',
      'owner_account' => array(
        'openid' => '',
        'account' => array(
          'name' => 'owner',
          'mail' => 'owner@example.com',
          'pass' => array(
            'pass1' => 'admin',
            'pass2' => 'admin',
          ),
        ),
      ),
      'update_status_module' => array(1 => NULL),
      'clean_url' => TRUE,
      'site_template' => '',
      'features' => '',
      'acquia_gardens_local_user_accounts' => TRUE,
      'acquia_gardens_disable_ga' => TRUE,
      'acquia_gardens_skip_xmlrpc' => TRUE,
      'gardens_client_name' => '', // Gardens sites have this empty, but enterprise sites have it filled.
    ),
  ),
);
// Get the user-specified settings from the command line.
$user_settings = array();
$args = $_SERVER['argv'];
array_shift($args);
if (!empty($args)) {
  foreach ($args as $arg) {
  // Command-line parameters are entered as param=value.
    list($parameter, $value) = explode('=', $arg, 2);
    $user_settings[$parameter] = $value;
  }
}

// Replace the default settings with any user-entered ones, and install Drupal.
foreach ($user_settings as $key => $value) {
  switch ($key) {
    case 'profile':
      $settings['parameters']['profile'] = $value;
      break;
    case 'locale':
      $settings['parameters']['locale'] = $value;
      break;
    case 'name':
    case 'user2_name':
      $settings['forms']['install_configure_form']['owner_account']['account']['name'] = $value;
      break;
    case 'mail':
    case 'user2_mail':
      $settings['forms']['install_configure_form']['owner_account']['account']['mail'] = $value;
      break;
    case 'openid':
    case 'user2_openid':
      $settings['forms']['install_configure_form']['owner_account']['openid'] = $value;
      break;
    case 'pass':
    case 'user2_pass':
      $settings['forms']['install_configure_form']['owner_account']['account']['pass']['pass1'] = $value;
      $settings['forms']['install_configure_form']['owner_account']['account']['pass']['pass2'] = $value;
      break;
    case 'user1_name':
      $settings['forms']['install_configure_form']['account']['name'] = $value;
      break;
    case 'user1_mail':
      $settings['forms']['install_configure_form']['account']['mail'] = $value;
      break;
    case 'user1_openid':
      $settings['forms']['install_configure_form']['openid'] = $value;
      break;
    case 'user1_pass':
      $settings['forms']['install_configure_form']['account']['pass']['pass1'] = $value;
      $settings['forms']['install_configure_form']['account']['pass']['pass2'] = $value;
      break;
    case 'url':
      $settings['server']['url'] = $value;
    default:
      if (isset($settings['forms']['install_settings_form'][$key])) {
        $settings['forms']['install_settings_form'][$key] = $value;
      }
      elseif (isset($settings['forms']['install_configure_form'][$key])) {
        $settings['forms']['install_configure_form'][$key] = $value;
      }
      break;
  }
}

// The final value of the database driver must be used to key part of the
// database settings form array, and the rest of the form must be set up to
// allow proper validation of the chosen driver.
$settings['forms']['install_settings_form'] = array(
  'driver' => $settings['forms']['install_settings_form']['driver'],
  $settings['forms']['install_settings_form']['driver'] => $settings['forms']['install_settings_form'],
  'op' => 'Save and continue',
);

try {
  $start_time = time();
  install_drupal($settings);
  // The themebuilder may have created an initial theme during installation,
  // which the webserver needs to be able to write to later on (when the
  // themebuilder is being used). But if this script was run by a different
  // user than the webserver, it won't be able to. To fix that, we make the
  // theme world-writable.
  $default_theme = variable_get('theme_default');
  if (!empty($default_theme)) {
    $theme_dir = DRUPAL_ROOT . '/' . conf_path() . '/themes/mythemes/' . $default_theme;
    if (is_dir($theme_dir)) {
      shell_exec('chmod -R 777 ' . escapeshellarg($theme_dir));
    }
  }
  $elapsed_time = time() - $start_time;
  $memory_used = memory_get_peak_usage();
  print "Installation finished successfully (took $elapsed_time seconds and used $memory_used of memory)!\n";
}
catch (Exception $e) {
  print "Installation failed with the following message: {$e->getMessage()}.\n";
  exit(1);
}
