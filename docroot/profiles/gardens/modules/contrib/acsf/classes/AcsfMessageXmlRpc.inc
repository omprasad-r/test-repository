<?php

/**
 * @file
 * This class is an implementation of our XML-RPC service.
 */

class AcsfMessageXmlRpc extends AcsfMessage {
  protected $ah_hostname_full;
  protected $ah_hostname_base;

  /**
   * Custom constructor to look up the hostname.
   */
  public function __construct($method, $endpoint, $parameters = NULL, AcsfConfig $config = NULL, $ah_site = NULL, $ah_env = NULL, Closure $callback = NULL) {
    parent::__construct($method, $endpoint, $parameters, $config, $ah_site, $ah_env, $callback);
    require_once DRUPAL_ROOT . '/includes/common.inc';
    require_once DRUPAL_ROOT . '/includes/xmlrpc.inc';
    $this->ah_hostname_full = php_uname('n');
    $this->ah_hostname_base = reset(explode('.', $this->ah_hostname_full));
  }

  /**
   * Implements AcsfMessage::sendMessage().
   */
  protected function sendMessage($url, $method, $endpoint, $parameters, $username, $password) {
    $xmlrpc_options = array(
      'headers' => array(
        'timeout' => 5,
      ),
    );
    if (!is_array($parameters)) {
      $parameters = array($parameters);
    }
    $authenticated = isset($username) && isset($password);

    $xmlrpc_url = "$url/xmlrpc.php?caller={$this->ah_hostname_base}&whoami={$_SERVER['SCRIPT_FILENAME']}&method={$endpoint}";

    if ($authenticated) {
      $xmlrpc_options['headers']['Authorization'] = 'Basic ' . base64_encode($username . ':' . $password);
    }

    // Try the call up to 3 times. If a retry failure occurs, log it and
    // keep trying, except on the last try throw it. If success or a non-retry
    // failure occurs, fall out of the loop.
    $max_tries = 3;
    for ($tries = 1; $tries <= $max_tries; $tries++) {
      // The timeout argument to send() is a connection timeout. Once
      // the connection succeeds, we'll wait forever for a response.
      $timeout = 5;
      $response = xmlrpc($xmlrpc_url, array($endpoint => $parameters), $xmlrpc_options);

      $errno = xmlrpc_errno();
      $message = xmlrpc_error_msg();
      if (empty($response)) {
        $error_message = "$endpoint: communication error: errno: {$errno}; errstr: {$message}";
      }
      else if (preg_match('/500 Internal Server Error/i', $message)) {
        // Don't retry on internal server errors.
        break;
      }
      else if ($errno == 5) {
        // "Didn't receive 200 OK from remote server"
        $error_message = "$endpoint: Error - Fault Code: {$errno}; Fault Reason: {$message}";
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
    if ($errno) {
      // We might not have a Drupal bootstrap yet.
      if (function_exists('watchdog')) {
        watchdog('gardens_xmlrpc', 'Response error - method: @method - code: @code - message: @message', array('@method' => $endpoint, '@code' => $errno, '@message' => $message), WATCHDOG_ERROR);
      }
      throw new AcsfMessageFailureException ("$method: Error - Fault Code: {$errno}; Fault Reason: {$message}");
    }
    else {
      return new AcsfMessageResponseXmlRpc($endpoint, 0, $response);
    }
  }
}
