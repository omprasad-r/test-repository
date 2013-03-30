<?php

require_once('OAuth.php');

/**
 * The OAuthClient class is a simple two-legged OAuth client that makes OAuth
 * requests via cURL.
 */
class OAuthClient {
  function __construct($options = array()) {
    foreach ($options as $option => $value) {
      $this->$option = $value;
    }
    $this->consumer = new OAuthConsumer($this->key, $this->secret);
    $this->signature_method = new OAuthSignatureMethod_HMAC_SHA1();
  }

  /**
   * Returns the POST parameters for this request, if any.
   *
   * @return array
   *   The POST parameters for this request, or an empty array if none.
   */
  function getData() {
    if (!isset($this->data)) {
      $this->data = array();
    }
    return $this->data;
  }

  /**
   * Returns the full URL to which a cURL request is being made.
   *
   * @return string
   *   The URL being requested (i.e. http://example.com/api/method.json).
   */
  function getAbsolutePath() {
    return $this->endpoint . $this->path;
  }

  /**
   * Returns the HTTP method for this request (i.e. GET, POST, PUT, or DELETE).
   *
   * @return string
   *   The HTTP method being used for this request. Defaults to GET.
   */
  function getHttpMethod() {
    if (!isset($this->httpMethod)) {
      $this->httpMethod = 'GET';
    }
    return $this->httpMethod;
  }

  /**
   * Returns the OAuthRequest object being used for this request.
   *
   * @return OAuthRequest
   */
  function getOAuthRequest() {
    if (!isset($this->request)) {
      $this->request = OAuthRequest::from_consumer_and_token($this->consumer, NULL, $this->getHttpMethod(), $this->getAbsolutePath());
      $this->request->sign_request($this->signature_method, $this->consumer, NULL);
    }
    return $this->request;
  }

  /**
   * Returns the cURL options being used for this request.
   *
   * @return array
   *   An array of options suitable for curl_setopt_array().
   */
  function getCurlOptions() {
    $method = $this->getHttpMethod();
    $request = $this->getOAuthRequest();
    switch ($method) {
      case 'GET':
        $options = array(
          CURLOPT_RETURNTRANSFER => TRUE,
          CURLOPT_URL => $request->to_url(),
          CURLOPT_HEADER => TRUE,
        );
        break;
      case 'POST':
        $options = array(
          CURLOPT_RETURNTRANSFER => TRUE,
          CURLOPT_URL => $request->to_url(),
          CURLOPT_POST => TRUE,
          CURLOPT_POSTFIELDS => json_encode($this->getData()),
          CURLOPT_HEADER => TRUE,
          CURLOPT_HTTPHEADER => array('Content-Type: application/json'),
        );
        break;
      case 'PUT':
        $options = array(
          CURLOPT_RETURNTRANSFER => TRUE,
          CURLOPT_URL => $request->to_url(),
          CURLOPT_CUSTOMREQUEST => 'PUT',
          CURLOPT_POSTFIELDS => json_encode($this->getData()),
          CURLOPT_HEADER => TRUE,
          CURLOPT_HTTPHEADER => array('Content-Type: application/json'),
        );
        break;
      case 'DELETE':
        $options = array(
          CURLOPT_RETURNTRANSFER => TRUE,
          CURLOPT_URL => $request->to_url(),
          CURLOPT_CUSTOMREQUEST => 'DELETE',
          CURLOPT_HEADER => TRUE,
        );
        break;
      default:
        throw new OAuthClientException("Only GET, POST, PUT, and DELETE are allowable HTTP methods.");
    }
    return $options;
  }

  function exec() {
    $options = $this->getCurlOptions();
    $ch = curl_init();
    curl_setopt_array($ch, $options);
    $this->result = curl_exec($ch);
    return $this->code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
  }

  function getResultCode() {
    if ($this->code) {
      return $this->code;
    }
    else {
      throw new OAuthClientException("No cURL request has been made yet.");
    }
  }

  function getResult() {
    if (isset($this->result)) {
      return $this->result;
    }
    else {
      throw new OAuthClientException("No cURL request has been made yet.");
    }
  }
}

class OAuthClientException extends Exception {}
