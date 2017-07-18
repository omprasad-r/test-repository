<?php
/**
 * @file
 * wmg_magento_api.features.field_instance.inc
 */

/**
 * Implements hook_field_default_field_instances().
 */
function wmg_magento_api_field_default_field_instances() {
  $field_instances = array();

  // Exported field_instance: 'user-user-field_recurring_profile_id'
  $field_instances['user-user-field_recurring_profile_id'] = array(
    'bundle' => 'user',
    'default_value' => NULL,
    'deleted' => 0,
    'description' => '',
    'display' => array(
      'default' => array(
        'label' => 'above',
        'module' => 'text',
        'settings' => array(),
        'type' => 'text_default',
        'weight' => 26,
      ),
    ),
    'entity_type' => 'user',
    'field_name' => 'field_recurring_profile_id',
    'label' => 'Recurring profile id',
    'required' => 0,
    'settings' => array(
      'text_processing' => 0,
      'user_register_form' => 0,
    ),
    'widget' => array(
      'active' => 1,
      'module' => 'text',
      'settings' => array(
        'size' => 60,
      ),
      'type' => 'text_textfield',
      'weight' => 22,
    ),
  );

  // Translatables
  // Included for use with string extractors like potx.
  t('Recurring profile id');

  return $field_instances;
}
