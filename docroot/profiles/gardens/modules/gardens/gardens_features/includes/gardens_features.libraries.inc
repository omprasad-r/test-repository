<?php
/**
 * @file
 * Provides functions for enabling/disabling JavaScript libraries.
 *
 * To allow the user to enable/disable JavaScript libraries via the UI we can't
 * simply add them using drupal_add_library. Instead we need to adjust the conf
 * variables used by the javascript_libraries module.
 */

/**
 * Enables JavaScript libraries.
 *
 * @param $libraries
 *   An array of JavaScript libraries in the format the javascript_libraries
 *   module expects.
 */
function gardens_features_javascript_libraries_enable($libraries) {
  // Get the currently enabled libraries.
  $enabled_libraries = variable_get('javascript_libraries_drupal_libraries', array());

  // Add the new libraries.
  foreach ($libraries as $library => $value) {
    $enabled_libraries[$library] = array(
      'library' => $value['library'],
      'module' => $value['module'],
    );
  }

  // Save the list of libraries.
  variable_set('javascript_libraries_drupal_libraries', $enabled_libraries);
}

/**
 * Disables JavaScript libraries.
 *
 * @param $libraries
 *   An array of JavaScript libraries (variable names) to disable.
 */
function gardens_features_javascript_libraries_disable($libraries) {
  // Get the currently enabled libraries.
  $enabled_libraries = variable_get('javascript_libraries_drupal_libraries', array());

  // Remove the libraries.
  foreach ($libraries as $library) {
    unset($enabled_libraries[$library]);
  }

  // Save the list of libraries.
  variable_set('javascript_libraries_drupal_libraries', $enabled_libraries);
}

