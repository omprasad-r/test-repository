<?php

function theme_gardens_alerts(&$vars) {
  $alerts = $vars['alerts'];
  drupal_add_css(drupal_get_path('module', 'gardens_client') . '/gardens_alert.css', array('preprocess' => FALSE));
  drupal_add_library('system', 'drupal.ajax');
  drupal_add_js(drupal_get_path('module', 'gardens_client') . '/gardens_alert.js', array('preprocess' => FALSE));
  drupal_add_js(array(
    'gardensClient' => array(
      'resendEmailLink' => url('gardener/resend-verification-email/' . drupal_get_token('gardens_client_resend_verification_email')),
    ),
  ), array('type' => 'setting'));
  
  $output = '';
  foreach($alerts as $alert) {
    $output .= theme('gardens_alert', $alert);
  }
  return $output;
}

function template_preprocess_gardens_alert(&$vars) {
  $id = $vars['type'];
  $vars['close_link'] = l(t('Close'), $vars['close_path'], array('attributes' => array('class' => array('close'), 'id' => $id . '-close')));
  $ajax_settings = array();
  $ajax_settings[$id . '-close'] = array(
    'selector' => '#' . $id . '-close',
    'wrapper' => $id,
    'url' => base_path() . $vars['close_path'],
    'event' => 'click',
    'progress' => array(),
  );
  drupal_add_js(array('ajax' => $ajax_settings), 'setting');
  
}
