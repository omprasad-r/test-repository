<?php
/**
 * @file
 * Administrative forms
 */

// # Library Edit Forms
module_load_include('inc', 'environment_libraries', 'environment_libraries.class');
use Environment_Libraries\LibraryUtils;
/**
 * Form Element Process.
 */
function environment_libraries_environment_library_process(&$element, &$form_state, &$form) {
  // ## Build an environment_library element
  $library = !empty($element['#value']) ? $element['#value'] : NULL;
  $environments = LibraryUtils::get_environments();
  // The ajax request using ajax_process_form regenerates w/ a new id.
  // Instead of using a custom handler just use the element name.
  $id = check_plain($element['#name']);

  if (isset($form_state[$id . '_file_count'])) {
    $file_count = &$form_state[$id . '_file_count'];
  }
  elseif (isset($library)) {
    $file_count = LibraryUtils::count_files($library);
  }
  else {
    $file_count = 1;
  }
  $minify = module_exists('uglifyjs');

  // ### Setup the form element
  // with default values from an environment_libraries_library entity.
  if (!empty($library)) {
    $library = (object) $library;
    $element['name']['#value'] = isset($library->title) ? $library->title : $library->name;
    $element['machine_name']['#value'] = $library->machine_name;
    $element['environment']['#value'] = $library->environment;
    $element['expose_block']['#value'] = isset($library->expose_block) ? $library->expose_block : 0;
    $element['description']['#value'] = isset($library->item_description) ? $library->item_description : '';
    if (isset($library->dependencies)) {
      if (is_string($library->dependencies)) {
        $element['dependencies']['#value'] = isset($library->dependencies) ? explode(',', $library->dependencies) : NULL;
      }
      elseif (is_array($library->dependencies)) {
        $element['dependencies']['#value'] = $library->dependencies;
      }
    }
    for ($f = 0; $f < $file_count; $f++) {
      // Initialize each set of files.
      if (empty($element['files'][$f])) {
        $element['files'][$f] = environment_libraries_file_element($f);
      }
      $files = isset($library->config['files']) ? $library->config['files'] : $library->files;
      if (!isset($files) || !isset($files[$f])) {
        break;
      }
      foreach ($environments as $env => $environment) {
        if (isset($files[$f][$env])) {
          $file = &$files[$f][$env];
          $element['files'][$f][$env]['url']['#value'] = check_url($file['url']);
          $element['files'][$f][$env]['cache']['#value'] = isset($file['cache']) ? $file['cache'] : 0;
          $element['files'][$f][$env]['aggregate']['#value'] = isset($file['aggregate']) ? $file['aggregate'] : 0;
          $element['files'][$f][$env]['minify']['#value'] = isset($file['minify']) ? $file['minify'] : 0;
        }
        if (!$minify) {
          $element['files'][$f][$env]['minify']['#attributes']['disabled'] = TRUE;
        }
      }
      $element['files'][$f]['region']['#value'] = $files[$f]['region'];
      $element['files'][$f]['weight']['#value'] = $files[$f]['weight'];
    }
  }
  $element['#file_count'] = $form_state[$id . '_file_count'] = $file_count;
  $element['files']['add'] = array(
    '#type' => 'button',
    '#id' => $id . '--files-add',
    '#value' => t('Add File'),
    '#executes_submit_callback' => TRUE,
    '#submit' => array('environment_libraries_library_add_file'),
    '#ajax' => array(
      'callback' => 'environment_libraries_library_add_file_callback',
      // 'method' => 'before',
      // 'wrapper' => '#'.$id.'--files-add',
      'method' => 'replace',
      'wrapper' => ".{$id}--wrapper",
      // #{$form['#id']} // add #form_id?
      // 'wrapper' => "#{$element['#id']}",
    ),
    '#weight' => 1,
  );
  $element['#prefix'] = "<div class=\"$id--wrapper\">";
  $element['#suffix'] = "</div>";

  return $element;
}

/**
 * Environment Library Ajax Form Submit Callback.
 */
function environment_libraries_library_add_file($form, &$form_state) {
  // ## Ajax Callback - Add File
  // This callback is needed to increment the file count in the $form_state
  $element_id = $form_state['triggering_element']['#parents'][0];
  $element = $form[$element_id];
  $id = $element['#name'];
  $file_count = &$form_state[$id . '_file_count'];
  $file_count++;
  // We must set the form_state to rebuild so that the server-side form has the updated file count.
  // This re-triggers `environment_libraries_environment_library_process`
  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback to add a file.
 *
 * @param $form
 *   form elements array.
 * @param $form_state
 *   form state.
 * @return array
 */
function environment_libraries_library_add_file_callback($form, $form_state) {
  $ajax = array(
    '#type' => 'ajax',
    '#commands' => array(),
  );
  $element_id = $form_state['triggering_element']['#parents'][0];
  $element = $form[$element_id];
  // The file count cannot be incremented in this function as $form_state is
  // passed by value instead of by reference.
  // The default forms ajax doesn't support the full set of ajax framework
  // commands (only insert/prepend) so we build our own.
  // @see https://api.drupal.org/api/drupal/includes%21ajax.inc/group/ajax_commands/7
  $html = drupal_render($element);
  $command = 'ajax_command_' . $form_state['triggering_element']['#ajax']['method'];
  $selector = isset($form_state['triggering_element']['#ajax']['selector']) ? $form_state['triggering_element']['#ajax']['selector'] : $form_state['triggering_element']['#ajax']['wrapper'];

  $ajax['#commands'][] = $command($selector, $html);
  return $ajax;
}

/**
 * Form Submit Handler.
 */
function environment_libraries_edit_form_submit($form, &$form_state) {
  // ## Form Submit
  form_state_values_clean($form_state);

  $lid = NULL;
  // If there is an lid set in the form
  if (isset($form_state['values']['environment_libraries_lid'])) {
    $lid = $form_state['values']['environment_libraries_lid'];
  }
  $form_library = $form_state['values']['environment_libraries_library'];
  // then update the library entity based on the form.
  environment_libraries_edit_form_save_library($lid, $form_library);
  $form_state['redirect'] = 'admin/config/system/environment_libraries/libraries';
}

/**
 * Save the library to the database.
 *
 * @param $lid
 *   Library entity id.
 * @param $form_element
 *   Library form element array.
 * @return bool|mixed
 *   A libraries API library for an environment_library.
 */
function environment_libraries_edit_form_save_library($lid, &$form_element) {
  // TODO should this be handled by the field? To return a stdClass matching the entity?
  $environments = LibraryUtils::get_environments();
  $env_default = LibraryUtils::default_environment();

  // ### Get the library object
  if ($lid) {
    // from an existing library
    $library = environment_libraries_library_load($lid);
  }
  else {
    // or from a new one.
    $library = environment_libraries_library_create();
  }

  $library->title = $form_element['name'];
  $library->machine_name = check_plain($form_element['machine_name']);
  $library->dependencies = $form_element['dependencies'];
  $library->expose_block = $form_element['expose_block'];
  $library->environment = $form_element['environment'] ?: 'default';
  // If we want to force the environment.
  $library->item_description = check_plain($form_element['description']);

  // Create an array containing files for the library.
  $library_config = array(
    'files' => array(),
  );

  $f = 0;
  // ### Go through each file
  foreach (element_children($form_element['files']) as $i) {
    $file = $form_element['files'][$i];
    $library_config['files'][$f] = array(
      // TODO file title, version, etc?
      'region' => $form_element['files'][$f]['region'],
      'weight' => $form_element['files'][$f]['weight'],
    );
    $url = NULL;
    // and for each file's specified environments
    $HAS_FILES = FALSE;
    foreach ($environments as $env => $environment) {
      if (empty($file[$env]['url'])) {
        // (skipping empty environments)
        $library_config['files'][$f][$env] = NULL;
        continue;
      }
      else {
        $url = $file[$env]['url'];
        $HAS_FILES = TRUE;
      }

      // setup file info.
      $library_config['files'][$f][$env] = array(
        'url' => check_url($file[$env]['url']),
        'cache' => $file[$env]['cache'] ?: 0,
        'minify' => $file[$env]['minify'] ?: 0,
        'aggregate' => $file[$env]['aggregate'] ?: 0,
      );
      // Make sure there is a production library set to use as a fallback.
      if ($env !== $env_default && !isset($library_config['files'][$f][$env_default])) {
        $library_config['files'][$f][$env_default] = $file[$env];
      }
    }
    // If there are no files for this environment then skip it
    if ($HAS_FILES === FALSE) {
      unset($library_config['files'][$f]);
      continue;
    }

    // otherwise, get the file type from the extension.
    $path = LibraryUtils::parse_url($url);
    if (substr($path['path'], -strlen('.css')) === '.css') {
      $library_config['files'][$f]['type'] = 'css';
    }
    elseif (substr($path['path'], -strlen('.js')) === '.js') {
      $library_config['files'][$f]['type'] = 'js';
    }
    // Increment the file counter.
    $f++;
  }
  $library->config = $library_config;

  // ### Save the library
  $result = environment_libraries_library_save($library);
  // cache_clear_all($library->machine_name, 'cache_environment_libraries');
  // If the entity_save result is set then
  if ($result !== FALSE) {
    // the entity has been saved
    module_invoke_all('environment_libraries_library_save', $library);
    drupal_set_message(t('The library ' . $library->title . ' has been ' . (($result == SAVED_NEW) ? 'created' : 'saved')), 'status');
  }
  else {
    // otherwise, something has gone wrong.
    drupal_set_message(t('Something went wrong attempting to save the library ' . $form_element['name'] . '.'), 'error');
  }
  return $library;
}

/**
 * Form Validate Handler.
 */
function environment_libraries_environment_library_validate($element, &$form_state) {
  // TODO validate that dependencies are all valid libraries
  // TODO check_url() warnings.
  $params = drupal_get_query_parameters(arg());
  if ($params[4] == 'add') {
    $is_exists = environment_libraries_machinename_exists($element['machine_name'], $params[4]);
    if ($is_exists) {
      form_error($element['machine_name'], t('Name @machine_name already exists', array('@machine_name' => $element['machine_name'])));
    }
  } 
  
}
