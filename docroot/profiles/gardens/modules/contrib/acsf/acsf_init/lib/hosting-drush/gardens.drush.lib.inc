<?php
/**
 * @file Contains common utility functions when performing drush commands
 */
 
 function gardens_drush_includes() {
   if (file_exists('/usr/ah/lib/acquia-fields-connect.php')) {
    // Allow connections to the Fields master server.
    require_once '/usr/ah/lib/acquia-fields-connect.php';
  }
  else {
    drush_log('Could not find /usr/ah/lib/acquia-fields-connect.php', 'warning');
  }
 }
 
 // == XML-RPC integration ======================================================

/**
 * Make an XML-RPC request on the Gardener.
 *
 * @param $method
 *   The method to call on the Gardener. It will be prefixed with the
 *   "acquia.gardens" namespace.
 * @param $data
 *   The data to pass to the method being called. Defaults to an empty array.
 *
 * @return
 *   The result of the XML-RPC call, if any, or NULL if no result was available.
 */
function gardens_drush_call_gardener($method, $data = array()) {
  $path = drush_get_context('DRUSH_DRUPAL_ROOT')  .'/../library';
  @include_once("$path/acquia_gardens_xmlrpc.inc");
  if (function_exists('acquia_gardens_call')) {
    try {
      return acquia_gardens_call($method, $data);
    }
    catch (AcquiaFieldsRpcException $e) {
      drush_set_error('HOSTING_ERROR', "Error calling the Gardener: " . $e->getMessage());
      return NULL;
    }
  }
  else {
    // We are probably in a local development environment. Local developers
    // can put a file named acquia_gardens_xmlrpc.inc into their include path
    // if they want to mock up a version that works locally.
    drush_set_error('HOSTING_ERROR', "GardensError: AN-22475 - acquia_gardens_xmlrpc.inc was not included, gardens_client_call_gardener has failed GardensError: method: $method.");
    return NULL;
  }
}
