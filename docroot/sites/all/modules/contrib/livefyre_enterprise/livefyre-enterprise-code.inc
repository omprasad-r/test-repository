<?php
/**
 * @file
 * Enterprise Livefyre variables and functions.
 */

$lf_auth_delegate = variable_get('livefyre_auth_delegate', '');
$lf_callback = variable_get('livefyre_conv_load_callback', '');
$lf_custom_css = variable_get('livefyre_custom_css', '');

// Add auth delegates to the config.
if ($lf_auth_delegate != '') {
  $lf_config['delegate'] = $lf_auth_delegate;
}

// Add the callback function if there is one. Goes in config['onload'].
if ($lf_callback != '') {
  $lf_config['onload'] = $lf_callback;
}

// Add custom CSS inline.
if ($lf_custom_css != '') {
  drupal_add_css($lf_custom_css, $option['type'] = 'inline');
}
