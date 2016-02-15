<?php
/**
 * @file
 * Environment Libraries API Reference.
 *
 * It is recommended that you integrate environment_libraries into your existing set of environments
 * managed by a custom module specific to your deployment.
 * This module makes no serious attempt to manage environment configuration on it's own.
 */

/**
 * Override the list of environments to use for library files.
 *
 * @param array $environments
 *   The current set of environments used by environment_libraries.
 */
function hook_environment_libraries_environments_alter(array &$environments) {
  $environments = 'overridden environment list';
}

/**
 * Override the currently selected environment.
 *
 * @param $environment
 *   The current environment.
 */
function hook_environment_libraries_environment_current_alter(&$environment) {
  $environment = variable_get('overridden new current environment', 'prod');
}

/**
 * Implements hook_environment_libraries_library_save().
 *
 * @param $library
 *  An environment_library entity.
 */
function hook_environment_libraries_library_save($library) {
  // Do something when a library is saved.
}


// # Form Alter Hooks
/**
 * Implements hook_form_FORM_ID_alter().
 */
function hook_environment_libraries_list_alter(&$content) {
  $library_rows = &$content['environment_libraries_libraries_table']['#rows'];
}
