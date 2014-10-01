<?php

/**
 * @file
 * Functions related to plugin and module overrides.
 */

/**
 * Implements hook_wysiwyg_editor_settings_alter().
 */
function gardens_features_wysiwyg_editor_settings_alter(&$settings, &$context) {
  // Remove LTR and RTL buttons.
  if ($context['editor']['name'] == 'ckeditor') {
    if (empty($settings['removePlugins'])) {
      $settings['removePlugins'] = 'bidi';
    }
    elseif (strpos($settings['removePlugins'], 'bidi') === FALSE) {
      $settings['removePlugins'] .= ',bidi';
    }
  }

  // Remove the gardens-wmode plugin button. The functionality remains, but we
  // don't want the button in the WYSIWYG UI.
  if ($context['editor']['name'] == 'ckeditor' && !empty($settings['toolbar'])) {
    foreach ($settings['toolbar'] as $i => $toolbar ) {
      foreach ($toolbar as $j => $button) {
        if ($button == 'gardens-wmode') {
          unset($settings['toolbar'][$i][$j]);

          // Reindex array after the removed element.
          $settings['toolbar'][$i] = array_values($settings['toolbar'][$i]);
        }
      }
    }
  }
}

/**
 * Implements hook_wysiwyg_include_directory().
 */
function gardens_features_wysiwyg_include_directory($type) {
  switch ($type) {
    case 'plugins':
      return 'wysiwyg_plugins';
        break;
  }
}
