<?php
// $Id$

/**
 * Implements hook_install_tasks_alter().
 */
function gardens_install_tasks_alter(&$tasks, $install_state) {
  // @hack - We need the file path set as early as possible: even just before
  // system.module is installed (top of gardens_install_system_module()) would
  // be too late (the default files/ dir would be already created).
  $db_role = $GLOBALS['conf']['gardens_db_name'];
  $GLOBALS['conf']['file_public_path'] = "sites/g/files/{$db_role}/f";
  $tasks['install_system_module']['function'] = 'gardens_install_system_module';
}

/**
 * Installation task; install the system module.
 *
 * We want to set some extra variables before starting to install all the main
 * modules in our profile. This is as good a place to do it as any.
 */
function gardens_install_system_module(&$install_state) {
  // First call the "parent" task to set everything up appropriately; after
  // that we can start setting our custom variables.
  install_system_module($install_state);

  // Enable clean URLs now, so that any calls to url() during the module
  // installation phase will generate clean links.
  variable_set('clean_url', TRUE);

  // IMPORTANT!! If and when we stop using the gardens profile for installation,
  // we will need to find a point early enough in installation to set up the
  // file path so that it's in the right place during install.  Here is good
  // enough until that point.
  if ($db_role = variable_get('gardens_db_name', FALSE)) {
    variable_set('file_public_path', "sites/g/files/{$db_role}/f");
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Allows the profile to alter the site-configuration form. This is
 * called through custom invocation, so $form_state is not populated.
 */
function gardens_form_install_configure_form_alter(&$form, $form_state) {
  // Set default for site name field.
  $site_name = $_SERVER['SERVER_NAME'];
  $pos = strpos($_SERVER['SERVER_NAME'], '.');
  if ($pos !== FALSE) {
    $site_name = substr($site_name, 0, $pos);
  }
  $form['site_information']['site_name']['#default_value'] = $site_name;
  $form['admin_account']['account']['name']['#default_value'] = 'Gardens admin';
  $form['admin_account']['account']['mail']['#default_value'] = 'support@acquia.com';
  // Gardens doesn't use update status, so don't confuse developers who might
  // be installing Gardens by hand and otherwise select it accidentally.
  $form['update_notifications']['update_status_module']['#default_value'] = array();

  // Add field to specify OpenID for administrator.
  $form['admin_account']['openid'] = array(
    '#type' => 'textfield',
    '#title' => st('Gardens OpenID'),
    '#description' => st('OpenID provided by the Gardens Master server.'),
  );

  $form['owner_account'] = array(
    '#type' => 'fieldset',
    '#title' => st('Gardens site owner account'),
    '#collapsible' => FALSE,
  );
  $form['owner_account']['#tree'] = TRUE;
  $form['owner_account']['account']['name'] = array('#type' => 'textfield',
    '#title' => st('Username'),
    '#maxlength' => USERNAME_MAX_LENGTH,
    '#description' => st('Spaces are allowed; punctuation is not allowed except for periods, hyphens, and underscores.'),
    '#required' => TRUE,
    '#weight' => -10,
    '#attributes' => array('class' => array('username')),
  );

  $form['owner_account']['account']['mail'] = array('#type' => 'textfield',
    '#title' => st('E-mail address'),
    '#maxlength' => EMAIL_MAX_LENGTH,
    '#required' => TRUE,
    '#weight' => -5,
  );
  // Set a password.
  $form['owner_account']['account']['pass'] = array(
    '#type' => 'password_confirm',
    '#required' => TRUE,
    '#size' => 25,
    '#weight' => 0,
  );

  // Add field to specify OpenID for site owner.
  $form['owner_account']['openid'] = array(
    '#type' => 'textfield',
    '#title' => st('Site owner Gardens OpenID'),
    '#description' => st('OpenID provided by the Gardens Master server.'),
  );

  $form['acquia_gardens'] = array(
    '#type' => 'fieldset',
    '#title' => t('Drupal Gardens information'),
  );
  $form['acquia_gardens']['acquia_gardens_gardener_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Drupal Gardener URL'),
    '#description' => t('The complete URL (including http://) of the Drupal Gardener host associated with this site. This is used for login and user registration, as well as linking to the user global dashboards.'),
    '#default_value' => scarecrow_get_gardener_url(),
  );
  $form['acquia_gardens']['gardens_service_name_long'] = array(
    '#type' => 'textfield',
    '#title' => t('Service name (gardener name)'),
    '#description' => t('The human-readable name of this service (eg. "Drupal Gardens")'),
    '#default_value' => variable_get('gardens_service_name_long', 'Drupal Gardens'),
  );
  $form['acquia_gardens']['site_template'] = array(
    '#type' => 'textfield',
    '#title' => t('Initial site template'),
    '#description' => t('The machine-readable name of the site template to start off with. If a valid template name is not provided, a regular Drupal Gardens site will be created instead.'),
  );
  $form['acquia_gardens']['features'] = array(
    '#type' => 'textfield',
    '#maxlength' => 10000,
    '#title' => t('Initial site features and pages'),
    '#description' => t('A list of features to enable when the site is installed. Comma separated (with no spaces in between!). Features are from site_template.module. If you leave this empty, a default set of features will be installed for the template. If you use the special string "NONE", the template will be installed without any of its optional features (only the required ones).'),
  );
  $form['acquia_gardens']['mollom_public_key'] = array(
    '#type' => 'textfield',
    '#maxlength' => 128,
    '#title' => t('Public key for Mollom service'),
  );
  $form['acquia_gardens']['mollom_private_key'] = array(
    '#type' => 'textfield',
    '#maxlength' => 128,
    '#title' => t('Private key for Mollom service'),
  );
  $form['acquia_gardens']['install_themebuilder_screenshot_keys'] = array(
    '#type' => 'checkbox',
    '#title' => t('Install themebuilder screenshot keys'),
    '#description' => t("This allows the themebuilder screenshot service to take screenshots of this site's themes. Don't check this for local development, unless you are explicitly testing out the screenshot functionality."),
    '#default_value' => FALSE,
  );
  $form['acquia_gardens']['gardens_client_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Client name'),
    '#description' => t('Enterprise client name associated with this gardens site.
      If available, a profile named [client name] will be enabled.'),
    '#default_value' => '',
  );

  $form['acquia_gardens_development'] = array(
    '#type' => 'fieldset',
    '#title' => t('Developer features'),
  );
  $form['acquia_gardens_development']['acquia_gardens_disable_ga'] = array(
    '#type' => 'checkbox',
    '#title' => t('Disable extra Google Analytics tracking'),
    '#description' => t('This should be checked for local  development.'),
    '#default_value' => FALSE,
  );
  $form['acquia_gardens_development']['acquia_gardens_basic_developer_features'] = array(
    '#type' => 'checkbox',
    '#title' => t('Turn off page, CSS and JavaScript caching'),
    '#description' => t('This is particularly useful for theme development.'),
    // If the 'acquia_gardens_developer_mode' variable (which causes all
    // Gardens-related permission restrictions to be removed) is set in
    // settings.php, we also assume the developer wants to enable the more
    // mild features listed here, and check the box by default.
    '#default_value' => variable_get('acquia_gardens_developer_mode', FALSE),
  );
  $form['acquia_gardens_development']['acquia_gardens_advanced_developer_features'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable the Devel module'),
    '#description' => t('Used for advanced development. This setting only has an effect if the Devel module is present on your system.'),
    // Same as above. Note that we do not allow the 'acquia_gardens_developer_mode'
    // variable *itself* to be set via the installation profile, but rather reserve
    // that for settings.php.
    '#default_value' => variable_get('acquia_gardens_developer_mode', FALSE),
  );
  $form['acquia_gardens_development']['acquia_gardens_local_user_accounts'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow normal user accounts'),
    '#description' => t('Select this in order to be able to create accounts on this site and log in to them via normal methods. (If not set, logins can only happen through the Gardener, which this site must be connected to via OpenID.)'),
    // Same as above.
    '#default_value' => variable_get('acquia_gardens_developer_mode', FALSE),
  );
  $form['acquia_gardens_development']['acquia_gardens_skip_xmlrpc'] = array(
    '#type' => 'checkbox',
    '#title' => t('Disable XML-RPC communication with the Gardener'),
    '#description' => t('Select this for local development to skip XML-RPC calls for email verification and statistics collection.)'),
    // Same as above.
    '#default_value' => variable_get('acquia_gardens_developer_mode', FALSE),
  );
  // Add both existing submit function and our submit function,
  // since adding just ours cancels the automated discovery of the original.
  $form['#submit'] = array('gardens_installer_custom_submit', 'install_configure_form_submit');

  // Put action buttons on bottom of form.
  $form['actions']['#weight'] = 200;
}

/**
 * Custom submit handler for the Gardens install form.
 */
function gardens_installer_custom_submit($form, &$form_state) {
  // Attach the OpenID given in the installer to the first user.
  if (!empty($form_state['values']['openid'])) {
    $account = user_load(1);
    gardens_client_add_authmaps($account, array("authname_openid" => $form_state['values']['openid']));
  }
  // Setting an empty string for the code disables addition to the page.
  if (!empty($form_state['values']['acquia_gardens_disable_ga'])) {
    variable_set('gardens_misc_ga_tracking_code', '');
  }
  // Enable JS, CSS and page caching for regular Gardens installations.
  if (empty($form_state['values']['acquia_gardens_basic_developer_features'])) {
    variable_set('preprocess_js', 1);
    variable_set('preprocess_css', 1);
    variable_set('cache', 1);
  }
  // Otherwise, explicitly turn it off (just to be safe).
  else {
    variable_set('preprocess_js', 0);
    variable_set('preprocess_css', 0);
    variable_set('cache', 0);
    // Also make it so CSS and JS aggregation don't turn themselves back on;
    // see gardens_misc_cron().
    variable_set('acquia_gardens_keep_js_css_caching_off', TRUE);
  }

  // Syslog settings.
  $allowed_severity = array(
    WATCHDOG_EMERGENCY => WATCHDOG_EMERGENCY,
    WATCHDOG_ALERT => WATCHDOG_ALERT,
    WATCHDOG_CRITICAL => WATCHDOG_CRITICAL,
    WATCHDOG_ERROR => WATCHDOG_ERROR,
    WATCHDOG_WARNING => 0,
    WATCHDOG_NOTICE => 0,
    WATCHDOG_INFO => 0,
    WATCHDOG_DEBUG => 0,
  );
  variable_set('syslog_allowed_severity', $allowed_severity);

  // For advanced developers, enable the Devel module if it is present in the
  // file system.
  if (!empty($form_state['values']['acquia_gardens_advanced_developer_features'])) {
    if (db_query("SELECT 1 FROM {system} WHERE type = 'module' AND name = 'devel'")->fetchField()) {
      module_enable(array('devel'));
    }
  }

  // Skip XML-RPC communication with the gardener for local developer installs.
  if (!empty($form_state['values']['acquia_gardens_skip_xmlrpc'])) {
    variable_set('gardens_client_site_verification_status', array('verified' => TRUE));
    variable_set('gardens_client_send_stats', FALSE);
    variable_set('gardens_client_phone_home', FALSE);
    variable_set('gardens_debug_xmlrpc', TRUE);
  }

  // Save gardener URL information.
  variable_set('acquia_gardens_gardener_url', trim($form_state['values']['acquia_gardens_gardener_url'], '/'));
  variable_set('gardens_service_name_long', $form_state['values']['gardens_service_name_long']);

  if ($form_state['values']['gardens_service_name_long'] != 'Drupal Gardens') {
    variable_set('gardens_features_responsive_enabled', TRUE);
  }

  // At this point in the batch, gardens.install will not have been included if
  // the gardens verification profile was used.  Including it now keeps the the
  // next statement from failing.
  include_once DRUPAL_ROOT . '/profiles/gardens/gardens.install';

  // Since the acquia_gardens_gardener_url variable has now been manually set,
  // redefine user notification emails so that they contain links to the new
  // gardener URL.
  gardens_setup_user_mail();

  // Allow local user logins, if requested. This must run after the call to
  // gardens_setup_user_mail(), since it sometimes needs to delete the mail
  // variables that were set there.
  if (!empty($form_state['values']['acquia_gardens_local_user_accounts'])) {
    scarecrow_allow_local_user_logins();
  }

  $account_name = $form_state['values']['owner_account']['account']['name'];
  $account_mail = $form_state['values']['owner_account']['account']['mail'];
  if (!user_load_by_name($account_name) && !user_load_by_mail($account_mail)) {
    $owner_account->is_new = TRUE;
    $edit = $form_state['values']['owner_account']['account'];
    $edit['status'] = 1;
    $edit['roles'][variable_get('user_admin_role', 0)] = 1;
    $edit['roles'][variable_get('gardens_site_owner_role', 0)] = 1;
    // Set login to non-zero to avoid e-mail verification needed error.
    $edit['login'] = 1;
    $owner_account = user_save($owner_account, $edit);
  }
  else {
    $owner_account = user_load_by_name($account_name);
  }

  if (!empty($form_state['values']['owner_account']['openid'])) {
    gardens_client_add_authmaps($owner_account, array("authname_openid" => $form_state['values']['owner_account']['openid']));
  }

  // Enable the chosen site template and features, and finalize the template
  // selection.
  $template_installed = FALSE;
  if (!empty($form_state['values']['site_template']) && in_array($form_state['values']['site_template'], array_keys(site_template_get_all_templates()))) {
    $template = $form_state['values']['site_template'];
    if (!empty($form_state['values']['features'])) {
      // Special string to indicate an empty list of features should be passed
      // in to the site template.
      if ($form_state['values']['features'] == 'NONE') {
        $features = array();
      }
      else {
        $features = explode(',', $form_state['values']['features']);
      }
    }
    else {
      $features = NULL;
    }
    site_template_install_features($template, $features);
    $template_installed = TRUE;
  }
  site_template_finalize_template_selection();

  // Configure mollom
  if (!empty($form_state['values']['mollom_public_key']) && !empty($form_state['values']['mollom_private_key'])) {
    gardens_misc_update_mollom_keys_if_necessary($form_state['values']['mollom_public_key'], $form_state['values']['mollom_private_key']);
  }

  // Install the themebuilder screenshot keys.
  if (!empty($form_state['values']['install_themebuilder_screenshot_keys'])) {
    gardens_misc_install_themebuilder_screenshot_keys();
  }

  // Copy the default user picture into the public:// directory.
  $image_path = drupal_get_path('module', 'gardens_misc') . '/AnonymousPicture.gif';
  file_unmanaged_copy($image_path, NULL, FILE_EXISTS_REPLACE);
  // Configure the default user picture.
  variable_set('user_picture_default', 'public://AnonymousPicture.gif');

  // Set the oembed cache to *not* be cleared on general cache flushes.
  variable_set('oembed_cache_flush', FALSE);

  // Enable user pictures on nodes and comments.
  $theme_settings = variable_get('theme_settings', array());
  $theme_settings['toggle_node_user_picture'] = 0;
  $theme_settings['toggle_comment_user_picture'] = 1;
  variable_set('theme_settings', $theme_settings);

  // Enable media on for Full HTML.
  include_once DRUPAL_ROOT . '/profiles/gardens/modules/acquia/gardens_misc/gardens_misc.install';
  gardens_misc_update_7001();

  // For normal installs, we're done here, so the site's final theme is set and
  // we can copy it to the 'mythemes' directory and configure it so that the
  // themebuilder can use it.
  //
  // For preinstalled sites on Hosting, though, if we haven't installed a site
  // template here then we know we will do so later (when the site is claimed
  // by a real user), and since that process can result in the theme changing
  // again, we'll wait until then to copy the theme and not do it here. See
  // install_gardens().
  if (!isset($GLOBALS['gardens_install_op']) || !defined('GARDENS_SIGNUP_SITE_OPERATION_INSTALL') || $GLOBALS['gardens_install_op'] != GARDENS_SIGNUP_SITE_OPERATION_INSTALL || $template_installed) {
    $default_theme_name = themebuilder_compiler_copy_theme(variable_get('theme_default', 'bartik'), 'My theme');
    gardens_misc_replace_default_theme($default_theme_name);
  }

  // Store any client name set in the form.
  $profile_name = empty($form_state['values']['gardens_client_name']) ? '' : $form_state['values']['gardens_client_name'];
  variable_set('gardens_client_name', $profile_name);

  // Rebuild the sitemap so we pick up all menu links that were created earlier
  // in the setup process.
  //
  // Note: This normally would happen automatically via hooks that the
  // xmlsitemap module implements when a menu link is saved, but our
  // site_template_menu_link_save() function results in this happening
  // incorrectly, due to the workarounds that function is forced to use.
  gardens_rebuild_xmlsitemap();
}

/**
 * Enable the specified theme.
 *
 * @param string $theme_name
 *   The name of the theme to enable.
 */
function gardens_profile_set_default_theme($theme_name) {
  db_update('system')
  ->fields(array('status' => 1))
  ->condition('type', 'theme')
  ->condition('name', $theme_name)
  ->execute();
  block_theme_initialize($theme_name);
  variable_set('theme_default', $theme_name);
}


/**
 * Creates a new block. Adapted from the D6 version of install_profile_api.
 */
function gardens_profile_add_block($module, $delta, $theme, $status, $weight, $region, $visibility = 0, $pages = '', $custom = 0, $title = '', $cache = -1) {
  $query = db_insert('block')
    ->fields(array('module', 'delta', 'theme', 'status', 'weight', 'region', 'visibility', 'pages', 'custom', 'title', 'cache'));
  $query->values(array(
      'module' => $module,
      'delta' => $delta,
      'theme' => $theme,
      'status' => $status,
      'weight' => $weight,
      'region' => $region,
      'visibility' => $visibility,
      'pages' => $pages,
      'custom' => $custom,
      'title' => $title,
      'cache' => $cache,));
  // If what we just added was a custom block, also add it to other active
  // themes. If we don't, the title will be lost.
  if ($module == 'block') {
    foreach (list_themes() as $key => $active_theme) {
      if ($active_theme->status) {
        $query->values(array(
          'module' => $module,
          'delta' => $delta,
          'theme' => $active_theme->name,
          'status' => $status,
          'weight' => $weight,
          'region' => $region,
          'visibility' => $visibility,
          'pages' => $pages,
          'custom' => $custom,
          'title' => $title,
          'cache' => $cache,
        ));
      }
    }
  }
  $query->execute();
}

/**
 * Creates a new custom block, but does not add it to any themes.
 *
 * @param {string} $body
 *   The content of the block.
 * @param {string} $description
 *   Block description, to be used on the administration page.
 * @param {int} $format
 *   The format for the block's content. Defaults to the fallback format.
 * @return {int}
 *   The box id of the newly created block.
 */
function gardens_profile_add_box($body, $description, $format = 0) {
  // Set default options.
  $box_id = db_insert('block_custom')
    ->fields(array(
      'body' => $body,
      'info' => $description,
      'format' => empty($format) ? filter_fallback_format() : $format,
    ))
    ->execute();
  return $box_id;
}

/**
 * Setup Timeago module.
 */
function gardens_profile_timeago_date_format_types_setup() {
  if (module_exists('timeago')) {
    _scarecrow_setup_timeago();
  }
}
