<?php
/**
 * @file
 * Gigya fields from Drupal.
 */

/**
 * Implements hook_field_info().
 */
function gigya_field_info() {
  return array(
    'gigya_sharebar' => array(
      'label' => t('Gigya Share bar'),
      'description' => t('Gigya Share bar field'),
      'default_formatter' => 'gigya_sharebar_formatter_default',
      'default_widget' => 'sharebar',
    ),
    'gigya_reactions' => array(
      'label' => t('Gigya Reactions'),
      'description' => t('Gigya Reactions field'),
      'default_formatter' => 'gigya_reactions_formatter_default',
      'default_widget' => 'reactions',
    ),
    'gigya_ratings' => array(
      'label' => t('Gigya Ratings'),
      'description' => t('Gigya Rating field'),
      'default_formatter' => 'gigya_ratings_formatter_default',
      'default_widget' => 'ratings',
    ),
  );
}

/**
 * Implements hook_form_alter().
 */
function gigya_form_field_ui_field_edit_form_alter(&$form, $form_state) {
  $field = $form['#field'];
  $instance = $form['#instance'];
  if ($field['type'] == 'gigya_sharebar' || $field['type'] == 'gigya_reactions' || $field['type'] == 'gigya_reactions') {
    // Multiple values is not supported.
    $form['field']['cardinality']['#access'] = FALSE;
    $form['field']['cardinality']['#value'] = 1;
    // No need and point to set a default value.
    $form['instance']['default_value_widget']['#access'] = FALSE;
    $form['instance']['required']['#access'] = FALSE;
    $form['instance']['description']['#access'] = FALSE;
  }
}

/**
 * Implements hook_field_is_empty().
 */
function gigya_field_is_empty($item, $field) {
  return FALSE;
}

/**
 * Implements hook_field_settings_form().
 */
function gigya_field_settings_form($field, $instance) {
  $form = array();
  return $form;
}

/**
 * Implements hook_field_settings_form().
 */
function gigya_field_instance_settings_form($field, $instance) {
  $form = array();
  $form['message'] = array(
    '#markup' => t('All settings are in the display fields tab'),
  );
  return $form;
}

/**
 * Implements hook_field_widget_info().
 */
function gigya_field_widget_info() {
  $widgets = array();
  $widgets['sharebar'] = array(
    'label' => t('Gigya Share bar'),
    'field types' => array('gigya_sharebar'),
    'behaviors' => array('multiple values' => FIELD_BEHAVIOR_NONE),
  );
  $widgets['reactions'] = array(
    'label' => t('Gigya Reactions'),
    'field types' => array('gigya_reactions'),
    'behaviors' => array('multiple values' => FIELD_BEHAVIOR_NONE),
  );
  $widgets['ratings'] = array(
    'label' => t('Gigya Ratings'),
    'field types' => array('gigya_ratings'),
    'behaviors' => array('multiple values' => FIELD_BEHAVIOR_NONE),
  );
  return $widgets;
}


/**
 * Implements hook_field_formatter_info().
 */
function gigya_field_formatter_info() {
  return array(
    'gigya_sharebar_formatter_default' => array(
      'label' => t('Gigya Share bar'),
      'field types' => array('gigya_sharebar'),
      'settings' => _gigya_field_info_formatter_default_settings('gigya_sharebar_formatter_default'),
    ),
    'gigya_reactions_formatter_default' => array(
      'label' => t('Gigya Reactions'),
      'field types' => array('gigya_reactions'),
      'settings' => _gigya_field_info_formatter_default_settings('gigya_reactions_formatter_default'),
    ),
    'gigya_ratings_formatter_default' => array(
      'label' => t('Gigya Ratings'),
      'field types' => array('gigya_ratings'),
      'settings' => array(
        'gigya_ratings_container_id' => 'gigya-ratings',
        'gigya_ratings_category_id' => '',
        'gigya_ratings_show_read' => TRUE,
        'gigya_ratings_show_write' => TRUE,
        'gigya_ratings_linked_comments' => '',
      ),
    ),
  );
}

/**
 * Returns an array default settings for the specified formatter type.
 *
 * Allows dynamic retrieval of global settings bypassing the field info cache
 * that would be used in field_info_formatter_settings($type).
 *
 * @param string $formatter_type
 *   The type of the formatter.
 *
 * @return array
 *   An array of formatter settings.
 */
function _gigya_field_info_formatter_default_settings($formatter_type) {
  $settings = array();
  switch ($formatter_type) {
    case 'gigya_sharebar_formatter_default':
      $settings = array(
        'gigya_sharebar_container_id' => variable_get('gigya_share_container_id', 'gigya-share'),
        'gigya_sharebar_buttons' => variable_get('gigya_share_buttons', GIGYA_DEFAULT_SHARE_BOTTONS),
        'gigya_sharebar_layout' => variable_get('gigya_share_layout', 'horizontal'),
        'gigya_sharebar_showcounts' => variable_get('gigya_share_showcounts', 'right'),
        'gigya_sharebar_title' => variable_get('gigya_share_title', '[node:title]'),
        'gigya_sharebar_desc' => variable_get('gigya_share_desc', '[node:summary]'),
        'gigya_sharebar_link_back' => variable_get('gigya_share_link_back', '[node:url:absolute]'),
        'gigya_sharebar_image_behavior' => variable_get('gigya_share_image_behavior', 'default'),
        'gigya_sharebar_image_url' => variable_get('gigya_share_image_url', ''),
        'gigya_sharebar_short_url' => variable_get('gigya_share_short_url', FALSE),
        'gigya_sharebar_advanced' => variable_get('gigya_share_advanced', ''),
        'gigya_sharebar_override_default' => FALSE,
      );
      break;

    case 'gigya_reactions_formatter_default':
      $settings = array(
        'gigya_reactions_bar_id' => variable_get('gigya_reactions_bar_id', 'reactions-bar'),
        'gigya_reactions_container_id' => variable_get('gigya_reactions_container_id', 'gigya-reactions'),
        'gigya_reactions_title' => variable_get('gigya_reactions_title', '[node:title]'),
        'gigya_reactions_link_back' => variable_get('gigya_reactions_link_back', '[node:url:absolute]'),
        'gigya_reactions_buttons' => NULL,
        'gigya_reactions_show_counts' => variable_get('gigya_reactions_show_counts', 'right'),
        'gigya_reactions_layout' => variable_get('gigya_reactions_layout', 'horizontal'),
        'gigya_reactions_multiple_reactions' => variable_get('gigya_reactions_multiple_reactions', TRUE),
        'gigya_reactions_image_behavior' => variable_get('gigya_reactions_image_behavior', 'default'),
        'gigya_reactions_image_url' => variable_get('gigya_reactions_image_url', ''),
        'gigya_reactions_advanced' => variable_get('gigya_reactions_advanced', ''),
        'gigya_reactions_count_type' => variable_get('gigya_reactions_count_type', 'number'),
        'gigya_reactions_override_default' => FALSE,
      );
      break;
  }

  return $settings;
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function gigya_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  $form = array();
  if ($field['type'] == 'gigya_sharebar') {
    $form['gigya_sharebar_override_default'] = array(
      '#type' => 'checkbox',
      '#title' => t('Override default settings'),
      '#default_value' => isset($settings['gigya_sharebar_override_default']) ? $settings['gigya_sharebar_override_default'] : FALSE,
      '#attributes' => array('class' => array('sharebar-override')),
    );
    $form['gigya_sharebar_container_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Container ID'),
      '#size' => 60,
      '#prefix' => '<div class="gigya-sharebar-field-settings">',
      '#default_value' => isset($settings['gigya_sharebar_container_id']) ? $settings['gigya_sharebar_container_id'] : variable_get('gigya_share_container_id', 'gigya-share'),
    );
    $form['gigya_sharebar_buttons'] = array(
      '#type' => 'textfield',
      '#title' => t('Providers'),
      '#size' => 160,
      '#maxlength' => 400,
      '#default_value' => isset($settings['gigya_sharebar_buttons']) ? $settings['gigya_sharebar_buttons'] : variable_get('gigya_share_buttons', GIGYA_DEFAULT_SHARE_BOTTONS),
    );
    $form['gigya_sharebar_layout'] = array(
      '#type' => 'select',
      '#title' => t('Sharebar layout'),
      '#options' => array(
        'horizontal' => t('Horizontal'),
        'vertical' => t('Vertical'),
      ),
      '#default_value' => isset($settings['gigya_sharebar_layout']) ? $settings['gigya_sharebar_layout'] : variable_get('gigya_sharebar_layout', 'horizontal'),
    );
    $form['gigya_sharebar_showcounts'] = array(
      '#type' => 'select',
      '#title' => t('Show counts'),
      '#description' => t('Where to display the share counts in relation to the share button'),
      '#options' => array(
        'right' => t('Right'),
        'top' => t('Top'),
        'none' => t('none'),
      ),
      '#default_value' => isset($settings['gigya_sharebar_showcounts']) ? $settings['gigya_sharebar_showcounts'] : variable_get('gigya_sharebar_showcounts', 'right'),
    );
    $form['gigya_sharebar_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Share title'),
      '#size' => 60,
      '#default_value' => !empty($settings['gigya_sharebar_title']) ? $settings['gigya_sharebar_title'] : variable_get('gigya_share_title', '[node:title]'),
    );
    $form['gigya_sharebar_desc'] = array(
      '#type' => 'textfield',
      '#title' => t('Description'),
      '#size' => 60,
      '#default_value' => !empty($settings['gigya_sharebar_desc']) ? $settings['gigya_sharebar_desc'] : variable_get('gigya_share_desc', '[node:summary]'),
    );
    $form['gigya_sharebar_link_back'] = array(
      '#type' => 'textfield',
      '#title' => t('Linkback URL'),
      '#size' => 60,
      '#default_value' => !empty($settings['gigya_sharebar_link_back']) ? $settings['gigya_sharebar_link_back'] : variable_get('gigya_share_link_back', '[node:url:absolute]'),
    );
    $form['gigya_sharebar_image_behavior'] = array(
      '#type' => 'radios',
      '#title' => t('Image'),
      '#options' => array(
        'default' => t('Use image tag if exists, first image on post otherwise.'),
        'first' => t('First image on the post'),
        'url' => t('Specify an image URL'),
      ),
      '#default_value' => !empty($settings['gigya_sharebar_image_behavior']) ? $settings['gigya_sharebar_image_behavior'] : variable_get('gigya_share_image_behavior', 'default'),
      '#attributes' => array('class' => array('image-behv')),
    );
    $form['gigya_sharebar_image_url'] = array(
      '#type' => 'textfield',
      '#title' => t('The URL of the image to share'),
      '#size' => 20,
      '#states' => array(
        'visible' => array(
          '.image-behv' => array('value' => 'url'),
        ),
      ),
      '#default_value' => !empty($settings['gigya_sharebar_image_url']) ? $settings['gigya_sharebar_image_url'] : variable_get('gigya_share_image_url', ''),
    );
    $form['gigya_sharebar_short_url'] = array(
      '#type' => 'checkbox',
      '#title' => t('Share using short URLs'),
      '#default_value' => !empty($settings['gigya_sharebar_short_url']) ? $settings['gigya_sharebar_short_url'] : variable_get('gigya_share_short_url', FALSE),
    );
    $adv_parmas = !empty($settings['gigya_sharebar_advanced']) ? $settings['gigya_sharebar_advanced'] : variable_get('gigya_share_advanced', '');
    $field_name = $field['field_name'];
    $form['gigya_sharebar_advanced'] = array(
      '#type' => 'textarea',
      '#title' => t('Advanced showSharebarBarUI settings for @field', array('@field' => $field_name)),
      '#element_validate' => array('gigya_key_val_filed_validate'),
      '#default_value' => !empty($adv_parmas) ? gigya_values_array_to_string($adv_parmas) : '',
      '#description' => t('Use this textbox to enter your own settings. If you are not using this box, make sure it is empty. </br>See list of available !parameters', array(
          '!parameters' => l(t('parameters'), 'http://developers.gigya.com/020_Client_API/020_Methods/socialize.showShareBarUI#ShareButton_Object', array(
              'external' => TRUE,
              'attributes' => array('target' => '_blank'),
            )
          ),
        )
      ),
    );
    $form['tokens_list'] = array(
      '#markup' => theme('token_tree', array('token_types' => array('node'), 'global_types' => FALSE)),
      '#suffix' => '</div>',
    );
  }

  // Reaction settings.
  if ($field['type'] == 'gigya_reactions') {
    $form['gigya_reactions_override_default'] = array(
      '#type' => 'checkbox',
      '#title' => t('Override default settings'),
      '#default_value' => isset($settings['gigya_reactions_override_default']) ? $settings['gigya_reactions_override_default'] : FALSE,
      '#attributes' => array('class' => array('reactions-override')),
    );
    $form['gigya_reactions_container_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Container ID'),
      '#size' => 20,
      '#prefix' => '<div class="gigya-reaction-field-settings">',
      '#default_value' => isset($settings['gigya_reactions_container_id']) ? $settings['gigya_reactions_container_id'] : 'gigya-reactions',
    );
    $form['gigya_reactions_bar_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Bar ID'),
      '#size' => 20,
      '#default_value' => isset($settings['gigya_reactions_bar_id']) ? $settings['gigya_reactions_bar_id'] : $instance['field_name'],
    );
    $form['gigya_reactions_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Reaction title'),
      '#size' => 60,
      '#default_value' => !empty($settings['gigya_reactions_title']) ? $settings['gigya_reactions_title'] : '[node:title]',
    );
    $form['gigya_reactions_link_back'] = array(
      '#type' => 'textfield',
      '#title' => t('Linkback URL'),
      '#size' => 60,
      '#default_value' => !empty($settings['gigya_rieactions_link_back']) ? $settings['gigya_reactions_link_back'] : '[node:url:absolute]',
    );
    $form['gigya_reactions_show_counts'] = array(
      '#type' => 'select',
      '#title' => t('Show counts'),
      '#options' => array(
        'right' => t('Right'),
        'top' => t('Top'),
        'none' => t('None'),
      ),
      '#default_value' => !empty($settings['gigya_reactions_show_counts']) ? $settings['gigya_reactions_show_counts'] : 'right',
    );
    $form['gigya_reactions_layout'] = array(
      '#type' => 'select',
      '#title' => t('Layout'),
      '#options' => array(
        'horizontal' => t('Horizontal'),
        'vertical' => t('Vertical'),
      ),
      '#default_value' => !empty($settings['gigya_reactions_layout']) ? $settings['gigya_reactions_layout'] : 'horizontal',
    );
    $form['gigya_reactions_multiple_reactions'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use multiple reactions'),
      '#default_value' => !empty($settings['gigya_reactions_multiple_reactions']) ? $settings['gigya_reactions_multiple_reactions'] : TRUE,

    );
    $form['gigya_reactions_count_type'] = array(
      '#type' => 'select',
      '#title' => t('Count type'),
      '#options' => array(
        'number' => t('Number'),
        'percentage' => t('Percentage'),
      ),
      '#default_value' => !empty($settings['gigya_reactions_count_type']) ? $settings['gigya_reactions_count_type'] : variable_get('gigya_reactions_count_type', 'number'),
    );
    $form['gigya_reactions_image_behavior'] = array(
      '#type' => 'radios',
      '#title' => t('Image'),
      '#options' => array(
        'default' => t('Use image tag if exists, first image on post otherwise.'),
        'first' => t('First image on the post'),
        'url' => t('Specify an image url'),
      ),
      '#default_value' => !empty($settings['gigya_reactions_image_behavior']) ? $settings['gigya_reactions_image_behavior'] : variable_get('gigya_reactions_image_behavior', 'default'),
      '#attributes' => array('class' => array('image-behv')),
    );
    $form['gigya_reactions_image_url'] = array(
      '#type' => 'textfield',
      '#title' => t('The URL of the image to share'),
      '#size' => 20,
      '#states' => array(
        'visible' => array(
          '.image-behv' => array('value' => 'url'),
        ),
      ),
      '#default_value' => !empty($settings['gigya_reactions_image_url']) ? $settings['gigya_reactions_image_url'] : variable_get('gigya_reactions_image_url', ''),
    );
    $adv_parmas = !empty($settings['gigya_reactions_advanced']) ? $settings['gigya_reactions_advanced'] : variable_get('gigya_reactions_advanced', '');
    $field_name = $field['field_name'];
    $form['gigya_reactions_advanced'] = array(
      '#type' => 'textarea',
      '#title' => t('Advanced showReactionsBarUI settings for @field', array('@field' => $field_name)),
      '#element_validate' => array('gigya_key_val_filed_validate'),
      '#default_value' => !empty($adv_parmas) ? gigya_values_array_to_string($adv_parmas) : '',
      '#description' => t('Use this textbox to enter your own settings. If you are not using this box, make sure it is empty. </br>See list of available !parameters', array(
          '!parameters' => l(t('parameters'), 'http://developers.gigya.com/020_Client_API/020_Methods/socialize.showReactionsBarUI', array(
              'external' => TRUE,
              'attributes' => array('target' => '_blank'),
            )
          ),
        )
      ),
      '#suffix' => '</div>',
    );
    $form['gigya_reactions_buttons'] = array(
      '#type' => 'textarea',
      '#title' => t('Reactions Buttons'),
      '#rows' => 20,
      '#default_value' => !empty($settings['gigya_reactions_buttons']) ? $settings['gigya_reactions_buttons'] : '',
      '#description' => t('Use this textbox to put in your own settings. See !examples', array(
          '!examples' => l(t('examples'), 'http://drupal.org/node/1680606#reactions', array(
              'external' => TRUE,
              'attributes' => array('target' => '_blank'),
            )
          ),
        )
      ),
    );
  }
  // Ratings settings.
  if ($field['type'] == 'gigya_ratings') {
    if ($instance['entity_type'] === 'node') {
      $type = $instance['bundle'];
    }
    $form['gigya_ratings_container_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Container ID'),
      '#size' => 20,
      '#prefix' => '<div class="gigya-ratings-field-settings">',
      '#default_value' => !empty($settings['gigya_ratings_container_id']) ? $settings['gigya_ratings_container_id'] : 'gigya-ratings',
    );
    $form['gigya_ratings_category_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Category ID'),
      '#size' => 20,
      '#default_value' => !empty($settings['gigya_ratings_category_id']) ? $settings['gigya_ratings_category_id'] : variable_get('gigya_comments_categoryid_' . $type, ''),
    );
    $form['gigya_ratings_linked_comments'] = array(
      '#type' => 'textfield',
      '#title' => t('Comments continer ID'),
      '#size' => 20,
      '#default_value' => !empty($settings['gigya_ratings_linked_comments']) ? $settings['gigya_ratings_linked_comments'] : variable_get('gigya_comments_container_' . $type, 'comments'),
    );
    $form['gigya_ratings_show_read'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show Read reviews link'),
      '#default_value' => !empty($settings['gigya_ratings_show_read']) ? $settings['gigya_ratings_show_read'] : TRUE,
    );
    $form['gigya_ratings_show_write'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show "Write your review" button'),
      '#default_value' => !empty($settings['gigya_ratings_show_write']) ? $settings['gigya_ratings_show_write'] : TRUE,
      '#suffix' => '</div>',
    );

  }
  $form['#attached']['js'] = array(
    drupal_get_path('module', 'gigya') . '/js/gigya.admin.behaviors.js',
  );
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'gigya') . '/css/gigya_admin.css',
  );
  return $form;
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function gigya_field_formatter_settings_summary($field, $instance, $view_mode) {
  $settings = $instance['display'][$view_mode]['settings'];
  $summary = '<div>ok</div>';
  if ($field['type'] == 'gigya_sharebar') {
    $summary = t('Title: @title, Description: @desc, Link back: @link, Image behavior: @img, Short URLs: @surl', array(
        '@title' => $settings['gigya_sharebar_title'],
        '@desc' => $settings['gigya_sharebar_desc'],
        '@link' => $settings['gigya_sharebar_link_back'],
        '@img' => $settings['gigya_sharebar_image_behavior'],
        '@surl' => $settings['gigya_sharebar_short_url'] ? 'TRUE' : 'FALSE',
      )
    );
  }
  if ($field['type'] == 'gigya_reactions') {
    $summary = t('Title: @title, Link back: @link, Continer ID: @cid, Bar ID: @bid, Show counts: @show_counts, Layout: @layout, Multiple reactions: @mr', array(
        '@title' => $settings['gigya_reactions_title'],
        '@link' => $settings['gigya_reactions_link_back'],
        '@cid' => $settings['gigya_reactions_container_id'],
        '@bid' => $settings['gigya_reactions_bar_id'],
        '@show_counts' => $settings['gigya_reactions_show_counts'],
        '@layout' => $settings['gigya_reactions_layout'],
        '@mr' => $settings['gigya_reactions_multiple_reactions'] ? 'TRUE' : 'FALSE',
      )
    );
  }
  if ($field['type'] == 'gigya_ratings') {
    $summary = t('Continer ID: @cid, Category ID: @catid, Comments continer ID: @comments, Show read link: @showr, Show write link: @showw', array(
        '@cid' => $settings['gigya_ratings_container_id'],
        '@catid' => $settings['gigya_ratings_category_id'],
        '@comments' => $settings['gigya_ratings_container_id'],
        '@showr' => $settings['gigya_ratings_show_read'] ? 'TRUE' : 'FALSE',
        '@showw' => $settings['gigya_ratings_show_write'] ? 'TRUE' : 'FALSE',
      )
    );
  }
  return $summary;
}

/**
 * Implements hook_field_prepare_view().
 */
function gigya_field_prepare_view($entity_type, $entities, $field, $instances, $langcode, &$items) {
  foreach ($entities as $id => $entity) {
    // Populating the $items[$id] array even for items with no value forces
    // the render system to output a widget.
    $items[$id][]['sharebar'] = 0;
  }
}

/**
 * Implements hook_field_formatter_view().
 */
function gigya_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  drupal_add_js(drupal_get_path('module', 'gigya') . '/js/gigya_sharebar.js');
  drupal_add_js(drupal_get_path('module', 'gigya') . '/js/gigya_reactions.js');
  drupal_add_js(drupal_get_path('module', 'gigya') . '/js/gigya_ratings.js');
  drupal_add_css(drupal_get_path('module', 'gigya') . '/css/gigya.css');
  $settings = $display['settings'];
  if ($field['type'] == 'gigya_sharebar') {
    $defaults = _gigya_field_info_formatter_default_settings($display['type']);
    if (!empty($settings['gigya_sharebar_override_default'])) {
      $defaults = array_filter($defaults);
      $settings += $defaults;
    }
    else {
      $settings = $defaults;
    }
    $share_settings = array(
      'gigyaSharebar' => array(
        'ua' => array(
          'linkBack' => filter_xss(token_replace($settings['gigya_sharebar_link_back'], array($entity_type => $entity))),
          'title' => filter_xss(token_replace($settings['gigya_sharebar_title'], array($entity_type => $entity))),
          'description' => filter_xss(token_replace($settings['gigya_sharebar_desc'], array($entity_type => $entity))),
          'imageBhev' => $settings['gigya_sharebar_image_behavior'],
          'imageUrl' => check_url($settings['gigya_sharebar_image_url']),
        ),
        'shareButtons' => $settings['gigya_sharebar_buttons'],
        'shortURLs' => !empty($settings['gigya_sharebar_short_url']) ? 'always' : 'never',
        'containerID' => $settings['gigya_sharebar_container_id'],
        'showCounts' => $settings['gigya_sharebar_showcounts'],
        'layout' => $settings['gigya_sharebar_layout'],
      ),
    );
    // Allow other modules to modify global configuration.
    if (!empty($settings['gigya_sharebar_advanced'])) {
      foreach ($settings['gigya_sharebar_advanced'] as $key => $set) {
        $share_settings['gigyaSharebar'][$key] = gigya_json_to_php($set);
      }
    }
    drupal_alter('gigya_sharebar', $share_settings);
    $sharebars['gigyaSharebars'][] = $share_settings;
    drupal_add_js($sharebars, 'setting');
    $element = array();
    $element[0] = array('#markup' => '<div id="' . $settings['gigya_sharebar_container_id'] . '"></div>');
  }
  if ($field['type'] == 'gigya_reactions') {
    $reactions = (string) str_replace(array("\r", "\r\n", "\n"), '', $settings['gigya_reactions_buttons']);
    $reactions = (string) preg_replace('/\s+/', ' ', $reactions);
    $defaults = _gigya_field_info_formatter_default_settings($display['type']);
    if (!empty($settings['gigya_reactions_override_default'])) {
      $defaults = array_filter($defaults);
      $settings += $defaults;
    }
    else {
      $settings = $defaults;
    }
    $reactions_settings = array(
      'gigyaReactions' => array(
        'reactions' => $reactions,
        'ua' => array(
          'title' => filter_xss(token_replace($settings['gigya_reactions_title'], array($entity_type => $entity))),
          'linkBack' => check_url(token_replace($settings['gigya_reactions_link_back'], array($entity_type => $entity))),
          'imageBhev' => $settings['gigya_reactions_image_behavior'],
          'imageUrl' => $settings['gigya_reactions_image_url'],
        ),
        'barID' => token_replace($settings['gigya_reactions_bar_id'], array($entity_type => $entity)),
        'containerID' => $settings['gigya_reactions_container_id'],
        'multipleReactions' => !empty($settings['gigya_reactions_multiple_reactions']) ? 'true' : 'false',
        'showCounts' => $settings['gigya_reactions_show_counts'],
        'layout' => $settings['gigya_reactions_layout'],
        'countType' => $settings['gigya_reactions_count_type'],
        'scope' => variable_get('gigya_reactions_af_scope', 'external'),
        'privacy' => variable_get('gigya_activety_feed_privacy', 'private'),
        'context' => array('id' => $settings['gigya_reactions_bar_id']),
      ),
    );

    // Merge extra parameters.
    if (!empty($settings['gigya_reactions_advanced'])) {
      foreach ($settings['gigya_reactions_advanced'] as $key => $set) {
        $share_settings['gigyaReacations'][$key] = gigya_json_to_php($set);
      }
      $reactions_settings['gigyaReactions'] = array_merge($reactions_settings['gigyaReactions'], $settings['gigya_reactions_advanced']);
    }

    // Allow other modules to modify global configuration.
    drupal_alter('gigya_reactions', $reactions_settings);
    $reactionbars['gigyaReactionsBars'][] = $reactions_settings;
    drupal_add_js($reactionbars, 'setting');
    $element = array();
    $element[0] = array('#markup' => '<div id="' . $settings['gigya_reactions_container_id'] . '"></div>');
  }
  if ($field['type'] == 'gigya_ratings') {
    $ratings_settings = array(
      'containerID' => $settings['gigya_ratings_container_id'],
      'categoryID' => $settings['gigya_ratings_category_id'],
      'streamID' => $entity->nid,
      'linkedCommentsUI' => $settings['gigya_ratings_linked_comments'],
      'showReadReviewsLink' => $settings['gigya_ratings_show_read'],
      'showCommentButton' => $settings['gigya_ratings_show_write'],
    );
    // Allow other modules to modify global configuration.
    drupal_alter('gigya_reatings', $ratings_settings);
    $ratings['gigyaRaitingsInstances'][] = $ratings_settings;
    drupal_add_js($ratings, 'setting');
    $element = array();
    $element[0] = array('#markup' => '<div id="' . $settings['gigya_ratings_container_id'] . '"></div>');
  }
  return $element;
}
