<?php
/**
 * @file
 * Environment Libraries Blocks.
 */

// # Block Definitions
/**
 * Define the environment_libraries_blocks for select environment_libraries_library entities.
 *
 * Implements hook_block_info().
 */
function environment_libraries_block_info() {
  // Create blocks when expose_block is checked.
  $blocks = array();
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'environment_libraries_library')
    ->propertyCondition('expose_block', 1);
  $result = $query->execute();

  if (isset($result['environment_libraries_library'])) {
    $ids = array_keys($result['environment_libraries_library']);
    $libraries = entity_load('environment_libraries_library', $ids);
  }
  if (!empty($libraries)) {
    foreach ($libraries as $library) {
      $lid = $library->lid;
      $title = $library->title;
      $bl_id = 'environment_libraries_block__' . $lid;

      $blocks[$bl_id] = array();
      $blocks[$bl_id]['info'] = t('[Library] ') . $title;
      $blocks[$bl_id]['cache'] = DRUPAL_CACHE_GLOBAL;
      $blocks[$bl_id]['properties'] = array('administrative' => FALSE, 'lid' => $lid);
    }
  }
  return $blocks;
}

/**
 * Add the libraries to the page when this block is rendered.
 *
 * Implements hook_block_view().
 */
function environment_libraries_block_view($delta = '') {
  // ### Create renderable array for environment_libraries_blocks
  $block = array();
  $bl_id = explode('__', $delta);
  if (count($bl_id) == 2 && $bl_id[0] == 'environment_libraries_block' && is_numeric($bl_id[1])) {
    // #### Get the id of the library from the block id
    $lid = $bl_id[1];
    // and add the library to the page.
    if ($library = environment_libraries_library_load($lid)) {
      $render_library = entity_view('environment_libraries_library', array($library));
      // Render the library entity using theme_environment_library().
      // @see environment_libraries.theme.inc
      drupal_render($render_library);
      $block['content'] = NULL;
      $block['subject'] = NULL;
    }
  }
}

/**
 * Setup the block / entity configure form.
 *
 * Implements hook_block_configure().
 */
function environment_libraries_block_configure($delta = '') {
  // ## Modify Library Block Configure Page
  if (!strpos($delta, 'environment_libraries_block') === 0) {
    return;
  }
  $bl_id = explode('__', $delta);
  $form = array();
  if (count($bl_id) == 2 && is_numeric($bl_id[1])) {
    $lid = $bl_id[1];
    // Add a link to the library edit page.
    $form['edit_library'] = array(
      '#type' => 'link',
      '#title' => 'Edit Library',
      '#href' => "admin/config/system/environment_libraries/$lid/edit",
      '#attributes' => array('target' => '_blank'),
      '#description' => t('Edit the library for this block'),
    );
  }

  return $form;
}

/**
 * Parse a block id to find the environment_libraries_library id.
 */
function environment_libraries_block_lid($delta) {
  if (!isset($delta) || strpos($delta, 'environment_libraries_block') === 0) {
    return FALSE;
  }
  $bl_id = explode('__', $delta);
  if (count($bl_id) == 2 && is_numeric($bl_id[1])) {
    return $bl_id[1];
  }
  return FALSE;
}
