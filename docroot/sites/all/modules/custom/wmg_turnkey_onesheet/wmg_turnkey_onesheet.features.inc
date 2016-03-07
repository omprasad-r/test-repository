<?php
/**
 * @file
 * wmg_turnkey_onesheet.features.inc
 */

/**
 * Implements hook_views_api().
 */
function wmg_turnkey_onesheet_views_api($module = NULL, $api = NULL) {
  return array("api" => "3.0");
}

/**
 * Implements hook_node_info().
 */
function wmg_turnkey_onesheet_node_info() {
  $items = array(
    'turnkey_onesheet' => array(
      'name' => t('Turnkey Onesheet'),
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
