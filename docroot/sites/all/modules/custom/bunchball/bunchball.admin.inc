<?php
/**
 * @file
 *    Main admin form for bunchball module.
 */

/**
 * Form callback for admin form.
 */
function bunchball_admin_form($form, &$form_state) {

  ctools_include('dependent');

  $form['#action'] = url('admin/config/people/bunchball/confirm');

  $links = array(
    '!content_actions' => l(t('content actions'), 'admin/config/people/bunchball/entities'),
    '!user_actions' => l(t('user actions'), 'admin/config/people/bunchball/user-interaction'),
  );
  $form['description'] = array(
    '#markup' => t('Connection to Bunchball Nitro requires a subscription. Once subscribed, use the values on the Nitro Admin console to complete the fields below to connect the service.  Then configure the !content_actions and !user_actions to track.', $links),
  );

  $form['bunchball_apikey'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($form_state['values']['bunchball_apikey']) ? $form_state['values']['bunchball_apikey'] : variable_get('bunchball_apikey', ''),
    '#title' => t('API Key'),
    '#required' => TRUE,
  );

  $form['bunchball_apisecret'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($form_state['values']['bunchball_apisecret']) ? $form_state['values']['bunchball_apisecret'] : variable_get('bunchball_apisecret', ''),
    '#title' => t('API Secret'),
    '#required' => TRUE,
  );

  $form['bunchball_admin_userid'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($form_state['values']['bunchball_admin_userid']) ? $form_state['values']['bunchball_admin_userid'] : variable_get('bunchball_admin_userid'),
    '#title' => t('Admin username'),
    '#required' => TRUE,
  );

  $form['bunchball_admin_password'] = array(
    '#type' => 'password',
    '#default_value' => isset($form_state['values']['bunchball_admin_password']) ? $form_state['values']['bunchball_admin_password'] : variable_get('bunchball_admin_password'),
    '#title' => t('Admin password'),
    '#preserve' => TRUE,
  );

  $form['bunchball_environment'] = array(
    '#type' => 'select',
    '#description' => t('Point to Sandbox for testing and Production for your live environment.'),
    '#options' => array(
      0 => t('Choose environment'),
      'production' => t('Production'),
      'sandbox' => t('Sandbox'),
    ),
    '#required' => TRUE,
    '#default_value' => isset($form_state['values']['bunchball_environment']) ? $form_state['values']['bunchball_environment'] : variable_get('bunchball_environment', ''),
  );

  $form['bunchball_production_url'] = array(
    '#title' => t('Production server url'),
    '#type' => 'textfield',
    '#default_value' => isset($form_state['values']['bunchball_production_url']) ? $form_state['values']['bunchball_production_url'] : variable_get('bunchball_production_url', ''),
    '#process' => array('ctools_dependent_process'),
    '#dependency' => array('edit-bunchball-environment' => array('production')),
    '#description' => t('The Buncball production URL. Most likely http://<domain>.bunchball.net/nitro/xml'),
  );

  $form['bunchball_sandbox_url'] = array(
    '#title' => t('Sandbox server url'),
    '#type' => 'textfield',
    '#default_value' => isset($form_state['values']['bunchball_sandbox_url']) ? $form_state['values']['bunchball_sandbox_url'] : variable_get('bunchball_sandbox_url', 'http://sandbox.bunchball.net/nitro/xml'),
    '#process' => array('ctools_dependent_process'),
    '#dependency' => array('edit-bunchball-environment' => array('sandbox')),
    '#description' => t('The Bunchball sandbox URL. Most likely http://sandbox.bunchball.net/nitro/xml'),
  );

  $form['bunchball_unique_id_fieldset'] = array(
    '#title' => t('User ID settings'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
  );

  $form['bunchball_unique_id_fieldset']['bunchball_unique_id_include_site_hash'] = array(
    '#type' => 'radios',
    '#options' => array(
      TRUE => t('Allow user tracking on a per-site basis'),
      FALSE => t('Allow user tracking across multiple sites'),
    ),
    '#default_value' => variable_get('bunchball_unique_id_include_site_hash', TRUE),
  );

  $unique_id_options = array(
    'userid' => t('Drupal userid'),
    'username' => t('Drupal username'),
    'email' => t('Email address'),
  );

  if (module_exists('janrain_capture')) {
    $unique_id_options['janrain_uuid'] = t('Janrain UUID');
  }

  $form['bunchball_unique_id_fieldset']['bunchball_unique_id'] = array(
    '#title' => t('Select an ID to identify users for Bunchball service'),
    '#type' => 'select',
    '#options' => $unique_id_options,
    '#default_value' => variable_get('bunchball_unique_id', 'email'),
  );

  if (module_exists('janrain_capture')) {
    unset($unique_id_options['janrain_uuid']);
    $form['bunchball_unique_id_fieldset']['bunchball_janrain_fallback'] = array(
      '#title' => t('Janrain UUID fallback'),
      '#type' => 'select',
      '#options' => $unique_id_options,
      '#default_value' => variable_get('bunchball_janrain_fallback', 'email'),
      '#states' => array(
        'visible' => array(
          ':input[name="bunchball_unique_id"]' => array('value' => 'janrain_uuid'),
        ),
      ),
      '#description' => t('This value will be used when the Janrain UUID is not availible for the user.'),
    );
  }

  if (_bunchball_is_configured()) {
    $form['bunchball_unique_id_fieldset']['warning'] = array(
      '#markup' => '<span style="color:red">' . t('Since gamification data is currently being tracked, any changes will reset gamification data for existing users because the Bunchball service identifies them as new users.') . '</span>',
    );
  }

  $form['bunchball_dev_options_fieldset'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Development and performance tuning options'),
  );

  $timeout_options = array();
  foreach(range(5, 60, 5) as $timeout) {
    $timeout_options[(float)$timeout] = t('@timeout sec', array('@timeout' => $timeout));
  }
  $form['bunchball_dev_options_fieldset']['bunchball_client_timeout'] = array(
    '#type' => 'select',
    '#options' => $timeout_options,
    '#title' => t('Client request timeout'),
    '#default_value' => isset($form_state['values']['bunchball_client_timeout']) ? $form_state['values']['bunchball_client_timeout'] : variable_get('bunchball_client_timeout', 10.0),
    '#description' => t('The default timeout for the API client requests.
      Setting this value too high can cause page requests to hang for the time
      specified in this value if the bunchball server is unresponsive.'),
  );

  $form['bunchball_dev_options_fieldset']['bunchball_logger_timeout'] = array(
    '#type' => 'select',
    '#options' => $timeout_options,
    '#title' => t('Client logging timeout'),
    '#default_value' => isset($form_state['values']['bunchball_logger_timeout']) ? $form_state['values']['bunchball_logger_timeout'] : variable_get('bunchball_logger_timeout', 10.0),
    '#description' => t('The default time for the API client to log an action.
      This value can be set lower than the client timeout.'),
  );

  $form['bunchball_dev_options_fieldset']['bunchball_debug_mode'] = array(
    '#type' => 'select',
    '#options' => array(
      BUNCHBALL_DEBUG_NONE => t('Off'),
      BUNCHBALL_DEBUG_LOG => t('Log only'),
      BUNCHBALL_DEBUG_LOG | BUNCHBALL_DEBUG_DISPLAY => t('Display and log'),
    ),
    '#title' => t('Debug mode'),
    '#default_value' => isset($form_state['values']['bunchball_debug_mode']) ? $form_state['values']['bunchball_debug_mode'] : variable_get('bunchball_debug_mode', 0),
    '#description' => t('Enabling debug mode will log messages sent to and from
      the Bunchball service. Select "Display and log" to show logging results to
      users.'),
  );

  $form['#submit'][] = 'bunchball_admin_form_submit';

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}

/**
 * Form submit handler for the bunchball admin form.
 *
 * Drops session cache in case the bunchball credentials are changed.
 */
function bunchball_admin_form_submit($form, &$form_state) {
  $api_key_changed = $form_state['values']['bunchball_apikey'] != variable_get('bunchball_apikey');
  $api_secret_changed = $form_state['values']['bunchball_apisecret'] != variable_get('bunchball_apisecret');
  $environment_changed = $form_state['values']['bunchball_environment'] != variable_get('bunchball_environment');
  $production_url_changed = $form_state['values']['bunchball_production_url'] != variable_get('bunchball_production_url');
  $sandbox_url_changed = $form_state['values']['bunchball_sandbox_url'] != variable_get('bunchball_sandbox_url');

  if ($api_key_changed || $api_secret_changed || $environment_changed || $production_url_changed || $sandbox_url_changed) {
    db_query('DELETE FROM {cache_bunchball_session}');
  }
}

/**
 * Page callback for viewing the bunchball event log.
 */
function bunchball_log_page() {
  $header = array(
    array('data' => t('ID'), 'field' => 'l.blid', 'sort' => 'desc'),
    array('data' => t('Date'), 'field' => 'l.timestamp'),
    t('Message'),
    array('data' => t('User'), 'field' => 'l.uid'),
  );

  $result = db_select('bunchball_log', 'l')
    ->extend('PagerDefault')
    ->extend('TableSort')
    ->fields('l')
    ->limit(50)
    ->orderByHeader($header)
    ->execute();

  $rows = array();
  foreach ($result as $row) {
    $rows[] = array('data' =>
      array(
        $row->blid,
        format_date($row->timestamp, 'short'),
        t($row->message, unserialize($row->variables)),
        $row->uid,
      ),
    );
  }

  $build = array();
  $build['bunchball_log_truncate'] = drupal_get_form('bunchball_log_truncate_form');
  $build['bunchball_log_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No log messages available.'),
  );

  $build['bunchball_log_pager'] = array('#theme' => 'pager');

  return $build;
}

function bunchball_log_truncate_form($form, &$form_state) {
  $form['truncate'] = array(
    '#type' => 'submit',
    '#value' => t('Clear logs'),
  );

  return $form;
}

function bunchball_log_truncate_form_submit($form, &$form_state) {
  db_query('TRUNCATE TABLE {bunchball_log}');
}

/**
 * Bunchball settings confirmation form.
 *
 * This form recieves info submitted by bunchball_admin_form().
 * On changing the unique id a confirmation form is rendered, else the values
 * are saved without any distraction.
 *
 * @see bunchball_admin_form()
 */
function bunchball_admin_confirm_form($form, &$form_state) {
  if (empty($form_state['input'])) {
    drupal_goto('admin/config/people/bunchball');
  }

  // If the IDs changed, show a confirmation form.
  $unique_id_changed = $form_state['input']['bunchball_unique_id'] != variable_get('bunchball_unique_id', 'email');
  $janrain_unique_id_changed = module_exists('janrain_capture') ?
    ($form_state['input']['bunchball_janrain_fallback'] != variable_get('bunchball_janrain_fallback')) :
    FALSE;
  if ($unique_id_changed || $janrain_unique_id_changed) {
    $f = array();
    $f['#submit'][] = 'system_settings_form_submit';

    foreach ($form_state['input'] as $variable_name => $value) {
      $f[$variable_name] = array(
        '#type' => 'hidden',
        '#value' => $value,
      );
    }

    return confirm_form($f,
      t('Are you sure you want to change the User ID settings?'),
      'admin/config/people/bunchball',
      t('Since gamification data is currently being tracked, saving changes to User ID settings will reset gamification data for existing users.'),
      t('Save'),
      t('Cancel')
    );
  }

  // IDs didn't change, save the values and redirect back to the form.
  $fake_form_state = array('values' => $form_state['input'], 'buttons' => array());
  if (!$form_state['input']['bunchball_admin_password']) {
    $fake_form_state['values']['bunchball_admin_password'] = variable_get('bunchball_admin_password');
  }
  system_settings_form_submit($form, $fake_form_state);

  drupal_goto('admin/config/people/bunchball');
}
