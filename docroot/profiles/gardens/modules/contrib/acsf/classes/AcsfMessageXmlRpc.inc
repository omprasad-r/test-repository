<?php

/**
 * @file
 * This class is an implementation of our XMLRPC service.
 */

class AcsfMessageXmlRpc extends AcsfMessage {
  protected $ah_hostname_full;
  protected $ah_hostname_base;

  /**
   * Custom constructor to look up the hostname.
   */
  public function __construct($endpoint, $parameters = NULL, AcsfConfig $config = NULL, $ah_site = NULL, $ah_env = NULL, Closure $callback = NULL) {
    parent::__construct($endpoint, $parameters, $config, $ah_site, $ah_env, $callback);
    $this->ah_hostname_full = php_uname('n');
    $this->ah_hostname_base = reset(explode('.', $this->ah_hostname_full));
  }

  /**
   * Implements AcsfMessage::sendMessage().
   */
  protected function sendMessage($url, $endpoint, $parameters, $username, $password) {
    require_once 'XML/RPC.php';
    if (!is_array($parameters)) {
      $parameters = array($parameters);
    }
    $authenticated = isset($username) && isset($password);
  
    // See http://pear.php.net/manual/en/package.webservices.xml-rpc.examples.php
    foreach ($parameters as &$param) {
      $param = XML_RPC_encode($param);
    }
    $message = new XML_RPC_Message($endpoint, $parameters);

    // @todo $format == "none" ??
    $client = new XML_RPC_Client("/xmlrpc.php?caller=" . $this->ah_hostname_base . "&whoami={$_SERVER['SCRIPT_FILENAME']}&format={$format}&method={$endpoint}", $url);
  
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
        $error_message = "$endpoint: communication error: errno: {$client->errno}; errstr: {$client->errstr}";
      }
      else if (preg_match('/500 Internal Server Error/i', $response->faultString())) {
        // Don't retry on internal server errors.
        break;
      }
      else if ($response->faultCode() == 5) {
        // "Didn't receive 200 OK from remote server"
        $error_message = "$endpoint: Error - Fault Code: {$response->faultCode()}; Fault Reason: {$response->faultString()}";
      }
      else {
        // Success or non-retry failure.
        break;
      }
  
      // We have a retry failure.
      if ($tries < $max_tries - 1) {
        // We might not have a Drupal bootstrap yet.
        if (function_exists('watchdog')) {
          watchdog('gardens_xmlrpc', 'Communication error - method: @method - message: @message - tries: @tries/@maxtries', array('@method' => $endpoint, '@message' => $error_message, '@tries' => $tries, '@maxtries' => $max_tries), WATCHDOG_WARNING);
        }
      }
      else {
        throw new AcsfMessageFailureException ($error_message);
      }
  
      sleep(1);
    }
  
    // We have success or a non-retry failure. $response is guaranteed
    // not to be empty.
    if ($response->faultCode()) {
      // We might not have a Drupal bootstrap yet.
      if (function_exists('watchdog')) {
        watchdog('gardens_xmlrpc', 'Response error - method: @method - code: @code - message: @message', array('@method' => $endpoint, '@code' => $response->faultCode(), '@message' => $response->faultString()), WATCHDOG_ERROR);
      }
      throw new AcsfMessageFailureException ("$method: Error - Fault Code: {$response->faultCode()}; Fault Reason: {$response->faultString()}");
    }
    else {
      $value = XML_RPC_decode($response->value());
      return new AcsfMessageResponseXmlRpc($endpoint, 0, $value);
    }
  }
}
