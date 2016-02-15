<?php
/**
 * @file
 * Context Integration
 *
 * @see sites/all/modules/contrib/context/API.txt
 */

// # Environment Libraries Context Integration.
/**
 * Implements hook_context_plugins().
 */
function environment_libraries_context_plugins() {
  // ## Define our libraries context plugin
  $plugins = array();
  $plugins['environment_libraries'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'environment_libraries') . "/context",
      'file' => 'environment_libraries_context_reaction_library.inc',
      'class' => 'environment_libraries_context_reaction_library',
      'parent' => 'context_reaction',
    ),
  );
  return $plugins;
}

/**
 * Implements hook_context_registry().
 */
function environment_libraries_context_registry() {
  // ## Register our plugin
  return array(
    'reactions' => array(
      'environment_libraries' => array(
        'title' => t('Libraries'),
        'plugin' => 'environment_libraries',
      ),
    ),
  );
}

/**
 * Implements hook_context_page_reaction().
 */
function environment_libraries_context_page_reaction() {
  // Call our plugin when appropriate.
  if ($plugin = context_get_plugin('reaction', 'environment_libraries')) {
    $plugin->execute();
  }
}
