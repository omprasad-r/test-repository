<?php
/**
 * @file
 * site_guard module's admin funcions.
 */

/**
 * Form for enabling/disabling Site Guard
 */
function site_guard_toggle_form($form, &$form_state) {
  $form['description'] = array(
    '#type' => 'item',
    '#title' => t('Site Guard settings'),
    '#description' => t('Site Guard prevents crawlers and prying eyes from viewing content on your site.'),
  );

  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable'),
    '#default_value' => module_exists('shield'),
  );

  $form['credentials'] = array(
    '#type' => 'fieldset',
    '#title' => t('Credentials'),
    '#description' => t('Specify the username and password to gain access to the protected site'),
    '#states' => array(
      'visible' => array(
        ':input[name="enabled"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['credentials']['shield_user'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#required' => TRUE,
    '#default_value' => variable_get('shield_user', ''),
  );

  $form['credentials']['shield_pass'] = array(
    '#type' => 'textfield',
    '#title' => t('Password'),
    '#required' => TRUE,
    '#default_value' => variable_get('shield_pass', ''),
  );

  $form['auth_message'] = array(
    '#type' => 'fieldset',
    '#title' => t('Authentication Message'),
    '#description' => t('The message for the authentication request popup.'),
    '#states' => array(
      'visible' => array(
        ':input[name="enabled"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['auth_message']['shield_print'] = array(
    '#type' => 'textfield',
    '#title' => t('Message body'),
    '#description' => t("You can use [user] and [pass] to print the user and the password respectively, or you can leave it empty. For example, \"You cannot access this site. Please contact your system administrator for access.\""),
    '#default_value' => variable_get('shield_print', 'Hello, user: [user], pass: [pass]!'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit')
  );
  return $form;
}

/**
 * Submit callback for the site_guard toggle form.
 */
function site_guard_toggle_form_submit($form, &$form_state) {
  if ($form_state['values']['enabled']) {
    site_guard_enable_site_guard($form_state['values']['shield_user'], $form_state['values']['shield_pass'], $form_state['values']['shield_print']);
  }
  else {
    site_guard_disable_site_guard();
  }
}
