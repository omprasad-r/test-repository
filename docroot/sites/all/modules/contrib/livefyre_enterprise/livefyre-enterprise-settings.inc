<?php
/**
 * @file
 * Enterprise level settings forms.
 */

$form['livefyre_enterprise_network'] = array(
  '#type' => 'textfield',
  '#title' => t('Livefyre Network'),
  '#default_value' => variable_get('livefyre_enterprise_network', 'livefyre.com'),
  '#description' => t("Livefyre Network. Do not change unless on a custom network."),
  '#required' => TRUE,
);

$form['livefyre_enterprise_network_key'] = array(
  '#type' => 'textfield',
  '#title' => t('Livefyre Network Key'),
  '#default_value' => variable_get('livefyre_enterprise_network_key'),
  '#description' => t("Livefyre Network Key. Do not change unless on a custom network."),
  '#required' => TRUE,
);

$form['livefyre_enterprise_auth_delegate'] = array(
  '#type' => 'textarea',
  '#title' => t('Livefyre Authentication Delegate'),
  '#description' => t("Set the Authentication Delegate for Livefyre. This will override 'delegate' in the App configuration. This function must return an auth delegate variable built according to the following: <a href='https://github.com/Livefyre/livefyre-docs/wiki/Comments-3-Integration-Guide#wiki-single-sign-on'> Livefyre Single Sign On</a>"),
  '#default_value' => variable_get('livefyre_enterprise_auth_delegate', ''),
  '#resizable' => TRUE,
  '#rows' => 5,
);

$form['livefyre_enterprise_conv_load_callback'] = array(
  '#type' => 'textarea',
  '#title' => t('Livefyre Conversation Load Callback'),
  '#description' => t("Set the function to call back to when the widget loads. This will override 'onload' in the App configuration."),
  '#default_value' => variable_get('livefyre_enterprise_conv_load_callback', ''),
  '#resizable' => TRUE,
  '#rows' => 5,
);

$form['livefyre_enterprise_custom_css'] = array(
  '#type' => 'textarea',
  '#title' => t('Livefyre Custom CSS'),
  '#description' => t("Add custom CSS to your Livefyre Comment Widget. This CSS will appear inline on the page. Please use only Livefyre specific adjustments or rules will be applied to non-Livefyre tags."),
  '#default_value' => variable_get('livefyre_enterprise_custom_css', ''),
  '#resizable' => TRUE,
  '#rows' => 5,
);

$form['livefyre_enterprise_environment'] = array(
  '#type' => 'checkbox',
  '#title' => t('Livefyre Production Envrionment'),
  '#default_value' => variable_get('livefyre_enterprise_environment', 1),
  '#description' => t("Livefyre Environment. Select this checkbox if you are now using your Livefyre Production Credentials."),
);

$form['livefyre_enterprise_jr_capture'] = array(
  '#type' => 'checkbox',
  '#title' => t('Janrain Capture'),
  '#default_value' => variable_get('livefyre_enterprise_jr_capture', 0),
  '#description' => t("Livefyre Integration with Janrain Capture. Select this checkbox if you are integrating with Janrain Capture Profiling System."),
);
