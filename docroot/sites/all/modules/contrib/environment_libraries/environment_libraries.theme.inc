<?php
/**
 * @file
 * Functions related to theming environment_library entities & fields.
 */

// # Theme Functions
module_load_include('inc', 'environment_libraries', 'environment_libraries.class');
use Environment_Libraries\LibraryUtils;

/**
 * Implements hook_theme().
 */
function environment_libraries_theme($existing, $type, $theme, $path) {
  return array(
    'environment_libraries_library' => array(
      'function' => 'theme_environment_libraries_library',
    ),
    'environment_library' => array(
      'render element' => 'element',
      'arguments' => array('element' => NULL),
    ),
  );
}

/**
 * Load a library entity on a page.
 *
 * Implements theme_entity().
 *
 * @see https://www.drupal.org/node/1238606
 */
function theme_environment_libraries_library(&$variables) {
  // ### Render an environment_libraries_library entity
  $library = $variables['environment_libraries_library'];
  // If the library has files then load it,
  if (isset($library->config['files']) && count($library->config['files']) > 0) {
    // TODO test if we need to pass en every_page flag.
    environment_libraries_load($library->module ?: 'environment_libraries', $library->machine_name, $library->environment);
  }
  elseif (!empty($library->dependencies)) {
    // otherwise, just load it's dependencies;
    // this is allowed for use cases simply exposing libraries from
    // other modules / system.
    $dependencies = environment_libraries_get_dependencies($library);
    foreach ($dependencies as $dep) {
      environment_libraries_load($dep[0], $dep[1], $library->environment);
    }
  }
  // Don't render anything, we just want the library in preprocess.
  return '';
}

/**
 * Template function for an environment_library form element.
 *
 * @param array $variables
 *   Theme template variables.
 *
 * @return string
 *   Rendered environment library form element.
 */
function theme_environment_library(array &$variables) {
  $element = &$variables['element'];
  $library = NULL;
  if (!empty($element['#value'])) {
    $library = $element['#value'];
  }
  $environments = environment_libraries_environments();
  $current_environment = environment_libraries_environment_current($library);
  $id = $element['#id'];
  // Minification is an option if uglifyjs module is enabled.
  $_attributes = drupal_attributes($element['#attributes']);
  $file_count = $element['#file_count'];

  // ## Build the html for the field
  // ### Header
  $output = "<div id=\"$id\" $_attributes ><header>" . drupal_render($element['name']) . drupal_render($element['machine_name']) . drupal_render($element['environment']) . "</header>";
  // ### Files
  for ($f = 0; $f < $file_count; $f++) {
    $file = &$element['files'][$f];
    foreach ($environments as $env => $environment) {
      $cur = ($env === $current_environment) ? 'current' : '';
      $file[$env]['#prefix'] = "<div class=\"environment $env $cur\">";
      $file[$env]['#suffix'] = '</div>';
    }
    // TODO check array union of element_children($file) & $environments.
    $output .= drupal_render($file);
  }
  $output .= drupal_render($element['files']['add']);
  // ### Settings
  $output .= "<div class=\"environment-library-settings\">";
  $output .= drupal_render($element['dependencies']);
  $output .= drupal_render($element['expose_block']);
  $output .= drupal_render($element['description']);
  $output .= "</div></div>";
  return $output;
}
