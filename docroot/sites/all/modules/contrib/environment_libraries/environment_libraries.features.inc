<?php
/**
 * @file
 * Features Integration
 *
 * Allow environment_libraries to be exported via features.
 */

/**
 * Implements hook_features_api().
 */
function environment_libraries_features_api() {
  return array(
    'environment_libraries_library' => array(
      'name' => 'Environment Libraries',
      'module' => 'environment_libraries',
      'default_file' => FEATURES_DEFAULTS_INCLUDED,
      'default_hook' => 'environment_libraries_default_library',
      'feature_source' => TRUE,
    ),
  );
}

/**
 * This hook will alert features of which specific items of this component may
 * be exported. For instances, in this case, we want to make available all the
 * existing items.  If there are no items to be exported, this component will
 * not be made available in the features export page.
 *
 * [component hook] Implements hook_features_export_options().
 *
 *
 * @return array
 *   A keyed array of items, suitable for use with a FormAPI select or checkboxes element.
 */
function environment_libraries_library_features_export_options() {
  $options = array();
  $libraries = environment_libraries_library_load_multiple();

  foreach ($libraries as $lid => $library) {
    $options[$library->machine_name] = $library->title;
  }
  return $options;
}


/**
 * [component hook] Implements hook_features_export().
 *
 * This is a component hook, rather then a module hook, therefore this is the
 * callback from hook_features_api which relates to the specific component we
 * are looking to export. When a specific instance of the component we are
 * looking to export is selected, this will include the necessariy item, plus
 * any dependencies into our export array.
 *
 * @param array $data
 *   This is the machine name for the component in question.
 * @param array &$export
 *   Array of all components to be exported.
 * @param string $module_name
 *   The name of the feature module to be generated.
 *
 * @return array
 *   The pipe array of further processors that should be called.
 */
function environment_libraries_library_features_export($data, &$export, $module_name) {
  // We have module dependencies in order for this module to function properly
  // so we'll add them here.
  $export['dependencies']['environment_libraries'] = 'environment_libraries';

  // The following is the simplest implementation of a straight object export
  // with no further export processors called.
  foreach ($data as $component) {
    $export['features']['environment_libraries_library'][$component] = $component;
  }
  return array();
}

/**
 * [component hook] Implements hook_features_export_render().
 *
 * Render one or more component objects to code.
 *
 * @param string $module_name
 *   The name of the feature module to be exported.
 * @param array $data
 *   An array of machine name identifiers for the objects to be rendered.
 * @param array $export
 *   The full export array of the current feature being exported. This is only
 *   passed when hook_features_export_render() is invoked for an actual feature
 *   update or recreate, not during state checks or other operations.
 *
 * @return array
 *   An associative array of rendered PHP code where the key is the name of the
 *   hook that should wrap the PHP code. The hook should not include the name
 *   of the module, e.g. the key for `hook_example` should simply be `example`.
 */
function environment_libraries_library_features_export_render($module_name, $data, $export = NULL) {
  $code = array();
  $code[] = '  $environment_libraries = array();';
  foreach ($data as $machine_name) {
    // Retrieve the library information
    $library = environment_libraries_library_load_by_machine_name($machine_name);
    // and add the library code to the feature.
    if (isset($library->machine_name)) {
      unset($library->lid);
      $code[] = '  $environment_libraries["' . $machine_name . '"] = ' . features_var_export($library, '  ') . ';';
    }
  }
  $code[] = '  return $environment_libraries;';
  $code = implode("\n", $code);
  return array('environment_libraries_default_library' => $code);
}

/**
 * [component hook] Implements hook_features_rebuild().
 */
function environment_libraries_library_features_rebuild($module) {
  $libraries = module_invoke($module, 'environment_libraries_default_library');
  // Loop over the items we need to recreate.
  foreach ($libraries as $i => $l) {
    $library = (object) $l;
    // If the machine name already exists then get it's lid.
    $existing_library = environment_libraries_library_load_by_machine_name($library->machine_name);
    if ($existing_library !== FALSE) {
      $library->lid = $existing_library->lid;
    }
    environment_libraries_library_save($library);
  }
}

/**
 * [component hook] Implements hook_features_revert().
 */
function environment_libraries_library_features_revert($module) {
  environment_libraries_library_features_rebuild($module);
}
