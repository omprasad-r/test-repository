<?php

/**
 * @file
 * Admin settings form for Capture version
 */

/**
 * Sets the Capture version.
 */
function janrain_capture_version_settings() {
  $form['description'] = array(
    '#markup' => '<p>' . t('Connection to Janrain Capture requires a subscription. Once subscribed, use the values on the Janrain dashboard to complete the fields below to connect the service.') . '</p>',
  );
  $janrain_capture_ver = variable_get('janrain_capture_ver', '1.0');

  $form['janrain_capture_ver'] = array(
    '#type' => 'select',
    '#title' => t('Capture Version'),
    '#options' => array(
      '2.0' => t('Capture'),
      '1.0' => t('Capture Legacy'),
    ),
    '#default_value' => !empty($janrain_capture_ver) ? $janrain_capture_ver : '1.0',
    '#description' => t('Which version of capture are you using? Chose and save before configuring. Settings below will change accordingly and are stored separately.'),
  );

  return system_settings_form($form);
}

