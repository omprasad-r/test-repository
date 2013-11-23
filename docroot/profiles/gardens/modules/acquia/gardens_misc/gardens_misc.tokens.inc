<?php
// $Id$

/**
 * @file
 * Builds placeholder replacement tokens for Gardens-related data.
 */

/**
 * Implements hook_token_info().
 */
function gardens_misc_token_info() {
  if (scarecrow_force_openid()) {
    $description = t('The URL of a !service_name_long registration overlay window appearing above the front page of the site.', array('!service_name_long' => variable_get('gardens_service_name_long', 'Drupal Gardens')));
  }
  else {
    $description = t('The URL of the site\'s registration page.');
  }
  $data['tokens']['site']['register-url'] = array(
    'name' => t('Registration URL'),
    'description' => $description,
  );

  return $data;
}

/**
 * Implements hook_tokens().
 */
function gardens_misc_tokens($type, $tokens, array $data = array(), array $options = array()) {
  if ($type == 'site' && isset($tokens['register-url'])) {
    if (scarecrow_force_openid()) {
      $url_options = array(
        'absolute' => TRUE,
        'fragment' => 'overlay=gardener/register',
      );
      if (isset($options['language'])) {
        $url_options['language'] = $options['language'];
      }
      return array($tokens['register-url'] => url('<front>', $url_options));
    }
    else {
      // @TODO - The url options might need to change when we have the registration overlay
      // in place.
      $url_options = array('absolute' => TRUE);
      if (isset($options['language'])) {
        $url_options['language'] = $options['language'];
      }
      return array($tokens['register-url'] => url('user/register', $url_options));
    }
  }
}
