<?php
/**
 * @file
 * florida_hospital_general_sidebar_menus.features.inc
 */

/**
 * Implements hook_views_api().
 */
function florida_hospital_general_sidebar_menus_views_api() {
  list($module, $api) = func_get_args();
  if ($module == "views" && $api == "views_default") {
    return array("version" => "3.0");
  }
}
