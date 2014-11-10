<?php
/**
 * @file
 * Admin functions for the module.
 */

/**
 * Admin settings form.
 */
function livefyre_enterprise_admin($form, &$form_state) {
  $form = array();

  $form['livefyre_enterprise_site_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Livefyre Site ID'),
    '#default_value' => variable_get('livefyre_enterprise_site_id'),
    '#description' => t("The site ID supplied by Livefyre."),
    '#required' => TRUE,
  );

  $form['livefyre_enterprise_site_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Livefyre Site Key'),
    '#default_value' => variable_get('livefyre_enterprise_site_key'),
    '#description' => t("The Site Key supplied by Livefyre."),
    '#required' => TRUE,
  );

  // The Enterprise Hook is used to build the plugin to either include
  // enterprise level functionality. Without this comment, things will not
  // build nicely and sed will most likely break.
  // Enterprise Hook
  include "livefyre-enterprise-settings.inc";

  $form['livefyre_enterprise_sync_activity_number'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of synchronized activity messages'),
    '#default_value' => variable_get('livefyre_enterprise_sync_activity_number', 25),
    '#description' => t("Activity messages are synchronized at every cron run. Please define how many activity messages should be processed at a cron run."),
    '#required' => TRUE,
    '#element_validate' => array('element_validate_integer_positive'),
    '#size' => 10,
  );

  return system_settings_form($form);
}
