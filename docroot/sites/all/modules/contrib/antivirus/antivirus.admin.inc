<?php

/**
 * @file
 * Administrative forms and functions of the Antivirus module.
 */

/**
 * Generates an administration form for configuring the Antivirus module.
 */
function antivirus_admin_form() {
  $form = array();

  $form['behavior'] = array(
    '#title' => t('Behavior'),
    '#type' => 'fieldset',
  );

  $form['behavior']['antivirus_action_error'] = array(
    '#title' => t('When a scan fails'),
    '#type' => 'radios',
    '#options' => array(
      ANTIVIRUS_FAIL_ERROR => t('Prevent upload'),
      ANTIVIRUS_FAIL_MSG => t('Allow upload and show an error'),
      ANTIVIRUS_FAIL_SILENT => t('Allow upload and do not show an error'),
    ),
    '#required' => TRUE,
    '#default_value' => variable_get('antivirus_action_error', ANTIVIRUS_FAIL_ERROR),
  );

  $form['behavior']['antivirus_notify'] = array(
    '#title' => t('When a virus is detected'),
    '#type' => 'radios',
    '#options' => array(
      ANTIVIRUS_NOTIFY_NONE => t('Do nothing'),
      ANTIVIRUS_NOTIFY_ADMIN => t('Notify administrators'),
    ),
    '#required' => TRUE,
    '#description' => t('This module relies on the !rules-module for advanced notifications. To send complex alerts when viruses are detected, you must configure Rules appropriately.', array('!rules-module' => l(t('Rules module'), 'http://drupal.org/project/rules'))),
    '#default_value' => variable_get('antivirus_notify', ANTIVIRUS_NOTIFY_NONE),
  );

  $form['behavior']['notifications'] = array(
    '#title' => t('Notifications'),
    '#type' => 'fieldset',
    '#states' => array(
      'visible' => array(
        ':input[name="antivirus_notify"]' => array('value' => ANTIVIRUS_NOTIFY_ADMIN),
      ),
      'required' => array(
        ':input[name="antivirus_notify"]' => array('value' => ANTIVIRUS_NOTIFY_ADMIN),
      ),
    ),
  );

  $form['behavior']['notifications']['antivirus_notify_email'] = array(
    '#title' => t('Notification e-mail addresses'),
    '#type' => 'textfield',
    '#description' => t('A comma-separated list of e-mail addresses to notify on virus detection.'),
    '#default_value' => variable_get('antivirus_notify_email', variable_get('site_mail', ini_get('sendmail_from'))),
  );

  $form['behavior']['notifications']['antivirus_notify_msg'] = array(
    '#title' => t('Notification message'),
    '#type' => 'textarea',
    '#description' => t('A comma-separated list of e-mail addresses to notify on virus detection.'),
    '#default_value' => variable_get('antivirus_notify_msg', ''),
  );

  $form['development'] = array(
    '#title' => t('Development'),
    '#type' => 'fieldset',
  );

  $form['development']['antivirus_debug'] = array(
    '#title' => t('Enable debugging'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('antivirus_debug', FALSE),
    '#description' => t('Allow scanners to produce more verbose output.'),
  );

  return system_settings_form($form);
}

/**
 * Validates the Antivirus administration form.
 */
function antivirus_admin_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['antivirus_notify_email'])) {
    // Validate and each e-mail recipient.
    $recipients = explode(',', $form_state['values']['antivirus_notify_email']);

    foreach ($recipients as &$recipient) {
      $recipient = trim($recipient);
      if (!valid_email_address($recipient)) {
        form_set_error('antivirus_notify_email', t('%recipient is an invalid e-mail address.', array('%recipient' => $recipient)));
      }
    }
  }
}

/**
 * Generates an administration form for managing antivirus scanners.
 */
function antivirus_admin_overview() {
  // Overview of all formats.
  $scanners = antivirus_get_scanners();

  $form['#tree'] = TRUE;
  foreach ($scanners as $id => $scanner) {
    if (!isset($scanner['hidden']) || !$scanner['hidden']) {
      $op = antivirus_scanner_is_enabled($id) ? 'disable' : 'enable';
      $rows[] = array(
        $scanner['name'],
        antivirus_scanner_is_enabled($id) ? t('Enabled') : t('Disabled'),
        l(t('configure'), "admin/config/system/antivirus/scanner/$id"),
        l($op, "admin/config/system/antivirus/scanner/$id/$op", array('query' => array('destination' => current_path()))),
      );
    }
  }

  $headers = array(t('Name'), t('Status'),
    array(
      'data' => t('Operations'),
      'colspan' => 2,
    ),
  );

  return theme('table', array('rows' => $rows, 'header' => $headers));
}

/**
 * Generates an administration form for configuring antivirus scanners.
 */
function antivirus_admin_scanner_form($form, $form_state, $scanner) {
  $scanners = antivirus_get_scanners();

  if (!isset($scanners[$scanner]) || (isset($scanners[$scanner]['configurable']) && !$scanners[$scanner]['configurable'])) {
    drupal_not_found();
  }

  $scanner_object = new $scanners[$scanner]['class'];

  $form['scanner_info'] = array(
    '#title' => t('Scanner information'),
    '#type' => 'fieldset',
  );

  // Allow the scanner to report its status.
  $messages = $scanner_object->verify();

  if (!empty($messages)) {
    foreach ($messages as $type => $list) {
      $form['scanner_info'][$type] = array(
        '#theme_wrappers' => array('container'),
        '#attributes' => array('class' => array('messages', $type)),
        'messages' => array(
          '#theme' => 'item_list',
          '#items' => $list,
        ),
      );
    }
  }

  // Scanners without a version (such as TestScanner) will return NULL here.
  if ($version = $scanner_object->getVersion()) {
    $form['scanner_info']['version'] = array(
      '#theme_wrappers' => array('container'),
      '#markup' => t('Current version: %version', array('%version' => $version)),
    );
  }

  $form['scanner_flags'] = array(
    '#title' => t('Executable flags'),
    '#type' => 'checkboxes',
    '#options' => array(),
    '#default_value' => variable_get("antivirus_scanner_${scanner}_flags", array()),
  );

  // Allow specific scanner to add to or modify the config form.
  $scanner_object->configure($form);

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Handles submission of the administration form for configuring scanners.
 */
function antivirus_admin_scanner_form_submit($form, $form_state) {
  $scanner = $form_state['build_info']['args'][0];
  $scanners = antivirus_get_scanners();

  $scanner_object = new $scanners[$scanner]['class'];
  variable_set("antivirus_scanner_${scanner}_flags", $form_state['values']['scanner_flags']);
  $scanner_object->save($form_state['values']);

  $scanner_object->status();
  drupal_set_message(t('Scanner settings saved.'));
  drupal_goto('admin/config/system/antivirus/scanner');
}

/**
 * Runs a manual scan on uploaded files.
 */
function antivirus_admin_run_scan() {
  $form = array();

  $form['scan_target'] = array(
    '#title' => t('Target'),
    '#type' => 'checkboxes',
    '#options' => array(
      'public' => t('Public files directory'),
      'temp' => t('Temporary files directory'),
    ),
    '#required' => TRUE,
    '#default_value' => array('public'),
  );

  // Add private directory if enabled.
  if (variable_get('file_private_path', FALSE)) {
    $form['scan_target']['#options']['private'] = t('Private files directory');
  }

  $form['scan'] = array(
    '#type' => 'submit',
    '#value' => t('Scan'),
  );

  return $form;
}

/**
 * Handles the submission of the manual scan form.
 */
function antivirus_admin_run_scan_submit($form, &$form_state) {
  $targets = array();

  $target_paths = array(
    'public' => 'public://',
    'private' => 'private://',
    'temp' => 'temporary://',
  );

  foreach ($form_state['values']['scan_target'] as $target => $value) {
    if ($value) {
      $targets[] = $target_paths[$target];
    }
  }

  antivirus_scan()->addTargets($targets)->scan();
}

/**
 * Generates a form for enabling a scanner.
 */
function antivirus_admin_scanner_enable_form($form, $form_state, $scanner) {
  if (antivirus_scanner_is_enabled($scanner)) {
    drupal_not_found();
  }

  $scanner_info = antivirus_get_scanners($scanner);
  return confirm_form($form, t('Are you sure you want to enable the %scanner scanner?', array('%scanner' => $scanner_info['name'])), 'admin/config/system/antivirus/scanner');
}

/**
 * Handles scanner enabling.
 */
function antivirus_admin_scanner_enable_form_submit($form, &$form_state) {
  antivirus_scanner_enable($form_state['build_info']['args'][0]);
}

/**
 * Generates a form for disabling a scanner.
 */
function antivirus_admin_scanner_disable_form($form, $form_state, $scanner) {
  if (!antivirus_scanner_is_enabled($scanner)) {
    drupal_not_found();
  }

  $scanner_info = antivirus_get_scanners($scanner);
  return confirm_form($form, t('Are you sure you want to disable the %scanner scanner?', array('%scanner' => $scanner_info['name'])), 'admin/config/system/antivirus/scanner');
}

/**
 * Handles scanner disabling.
 */
function antivirus_admin_scanner_disable_form_submit($form, &$form_state) {
  antivirus_scanner_disable($form_state['build_info']['args'][0]);
}

/**
 * Generates the title for a particular scanner's settings page.
 */
function _antivirus_scanner_get_title($scanner) {
  $scanners = antivirus_get_scanners();

  if (isset($scanners[$scanner])) {
    return t('@scanner-name settings', array('@scanner-name' => $scanners[$scanner]['name']));
  }
}

