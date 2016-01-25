<?php
/**
 * @file
 * turnkey_toaster.features.inc
 */

/**
 * Implements hook_views_api().
 */
function turnkey_toaster_views_api($module = NULL, $api = NULL) {
  return array("api" => "3.0");
}

/**
 * Implements hook_node_info().
 */
function turnkey_toaster_node_info() {
  $items = array(
    'turnkey_toaster' => array(
      'name' => t('Turnkey Toaster'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  drupal_alter('node_info', $items);
  return $items;
}
