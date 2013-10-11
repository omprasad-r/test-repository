<?php
/**
 * @file
 * Administrative forms and UI for the MASt module.
 */

/**
 * Settings form for the MASt configuration.
 */
function mast_settings_form() {
  $form = array();
  $form['general'] = array(
    '#title' => t('General settings'),
    '#type' => 'fieldset',
  );
  $form['general']['mast_max_allowed_node_types'] = array(
    '#type' => 'textfield',
    '#title' => t('Max allowed node types'),
    '#description' => t('Set the maximum number of content types that can be MASt enabled, set to 0 for unlimited'),
    '#default_value' => variable_get('mast_max_allowed_node_types', MAST_MAX_ALLOWED_NODE_TYPES),
    '#size' => 5,
  );
  $form['oauth'] = array(
    '#title' => t('OAuth settings'),
    '#type' => 'fieldset',
  );
  $oauth_login = variable_get('oauth_common_login_path', 'user/login');
  $use_mast_login = variable_get('mast_use_mast_login', 0) && $oauth_login == 'mast/login';
  $form['oauth']['mast_use_mast_login'] = array(
    '#type' => 'checkbox',
    '#title' => t('Override oAuth login path'),
    '#description' => t('If checked, the MASt module will ensure that its own login page, mast/login, is used instead of user/login for the oAuth login flow.'),
    '#default_value' => $use_mast_login,
  );
  $form['roles'] = array(
    '#title' => t('User listing settings'),
    '#description' => t('These settings determine whether the users listing provided for the author field is restricted to certain roles. If no roles are selected, all authenticated users will be included in the list.'),
    '#type' => 'fieldset',
  );
  $roles = user_roles(TRUE);
  // Don't include the Authenticated Users role in the list.
  unset($roles[DRUPAL_AUTHENTICATED_RID]);
  $form['roles']['mast_settings_roles'] = array(
    '#type' => 'checkboxes',
    '#options' => $roles,
    '#title' => t('Roles'),
    '#default_value' => variable_get('mast_settings_roles', array())
  );
  $form['mast_use_original_author'] = array(
    '#title' => t('Use Original Author module to find content created by a given user.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('mast_use_original_author', 1),
  );
  $form['#submit'][] = 'mast_settings_form_submit';
  $form['#validate'][] = 'mast_settings_form_validate';
  return system_settings_form($form);
}

/**
 * Validation callback for the settings form.
 */
function mast_settings_form_validate($form, &$form_state) {
  if ($form_state['values']['mast_max_allowed_node_types']) {
    $max_allowed_node_types = $form_state['values']['mast_max_allowed_node_types'];
    // Don't allow setting this value to a number lower than the number of
    // already enabled node types.
    $mast_bundle_settings = variable_get('mast_bundle_settings', array());
    if (isset($mast_bundle_settings['node']) && count($mast_bundle_settings['node']) > $max_allowed_node_types) {
      // See how many node types are actually enabled.
      $currently_enabled = 0;
      foreach ($mast_bundle_settings['node'] as $bundle_name => $settings) {
        if ($settings['enabled']) {
          $currently_enabled++;
        }
      }
      if ($currently_enabled > $max_allowed_node_types) {
        form_set_error('mast_max_allowed_node_types', t('You cannot set the maximum allowed node types to a number less than the number of node types that are already enabled. First disable the node types that should not be MASt-enabled'));
      }
    }
  }
}

/**
 * Submit callback for the settings form.
 *
 * Sets oAuth module's oauth_common_login_path variable according to the
 * mast_use_mast_login setting.
 */
function mast_settings_form_submit($form, &$form_state) {
  $oauth_login = variable_get('oauth_common_login_path', 'user/login');
  if ($form_state['values']['mast_use_mast_login']) {
    variable_set('oauth_common_login_path', 'mast/login');
  }
  elseif ($oauth_login == 'mast/login') {
    variable_del('oauth_common_login_path');
  }
}

/**
 * Page callback; provides a mobile-friendly user login form.
 */
function mast_user_login_page() {
  _mast_responsive_meta_add();
  drupal_set_title(t('Sign in to @site-name', array('@site-name' => variable_get('site_name', 'Drupal'))));

  return drupal_get_form('user_login');
}

/**
 * Page callback; provides a mobile-friendly user password form.
 */
function mast_user_pass() {
  _mast_responsive_meta_add();
  drupal_set_title(t('Request a new password'));

  module_load_include('inc', 'user', 'user.pages');
  return drupal_get_form('user_pass');
}

/**
 * Adds a meta element to set viewport behavior on mobile devices.
 */
function _mast_responsive_meta_add() {
  $element = array(
    '#tag' => 'meta',
    '#attributes' => array('name' => 'viewport', 'content' => 'width=device-width, initial-scale=1, maximum-scale=1'),
  );
  drupal_add_html_head($element, 'mast_login_flow_responsive');
}

/**
 * Implements hook_preprocess_page().
 */
function mast_preprocess_page(&$variables) {
  $path = current_path();
  if ($path  == 'mast/login' || $path == 'mast/password') {
    // If there are any messages for this request, add a class so that we may
    // apply some styling.
    if (!empty($_SESSION['messages'])) {
      $variables['classes_array'][] = 'with-messages';

      // Rewrite the form validation error that gets thrown by the user login
      // form when the user submits the form using erroneous credentials so that
      // the forgotten password link points to the MASt password page.
      if (!empty($_SESSION['messages']['error'])) {
        foreach ($_SESSION['messages']['error'] as $key => $error_message) {
          if (strpos($error_message, 'Sorry, unrecognized username or password.') === 0) {
            $_SESSION['messages']['error'][$key] = t('Sorry, unrecognized username or password. <a href="@url">Have you forgotten your password?</a>', array('@url' => url('mast/password')));
          }
        }
      }
    }
  }
}

/**
 * Form submit function().
 */
function mast_user_pass_form_submit($form, &$form_state) {
  // Redirect the user to the login page after the user_pass form is
  // successfully submitted.
  $form_state['redirect'] = 'mast/login';
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function mast_form_user_login_alter(&$form, &$form_state, $form_id) {
  if (current_path() != 'mast/login') {
    return;
  }

  _mast_login_flow_form_setup($form);

  $form['pass']['#attributes']['placeholder'] = t('Password');
  $form['actions']['submit']['#value'] = t('Sign in');

  // Add a "Forgot your password?" link.
  $form['actions']['forgot_password'] = array(
    '#type' => 'link',
    '#title' => t('Forgot your password?'),
    '#href' => 'mast/password',
    '#attributes' => array('class' => array('forgot-password')),
  );
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function mast_form_user_pass_alter(&$form, &$form_state, $form_id) {
  if (current_path() != 'mast/password') {
    return;
  }

  _mast_login_flow_form_setup($form);

  $form['actions']['submit']['#value'] = t('Submit');
  $form['#submit'][] = 'mast_user_pass_form_submit';
}

/**
 * Sets up the login flow forms with some common adjustments.
 */
function _mast_login_flow_form_setup(&$form) {
  // Add styles for the MASt login flow.
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'mast') . '/css/mast.login.css' => array('preprocess' => FALSE),
  );

  // Add HTML5 placeholder attributes to the username and password fields.
  if (!empty($form['name'])) {
    $form['name']['#attributes']['placeholder'] = t('Email or username');
  }
  if (!empty($form['pass'])) {
    $form['pass']['#attributes']['placeholder'] = t('Password');
  }

  $form['actions']['#attributes']['class'][] = 'clearfix';
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Disables the checkbox for editing the MASt app user account on the
 * admin/people page.
 */
function mast_form_user_admin_account_alter(&$form, &$form_state) {
  $mast_uid = variable_get('mast_app_uid');
  $form['accounts'][$mast_uid]['#disabled'] = TRUE;
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Modifies the form at user/[mast-user-id]/cancel to remove access to the form
 * fields (and submit button) and display a message about why it is not possible
 * to cancel this account.
 */
function mast_form_user_cancel_confirm_form_alter(&$form, &$form_state, $form_id) {
  $account = $form['_account']['#value'];
  $mast_uid = variable_get('mast_app_uid');
  if ($account->uid == $mast_uid) {
    foreach (array('user_cancel_method', 'user_cancel_confirm', 'user_cancel_notify', 'description') as $element) {
      $form[$element]['#access'] = FALSE;
    }
    $form['actions']['submit']['#access'] = FALSE;
    $form['not_allowed'] = array(
      '#markup' => 'You cannot cancel the MASt mobile user account because it is needed for MASt module to function. Uninstall MASt module if you really want to delete this account.'
    );
  }
  $form['#validate'][] = 'mast_validate_account_cancellation';
}

/**
 * Validation callback for the user cancel form.
 *
 * Extra protection against deleting the MASt user account.
 */
function mast_validate_account_cancellation($form, &$form_state) {
  $mast_uid = variable_get('mast_app_uid');
  if ($form_state['values']['uid'] == $mast_uid) {
    form_set_error('uid', 'Cannot cancel the MASt mobile user account.');
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Add a Mobile App Services fieldset to the node type form with a checkbox to
 * MASt-enable the content type.
 */
function mast_form_node_type_form_alter(&$form, &$form_state, $form_id) {
  $form['mast'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mobile app'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => TRUE,
    '#group' => 'additional_settings',
    '#attributes' => array('class' => array('mast-settings-form')),
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'mast') . '/js/mast.admin.js'),
    ),
  );

  $bundle = $form['#node_type']->type;

  $mast_bundle_settings = variable_get('mast_bundle_settings', array());
  if (!isset($mast_bundle_settings['node']) || !isset($mast_bundle_settings['node'][$bundle])) {
    $mast_bundle_settings['node'][$bundle] = array(
      'enabled' => FALSE,
      'short_name' => $bundle,
      'icon' => '',
    );
    variable_set('mast_bundle_settings', $mast_bundle_settings);
  }

  $max_allowed_node_types = variable_get('mast_max_allowed_node_types', MAST_MAX_ALLOWED_NODE_TYPES);
  $enable_allowed = TRUE;
  if (!$mast_bundle_settings['node'][$bundle]['enabled']) {
    if ($max_allowed_node_types && count($mast_bundle_settings['node']) >= $max_allowed_node_types) {
      // See how many node types are actually enabled.
      $currently_enabled = 0;
      foreach ($mast_bundle_settings['node'] as $bundle_name => $settings) {
        if ($settings['enabled']) {
          $currently_enabled++;
        }
      }
      // If we have reached the limit do not allow it to be enabled.
      if ($currently_enabled >= $max_allowed_node_types) {
        $enable_allowed = FALSE;
      }
    }
  }

  // If this content type is eligible to be MASt-enabled, display the checkbox
  // for enabling it.
  if (mast_eligible('node', $bundle)) {
    if (!$enable_allowed) {
      $form['mast']['enable_not_allowed'] = array(
        '#markup' => '<span class="error">' . t('You cannot enable this content type for MASt because the maximum allowed number of content types has been reached. !clickhere to change this setting and enable more content types.', array('!clickhere' => l(t('Click here'), 'admin/config/content/mast'))) . '</span>'
      );
    }
    $form['#submit'][] = 'mast_node_type_form_submit';
    $form['mast']['enabled'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow submission through a mobile app'),
      '#default_value' => $mast_bundle_settings['node'][$bundle]['enabled'],
      '#disabled' => !$enable_allowed,
    );

    $icon_options = array(
      'album' => t('Album'),
      'audio' => t('Audio'),
      'blog' => t('Blog'),
      'book' => t('Book'),
      'event' => t('Event'),
      'faq' => t('FAQ'),
      'forum' => t('Forum'),
      'gallery' => t('Gallery'),
      'image' => t('Image'),
      'page' => t('Page'),
      'photo' => t('Photo'),
      'poll' => t('Poll'),
      'video' => t('Video'),
      'webform' => t('Webform'),
    );
    $form['mast']['icon'] = array(
      '#type' => 'radios',
      '#title' => t('Select an icon to use for this content type'),
      '#options' => $icon_options,
      '#icons' => $icon_options,
      '#default_value' => isset($mast_bundle_settings['node'][$bundle]['icon']) && !empty($mast_bundle_settings['node'][$bundle]['icon']) ? $mast_bundle_settings['node'][$bundle]['icon'] : key($icon_options),
      '#theme' => 'mast_icon_options',
      '#attributes' => array('class' => array('clearfix')),
      '#states' => array(
        'visible' => array(
          ':input[name="mast[enabled]"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form['mast']['short_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Short name'),
      '#default_value' => $mast_bundle_settings['node'][$bundle]['short_name'],
      '#size' => 32,
      '#maxlength' => 32,
      '#required' => FALSE,
      '#description' => t("Enter a short name for the title of the content type displayed by the mobile app. For example, change 'Basic page' to 'Page'."),
      '#states' => array(
        'visible' => array(
          ':input[name="mast[enabled]"]' => array('checked' => TRUE),
        )
      )
    );
  }
  else {
    $field_types = mast_bundle_get_unique_required_field_types('node', $bundle);
    $unsupported_types = mast_bundle_ineligible_field_types($field_types);
    if (!empty($unsupported_types)) {
      $form['mast']['info'] = array(
        '#markup' => t('This content type is not eligible to be mobile enabled because it contains required fields of the following incompatible field types: !types', array('!types' => '<em>' . implode(', ', $unsupported_types) . '</em>'))
      );
    }
    else {
      $form['mast']['info'] = array(
        '#markup' => t('This content type has been explicitly specified by one of your enabled modules as not being eligible to be MASt-enabled. This usually means there are aspects of the content type that cannot be supported by MASt.')
      );
    }
  }
}

/**
 * Submit handler for node_type_form.
 *
 * Stores whether or not this node type is MASt-enabled.
 */
function mast_node_type_form_submit($form, &$form_state) {
  $value = variable_get('mast_bundle_settings', array());
  if (!empty($form_state['values']['mast'])) {
    $value['node'][$form_state['values']['type']] = $form_state['values']['mast'];
    variable_set('mast_bundle_settings', $value);
  }
}

/**
 * Theme function for mast icon options.
 *
 * @ingroup themeable
 */
function theme_mast_icon_options($variables) {
  $icons = $variables['mast_icon'];

  $markup = '';
  foreach (element_children($icons) as $icon_name) {
    $element = $icons[$icon_name];

    $icon_attributes = array(
      'id' => drupal_html_class('mast-icon-content-type-' . $icon_name),
      'class' => array(
        'sprite-mast-icon',
      ),
    );
    $title = '<span' . drupal_attributes($icon_attributes) . '></span>';
    $title .= '<span class="mast-icon-name">' . $icons[$icon_name]['#title'] . '</span>';
    $element['#title'] = $title;

    $markup .= drupal_render($element);
  }

  $build = array(
    'icons' => array('#markup' => $markup),
    '#attached' => array(
      'css' => array(drupal_get_path('module', 'mast') . '/css/mast.admin.css'),
    ),
  );

  return drupal_render($build);
}

/**
 * Implements hook_field_create_instance().
 */
function mast_field_create_instance($instance) {
  mast_field_instance_changed($instance);
}

/**
 * Implements hook_field_update_instance().
 */
function mast_field_update_instance($instance, $prior_instance) {
  mast_field_instance_changed($instance);
}

/**
 * Adjusts bundle eligibility for new/changed field instance and de-MASt-enables
 * it if necessary.
 */
function mast_field_instance_changed($instance) {
  $entity_type = $instance['entity_type'];
  $bundle = $instance['bundle'];
  // Update MASt eligibility info for this bundle.
  mast_bundle_update_eligibility($entity_type, $bundle);

  $field = field_info_field($instance['field_name']);
  $required = isset($instance['required']) && $instance['required'];
  if ($required && !mast_field_type_supported($field['type']) && mast_enabled($entity_type, $bundle)) {
    // If this content type had previously been set as MASt-enabled, disable it
    // and let the user know that it has been disabled.
    $mast_bundle_settings = variable_get('mast_bundle_settings', array());
    $mast_bundle_settings[$entity_type][$bundle]['enabled'] = FALSE;
    variable_set('mast_bundle_settings', $mast_bundle_settings);
    drupal_set_message(t('The @bundle @type bundle has been set to not mobile enabled because a required field of an incompatible field type has been added to it', array('@bundle' => $bundle, '@type' => $entity_type)));
  }
}

/**
 * Implements hook_field_delete_instance().
 */
function mast_field_delete_instance($instance) {
  $field = field_info_field($instance['field_name']);
  $required = isset($instance['required']) && $instance['required'];
  if ($required && !mast_field_type_supported($field['type'])) {
    // If this content type had previously been flagged as ineligible, it might
    // now be eligible as a required field of an unsupported field type has just
    // been removed.
    mast_bundle_update_eligibility($instance['entity_type'], $instance['bundle']);
  }
}

/**
 * Updates the cached eligibility state for a bundle.
 */
function mast_bundle_update_eligibility($entity_type, $bundle) {
  $eligibility = &drupal_static('mast_get_bundle_eligibility', array());
  $bundle_eligibility = mast_get_bundle_eligibility();
  $eligible = _mast_determine_eligibility($entity_type, $bundle);
  if (!isset($bundle_eligibility[$entity_type])) {
    $bundle_eligibility[$entity_type] = array($bundle => $eligible);
  }
  else {
    $bundle_eligibility[$entity_type][$bundle] = $eligible;
  }
  cache_set('mast_bundle_eligibility', $bundle_eligibility);
  $eligibility = $bundle_eligibility;
}

/**
 * Returns the cached eligibility state for all bundles.
 */
function mast_get_bundle_eligibility() {
  $eligibility = &drupal_static(__FUNCTION__, array());
  if (empty($eligibility)) {
    if ($cache = cache_get('mast_bundle_eligibility')) {
      $eligibility = $cache->data;
    }
  }
  return $eligibility;
}

/**
 * Calculates whether a bundle is eligible to be MASt-enabled.
 */
function _mast_determine_eligibility($entity_type, $bundle) {
  // If any module declares this bundle ineligible, then it is ineligible.
  $ineligible_bundles = module_invoke_all('mast_unsupported_bundles');
  if (isset($ineligible_bundles[$entity_type]) && in_array($bundle, $ineligible_bundles[$entity_type])) {
    return FALSE;
  }

  // Check whether the content type contains field types that prevent it from
  // being eligible to be MASt-enabled.
  $field_types = mast_bundle_get_unique_required_field_types($entity_type, $bundle);
  $ineligible_field_types = mast_bundle_ineligible_field_types($field_types);
  return empty($ineligible_field_types);
}

/**
 * Returns an array containing the unique field types of the required fields in
 * a given bundle.
 */
function mast_bundle_get_unique_required_field_types($entity_type, $bundle) {
  $field_types = &drupal_static(__FUNCTION__, array());
  if (empty($field_types) || !isset($field_types[$entity_type]) || !isset($field_types[$entity_type][$bundle])) {
    $instances = field_info_instances($entity_type, $bundle);
    $types = array();
    foreach ($instances as $instance) {
      if ($instance['required']) {
        $field = field_info_field($instance['field_name']);
        $types[] = $field['type'];
      }
    }
    $field_types[$entity_type][$bundle] = array_unique($types);
  }
  return $field_types[$entity_type][$bundle];
}

/**
 * Takes an array of field types and returns an array containing those that are
 * not supported.
 */
function mast_bundle_ineligible_field_types($field_types) {
  $ineligible_types = array();
  foreach ($field_types as $type) {
    if (!mast_field_type_supported($type)) {
      $ineligible_types[] = $type;
    }
  }
  return $ineligible_types;
}
