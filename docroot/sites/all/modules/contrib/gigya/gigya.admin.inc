<?php

/**
 * @file
 * Provides the administration settings for the Gigya module.
 */

/**
 * Form builder for the Gigya administration form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   The Gigya administration form.
 */
function gigya_admin_form($form, &$form_state) {
  global $user, $base_path;
  include_once DRUPAL_ROOT . '/' . 'modules/user/user.pages.inc';
  module_load_include('inc', 'gigya');
  $form = array();
  $form['#attached']['js'] = array(drupal_get_path('module', 'gigya') . '/js/gigya.admin.behaviors.js');
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'gigya') . '/css/gigya_admin.css',
  );
  $form['keys'] = array(
    '#type' => 'fieldset',
    '#title' => t('Gigya Socialize keys'),
    '#description' => t('Keys needed for connecting to the Gigya Socialize service.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => -400,
  );
  $form['keys']['gigya_APIKey'] = array(
    '#type' => 'textfield',
    '#title' => t('Gigya API Key'),
    '#description' => t('Specify the Gigya API key for this domain'),
    '#default_value' => variable_get('gigya_APIKey', ''),
    '#required' => TRUE,
    '#weight' => -300,
  );
  $form['keys']['gigya_SecretKey'] = array(
    '#type' => 'textfield',
    '#title' => t('Gigya Secret Key'),
    '#description' => t('Specify the Gigya Secret (Base64 encoded) key for this domain'),
    '#default_value' => variable_get('gigya_SecretKey', ''),
    '#required' => TRUE,
    '#weight' => -200,
  );
  $from['gigya_networks'] = array(
    '#type' => 'fieldset',
    '#title' => t('Giya network providers'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['gigya_networks']['gigya_network_list'] = array(
    '#type' => 'textfield',
    '#title' => t('Providers'),
    '#size' => 60,
    '#maxlength' => 400,
    '#description' => t('Comma separated list of networks that would be included. For example: "Facebook, Twitter, Yahoo". <br />* means all networks. See list of available !providers',
      array(
        '!providers' => l(t('providers.'), 'http://developers.gigya.com/020_Client_API/020_Methods/Socialize.showLoginUI', array(
            'external' => TRUE,
            'attributes' => array('target' => '_blank'),
          )
        ),
      )
    ),
    '#default_value' => variable_get('gigya_network_list', '*'),
  );
  $form['gigya_language'] = array(
    '#type' => 'select',
    '#title' => t('Interface language'),
    '#description' => t('Please select the interface language'),
    '#options' => gigya_get_languages(),
    '#default_value' => variable_get('gigya_language', 'en'),
  );
  $form['gigya_social_analytics'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Google Social Analytics'),
    '#default_value' => variable_get('gigya_social_analytics', FALSE),
  );
  $form['gigya_data_center'] = array(
    '#type' => 'select',
    '#title' => t('Data Center'),
    '#description' => t('Please chose the data center to work opposite'),
    '#options' => array('us1.gigya.com' => 'US', 'eu1.gigya.com' => 'EU'),
    '#default_value' => variable_get('gigya_data_center', 'us1.gigya.com'),
  );
  $form['gigya_advanced_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Gigya advanced settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $extra_parms = variable_get('gigya_global_extra', array());
  $form['gigya_advanced_settings']['gigya_global_extra'] = array(
    '#type' => 'textarea',
    '#title' => t('Custom Global Conf'),
    '#rows' => 5,
    '#element_validate' => array('gigya_key_val_filed_validate'),
    '#description' => t('Enter values in key|value format. See list of available !parameters', array(
        '!parameters' => l(t('parameters'), 'http://developers.gigya.com/030_API_reference/010_Client_API/010_Objects/Conf_object', array(
            'external' => TRUE,
            'attributes' => array('target' => '_blank'),
          )
        ),
      )
    ),
    '#default_value' => !empty($extra_parms) ? gigya_values_array_to_string($extra_parms) : '',
    '#attributes' => array('placeholder' => 'facebookExtraPermissions|rsvp_event,sms'),
  );
  $form['gigya_advanced_settings']['gigya_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Gigya debugging'),
    '#description' => t('Write Gigya API calls to <a href="@watchdog">Drupal Watchdog</a> log', array('@watchdog' => url('admin/reports/dblog'))),
    '#default_value' => variable_get('gigya_debug', FALSE),
  );
  return system_settings_form($form);
}

/**
 * LoginUI admin form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return mixed
 *   Returns login admin from.
 */
function gigya_login_admin_form($form, $form_state) {
  $form['gigya_login_mode'] = array(
    '#type' => 'radios',
    '#title' => t('User Management System'),
    '#options' => array(
      'drupal' => t('Drupal only'),
      'drupal_and_gigya' => t('Drupal + Social Login'),
      'gigya' => t('Social Login only'),
      'raas' => t('Registration-as-a-Service'),
    ),
    '#default_value' => variable_get('gigya_login_mode', 'drupal_and_gigya'),
    '#pre_render' => array('gigya_check_raas'),
  );

  // Social Login settings.
  $form['gigya_social_login'] = array(
    '#type' => 'fieldset',
    '#title' => t('Social Login Settings'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['gigya_social_login']['gigya_login_buttons_style'] = array(
    '#type' => 'select',
    '#title' => t('Button style'),
    '#options' => array(
      'standard' => t('Icons'),
      'fullLogo' => t('Full logos'),
    ),
    '#default_value' => variable_get('gigya_login_buttons_style', 'standard'),
  );
  $form['gigya_social_login']['gigya_login_uiconfig_urlredirect'] = array(
    '#type' => 'textfield',
    '#title' => t('URL redirect after login'),
    '#default_value' => variable_get('gigya_login_uiconfig_urlredirect', ''),
    '#description' => t('Choose an alternate URL to direct users to after login. Defaults to the current page. Do not use a beginning or ending slash!'),
  );
  $form['gigya_social_login']['gigya_login_hide_title'] = array(
    '#type' => 'checkbox',
    '#title' => t('Hide title'),
    '#description' => t('Hide login block title'),
    '#default_value' => variable_get('gigya_login_hide_title', FALSE),
  );
  $form['gigya_social_login']['gigya_login_uiconfig_terms'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show terms link'),
    '#description' => t('Display the Gigya terms of service link'),
    '#default_value' => variable_get('gigya_login_uiconfig_terms', FALSE),
  );
  $form['gigya_social_login']['gigya_login_plugin_size'] = array(
    '#type' => 'checkbox',
    '#title' => t('Set custom size for plugin'),
    '#default_value' => variable_get('gigya_login_plugin_size', FALSE),
  );
  $form['gigya_social_login']['gigya_login_plugin_width'] = array(
    '#prefix' => '<div class="height-with">',
    '#type' => 'textfield',
    '#title' => t('W:'),
    '#size' => 5,
    '#states' => array(
      'visible' => array(
        '#edit-gigya-login-plugin-size' => array('checked' => TRUE),
      ),
    ),
    '#default_value' => variable_get('gigya_login_plugin_width', GIGYA_DEFAULT_LOGINUI_WIDTH),
  );
  $form['gigya_social_login']['gigya_login_plugin_height'] = array(
    '#type' => 'textfield',
    '#title' => t('H:'),
    '#size' => 5,
    '#states' => array(
      'visible' => array(
        '#edit-gigya-login-plugin-size' => array('checked' => TRUE),
      ),
    ),
    '#default_value' => variable_get('gigya_login_plugin_height', GIGYA_DEFAULT_LOGINUI_HEIGHT),
    '#suffix' => '</div>',
  );
  $form['gigya_social_login']['gigya_login_plugins_login'] = array(
    '#type' => 'select',
    '#title' => t('Login behavior'),
    '#options' => array(
      'loginExistingUser' => t('Login existing user'),
      'alwaysLogin' => t('Always login'),
    ),
    '#default_value' => variable_get('gigya_login_plugins_login', 'loginExistingUser'),
  );
  // RaaS settings.
  $form['gigya_ras'] = array(
    '#type' => 'fieldset',
    '#title' => t('Registration-as-a-Service Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#states' => array(
      'visible' => array(
        ':input[name="gigya_login_mode"]' => array('value' => 'raas'),
      ),
    ),
  );
  $form['gigya_ras']['message'] = array(
    '#markup' => t("Please make sure to initially configure RaaS in Gigya's site. Go to the !Builder page
     after logging in to Gigya's site. The page presents the list of predefined default screen-sets, each has an ID.
     Click on the \"Visual Editor\" link next to a screen-set that you wish to use. This will open the !Editor window.
      You can modify the screens, or just hit the \"Save\" button to activate it. Please make sure that the screen-sets
       IDs that are defined below match the IDs of the screen-sets you have configured in the !Builder page.",
      array(
        '!Builder' => l(t('UI Builder'),
          'https://platform.gigya.com/site/partners/Settings.aspx#cmd%3DUserManagement360.ScreenSets',
          array('external' => TRUE, 'attributes' => array('target' => '_blank'))
        ),
        '!Editor' => l(t('Visual Editor'), 'http://developers.gigya.com/010_Developer_Guide/10_UM360/040_Raas/020_UI_Builder#Visual_Editor',
          array('external' => TRUE, 'attributes' => array('target' => '_blank'))
        ),
      )
    ),
  );
  // Login registration screen set.
  $form['gigya_ras']['login_secreen_set'] = array(
    '#type' => 'fieldset',
    '#title' => t('Login/Registration Screen Sets'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['gigya_ras']['login_secreen_set']['web_screen_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Web Screen Set ID'),
    '#size' => 30,
    '#default_value' => variable_get('web_screen_id', 'Login-web'),
  );
  $form['gigya_ras']['login_secreen_set']['mobile_screen_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Mobile Screen Set ID'),
    '#size' => 30,
    '#default_value' => variable_get('mobile_screen_id', 'Mobile-login'),
  );
  $form['gigya_ras']['login_secreen_set']['login_screen_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Login Screen ID'),
    '#size' => 30,
    '#default_value' => variable_get('login_screen_id', 'gigya-login-screen'),
  );
  $form['gigya_ras']['login_secreen_set']['register_screen_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Register Screen ID'),
    '#size' => 30,
    '#default_value' => variable_get('register_screen_id', 'gigya-register-screen'),
  );
  // Profile settings.
  $form['gigya_ras']['profile_screen_set'] = array(
    '#type' => 'fieldset',
    '#title' => t('Profile Screen Sets'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['gigya_ras']['profile_screen_set']['profile_web_screen_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Web Screen Set ID'),
    '#size' => 30,
    '#default_value' => variable_get('profile_web_screen_id', 'Profile-web'),
  );
  $form['gigya_ras']['profile_screen_set']['profile_mobile_screen_id'] = array(
    '#type' => 'textfield',
    '#title' => t('mobile Screen Set ID'),
    '#size' => 30,
    '#default_value' => variable_get('profile_mobile_screen_id', 'Profile-mobile'),
  );
  // Links and labels.
  $form['gigya_ras']['labels_links'] = array(
    '#type' => 'fieldset',
    '#title' => t("Links' Labels"),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['gigya_ras']['labels_links']['message'] = array(
    '#markup' => t('Specify the labels of the links that invoke the screen-sets.'),
  );
  $form['gigya_ras']['labels_links']['raas_login_label'] = array(
    '#type' => 'textfield',
    '#title' => t('Login'),
    '#size' => 30,
    '#default_value' => variable_get('raas_login_label', 'Login'),
  );
  $form['gigya_ras']['labels_links']['raas_register_label'] = array(
    '#type' => 'textfield',
    '#title' => t('Register'),
    '#size' => 30,
    '#default_value' => variable_get('raas_register_label', 'Register'),
  );
  $form['gigya_ras']['labels_links']['raas_profile_label'] = array(
    '#type' => 'textfield',
    '#title' => t('Profile'),
    '#size' => 30,
    '#default_value' => variable_get('raas_profile_label', 'Profile'),
  );
  $form['gigya_login_advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced showLoginUI settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => 1,
  );
  $form['gigya_login_advanced']['gigya_login_uiconfig_containerID'] = array(
    '#type' => 'textfield',
    '#title' => t('showLoginUI container ID'),
    '#default_value' => variable_get('gigya_login_uiconfig_containerID', GIGYA_DEFAULT_LOGINUI_CONTAINERID),
    '#description' => t('div ID for loginUI'),
    '#required' => TRUE,
  );
  $login_extra_parms = variable_get('gigya_login_extra', array());
  $form['gigya_login_advanced']['gigya_login_extra'] = array(
    '#type' => 'textarea',
    '#title' => t('Advanced showLoginUI settings'),
    '#rows' => 5,
    '#element_validate' => array('gigya_key_val_filed_validate'),
    '#description' => t('Enter values in key|value format. See list of available !parameters',
      array(
        '!parameters' => l(t('parameters'), 'http://developers.gigya.com/030_Gigya_Socialize_API_2.0/030_API_reference/010_Client_API_%28JavaScript%29/Social_service/Socialize.showLoginUI',
          array('external' => TRUE, 'attributes' => array('target' => '_blank'))
        ),
      )
    ),
    '#default_value' => !empty($login_extra_parms) ? gigya_values_array_to_string($login_extra_parms) : '',
  );

  // ConnectUI.
  $form['gigya_connect_advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced showAddConnectionsUI settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => 2,
  );
  $form['gigya_connect_advanced']['gigya_connect_uiconfig_containerID'] = array(
    '#type' => 'textfield',
    '#title' => t('showAddConnectionsUI container ID:'),
    '#default_value' => variable_get('gigya_connect_uiconfig_containerID', GIGYA_DEFAULT_CONNECTUI_CONTAINERID),
    '#description' => t('div ID for showAddConnectionsUI'),
  );
  $connect_extra_parms = variable_get('gigya_connect_extra', array());
  $form['gigya_connect_advanced']['gigya_connect_extra'] = array(
    '#type' => 'textarea',
    '#title' => t('Advanced showAddConnectionsUI settings'),
    '#rows' => 5,
    '#element_validate' => array('gigya_key_val_filed_validate'),
    '#description' => t('Enter values in key|value format. See list of available !parameters',
      array(
        '!parameters' => l(t('parameters'), 'http://developers.gigya.com/020_Client_API/020_Methods/socialize.showAddConnectionsUI',
          array('external' => TRUE, 'attributes' => array('target' => '_blank'))
        ),
      )
    ),
    '#default_value' => !empty($connect_extra_parms) ? gigya_values_array_to_string($connect_extra_parms) : '',
  );

  $form['#attached']['js'] = array(drupal_get_path('module', 'gigya') . '/js/gigya.admin.behaviors.js');
  $form['#attached']['css'] = array(drupal_get_path('module', 'gigya') . '/css/gigya_admin.css');
  return system_settings_form($form);
}

/**
 * Implements hook_form_validate().
 */
function gigya_admin_form_validate($form, &$form_state) {
  // Vaidate the the api key and secret key.
  $valid = gigya_validate($form_state['values']['gigya_APIKey'], $form_state['values']['gigya_SecretKey'], $form_state['values']['gigya_data_center']);
  if ($valid !== TRUE) {
    if (is_object($valid)) {
      $code = $valid->getErrorCode();
      if ($code == 301001) {
        form_set_error('gigya_data_center', t('Wrong data center please change.'));
      }
      else {
        form_set_error('', t('Your API key or Secret key are not valid'));
        form_set_error('keys', $valid->getErrorMessage());
      }
    }
    else {
      form_set_error('keys', t('Your API key or Secret key are not valid'));
    }
  }
  if (!empty($form_state['values']['gigya_global_extra'])) {
    $errors = gigya_validate_json($form_state['values']['gigya_global_extra']);
    if (is_array($errors)) {
      foreach ($errors as $err) {
        form_set_error('gigya_global_extra', $err);
      }
    }
  }
}

/**
 * Form builder for the Gigya sharebar administration form.
 *
 * @return array
 *   The Gigya sharebar administration form.
 */
function gigya_sharebar_settings_form() {
  $form['gigya_share_message'] = array(
    '#markup' => t('To position a Gigya Share Bar plugin, place a "Gigya Share Bar" field via the "Structure"/"Content types" menu'),
    '#weight' => -50,
  );
  $form['gigya_share_global'] = array(
    '#type' => 'fieldset',
    '#title' => t('Sharing global configuration'),
    '#collapsible' => FALSE,
    '#weight' => -10,
  );
  $form['gigya_share_global']['gigya_share_container_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Container ID'),
    '#size' => 60,
    '#default_value' => variable_get('gigya_share_container_id', 'gigya-share'),
  );
  $form['gigya_share_global']['gigya_share_buttons'] = array(
    '#type' => 'textfield',
    '#title' => t('Providers'),
    '#size' => 60,
    '#maxlength' => 400,
    '#default_value' => variable_get('gigya_share_buttons', GIGYA_DEFAULT_SHARE_BOTTONS),
    '#description' => t('List of relevant values is available !here under the "shareButtons" parameter',
      array(
        '!here' => l(t('here'), 'http://developers.gigya.com/020_Client_API/020_Methods/socialize.showShareBarUI',
          array('external' => TRUE, 'attributes' => array('target' => '_blank'))
        ),
      )
    ),
  );
  $form['gigya_share_global']['gigya_share_container_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Container ID'),
    '#size' => 60,
    '#default_value' => variable_get('gigya_share_container_id', 'gigya-share'),
  );
  $form['gigya_share_global']['gigya_share_layout'] = array(
    '#type' => 'select',
    '#title' => t('Share Bar layout'),
    '#options' => array(
      'horizontal' => t('Horizontal'),
      'vertical' => t('Vertical'),
    ),
    '#default_value' => variable_get('gigya_share_layout', 'horizontal'),
  );
  $form['gigya_share_global']['gigya_share_showcounts'] = array(
    '#type' => 'select',
    '#title' => t('Show counts'),
    '#description' => t('Where to display the share counts in relation to the share button'),
    '#options' => array(
      'right' => t('Right'),
      'top' => t('Top'),
      'none' => t('none'),
    ),
    '#default_value' => variable_get('gigya_share_showcounts', 'right'),
  );
  $form['gigya_share_global']['gigya_share_af_scope'] = array(
    '#type' => 'select',
    '#title' => t('Scope for publishing to Activity Feed'),
    '#description' => t('When publishing feed items, by default the feed items are published to social
     networks only and will not appear on the site\'s Activity Feed plugin. To change this behavior,
      you must change the publish scope to "Both".'
    ),
    '#options' => array(
      'external' => t('External'),
      'both' => t('Both'),
    ),
    '#default_value' => variable_get('gigya_share_af_scope', 'external'),
  );
  $form['gigya_share_global']['gigya_share_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Share title'),
    '#size' => 60,
    '#default_value' => variable_get('gigya_share_title', '[node:title]'),
  );
  $form['gigya_share_global']['gigya_share_desc'] = array(
    '#type' => 'textfield',
    '#title' => t('Default share description'),
    '#size' => 60,
    '#default_value' => variable_get('gigya_share_desc', '[node:summary]'),
  );
  $form['gigya_share_global']['gigya_share_link_back'] = array(
    '#type' => 'textfield',
    '#title' => t('Default linkback URL'),
    '#size' => 60,
    '#default_value' => variable_get('gigya_share_link_back', '[node:url:absolute]'),
  );
  $form['gigya_share_global']['gigya_share_image_behavior'] = array(
    '#type' => 'radios',
    '#title' => t('Image'),
    '#options' => array(
      'default' => t('Use image tag if exists, first image on post otherwise.'),
      'first' => t('First image on the post'),
      'url' => t('Specify an image URL'),
    ),
    '#default_value' => variable_get('gigya_share_image_behavior', 'default'),
    '#attributes' => array('class' => array('image-behv')),
  );
  $form['gigya_share_global']['gigya_share_image_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Default URL of the image to share'),
    '#size' => 20,
    '#states' => array(
      'visible' => array(
        '.image-behv' => array('value' => 'url'),
      ),
    ),
    '#default_value' => variable_get('gigya_share_image_url', ''),
  );
  $form['gigya_share_global']['gigya_share_short_url'] = array(
    '#type' => 'checkbox',
    '#title' => t('Share using short URLs'),
    '#default_value' => variable_get('gigya_share_short_url', FALSE),
  );
  $form['gigya_share_global']['gigya_share_tokens'] = array(
    '#type' => 'fieldset',
    '#title' => t('Token list'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['gigya_share_global']['gigya_share_tokens']['tokens_list'] = array(
    '#markup' => theme('token_tree', array('token_types' => array('node'), 'global_types' => FALSE)),
  );
  $form['gigya_share_global']['gigya_advanced_sharing'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced Share Bar settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $adv_parmas = variable_get('gigya_share_advanced', '');
  $form['gigya_share_global']['gigya_advanced_sharing']['gigya_share_advanced'] = array(
    '#type' => 'textarea',
    '#title' => t('Advanced showShareBarUI settings'),
    '#element_validate' => array('gigya_key_val_filed_validate'),
    '#default_value' => !empty($adv_parmas) ? gigya_values_array_to_string($adv_parmas) : '',
    '#description' => t('Use this textbox to enter your own settings. If you are not using this box, make sure it is empty. </br>See list of available !parameters', array(
        '!parameters' => l(t('parameters'), 'http://developers.gigya.com/020_Client_API/020_Methods/socialize.showShareBarUI', array(
            'external' => TRUE,
            'attributes' => array('target' => '_blank'),
          )
        ),
      )
    ),
  );
  $form['gigya_action_sharing'] = array(
    '#type' => 'fieldset',
    '#title' => t('Action sharing'),
  );
  $form['gigya_action_sharing']['gigya_rules_message'] = array(
    '#markup' => t('Gigya action sharing can be configured with !rules', array('!rules' => l(t('Rules'), 'admin/config/workflow/rules'))),
  );
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'gigya') . '/css/gigya_admin.css',
  );

  $form['#attached']['js'] = array(drupal_get_path('module', 'gigya') . '/js/gigya.admin.behaviors.js');
  return system_settings_form($form);
}

/**
 * Validate shearbar setting form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function gigya_sharebar_settings_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['gigya_share_advanced'])) {
    $errors = gigya_validate_json($form_state['values']['gigya_share_advanced']);
    if (is_array($errors)) {
      foreach ($errors as $err) {
        form_set_error('gigya_share_advanced', $err);
      }
    }
  }
}

/**
 * Form builder for the Gigya comments administration form.
 *
 * @return array
 *   The Gigya comments administration form.
 */
function gigya_comments_settings_form() {
  $form['gigya_comments_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Gigya Comments'),
    '#default_value' => variable_get('gigya_comments_enabled', TRUE),
  );
  $form['gigya_comments_categoryid'] = array(
    '#type' => 'textfield',
    '#title' => t('Gigya Comments category ID'),
    '#default_value' => variable_get('gigya_comments_categoryid', ''),
    '#description' => t('Category as set up in the Gigya admin site.'),
    '#attributes' => array('placeholder' => 'category1'),
  );
  $form['gigya_comments_container'] = array(
    '#type' => 'textfield',
    '#title' => t('Container ID'),
    '#size' => 30,
    '#description' => t('The container ID that the comments will be displayed in the default is "comments" (Drupal default)'),
    '#default_value' => variable_get('gigya_comments_container', 'comments'),
  );
  $form['gigya_comments_af_scope'] = array(
    '#type' => 'select',
    '#title' => t('Scope for publishing to Activity Feed'),
    '#description' => t('When publishing feed items, by default the feed items are published to social networks only and will not appear on the site\'s Activity Feed plugin. To change this behavior, you must change the publish scope to "Both".'),
    '#options' => array(
      'external' => t('External'),
      'both' => t('Both'),
    ),
    '#default_value' => variable_get('gigya_comments_af_scope', 'external'),
  );
  $form['gigya_comments_hide_drupal_comments'] = array(
    '#type' => 'checkbox',
    '#title' => t('Do not load Drupal comments'),
    '#description' => t("If checked Drupal comments will not load, only Gigya comments will be loaded. Note: this may harm the site's SEO as comments will not be indexed by search engines"),
    '#default_value' => variable_get('gigya_comments_hide_drupal_comments', TRUE),
  );

  $form['gigya_comments_advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $comments_extra_parms = variable_get('gigya_comments_extra', array());
  $form['gigya_comments_advanced']['gigya_comments_extra'] = array(
    '#type' => 'textarea',
    '#title' => t('Advanced showCommentsUI settings'),
    '#rows' => 5,
    '#element_validate' => array('gigya_key_val_filed_validate'),
    '#description' => t('Enter values in key|value format. See list of available !parameters', array(
        '!parameters' => l(t('parameters'), 'http://developers.gigya.com/020_Client_API/030_Comments/comments.showCommentsUI', array(
            'external' => TRUE,
            'attributes' => array('target' => '_blank'),
          )
        ),
      )
    ),
    '#default_value' => !empty($comments_extra_parms) ? gigya_values_array_to_string($comments_extra_parms) : '',
  );

  $form['#attached']['css'] = array(
    drupal_get_path('module', 'gigya') . '/css/gigya_admin.css',
  );
  $form['#attached']['js'] = array(drupal_get_path('module', 'gigya') . '/js/gigya.admin.behaviors.js');
  return system_settings_form($form);
}

/**
 * Validate comments setting form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function gigya_comments_settings_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['gigya_comments_extra'])) {
    $errors = gigya_validate_json($form_state['values']['gigya_comments_extra']);
    if (is_array($errors)) {
      foreach ($errors as $err) {
        form_set_error('gigya_comments_extra', $err);
      }
    }
  }
}

/**
 * Reactions form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return mixed
 *   Reactions admin form.
 */
function gigya_resctions_settings_form($form, $form_state) {
  $form = array();
  $form['gigya_reaction_message'] = array(
    '#markup' => t('To position a Gigya Reactions plugin, place a "Gigya Reactions" field via the "Structure"/"Content types" menu') . '<br /><br />',
    '#weight' => -50,
  );
  $form['gigya_reactions_global'] = array(
    '#type' => 'fieldset',
    '#title' => t('Gigya reaction global settings'),
    '#collapsible' => FALSE,
  );
  $form['gigya_reactions_global']['gigya_reactions_container_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Container ID'),
    '#size' => 20,
    '#default_value' => variable_get('gigya_reactions_container_id', 'gigya-reactions'),
  );
  $form['gigya_reactions_global']['gigya_reactions_bar_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Bar ID'),
    '#size' => 20,
    '#default_value' => variable_get('gigya_reactions_bar_id', 'reactions-bar'),
  );
  $form['gigya_reactions_global']['gigya_reactions_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Reaction title'),
    '#size' => 60,
    '#default_value' => variable_get('gigya_reactions_title', '[node:title]'),
  );
  $form['gigya_reactions_global']['gigya_reactions_link_back'] = array(
    '#type' => 'textfield',
    '#title' => t('Default linkback URL'),
    '#size' => 60,
    '#default_value' => variable_get('gigya_reactions_title', '[node:url:absolute]'),
  );
  $form['gigya_reactions_global']['gigya_reactions_show_counts'] = array(
    '#type' => 'select',
    '#title' => t('Show counts'),
    '#options' => array(
      'right' => t('Right'),
      'top' => t('Top'),
      'none' => t('None'),
    ),
    '#default_value' => variable_get('gigya_reactions_show_counts', 'right'),
  );
  $form['gigya_reactions_global']['gigya_reactions_layout'] = array(
    '#type' => 'select',
    '#title' => t('Layout'),
    '#options' => array(
      'horizontal' => t('Horizontal'),
      'vertical' => t('Vertical'),
    ),
    '#default_value' => variable_get('gigya_reactions_layout', 'horizontal'),
  );
  $form['gigya_reactions_global']['gigya_reactions_multiple_reactions'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use multiple Reactions'),
    '#default_value' => variable_get('gigya_reactions_multiple_reactions', TRUE),
  );
  $form['gigya_reactions_global']['gigya_reactions_count_type'] = array(
    '#type' => 'select',
    '#title' => t('Count type'),
    '#options' => array(
      'number' => t('Number'),
      'percentage' => t('Percentage'),
    ),
    '#default_value' => variable_get('gigya_reactions_count_type', 'number'),
  );
  $form['gigya_reactions_global']['gigya_reactions_af_scope'] = array(
    '#type' => 'select',
    '#title' => t('Scope for publishing to Activity Feed'),
    '#description' => t('When publishing feed items, by default the feed items are published to social networks only and will not appear on the site\'s Activity Feed plugin. To change this behavior, you must change the publish scope to "Both".'),
    '#options' => array(
      'external' => t('External'),
      'both' => t('Both'),
    ),
    '#default_value' => variable_get('gigya_reactions_af_scope', 'external'),
  );
  $form['gigya_reactions_global']['gigya_reactions_image_behavior'] = array(
    '#type' => 'radios',
    '#title' => t('Image'),
    '#options' => array(
      'default' => t('Use image tag if exists, first image on post otherwise.'),
      'first' => t('First image on the post'),
      'url' => t('Specify an image URL'),
    ),
    '#default_value' => variable_get('gigya_reactions_image_behavior', 'default'),
    '#attributes' => array('class' => array('image-behv')),
  );
  $form['gigya_reactions_global']['gigya_reactions_image_url'] = array(
    '#type' => 'textfield',
    '#title' => t('The URL of the image to share'),
    '#size' => 20,
    '#states' => array(
      'visible' => array(
        '.image-behv' => array('value' => 'url'),
      ),
    ),
    '#default_value' => variable_get('gigya_reactions_image_url', ''),
  );
  $form['gigya_reactions_global']['gigya_advanced_reactions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced Reactions settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $adv_parmas = variable_get('gigya_reactions_advanced', '');
  $form['gigya_reactions_global']['gigya_advanced_reactions']['gigya_reactions_advanced'] = array(
    '#type' => 'textarea',
    '#title' => t('Advanced showReactionsBarUI settings'),
    '#element_validate' => array('gigya_key_val_filed_validate'),
    '#default_value' => !empty($adv_parmas) ? gigya_values_array_to_string($adv_parmas) : '',
    '#description' => t('Use this text-box to enter your own settings. If you are not using this box, make sure it is empty. </br> See list of available !parameters', array(
        '!parameters' => l(t('parameters'), 'http://developers.gigya.com/020_Client_API/020_Methods/socialize.showReactionsBarUI', array(
            'external' => TRUE,
            'attributes' => array('target' => '_blank'),
          )
        ),
      )
    ),
  );
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'gigya') . '/css/gigya_admin.css',
  );

  $form['#attached']['js'] = array(drupal_get_path('module', 'gigya') . '/js/gigya.admin.behaviors.js');
  return system_settings_form($form);
}

/**
 * Validate reactions setting form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function gigya_resctions_settings_form_validate($form, $form_state) {
  if (!empty($form_state['values']['gigya_reactions_advanced'])) {
    $errors = gigya_validate_json($form_state['values']['gigya_reactions_advanced']);
    if (is_array($errors)) {
      foreach ($errors as $err) {
        form_set_error('gigya_reactions_advanced', $err);
      }
    }
  }
}

/**
 * Validate GM setting form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return mixed
 *   GM admin form.
 */
function gigya_gm_settings_form($form, $form_state) {
  $form['gigya_gm_message'] = array(
    '#markup' => t('To position Gigya Gamification plugins, create a Gamification block via the menu below,
     and then position it via the "Structure"/"Blocks" menu.') . '<br /><br />',
    '#weight' => -50,
  );
  $form['gigya_gm_global'] = array(
    '#type' => 'fieldset',
    '#title' => t('Gigya Gamification global settings'),
    '#collapsible' => FALSE,
  );
  $form['gigya_gm_global']['gigya_gm_enable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Gigya Gamification'),
    '#default_value' => variable_get('gigya_gm_enable', FALSE),
  );
  $form['gigya_gm_global']['gigya_gm_enable_notifications'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Gamification notifications'),
    '#default_value' => variable_get('gigya_gm_enable_notifications', FALSE),
  );
  $form['gigya_gm_global']['gigya_gm_leaderboard_period'] = array(
    '#type' => 'select',
    '#title' => t('Leaderboard time period'),
    '#options' => array(
      '7days' => t('7 Days'),
      'all' => t('All'),
    ),
    '#default_value' => variable_get('gigya_gm_leaderboard_period', '7days'),
  );
  $form['gigya_gm_global']['gigya_gm_total_count'] = array(
    '#type' => 'textfield',
    '#title' => t('Leaderboard user count'),
    '#size' => 2,
    '#description' => t('Valid values are between 1 to 23'),
    '#default_value' => variable_get('gya_gm_total_count', '12'),
  );
  $form['gigya_gm_blocks'] = array(
    '#type' => 'fieldset',
    '#title' => t('Gigya Gamification blocks'),
    '#collapsible' => FALSE,
  );
  $gm_blocks = variable_get('gigya_gm_blobks', array());
  if (!empty($gm_blocks)) {
    foreach ($gm_blocks as $block_id => $block) {
      $opts[$block_id] = $block['block_name'];
    }
    $form['gigya_gm_blocks']['gigya_gm_delete'] = array(
      '#type' => 'fieldset',
      '#title' => t('Delete blocks'),
      '#collapsible' => FALSE,
      '#weight' => -11,
      '#attributes' => array('class' => array('gm-delete')),
    );
    $form['gigya_gm_blocks']['gigya_gm_delete']['gigya_gm_delete_blocks'] = array(
      '#type' => 'checkboxes',
      '#title' => '',
      '#options' => $opts,
      '#weight' => -10,
    );
    $form['gigya_gm_blocks']['gigya_gm_delete']['gigya_gm_delete_blocks_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#ajax' => array(
        'callback' => 'gigya_gm_delete_block',
        'wrapper' => 'gigya-gm-settings-form',
        'method' => 'html',
        'effect' => 'fade',
      ),
    );
  }
  else {
    $form['gigya_gm_blocks']['gigya_gm_delete']['gigya_gm_delete_blocks'] = array(
      '#markup' => '<div id="gm-delete"></div>',
    );
  }
  $form['gigya_gm_blocks']['gigya_gm_new_block'] = array(
    '#type' => 'fieldset',
    '#title' => t('Create a new block'),
    '#collapsible' => FALSE,
  );
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_block_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Block name'),
    '#size' => 60,
  );

  // Achievements.
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_achievements'] = array(
    '#markup' => '<span class="label">' . t('Achievements') . '</span>',
    '#prefix' => '<div class="gm-element">',
  );
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_achievements_enable'] = array(
    '#type' => 'checkbox',
  );
  $achiv_def_id = gigya_gm_default_id($gm_blocks, 'achievements', 'divAchievements');
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_achievements_div-id'] = array(
    '#type' => 'textfield',
    '#title' => t('Div ID'),
    '#size' => 60,
    '#default_value' => $achiv_def_id,
  );
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_achievements_weight'] = array(
    '#type' => 'select',
    '#title' => t('Weight'),
    '#options' => drupal_map_assoc(range(-20, 20)),
    '#default_value' => 0,
    '#suffix' => '</div>',
  );

  // Challenge status.
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_challenge_status'] = array(
    '#markup' => '<span class="label">' . t('Challenge status') . '</span>',
    '#prefix' => '<div class="gm-element">',
  );
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_challenge_status_enable'] = array(
    '#type' => 'checkbox',
  );
  $ch_def_id = gigya_gm_default_id($gm_blocks, 'challenge_status', 'divChallengeStatus');
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_challenge_status_div-id'] = array(
    '#type' => 'textfield',
    '#title' => t('Div ID'),
    '#size' => 60,
    '#default_value' => $ch_def_id,
  );
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_challenge_status_weight'] = array(
    '#type' => 'select',
    '#title' => t('Weight'),
    '#options' => drupal_map_assoc(range(-20, 20)),
    '#default_value' => 0,
    '#suffix' => '</div>',
  );

  // User status.
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_user_status'] = array(
    '#markup' => '<span class="label">' . t('User status') . '</span>',
    '#prefix' => '<div class="gm-element">',
  );
  $us_def_id = gigya_gm_default_id($gm_blocks, 'user_status', 'divUserStatus');
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_user_status_enable'] = array(
    '#type' => 'checkbox',
  );
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_user_status_div-id'] = array(
    '#type' => 'textfield',
    '#title' => t('Div ID'),
    '#size' => 60,
    '#default_value' => $us_def_id,
  );
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_user_status_weight'] = array(
    '#type' => 'select',
    '#title' => t('Weight'),
    '#options' => drupal_map_assoc(range(-20, 20)),
    '#default_value' => 0,
    '#suffix' => '</div>',
  );

  // Leaderboard.
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_leaderboard'] = array(
    '#markup' => '<span class="label">' . t('Leaderboard') . '</span>',
    '#prefix' => '<div class="gm-element">',
  );
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_leaderboard_enable'] = array(
    '#type' => 'checkbox',
  );
  $lead_def_id = gigya_gm_default_id($gm_blocks, 'leaderboard', 'divLeaderboard');
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_leaderboard_div-id'] = array(
    '#type' => 'textfield',
    '#title' => t('Div ID'),
    '#size' => 60,
    '#default_value' => $lead_def_id,
  );
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_leaderboard_weight'] = array(
    '#type' => 'select',
    '#title' => t('Weight'),
    '#options' => drupal_map_assoc(range(-20, 20)),
    '#default_value' => 0,
    '#suffix' => '</div>',
  );
  $form['gigya_gm_blocks']['gigya_gm_new_block']['gigya_gm_blocks_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create a block'),
    '#ajax' => array(
      'callback' => 'gigya_gm_create_block',
      'wrapper' => 'gigya-gm-settings-form',
      'method' => 'html',
      'effect' => 'fade',
    ),
  );
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'gigya') . '/css/gigya_admin.css',
  );
  $form['#attached']['js'] = array(drupal_get_path('module', 'gigya') . '/js/gigya.admin.behaviors.js');
  $form['#attributes'] = array('class' => 'gigya-gm-settings-form');
  return system_settings_form($form);
}

/**
 * Creates GM blocks.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   blocks form.
 */
function gigya_gm_create_block($form, $form_state) {
  $gigya_gm_blobks = variable_get('gigya_gm_blobks', array());
  $values = $form_state['values'];
  // Create blocks.
  if (!empty($values['gigya_gm_blocks_block_name'])) {
    foreach ($values as $key => $val) {
      if (strpos($key, 'gigya_gm_blocks_') === 0 && $key !== 'gigya_gm_blocks_block_name') {
        $key = str_replace('gigya_gm_blocks_', '', $key);
        $karray = explode('_', $key);
        $prop = end($karray);
        array_pop($karray);
        $comp_key = implode('_', $karray);
        $componentes[$comp_key][$prop] = $val;
      }
    }
    $block_id = str_replace(' ', '_', $values['gigya_gm_blocks_block_name']);
    $gigya_gm_blobks[$block_id] = array(
      'block_name' => $values['gigya_gm_blocks_block_name'],
      'componentes' => $componentes,
    );
  }
  variable_set('gigya_gm_blobks', $gigya_gm_blobks);
  $form_state['input'] = array();
  $form = drupal_rebuild_form('gigya_gm_settings_form', $form_state, $form);
  return $form;
}

/**
 * Delete GM block.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   Delete block from.
 */
function gigya_gm_delete_block($form, $form_state) {
  $gigya_gm_blobks = variable_get('gigya_gm_blobks', array());
  $values = $form_state['values'];
  // Delete blocks.
  if (!empty($values['gigya_gm_delete_blocks'])) {
    foreach ($values['gigya_gm_delete_blocks'] as $block_id => $val) {
      if (!empty($val)) {
        unset($gigya_gm_blobks[$block_id]);
      }
    }
  }
  variable_set('gigya_gm_blobks', $gigya_gm_blobks);
  $form_state['input'] = array();
  $form = drupal_rebuild_form('gigya_gm_settings_form', $form_state, $form);
  return $form;
}

/**
 * Generate GM block id.
 *
 * @param array $blocks
 *   An array of the blocks
 * @param string $key
 *   The block key
 * @param string $id
 *   The block id.
 *
 * @return string
 *   A HTML id from the block.
 */
function gigya_gm_default_id($blocks, $key, $id) {
  $ids = array();
  foreach ($blocks as $block_id => $block) {
    $ids[] = $block['componentes'][$key]['div-id'];
  }
  $last_id = array_search($id, $ids);
  if ($last_id !== FALSE) {
    $parts = explode('-', end($ids));
    if (!empty($parts[1]) && is_numeric($parts[1])) {
      list($seen_id, $i) = $parts;
      $i++;
    }
    else {
      $i = 1;
    }
    $id = $id . '-' . $i;
  }
  return $id;
}

/**
 * Checks if the current api key has RaaS enabled.
 *
 * @param array $element
 *   The form element.
 *
 * @return mixed
 *   Validated element or an error.
 */
function gigya_check_raas(&$element) {
  $res = _gigya_api('accounts.getSchema', array(), TRUE);
  if ($res === 403036) {
    $element['raas']['#attributes']['disabled'] = 'disabled';
    $element['raas']['#attributes']['class'] = array('disabled');
    $element['raas']['#description'] = t("Gigya's Registration-as-a-Service (RaaS) is currently not part of your site package.
     Please contact your Gigya account manager to activate the package. RaaS is Gigya's end-to-end user management system package. Learn more !here. ",
      array(
        '!here' => l(t('here'), 'http://developers.gigya.com/010_Developer_Guide/10_UM360/040_Raas',
          array('external' => TRUE, 'attributes' => array('target' => '_blank'))
        ),
      )
    );
  }
  else {
    $element['raas']['#description'] = t("Selecting this option overrides Drupal's user management system.
     This requires additional administration steps. Learn more !here",
      array(
        '!here' => l(t('here'), 'http://developers.gigya.com/050_Partners/050_CMS_Modules/020_Drupal/Using_RaaS_with_Drupal',
          array('external' => TRUE, 'attributes' => array('target' => '_blank'))
        ),
      )
    );
    $element['drupal_and_gigya']['#description'] = t('<span class="warnning hidden"> Warning: this site is configured on Gigya server to use Registration-as-a-Service. Please contact your Gigya account manager for migration instruction.</span>');
    $element['gigya']['#description'] = t('<span class="warnning hidden"> Warning: this site is configured on Gigya server to use Registration-as-a-Service. Please contact your Gigya account manager for migration instruction.</span>');
  }
  return $element;
}


/**
 * Activity feed setting form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 *
 * @return mixed
 *   Form.
 */
function gigya_activity_feed_settings_form($form, $form_state) {
  $form['gigya_activety_feed_message'] = array(
    '#markup' => t('To position a Gigya Activity Feed plugin, enter the "Structure" -> "Blocks" menu and choose where
     to position the "Gigya Activity Feed" block.') . '<br /><br />',
    '#weight' => -50,
  );
  $form['gigya_activety_feed_global'] = array(
    '#type' => 'fieldset',
    '#title' => t('Gigya Activity Feed settings'),
    '#collapsible' => FALSE,
  );

  $form['gigya_activety_feed_global']['gigya_activety_feed_enable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Gigya Activity Feed'),
    '#default_value' => variable_get('gigya_activety_feed_enable', FALSE),
  );

  $form['gigya_activety_feed_global']['gigya_activety_feed_container_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Activity Feed container ID'),
    '#size' => 30,
    '#default_value' => variable_get('gigya_activety_feed_container_id', 'ActivityFeedDiv'),
  );
  $form['gigya_activety_feed_global']['gigya_activety_feed_tabs'] = array(
    '#type' => 'textfield',
    '#title' => t('Tabs and order'),
    '#size' => 30,
    '#description' => t("A comma delimited list of tabs names that defines which tabs to show and the tabs order.
     The optional tabs names are: \"everyone\", \"friends\", \"me\"."
    ),
    '#default_value' => variable_get('gigya_activety_feed_tabs', 'everyone,friends,me'),
  );

  $form['gigya_activety_feed_global']['gigya_activety_feed_size'] = array(
    '#type' => 'checkbox',
    '#title' => t('Set custom size for plugin'),
    '#default_value' => variable_get('gigya_activety_feed_size', FALSE),
  );
  $form['gigya_activety_feed_global']['gigya_activety_feed_width'] = array(
    '#prefix' => '<div class="height-with">',
    '#type' => 'textfield',
    '#title' => t('Width'),
    '#size' => 5,
    '#default_value' => variable_get('gigya_activety_feed_width', '309'),
    '#states' => array(
      'visible' => array(
        '#edit-gigya-activety-feed-size' => array('checked' => TRUE),
      ),
    ),
  );
  $form['gigya_activety_feed_global']['gigya_activety_feed_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Height'),
    '#size' => 5,
    '#default_value' => variable_get('gigya_activety_feed_height', '270'),
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        '#edit-gigya-activety-feed-size' => array('checked' => TRUE),
      ),
    ),
  );
  $form['gigya_activety_feed_global']['gigya_activety_feed_privacy'] = array(
    '#type' => 'select',
    '#title' => t('Activity Feed privacy level'),
    '#description' => t('The privacy level determines how the user action is presented in each of the Activity Feed plugin tabs.'),
    '#options' => array(
      'private' => t('Private'),
      'public' => t('Public'),
      'friends' => t('Friends'),
    ),
    '#default_value' => variable_get('gigya_activety_feed_privacy', 'private'),
  );
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'gigya') . '/css/gigya_admin.css',
  );
  $form['#attached']['js'] = array(drupal_get_path('module', 'gigya') . '/js/gigya.admin.behaviors.js');
  return system_settings_form($form);
}

/**
 * Checks if the json in parameters is valid.
 *
 * @param array $parms_array
 *   An array of the parameters.
 *
 * @return array|bool
 *   A php array if valid false if not.
 */
function gigya_validate_json($parms_array) {
  foreach ($parms_array as $parm) {
    json_decode($parm);
    $err = json_last_error();
    if ($err !== JSON_ERROR_NONE) {
      switch ($err) {
        case JSON_ERROR_DEPTH:
          $msg = 'json error - Maximum stack depth exceeded';
          break;

        case JSON_ERROR_STATE_MISMATCH:
          $msg = 'json error - Underflow or the modes mismatch';
          break;

        case JSON_ERROR_CTRL_CHAR:
          $msg = 'json error - Unexpected control character found';
          break;

        case JSON_ERROR_SYNTAX:
          $msg = 'json error - Syntax error, malformed JSON';
          break;

        case JSON_ERROR_UTF8:
          $msg = 'json error - Malformed UTF-8 characters, possibly incorrectly encoded';
          break;

        default:
          $msg = 'json error - Unknown error';
          break;
      }
      $json_errors[] = $msg;
    }
  }
  if (!empty($json_errors)) {
    return $json_errors;
  }
  return TRUE;
}
