<?php

/**
 * @file
 * Admin settings menu
 */

/**
 * Sets up the admin settings form for this module.
 */
function janrain_capture_settings() {
  $form['description'] = array(
    '#markup' => '<p>' . t('Connection to Janrain Capture requires a subscription. Once subscribed, use the values on the Janrain dashboard to complete the fields below to connect the service.') . '</p>',
  );

  $janrain_capture_main = variable_get('janrain_capture_main', array());
  $form['janrain_capture_main'] = array(
    '#type' => 'fieldset',
    '#title' => t('Application Settings'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['janrain_capture_main']['capture_client_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Client ID'),
    '#default_value' => !empty($janrain_capture_main['capture_client_id']) ? $janrain_capture_main['capture_client_id'] : '',
  );
  $form['janrain_capture_main']['capture_client_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Client Secret'),
    '#default_value' => !empty($janrain_capture_main['capture_client_secret']) ? $janrain_capture_main['capture_client_secret'] : '',
  );
  $form['janrain_capture_main']['capture_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Capture URL'),
    '#default_value' => !empty($janrain_capture_main['capture_address']) ? $janrain_capture_main['capture_address'] : '',
    '#description' => t('For example, myapp.janraincapture.com.'),
  );

  $janrain_capture_fields = variable_get('janrain_capture_fields', array());
  $form['janrain_capture_fields'] = array(
    '#type' => 'fieldset',
    '#title' => t('Field Settings'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['janrain_capture_fields']['capture_enforce_verification'] = array(
    '#type' => 'checkbox',
    '#title' => t('Require email verification'),
    '#default_value' => !empty($janrain_capture_fields['capture_enforce_verification']) ? $janrain_capture_fields['capture_enforce_verification'] : 0,
    '#description' => t('Prompt users who have not verified their email address to do so before completing authentication'),
  );
  $form['janrain_capture_fields']['capture_match_email'] = array(
    '#type' => 'checkbox',
    '#title' => t('Match users by email address'),
    '#default_value' => !empty($janrain_capture_fields['capture_match_email']) ? $janrain_capture_fields['capture_match_email'] : 0,
    '#description' => t('Associate user and Capture accounts if both share the same email address.'),
    '#states' => array(
      'enabled' => array(
        ':input[name="janrain_capture_fields[capture_enforce_verification]"]' => array('checked' => TRUE),
      )
    ),
  );
  $form['janrain_capture_fields']['capture_map_profile_pic'] = array(
    '#type' => 'checkbox',
    '#title' => t('Map profile pic'),
    '#default_value' => isset($janrain_capture_fields['capture_map_profile_pic']) ? $janrain_capture_fields['capture_map_profile_pic'] : 1,
    '#description' => t('Automatically map the Capture profile photo to the Drupal user profile picture.'),
  );
  $form['janrain_capture_fields']['capture_preferred_photo_variant'] = array(
    '#type' => 'select',
    '#title' => t('Preferred photo variant'),
    '#options' => array(
      'small' => t('Small'),
      'normal' => t('Normal'),
      'large' => t('Large'),
      'square_thumbnail' => t('Square Thumbnail'),
      'original' => t('Original')
    ),
    '#default_value' => isset($janrain_capture_fields['capture_preferred_photo_variant']) ? $janrain_capture_fields['capture_preferred_photo_variant'] : 'small',
    '#description' => t('Select the photo variant to use from the Capture profile.'),
  );
  $janrain_capture_optional = variable_get('janrain_capture_optional', array());
  $form['janrain_capture_optional'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced Settings (optional)'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['janrain_capture_optional']['capture_sso_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Single sign on URL'),
    '#default_value' => !empty($janrain_capture_optional['capture_sso_address']) ? $janrain_capture_optional['capture_sso_address'] : '',
    '#description' => t('For example, myapp.janrainsso.com.'),
  );
  $form['janrain_capture_optional']['backplane_server'] = array(
    '#type' => 'textfield',
    '#title' => t('Backplane server base URL'),
    '#default_value' => !empty($janrain_capture_optional['backplane_server']) ? $janrain_capture_optional['backplane_server'] : '',
    '#description' => t('For example, http://backplane1.janrainbackplane.com/v1.1.'),
  );
  $form['janrain_capture_optional']['backplane_bus_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Backplane bus name'),
    '#default_value' => !empty($janrain_capture_optional['backplane_bus_name']) ? $janrain_capture_optional['backplane_bus_name'] : '',
    '#description' => t('For example, domain.com.'),
  );
  $form['janrain_capture_optional']['backplane_js_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Backplane JavaScript URL'),
    '#default_value' => !empty($janrain_capture_optional['backplane_js_path']) ? $janrain_capture_optional['backplane_js_path'] : '',
    '#description' => t('The URL to your site\'s backplane.js file.'),
  );

  $form['janrain_capture_enforce'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enforce the use of Janrain Capture.'),
    '#default_value' => variable_get('janrain_capture_enforce', 0),
    '#description' => t('Make it impossible to login to this site outside of Janrain Capture. WARNING: DO NOT enable this setting until all of your user accounts have been migrated to Janrain Capture.'),
  );
  $form['#validate'] = array(
    'janrain_capture_settings_validate',
  );
  $form['#submit'] = array(
    'janrain_capture_settings_submit',
  );

  return system_settings_form($form);
}

/**
 * Validate callback for the settings form.
 */
function janrain_capture_settings_validate($form, &$form_state) {
  if ($form_state['values']['janrain_capture_enforce']) {
    // Do not allow the "enforce" setting to be set if the basic application
    // settings have not been set.
    if (empty($form_state['values']['janrain_capture_main']['capture_client_id']) ||
        empty($form_state['values']['janrain_capture_main']['capture_client_secret']) ||
        empty($form_state['values']['janrain_capture_main']['capture_address'])) {
      form_set_error('janrain_capture_enforce', t('You cannot enforce Janrain Capture for login because you have not configured the application settings. You must at least provide a Janrain Capture Client ID, Janrain Capture Client Secret and Janrain Capture Address'));
    }
    else {
      // Do not allow the current user to set the "enforce" property if they do
      // not have a Capture account themselves. Otherwise they could lock themselves
      // out of the site.
      $api = new JanrainCaptureApi();
      $profile = $api->loadUserEntity();

      if (!$profile) {
        form_set_error('janrain_capture_enforce', t('You cannot enforce Janrain Capture for login because you do not have a Janrain Capture account and would no longer be able to log into this site.'));
      }
    }
  }

  if ($form_state['values']['janrain_capture_fields']['capture_match_email'] && !$form_state['values']['janrain_capture_fields']['capture_enforce_verification']) {
    form_set_error('janrain_capture_fields][capture_match_email', t('You must enable the Enforce Email Verification setting in order to enable the Match Users on Email setting.'));
  }
}

/**
 * Submit callback for the settings form.
 */
function janrain_capture_settings_submit($form, &$form_state) {
  // Rebuild the menu in case the "enforce" setting changed.
  variable_set('menu_rebuild_needed', TRUE);
}
