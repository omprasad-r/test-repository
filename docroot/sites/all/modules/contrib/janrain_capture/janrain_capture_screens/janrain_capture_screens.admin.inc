<?php

/**
 * @file
 * Admin settings form for uploading Capture screens
 */

/**
 * Returns the screen title from a filename.
 *
 * @param string $screen
 *   Screen name.
 *   @see _janrain_capture_get_screens()
 *
 * @return string
 *   Readable, translated screen name.
 */
function _janrain_capture_get_screen_title($screen) {
  $parts = explode('.', $screen);
  $filename = $parts[0];

  if ($filename == 'signin') {
    $filename = 'Sign in';
  }

  $screen_title = ucfirst(str_replace('-', ' ', $filename));
  return t($screen_title);
}

/**
 * Form callback for customizing the Janrain Capture screens.
 */
function janrain_capture_admin_screens_settings($form, &$form_state) {
  $screens = _janrain_capture_get_screens();

  $form = array();
  $form['#attributes'] = array('enctype' => "multipart/form-data");

  $form['janrain_capture_screen'] = array(
    '#type' => 'vertical_tabs',
  );

  foreach ($screens as $screen) {
    list($screen_category, $extension) = explode('.', $screen);
    list($screen_category) = explode('/', $screen_category);
    $screen_machine_name = _janrain_capture_get_screen_machine_name($screen);

    if (!isset($form['janrain_capture_screen_' . $screen_category])) {
      $form['janrain_capture_screen_' . $screen_category] = array(
        '#type' => 'fieldset',
        '#title' => _janrain_capture_get_screen_title($screen),
        '#group' => 'janrain_capture_screen',
      );
    }

    $form['janrain_capture_screen_' . $screen_category]['janrain_capture_screen_' . $screen_machine_name] = array(
      '#type' => 'managed_file',
      '#title' => $screen,
      '#upload_location' => 'public://janrain_capture_screens/',
      '#upload_validators' => array('file_validate_extensions' => array($extension)),
      '#description' => t('Allowed extensions: %extension' , array('%extension' => $extension)),
      '#default_value' => variable_get('janrain_capture_screen_' . $screen_machine_name),
    );
  }

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));

  return $form;
}

/**
 * Form submit callback for saving the uploaded Janrain capture screens.
 *
 * @see http://api.drupal.org/api/drupal/developer%21topics%21forms_api_reference.html/7#managed_file
 */
function janrain_capture_admin_screens_settings_submit(&$form, &$form_state) {
  $screens = _janrain_capture_get_screens();
  foreach ($screens as $screen) {
    $screen_machine_name = _janrain_capture_get_screen_machine_name($screen);
    $variable_name = "janrain_capture_screen_${screen_machine_name}";

    variable_set($variable_name, $form_state['values'][$variable_name]);

    $file = file_load(variable_get($variable_name));
    if ($file) {
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
      file_usage_add($file, 'user', 'user', 1);
    }
  }
}
