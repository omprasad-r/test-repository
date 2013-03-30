<?php

/*
 * Enable theming plugins.
 *
 * We don't want to simply call drupal_add_library in hook_init because
 * we want to grant site builders the option to turn these plugins off from the
 * javascript_libraries UI. So we turn them on once initially.
 */
function florida_hospital_misc_enable_theming_plugins($plugins) {
  // Get the already-configured enabled libraries.
  $javascript_libraries_config = variable_get('javascript_libraries_drupal_libraries', array());
  // Add the new libraries.
  foreach($plugins as $key => $value) {
    $javascript_libraries_config[$key] = array(
      'library' => $value['library'],
      'module' => $value['module'],
    );
  }
  variable_set('javascript_libraries_drupal_libraries', $javascript_libraries_config);
}
