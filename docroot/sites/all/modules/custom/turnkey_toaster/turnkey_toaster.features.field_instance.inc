<?php
/**
 * @file
 * turnkey_toaster.features.field_instance.inc
 */

/**
 * Implements hook_field_default_field_instances().
 */
function turnkey_toaster_field_default_field_instances() {
  $field_instances = array();

  // Exported field_instance: 'node-turnkey_toaster-body'
  $field_instances['node-turnkey_toaster-body'] = array(
    'bundle' => 'turnkey_toaster',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => 'Let the user know what they will get by signing up for the list. Give them clear, concise direction',
    'display' => array(
      'default' => array(
        'label' => 'hidden',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 0,
      ),
      'teaser' => array(
        'label' => 'hidden',
        'module' => 'improved_text_trim',
        'settings' => array(
          'trim_length' => 600,
        ),
        'type' => 'text_summary_or_trimmed',
        'weight' => 0,
      ),
    ),
    'entity_type' => 'node',
    'field_name' => 'body',
    'label' => 'Body',
    'required' => 0,
    'settings' => array(
      'display_summary' => 1,
      'text_processing' => 1,
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'text',
      'settings' => array(
        'rows' => 1,
        'summary_rows' => 5,
      ),
      'type' => 'text_textarea_with_summary',
      'weight' => -4,
    ),
  );

  // Exported field_instance: 'node-turnkey_toaster-field_display_rates'
  $field_instances['node-turnkey_toaster-field_display_rates'] = array(
    'bundle' => 'turnkey_toaster',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => 'How often should the lightbox appear for site visitors?',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'list',
        'settings' => array(),
        'type' => 'list_default',
        'weight' => 2,
      ),
      'teaser' => array(
        'label' => 'above',
        'settings' => array(),
        'type' => 'hidden',
        'weight' => 0,
      ),
    ),
    'entity_type' => 'node',
    'field_name' => 'field_display_rates',
    'label' => 'Display rate',
    'required' => 0,
    'settings' => array(
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'options',
      'settings' => array(),
      'type' => 'options_select',
      'weight' => -2,
    ),
  );

  // Exported field_instance: 'node-turnkey_toaster-field_label_list_id'
  $field_instances['node-turnkey_toaster-field_label_list_id'] = array(
    'bundle' => 'turnkey_toaster',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => 'What is the Label List ID?',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 5,
      ),
      'teaser' => array(
        'label' => 'above',
        'settings' => array(),
        'type' => 'hidden',
        'weight' => 0,
      ),
    ),
    'entity_type' => 'node',
    'field_name' => 'field_label_list_id',
    'label' => 'Label List ID',
    'required' => 0,
    'settings' => array(
      'text_processing' => 0,
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'text',
      'settings' => array(
        'size' => 60,
      ),
      'type' => 'text_textfield',
      'weight' => 1,
    ),
  );

  // Exported field_instance: 'node-turnkey_toaster-field_newsletter_id'
  $field_instances['node-turnkey_toaster-field_newsletter_id'] = array(
    'bundle' => 'turnkey_toaster',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => 'What is the Newsletter ID?',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 3,
      ),
      'teaser' => array(
        'label' => 'above',
        'settings' => array(),
        'type' => 'hidden',
        'weight' => 0,
      ),
    ),
    'entity_type' => 'node',
    'field_name' => 'field_newsletter_id',
    'label' => 'Newsletter ID',
    'required' => 0,
    'settings' => array(
      'text_processing' => 0,
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'text',
      'settings' => array(
        'size' => 60,
      ),
      'type' => 'text_textfield',
      'weight' => -1,
    ),
  );

  // Exported field_instance: 'node-turnkey_toaster-field_preview_mode'
  $field_instances['node-turnkey_toaster-field_preview_mode'] = array(
    'bundle' => 'turnkey_toaster',
    'default_value' => array(
      0 => array(
        'value' => 0,
      ),
    ),
    'deleted' => 0,
    'description' => 'Checking Preview Mode will only show the toaster to logged-in administrators. This allows you to preview the toaster before pushing live. To make the toaster public, leave this box unchecked. ',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'list',
        'settings' => array(),
        'type' => 'list_default',
        'weight' => 10,
      ),
      'teaser' => array(
        'label' => 'above',
        'settings' => array(),
        'type' => 'hidden',
        'weight' => 0,
      ),
    ),
    'entity_type' => 'node',
    'field_name' => 'field_preview_mode',
    'label' => 'Preview Mode',
    'required' => 0,
    'settings' => array(
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'options',
      'settings' => array(
        'display_label' => 1,
      ),
      'type' => 'options_onoff',
      'weight' => 6,
    ),
  );

  // Exported field_instance: 'node-turnkey_toaster-field_toaster_position'
  $field_instances['node-turnkey_toaster-field_toaster_position'] = array(
    'bundle' => 'turnkey_toaster',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => 'Do you want to set the toaster to pop up relative to the top or the bottom of the site?  ',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'list',
        'settings' => array(),
        'type' => 'list_default',
        'weight' => 12,
      ),
      'teaser' => array(
        'label' => 'above',
        'settings' => array(),
        'type' => 'hidden',
        'weight' => 0,
      ),
    ),
    'entity_type' => 'node',
    'field_name' => 'field_toaster_position',
    'label' => 'Toaster Position',
    'required' => 0,
    'settings' => array(
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'options',
      'settings' => array(),
      'type' => 'options_select',
      'weight' => 8,
    ),
  );

  // Exported field_instance: 'node-turnkey_toaster-field_toaster_timeout'
  $field_instances['node-turnkey_toaster-field_toaster_timeout'] = array(
    'bundle' => 'turnkey_toaster',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => 'Displayed in milliseconds, the is how long the toaster will display before disappearing. ',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 11,
      ),
      'teaser' => array(
        'label' => 'above',
        'settings' => array(),
        'type' => 'hidden',
        'weight' => 0,
      ),
    ),
    'entity_type' => 'node',
    'field_name' => 'field_toaster_timeout',
    'label' => 'Timeout',
    'required' => 0,
    'settings' => array(
      'text_processing' => 0,
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'text',
      'settings' => array(
        'size' => 60,
      ),
      'type' => 'text_textfield',
      'weight' => 7,
    ),
  );

  // Exported field_instance: 'node-turnkey_toaster-field_turnkey_height'
  $field_instances['node-turnkey_toaster-field_turnkey_height'] = array(
    'bundle' => 'turnkey_toaster',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => 'Displayed in pixels, this is the distance the toaster will display from the toaster position (top or bottom depending on your selection above).',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'number',
        'settings' => array(
          'decimal_separator' => '.',
          'prefix_suffix' => TRUE,
          'scale' => 0,
          'thousand_separator' => ' ',
        ),
        'type' => 'number_integer',
        'weight' => 13,
      ),
      'teaser' => array(
        'label' => 'above',
        'settings' => array(),
        'type' => 'hidden',
        'weight' => 0,
      ),
    ),
    'entity_type' => 'node',
    'field_name' => 'field_turnkey_height',
    'label' => 'Distance from Position',
    'required' => 0,
    'settings' => array(
      'max' => '',
      'min' => 360,
      'prefix' => '',
      'suffix' => '',
      'user_register_form' => FALSE,
    ),
    'widget' => array(
      'active' => 0,
      'module' => 'number',
      'settings' => array(),
      'type' => 'number',
      'weight' => 9,
    ),
  );

  // Translatables
  // Included for use with string extractors like potx.
  t('Body');
  t('Checking Preview Mode will only show the toaster to logged-in administrators. This allows you to preview the toaster before pushing live. To make the toaster public, leave this box unchecked. ');
  t('Display rate');
  t('Displayed in milliseconds, the is how long the toaster will display before disappearing. ');
  t('Displayed in pixels, this is the distance the toaster will display from the toaster position (top or bottom depending on your selection above).');
  t('Distance from Position');
  t('Do you want to set the toaster to pop up relative to the top or the bottom of the site?  ');
  t('How often should the lightbox appear for site visitors?');
  t('Label List ID');
  t('Let the user know what they will get by signing up for the list. Give them clear, concise direction');
  t('Newsletter ID');
  t('Preview Mode');
  t('Timeout');
  t('Toaster Position');
  t('What is the Label List ID?');
  t('What is the Newsletter ID?');

  return $field_instances;
}
