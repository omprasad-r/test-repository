<?php

define('AH_HOSTNAME_FULL', php_uname('n'));
define('AH_HOSTNAME_BASE', reset(explode('.', AH_HOSTNAME_FULL)));

/**
 * Exception class for XML-RPC API calls.
 */
class AcquiaGardensRpcException extends Exception {}
/**
 * Exception class for XML-RPC API calls that look like
 * problems communicating with the server: timeout, connection
 * refused, etc.
 */
class AcquiaGardensRpcCommunicationException extends AcquiaGardensRpcException {}
/**
 * Exception class for XML-RPC API calls that return an
 * explicit RPC error.
 */
class AcquiaGardensRpcApiException extends AcquiaGardensRpcException {}


/**
 * Make an XML-RPC call to the Acquia Gardens master server.
 *
 * @param $method
 *   The method name to call, within the 'acquia.gardens' namespace (i.e.,
 *   'acquia.gardens.' will be automatically prepended to the passed in method
 *   name).
 * @param $data
 *   An array of parameters to send as part of the XML-RPC request.
 * @return
 *   The return value of the XML-RPC request. If an error is encountered, an
 *   exception is thrown.
 */
function acquia_gardens_call($method, $data = array(), $server = NULL) {
  $creds = _acquia_gardens_xmlrpc_creds('gardener');

  if (empty($creds['hostname'])) {
    $creds['hostname'] = _acquia_gardens_xmlrpc_server();
  }
  return acquia_gardens_xmlrpc_call('acquia.gardens.'. $method, $data, $creds['hostname'], $creds['username'], $creds['password']);
}

/**
 * Returns RPC credentials stored in the filesystem.
 *
 * These creds will be stored in /mnt/gfs/nobackup/gardens_xmlrpc_creds.ini using
 * normal INI format, e.g.
 *
 * [gardener]
 * hostname = "http://gardener.[stage].acquia-sites.com"
 * username = "acquiagardensrpc"
 * password = "[password]"
 *
 * @param String $site
 *  The name of the acquia site to retrieve, e.g. 'gardener'.
 *
 * @return Array
 *  An array ('hostname', 'username' and 'password') of the RPC configuration.
 */
function _acquia_gardens_xmlrpc_creds($site) {
  static $gardens_xmlrpc_creds = array();
  if (empty($gardens_xmlrpc_creds)) {
    $gardens_xmlrpc_creds = parse_ini_file("/mnt/gfs/nobackup/gardens_xmlrpc_creds.ini", TRUE);
  }

  if (empty($gardens_xmlrpc_creds[$site])) {
    watchdog('gardens_xmlrpc', 'Unable to locate RPC credentials.', array(), WATCHDOG_ERROR);
  }

  return $gardens_xmlrpc_creds[$site];
}

/**
 * Returns the hostname of the Acquia Gardens master server (i.e., the
 * Gardener).
 */
function _acquia_gardens_xmlrpc_server() {
  static $server_name, $stage, $domain;

  if (!isset($stage)) {
    // Get the stage from the local server name, and use it to find the
    // appropriate Gardener to talk to.
    $hostname = trim(`hostname`);
    list($server_name, $stage, $domain) = explode('.', $hostname, 3);
  }
  return "https://gardener.$stage.acquia-sites.com";
}

/**
 * Make an XML-RPC call to an arbitrary server.
 *
 * @param $method
 *   The method name to call.
 * @param $data
 *   Either a single parameter or an array of parameters to send as part of
 *   the XML-RPC request.
 * @param $server
 *   The hostname of the server to contact. This is currently assumed to have
 *   'xmlrpc.php' in its document root (i.e., it is assumed to be running
 *   Drupal and not be installed in a subdirectory).
 * @param $username
 *   The username for basic authentication of the XML-RPC request. If not set,
 *   no authentication will be performed.
 * @param $password
 *   The password for basic authentication of the XML-RPC request. If not set,
 *   no authentication will be performed.
 * @param $format
 *   The encoding format for the response data structure. The default,
 *   'none', encodes the return value directly in XML. 'json' and
 *   'php-serialize' encode the return value as a string in the specified
 *   format before encoding the string in XML; either of these options
 *   are twice as fast and use much less memory than 'none'.
 * @return
 *   The return value of the XML-RPC request.
 * @throws AcquiaGardensRpcException
 */
function acquia_gardens_xmlrpc_call($method, $data, $server, $username = NULL, $password = NULL, $format = 'none') {
  require_once 'XML/RPC.php';
  if (!is_array($data)) {
    $data = array($data);
  }
  $authenticated = isset($username) && isset($password);

  // See http://pear.php.net/manual/en/package.webservices.xml-rpc.examples.php
  $params = $data;
  foreach ($params as &$param) {
    $param = XML_RPC_encode($param);
  }
  $message = new XML_RPC_Message($method, $params);

  $client = new XML_RPC_Client("/xmlrpc.php?caller=".AH_HOSTNAME_BASE."&whoami={$_SERVER['SCRIPT_FILENAME']}&format={$format}&method={$method}", $server);

  // $client->setDebug(1);
  if ($authenticated) {
    $client->setCredentials($username, $password);
  }

  // Try the call up to 3 times. If a retry failure occurs, log it and
  // keep trying, except on the last try throw it. If success or a non-retry
  // failure occurs, fall out of the loop.
  $max_tries = 3;
  for ($tries = 0; $tries < $max_tries; ++$tries) {
    // The timeout argument to send() is a connection timeout. Once
    // the connection succeeds, we'll wait forever for a response.
    $timeout = 5;
    $response = $client->send($message, $timeout);
    if (empty($response)) {
      $error_message = "$method: communication error: errno: {$client->errno}; errstr: {$client->errstr}";
    }
    else if (preg_match('/500 Internal Server Error/i', $response->faultString())) {
      // Don't retry on internal server errors.
      break;
    }
    else if ($response->faultCode() == 5) {
      // "Didn't receive 200 OK from remote server"
      $error_message = "$method: Error - Fault Code: {$response->faultCode()}; Fault Reason: {$response->faultString()}";
    }
    else {
      // Success or non-retry failure.
      break;
    }

    // We have a retry failure.
    if ($tries < $max_tries - 1) {
      watchdog('gardens_xmlrpc', 'Communication error - method: @method - message: @message - tries: @tries/@maxtries', array('@method' => $method, '@message' => $error_message, '@tries' => $tries, '@maxtries' => $max_tries), WATCHDOG_WARNING);
    }
    else {
      throw new AcquiaGardensRpcCommunicationException($error_message);
    }

    sleep(1);
  }

  // We have success or a non-retry failure. $response is guaranteed
  // not to be empty.
  if ($response->faultCode()) {
    watchdog('gardens_xmlrpc', 'Response error - method: @method - code: @code - message: @message', array('@method' => $method, '@code' => $response->faultCode(), '@message' => $response->faultString()), WATCHDOG_ERROR);
    throw new AcquiaGardensRpcApiException("$method: Error - Fault Code: {$response->faultCode()}; Fault Reason: {$response->faultString()}");
  }
  else {
    return XML_RPC_decode($response->value());
  }
}
