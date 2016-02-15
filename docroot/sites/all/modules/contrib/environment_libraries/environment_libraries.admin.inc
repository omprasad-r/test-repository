<?php
/**
 * @file
 * Administrative Pages
 */

// # Administrative Pages
module_load_include('inc', 'environment_libraries', 'environment_libraries.class');
use Environment_Libraries\LibraryUtils;

/**
 * Implements hook_menu().
 */
function environment_libraries_menu() {
  // ## Menu Entries
  $items = array();
  // ### Main Admin Page
  $items['admin/config/system/environment_libraries'] = array(
    'title' => 'Environment Libraries',
    'description' => 'Manage CSS & Javascript libraries defined in Drupal or from external sources.',
    'page callback' => 'environment_libraries_libraries_list',
    'page arguments' => array(),
    'access arguments' => array('administer environment_libraries settings'),
    'type' => MENU_NORMAL_ITEM,
  );

  // #### Library Listing Tab
  $items['admin/config/system/environment_libraries/libraries'] = array(
    'title' => 'Libraries',
    'type' => MENU_DEFAULT_LOCAL_TASK,
  // This is the default tab
        // 'page callback' => 'drupal_get_form',
    'page callback' => 'environment_libraries_libraries_list',
    'page arguments' => array(),
    'access arguments' => array('administer environment_libraries settings'),
  );

  // #### Block Listing Tab
  $items['admin/config/system/environment_libraries/blocks'] = array(
    'title' => 'Blocks',
    'type' => MENU_LOCAL_TASK,
    'page callback' => 'environment_libraries_libraries_list',
    'page arguments' => array(TRUE),
    'access arguments' => array('administer environment_libraries settings'),
  );

  // #### Settings Tab.
  $items['admin/config/system/environment_libraries/settings'] = array(
    'title' => 'Settings',
    'type' => MENU_LOCAL_TASK,
    'page callback' => 'drupal_get_form',
    'page arguments' => array('environment_libraries_form_settings'),
    'access arguments' => array('administer environment_libraries settings'),
  );

  // ### Edit an environment_libraries library
  $items['admin/config/system/environment_libraries/%/edit'] = array(
    'title' => 'Edit an Environment Library',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('environment_libraries_edit_form', 4),
    'access arguments' => array('administer environment_libraries settings'),
    'type' => MENU_NORMAL_ITEM,
  );
  // ### Create an environment_libraries library
  $items['admin/config/system/environment_libraries/add'] = array(
    'title' => 'Edit an Environment Library',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('environment_libraries_edit_form'),
    'access arguments' => array('administer environment_libraries settings'),
    'type' => MENU_NORMAL_ITEM,
  );

  // ### Delete an environment_libraries_library
  $items['admin/config/system/environment_libraries/%/delete'] = array(
    'title' => 'Delete an Environment Library',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('environment_libraries_library_form_delete', 4),
    'access arguments' => array('administer environment_libraries settings'),
    'type' => MENU_NORMAL_ITEM,
  );

  // ### Download an environment_libraries_library
  $items['admin/config/system/environment_libraries/%/sync'] = array(
    'title' => 'Download an Environment Library from it\'s source',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('environment_libraries_library_form_sync', 4),
    'access arguments' => array('administer environment_libraries settings'),
    'type' => MENU_NORMAL_ITEM,
  );

  return $items;
}

/**
 * General module settings form.
 */
function environment_libraries_form_settings() {
  // ## Environment Libraries Settings Form
  $form = array();

  $form['environment_libraries_module_dependencies'] = array(
    '#type' => 'textfield',
    '#title' => t('Available Module Dependencies'),
    '#default_value' => variable_get('environment_libraries_module_dependencies', 'environment_libraries,system'),
    '#description' => t("Comma-separated list of enabled Drupal modules to expose libraries from as dependencies to environment_libraries."),
    '#required' => FALSE,
    '#weight' => 10,
    '#maxlength' => 1028,
  );

  $form['environment_libraries_environment_current'] = array(
    '#type' => 'radios',
    '#title' => t('Current Environment'),
    '#options' => environment_libraries_environments(),
    '#default_value' => environment_libraries_environment_current(),
    '#description' => t("Select the current environment. You may want to manage this via a drupal_alter override & custom module rather than relying on environment_libraries. If overriding via custom modules this form may not update the environment as expected."),
    '#weight' => 1,
  );

  drupal_alter('environment_libraries_form_settings', $form);
  return system_settings_form($form);
}

/**
 * List environment_libraries_block entities in a table with operations links.
 */
function environment_libraries_libraries_list($blocks_only = FALSE) {
  // ## List Environment Libraries Form
  $content = array();
  // Load all of our environment_libraries_library entities.
  $libraries = environment_libraries_library_load_multiple();

  $rows = array();
  foreach ($libraries as $library) {
    // If we only want to show libraries with blocks then skip all others.
    if ($blocks_only && !$library->expose_block) {
      continue;
    }
    $lid = $library->lid;
    // ### Create table rows for library entities
    $links = l(t('Edit'), "admin/config/system/environment_libraries/$lid/edit") . ' | ' . l(t('Delete'), "admin/config/system/environment_libraries/$lid/delete");

    // Get the current library file's settings. to see if it is cached.
    $library_files = LibraryUtils::list_files($library);
    $aggregate_config = array('cache' => 0, 'minify' => 0, 'aggregate' => 0);
    foreach ($library_files as $library_file) {
      $aggregate_config['cache'] = $aggregate_config['cache'] || $library_file['cache'];
      $aggregate_config['minify'] = $aggregate_config['minify'] || $library_file['minify'];
      $aggregate_config['aggregate'] = $aggregate_config['aggregate'] || $library_file['aggregate'];
    }

    if ($aggregate_config['cache'] == 1) {
      // If the library is cached then add an operations link to update it from
      // the remote source.
      $links .= ' | ' . l(t('Download'), "admin/config/system/environment_libraries/$lid/sync", array('attributes' => array('title' => t("Update the local cache version of this library's files from the external source."))));
    }

    if ($library->expose_block) {
      $links .= ' | ' . l(t('Block'), "admin/structure/block/manage/environment_libraries/environment_libraries_block__$lid/configure",
      array('#attributes' => array('disabled' => ($library->expose_block) ? FALSE : TRUE)));
    }
    // #### Build the row entry.
    $rows[] = array(
      'lid' => $lid,
      'config' => $aggregate_config,
      'data' => array(
        // 'id' => $library->lid,
        'name' => $library->title,
        'machine_name' => $library->machine_name,
        'item_description' => t($library->item_description),
        'config' => $links,
      ),
    );
  }

  // ### Build libraries table
  // @see [theme_table](https://api.drupal.org/api/drupal/includes%21theme.inc/function/theme_table/7) for details.
  $content['environment_libraries_libraries_table'] = array(
    '#theme' => 'table',
    '#rows' => $rows,
    '#header' => array(/*t('ID'),*/ t('Library'), t('Machine_Name'), t('Description'), t('Operations')),
    '#attributes' => array(
      'class' => array('environment-libraries--list-libraries'),
    ),
  );

  if (empty($libraries)) {
    // There are no libraries. Tell someone.
    $content['environment_libraries_libraries_none'] = array(
      '#type' => 'item',
      '#markup' => t('No Environment Libraries ' . (($blocks_only) ? 'blocks ' : '') . 'currently exist.'),
    );
  }

  // Add a link to create a new library.
  $content['environment_libraries_library_create'] = array(
    '#theme' => 'link',
    '#text' => 'Create Library',
    '#path' => 'admin/config/system/environment_libraries/add',
    '#options' => array(
      'attributes' => array(
        'class' => array('button'),
      ),
      'html' => TRUE,
    ),
  );
  drupal_alter('environment_libraries_list', $content);
  return $content;
}

/**
 * Load the environment_library form element on our edit form.
 *
 * @see environment_libraries.form_element.inc
 */
function environment_libraries_edit_form($form, &$form_state, $lid = NULL) {
  // ## Edit / Add a Library Form
  $IS_NEW = empty($lid);
  if ($IS_NEW) {
    // ### Add a new library
    $form['environment_libraries_library'] = array(
      '#type' => 'environment_library',
    );
  }
  else {
    // ### Edit an existing library
    $library = environment_libraries_library_load($lid);
    $form['environment_libraries_lid'] = array(
      '#type' => 'hidden',
      '#value' => $lid,
    );
    // Pass the existing library to our custom environment_library form element.
    $form["environment_libraries_library"] = array(
      '#type' => 'environment_library',
      '#default_value' => $library,
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t($IS_NEW ? 'Create Library' : 'Save Library'),
  );

  return $form;
}

/**
 * Confirm form to delete an environment_libraries_library.
 */
function environment_libraries_library_form_delete($form, $form_state, $lid) {
  // ## Delete Library
  $library = environment_libraries_library_load($lid);
  $form['lid'] = array('#type' => 'hidden', '#value' => $library->lid);
  $form['name'] = array('#type' => 'hidden', '#value' => $library->title);
  // TODO check for libraries with dependencies on this one.
  // Create a delete confirmation form.
  return confirm_form($form, t('Are you sure you want to delete the %name library?',
        array('%name' => $library->title)), 'admin/config/system/environment_libraries/libraries',
        'Make sure there are no other libraries with dependencies on this library before proceeding. Deleting this library will remove any associated block but will not update dependent libraries.',
         t('Delete'), t('Cancel')
  );
}
/**
 * Submit handler to delete an environment_libraries_library.
 */
function environment_libraries_library_form_delete_submit($form, &$form_state) {
  // ### Delete Library Confirm Handler
  // Handle the form delete submission.
  $library = environment_libraries_library_load($form_state['values']['lid']);
  environment_libraries_library_delete($library);
  drupal_set_message(t('The %name library has been removed.', array('%name' => $form_state['values']['name'])));
  $form_state['redirect'] = 'admin/config/system/environment_libraries/libraries';
}

// ## Library Sync Handler
/**
 * Confirm form to update an environment_libraries_library.
 */
function environment_libraries_library_form_sync($form, $form_state, $lid) {
  $library = environment_libraries_library_load($lid);
  $form['lid'] = array('#type' => 'hidden', '#value' => $library->lid);
  $form['name'] = array('#type' => 'hidden', '#value' => $library->title);
  // TODO check for libraries with dependencies on this one.
  // Create a delete confirmation form.
  return confirm_form($form, t('Are you sure you want to update the %name library?',
        array('%name' => $library->title)), 'admin/config/system/environment_libraries/libraries',
        'This will overwrite all locally cached files of the library with the external sources for the current environment. If JS and CSS are aggregated you may also need to clear cache OR run cron before the aggregated files are updated.',
         t('Download'), t('Cancel')
  );
}

/**
 * Submit handler to update an environment_libraries_library.
 */
function environment_libraries_library_form_sync_submit($form, &$form_state) {
  $library = environment_libraries_library_load($form_state['values']['lid']);
  $title = $library->title;
  $library_files = LibraryUtils::list_files($library);
  foreach ($library_files as $library_file) {
    if ($library_file['cache'] == 0) {
      continue;
    }
    environment_libraries_cache($library_file['url'], $library->lid, TRUE);
  }
  drupal_set_message(t('The cached files for the %title library have been been updated from their sources.', array('%title' => $title)));
  $destination = isset($_GET['destination']) ? $_GET['destination'] : 'admin/config/system/environment_libraries';
  // Redirect to ?destination
  $form_state['redirect'] = $destination;
}
