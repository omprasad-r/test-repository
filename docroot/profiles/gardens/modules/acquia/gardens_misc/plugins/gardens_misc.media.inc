<?php

function gardens_misc_form_media_add_from_url_alter(&$form) {
  $form['url']['#title'] = t('URL of an image.');
  $form['url']['#description'] = t('Type or paste any URL to an image. e.g. using Firefox, right-click an image on any web page and select "Copy Image Location" and then paste it here.');

  //There is a 1/2 built live preview of a URL, disabling for now.
  unset($form['preview']);
  if ($idx = array_search(drupal_get_path('module', 'media') . '/javascript/media-add_from_url.js', $form['#attached']['js'])) {
    unset($form['#attached']['js'][$idx]);
  }

}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function gardens_misc_form_media_add_upload_alter(&$form) {
  $form['upload']['#title'] = t('Upload a new image from your computer.');
  $form['submit']['#value'] = t('Upload');
  $form['#validate'][] = 'gardens_misc_media_set_file_dir';
}

/**
 * Sets a subdirectory for the media upload.
 */
function gardens_misc_media_set_file_dir($form, &$form_state) {
  $file_directory = '';
  if (!empty($form_state['build_info']['args'][0]['file_directory'])) {
    $file_directory = $form_state['build_info']['args'][0]['file_directory'];
  }
  $form_state['build_info']['args'][0]['file_directory'] = gardens_misc_prefix_file_dir($file_directory);
}

/**
 * Put the media button in its own toolbar so it can be styled differently
 */

function gardens_misc_wysiwyg_editor_settings_alter(&$settings, &$context) {
  $media = array_search('media', $settings['toolbar'][0]);
  if ($media) {
    // Remove the media button from the toolbar and reindex the toolbar array
    unset($settings['toolbar'][0][$media]);
    $settings['toolbar'][0] = array_values($settings['toolbar'][0]);

    // If the user can use the media button, add the button into its own toolbar
    if (media_access('administer media')) {
      $settings['toolbar'][] = array('media');
    }
  }
  // The default CKEditor 4 skin is 'moono'.
  $settings['skin'] = 'moono';

  // Check to see if we're creating an instance of the WYSIWYG for a comment form
  global $wysiwyg_comment;
  if ($wysiwyg_comment) {
    // Setup our new toolbar
    $new_toolbar =  array();

    // See if the allowed buttons for comments are enabled and add them to the
    // toolbar
    foreach(array('Bold', 'Italic', 'BulletedList', 'NumberedList', 'Link', 'Unlink', 'Blockquote') as $button) {
      if (in_array($button, $settings['toolbar'][0])) {
        $new_toolbar[0][] = $button;
      }
    }
    
    // Check to see if the media button is enabled
    if (isset($settings['toolbar'][1]) && in_array('media', $settings['toolbar'][1])) {
      $new_toolbar[1] = array('media');
    }
    $settings['toolbar'] = $new_toolbar;
  }
}

function gardens_misc_form_media_admin_type_manage_form_alter(&$form) {
  // This was requested in the story (https://backlog.acquia.com/browse/AN-16334)
  // but it doesn't seem like a wise move to me.
  // The defaults are "All" for any existing ones.
  // $form['match_type']['#default_value'] = 'all';
  $form['extensions']['#description'] = t('Specify allowed file extensions for this media type, separating each with a space (and no dots).');

  $fields_to_hide = array('match_type', 'match_type_other', 'streams', 'mimetypes');
  foreach ($fields_to_hide as $fieldname) {
    $form[$fieldname]['#access'] = FALSE;
  }
}
