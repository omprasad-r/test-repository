<?php

/**
 * @file
 * Audit trail related drush commands.
 */

/**
 * Implements of hook_drush_command().
 */
function audit_trail_drush_command() {
  $items = array();

  $items['audit-trail-manage'] = array(
    'description' => 'Manages tracking for a form.',
    'arguments' => array(
      'tracking_action' => dt("Either 'track' or 'untrack'."),
      'tracking_type' => dt("Specify what should be tracked on the form. For form changes enter 'changes'. For whole form submissions enter 'submissions'."),
      'pattern_type' => dt("Specify how the form to be tracked is identified. If the regexp should match the form id then enter 'form'. If the regexp should match a path then enter 'path'."),
      'pattern' => dt('Comma separated regexp patterns to find form(s) to be tracked.'),
    ),
    'required-arguments' => TRUE,
    'examples' => array(
      'Track changes on role administration forms.' => "drush audit-trail-manage track changes form '/^user_admin_roles$/,/^user_admin_role$/'",
      'Track submissions on role deletion form.' => "drush audit-trail-manage track submissions form '/^user_admin_role_delete_confirm$/'",
      'Stop tracking changes on all forms on /admin pages.' => "drush audit-trail-manage untrack changes page '|^admin/.*|'",
    ),
  );

  $items['audit-trail-stop-tracking'] = array(
    'description' => 'Stops tracking.',
    'arguments' => array(
      'tracking_type' => dt("Specify which type of tracking should be stopped. Leave it empty to remove both change and submission tracking. For form changes enter 'changes'. For whole form submissions enter 'submissions'."),
      'pattern_type' => dt("Enter 'form' to stop tracking on forms identified by form ids. Enter 'path' to stop tracking on forms identified by the path they appear on. Leave this empty to remove both of these."),
    ),
    'examples' => array(
      'Stop all tracking on the site.' => 'drush audit-trail-stop-tracking',
      'Stop tracking all form changes on the site.' => 'drush audit-trail-stop-tracking changes',
      'Stop tracking all path specific form submissions on the site.' => 'drush audit-trail-stop-tracking submissions path',
    ),
  );

  return $items;
}

/**
 * Handles audit-trail-manage - adds or removes tracking for a form.
 */
function drush_audit_trail_manage($tracking_action, $tracking_type, $pattern_type, $input_pattern) {
  module_load_include('inc', 'audit_trail', 'audit_trail.admin');
  $patterns = audit_trail_patterns_get($input_pattern, ',');
  if (!in_array($tracking_action, array('track', 'untrack'))) {
    drush_print(dt('Invalid tracking action has been specified. The first argument needs to be either track or untrack.'));
  }
  elseif (!in_array($tracking_type, array('changes', 'submissions'))) {
    drush_print(dt('Invalid tracking type has been specified. The second argument needs to be either changes or submissions.'));
  }
  elseif (!in_array($pattern_type, array('form', 'path'))) {
    drush_print(dt('Invalid pattern type has been specified. The third argument needs to be either form or path.'));
  }
  elseif ($invalid_patterns = audit_trail_admin_invalid_patterns_get($patterns)) {
    drush_print(dt('Invalid pattern(s) found: @pattern.', array('@pattern' => implode(', ', $invalid_patterns))));
  }
  else {
    _audit_trail_admin_tracking_manage($tracking_action, $tracking_type, $pattern_type, $patterns);
  }
}

/**
 * Stops form change and / or submission tracking.
 */
function drush_audit_trail_stop_tracking($tracking_type = '', $pattern_type = '') {
  if ($tracking_type && !in_array($tracking_type, array('changes', 'submissions'))) {
    drush_print(dt('Invalid tracking type has been specified. The first argument needs to be either changes or submissions.'));
  }
  elseif ($pattern_type && !in_array($pattern_type, array('form', 'path'))) {
    drush_print(dt('Invalid pattern type has been specified. The second argument needs to be either form or path.'));
  }
  else {
    $variable_fragment_tracking_types = $tracking_type ? array($tracking_type) : array('changes', 'submissions');
    if ($pattern_type) {
      $variable_fragment_pattern_types = $pattern_type == 'path' ? array('_path') : array('');
    }
    else {
      $variable_fragment_pattern_types = array('_path', '');
    }
    foreach ($variable_fragment_tracking_types as $variable_fragment_tracking_type) {
      foreach ($variable_fragment_pattern_types as $variable_fragment_pattern_type) {
        // Possible variables affected: audit_trail_form_changes,
        // audit_trail_form_changes_path, audit_trail_form_submissions
        // audit_trail_form_submissions_path.
        variable_del('audit_trail_form_' . $variable_fragment_tracking_type . $variable_fragment_pattern_type);
      }
    }
  }
}
