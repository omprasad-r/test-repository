<?php
/**
 * @file
 * wmg_turnkey_lightbox.features.inc
 */

/**
 * Implements hook_views_api().
 */
function wmg_turnkey_lightbox_views_api($module = NULL, $api = NULL) {
  return array("api" => "3.0");
}

/**
 * Implements hook_node_info().
 */
function wmg_turnkey_lightbox_node_info() {
  $items = array(
    'wmg_turnkey_lightbox' => array(
      'name' => t('Turnkey Lightbox'),
      'base' => 'node_content',
      'description' => t('A simple configurable light-box content type.'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  drupal_alter('node_info', $items);
  return $items;
}
