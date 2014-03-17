<?php

/**
 * @file
 * Gigya rules for Drupal.
 *
 * Development of this module is sponsered by Gigya, Inc.
 */

/**
 * Implements hook_rules_action_info().
 *
 * On behalf of the system module.
 */
function gigya_rules_action_info() {
  return array(
    'gigya_share_action' => array(
      'label' => t('Gigya show share ui action'),
      'group' => t('Gigya'),
      'parameter' => array(
        'gigya_shareui_title' => array(
          'label' => t('Share title'),
          'type' => 'text',
        ),
        'gigya_shareui_desc' => array(
          'label' => t('Share description'),
          'type' => 'text',
        ),
        'gigya_shareui_link_back' => array(
          'label' => t('Share link back'),
          'type' => 'text',
        ),
        'gigya_shareui_image_behavior' => array(
          'label' => t('Share image behavior'),
          'type' => 'token',
          'default value' => variable_get('gigya_share_image_behavior', 'default'),
          'options list' => 'gigya_rules_image_behavior_options',
          'restriction' => 'input',
        ),
        'gigya_shareui_image_url' => array(
          'label' => t('Share image URL'),
          'type' => 'uri',
          'default value' => variable_get('gigya_share_image_url', ''),
          'optional' => TRUE,

        ),
      ),
      'base' => 'gigya_show_share_ui_action',
    ),
  );
}

/**
 * Adds the js to show the share popup.
 *
 * @param string $title
 *   The share title.
 * @param string $desc
 *   The share description.
 * @param string $link_back
 *   A  url that share will link back to.
 * @param string $image_bhv
 *   The way that the we would take the image from.
 * @param strin $image_url
 *   The image url.
 */
function gigya_show_share_ui_action($title, $desc, $link_back, $image_bhv, $image_url) {
  $shareui_params = array(
    'linkBack' => $link_back,
    'title' => $title,
    'description' => $desc,
    'imageBhev' => $image_bhv,
    'scope' => variable_get('gigya_share_af_scope', 'external'),
    'privacy' => variable_get('gigya_activety_feed_privacy', 'private'),
  );
  if ($image_bhv == 'url' && !empty($image_url)) {
    $shareui_params['imageUrl'] = $image_url;
  }
  // Allow other module to alter parmeters.
  drupal_alter('gigya_shareui', $shareui_params);
  drupal_add_js(array('gigya' => array('shareUIParams' => $shareui_params)), 'setting');
}

/**
 * Returns the options for image behavior.
 * @return array
 *   An array of options for image behavior.
 */
function gigya_rules_image_behavior_options() {
  return array(
    'default' => t('Use image tag if exists, first image on post otherwise.'),
    'first' => t('First image on the post'),
    'url' => t('Specify an image url'),
  );
}
