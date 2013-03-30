<?php

/**
 * @file
 * Audit trail related administration pages.
 */

/**
 * Audit trail configuration form.
 */
function audit_trail_admin_configure($form, $form_state) {
  $form = array();

  $form['audit_trail_log_destination'] = array(
    '#type' => 'radios',
    '#title' => t('Log destination'),
    '#options' => array(
      'watchdog' => t('Watchdog'),
      'file' => t('File'),
    ),
    '#default_value' => variable_get('audit_trail_log_destination', 'watchdog'),
  );
  $form['audit_trail_log_file'] = array(
    '#type' => 'textfield',
    '#title' => t('Logfile location'),
    '#description' => t('If the logs are written to a file then specify the path to the file.'),
    '#default_value' => variable_get('audit_trail_log_file', 'temporary://drupal_audit_trail.log'),
    '#states' => array(
      'visible' => array(
        ':input[name="audit_trail_log_destination"]' => array('value' => 'file'),
      ),
    ),
  );

  $form['form_changes'] = array(
    '#type' => 'fieldset',
    '#title' => t('Track form changes'),
    '#description' => t('Specify which forms need to be tracked for changes.'),
  );
  $form['form_changes']['audit_trail_form_changes'] = array(
    '#type' => 'textarea',
    '#title' => t('Form ID patterns'),
    '#description' => t('Add one PHP regular expression per line that will match a form id. e.g. "/^.*node_form$/" without the quotes.'),
    '#default_value' => variable_get('audit_trail_form_changes', ''),
  );
  $form['form_changes']['audit_trail_form_changes_path'] = array(
    '#type' => 'textarea',
    '#title' => t('Path patterns'),
    '#description' => t('Add one PHP regular expression per line that will match a path. e.g. "|^admin.*|" without the quotes.'),
    '#default_value' => variable_get('audit_trail_form_changes_path', ''),
  );

  $form['form_submits'] = array(
    '#type' => 'fieldset',
    '#title' => t('Track form submissions'),
    '#description' => t('Specify the forms where submissions need to be tracked. A good example for tracking whole form submissions would be deletions, where the user would barely change anything on the form.'),
  );
  $form['form_submits']['audit_trail_form_submissions'] = array(
    '#type' => 'textarea',
    '#title' => t('Form ID patterns'),
    '#description' => t('Add one PHP regular expression per line that will match a form id. e.g. "/^.*node_form$/" without the quotes.'),
    '#default_value' => variable_get('audit_trail_form_submissions', ''),
  );
  $form['form_submits']['audit_trail_form_submissions_path'] = array(
    '#type' => 'textarea',
    '#title' => t('Path patterns'),
    '#description' => t('Add one PHP regular expression per line that will match a path. e.g. "|^admin.*|" without the quotes.'),
    '#default_value' => variable_get('audit_trail_form_submissions_path', ''),
  );

  $form['audit_trail_separator'] = array(
    '#type' => 'textfield',
    '#title' => t('Log component separator'),
    '#description' => t('The string to be used when joining the components of the audit trail logs.'),
    '#default_value' => variable_get('audit_trail_separator', ','),
  );

  $form['audit_trail_form_management_link'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Form tracking management link'),
    '#description' => t('Shows a link to start/stop tracking form changes or submissions.'),
    '#options' => array(
      'admin' => t('Show on admin page forms'),
      'nonadmin' => t('Show on non admin page forms'),
    ),
    '#default_value' => variable_get('audit_trail_form_management_link', array()),
  );

  return system_settings_form($form);
}

/**
 * Validates audit trail configuration.
 */
function audit_trail_admin_configure_validate($form, $form_state) {
  $variable_names = array(
    'audit_trail_form_changes',
    'audit_trail_form_changes_path',
    'audit_trail_form_submissions',
    'audit_trail_form_submissions_path',
  );
  foreach ($variable_names as $variable_name) {
    // Need to validate the regexps to ensure they are not going to throw
    // notices.
    $patterns = audit_trail_patterns_get($form_state['values'][$variable_name]);
    $invalid_patterns = audit_trail_admin_invalid_patterns_get($patterns);
    if ($invalid_patterns) {
      form_set_error($variable_name, t('The following regexps can not be used to match forms: @regexps.', array('@regexp' => implode(', ', $invalid_patterns))));
    }
  }
}

/**
 * Validates an array of patterns and returns invalid items.
 *
 * @param array $patterns
 *   Array of patterns.
 *
 * @return array
 *   Array of invalid patterns or an empty array.
 */
function audit_trail_admin_invalid_patterns_get($patterns) {
  $invalid_patterns = array();
  foreach ($patterns as $pattern) {
    // We are gagging preg_match to avoid a notice thrown on bad patterns.
    $preg_match_result = @preg_match($pattern, '');
    if ($preg_match_result === FALSE) {
      $invalid_patterns[] = $pattern;
    }
  }
  return $invalid_patterns;
}

/**
 * Manages form trackings with menu callback.
 *
 * @param string $action
 *   One of: track-submissions, track-changes, untrack-submissions,
 *   untrack-changes.
 * @param string $form_id
 *   The form to apply the $action on.
 */
function audit_trail_admin_tracking_manage($action, $form_id) {
  $allowed_actions = array(
    'track-submissions',
    'track-changes',
    'untrack-submissions',
    'untrack-changes',
  );
  if (in_array($action, $allowed_actions) && isset($_REQUEST['token']) && drupal_valid_token($_REQUEST['token'], 'audit-trail-' . $action . '-' . $form_id)) {
    $action_fragments = explode('-', $action);
    // One of: 'track' or 'untrack'.
    $tracking_action = $action_fragments[0];
    // One of: 'submissions' or 'changes'.
    $tracking_type = $action_fragments[1];

    // Form tracking pattern to be managed.
    $form_pattern = '/^' . $form_id . '$/';

    // Manage tracking.
    _audit_trail_admin_tracking_manage($tracking_action, $tracking_type, 'form', array($form_pattern));

    // Go back where we came from.
    drupal_goto();
  }
  else {
    drupal_access_denied();
  }
}

/**
 * Helper function to manage form tracking.
 *
 * @param string $tracking_action
 *   One of: track, untrack. Specifies if the pattern needs to be added or
 *   removed.
 * @param string $tracking_type
 *   One of: changes, submissions. Specifies what kind of tracking needs to be
 *   added or removed for the form.
 * @param string $pattern_type
 *   One of: form, path. Specifies what the pattern should be matched against,
 *   form ids or paths.
 * @param array $input_patterns
 *   An array of patterns to be added or removed to the already stored patterns.
 */
function _audit_trail_admin_tracking_manage($tracking_action, $tracking_type, $pattern_type, $input_patterns) {
  // For grepping purposes:
  // audit_trail_form_changes audit_trail_form_changes_path
  // audit_trail_form_submissions audit_trail_form_submissions_path
  $variable_name = 'audit_trail_form_' . $tracking_type . ($pattern_type == 'path' ? '_path' : '');
  $stored_patterns = array_map('trim', explode("\n", variable_get($variable_name, '')));
  $patterns_changed = FALSE;

  // Add tracking - if it is already present then should not add it again
  // (like someone double clicked the link).
  foreach ($input_patterns as $pattern) {
    if ($tracking_action == 'track' && !in_array($pattern, $stored_patterns)) {
      $stored_patterns[] = $pattern;
      $patterns_changed = TRUE;
    }
    if ($tracking_action == 'untrack' && in_array($pattern, $stored_patterns)) {
      $stored_patterns = array_diff($stored_patterns, array($pattern));
      $patterns_changed = TRUE;
    }
  }

  // Save if necessary.
  if ($patterns_changed) {
    variable_set($variable_name, implode("\n", $stored_patterns));
  }
}
