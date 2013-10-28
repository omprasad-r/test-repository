<?php

/**
 * @file
 * Drupal Gardens specific Janrain endpoints for the MASt iPhone app.
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function janrain_client_ctools_plugin_api($module, $api) {
  if ($module == 'oauth_common' && $api == 'oauth') {
    return array(
      'version' => 1,
    );
  }
  if ($module == 'services' && $api == 'services') {
    return array(
      'version' => 3,
    );
  }
}

/**
 * Implements hook_default_services_endpoint().
 */
function janrain_client_default_services_endpoint() {
  $endpoints = array();
  $endpoint = new stdClass;
  $endpoint->disabled = FALSE; /* Edit this to true to make a default endpoint disabled initially */
  $endpoint->api_version = 3;
  $endpoint->name = 'janrain_client';
  $endpoint->server = 'rest_server';
  $endpoint->path = 'janrain-client';
  $endpoint->authentication = array(
    'services_oauth' => array(
      'oauth_context' => 'mast',
      'oauth_type' => 'three-legged',
      'authorization' => '',
      'credentials' => 'token',
    ),
  );
  $endpoint->server_settings = array(
    'formatters' => array(
      'bencode' => TRUE,
      'json' => TRUE,
      'php' => TRUE,
      'rss' => TRUE,
      'xml' => TRUE,
      'jsonp' => FALSE,
    ),
    'parsers' => array(
      'application/json' => TRUE,
      'application/vnd.php.serialized' => TRUE,
      'application/x-www-form-urlencoded' => TRUE,
      'application/xml' => TRUE,
      'multipart/form-data' => TRUE,
      'text/xml' => TRUE,
    ),
  );
  $endpoint->resources = array(
    'janrain-client-user' => array(
      'operations' => array(
        'index' => array(
          'enabled' => '1',
          'settings' => array(
            'services_oauth' => array(
              'credentials' => '',
              'authorization' => '',
            ),
          ),
        ),
      ),
    ),
  );
  $endpoint->debug = 0;
  $endpoints['janrain_client'] = $endpoint;
  return $endpoints;
}

/**
 * Implements hook_services_resources().
 */
function janrain_client_services_resources() {
  $resources = array();

  $resources['janrain-client-user'] = array(
    'index' => array(
      'help' => 'Retrieves the URL for the user specific Social Publishing management page.',
      'callback' => 'janrain_client_services_user_index',
      'access callback' => 'janrain_client_services_user_index_access',
      'access arguments append' => FALSE,
      'file' => array('type' => 'inc', 'module' => 'janrain_client', 'name' => 'janrain_client.services'),
    ),
  );

  return $resources;
}

/**
 * Determines access to the user specific Social Publishing management page.
 */
function janrain_client_services_user_index_access() {
  return _janrain_client_own_networks_edit($GLOBALS['user']);
}

/**
 * Retrieves the URL for the user specific Social Publishing management page.
 */
function janrain_client_services_user_index() {
  return url('user/' . $GLOBALS['user']->uid . '/janrain-networks', array('absolute' => TRUE));
}
