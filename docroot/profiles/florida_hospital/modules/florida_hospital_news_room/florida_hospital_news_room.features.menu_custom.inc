<?php
/**
 * @file
 * florida_hospital_news_room.features.menu_custom.inc
 */

/**
 * Implements hook_menu_default_menu_custom().
 */
function florida_hospital_news_room_menu_default_menu_custom() {
  $menus = array();

  // Exported menu: menu-news-room
  $menus['menu-news-room'] = array(
    'menu_name' => 'menu-news-room',
    'title' => 'News Room',
    'description' => '',
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('News Room');


  return $menus;
}
