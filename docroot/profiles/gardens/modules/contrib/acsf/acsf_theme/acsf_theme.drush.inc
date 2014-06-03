<?php

/**
 * @file
 * Provides drush commands for theme change notifications.
 */

/**
 * Implements hook_drush_command()
 */
function acsf_theme_drush_command() {
  return array(
    'acsf-theme-notify' => array(
      'description' => dt('Sends a notification to the Factory that a theme event has occurred.'),
      'options' => array(
        'event' => dt('The type of theme event to notify the Factory about. Possible values are "create", "modify", and "delete". Defaults to "modify".'),
        'theme' => dt('The system name of the theme the event relates to. Defaults to the current theme.'),
      ),
    ),
  );
}

/**
 * Drush command callback: Sends a theme change notification to the Factory.
 */
function drush_acsf_theme_notify() {
  $theme = drush_get_option('theme', variable_get('theme_default'));
  $event = drush_get_option('event', 'modify');
  if (!in_array($event, array('create', 'modify', 'delete'))) {
    return drush_set_error(dt('Event type not supported. Possible values are "create", "modify", or "delete".'));
  }
  $notify = acsf_theme_notification();
  $response = $notify->sendNotification($event, $theme);
  drush_print($response['message']);
}
