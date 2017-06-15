<?php
/**
 * @file
 * wmg_magento_api.services.inc
 */

/**
 * Implements hook_default_services_endpoint().
 */
function wmg_magento_api_default_services_endpoint() {
  $export = array();

  $endpoint = new stdClass();
  $endpoint->disabled = FALSE; /* Edit this to true to make a default endpoint disabled initially */
  $endpoint->api_version = 3;
  $endpoint->name = 'restapi';
  $endpoint->server = 'rest_server';
  $endpoint->path = 'restapi';
  $endpoint->authentication = array();
  $endpoint->server_settings = array(
    'formatters' => array(
      'bencode' => TRUE,
      'json' => TRUE,
      'php' => TRUE,
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
    'subscription' => array(
      'actions' => array(
        'add' => array(
          'enabled' => '1',
        ),
        'getstatus' => array(
          'enabled' => '1',
        ),
      ),
    ),
  );
  $endpoint->debug = 1;
  $export['restapi'] = $endpoint;

  return $export;
}
