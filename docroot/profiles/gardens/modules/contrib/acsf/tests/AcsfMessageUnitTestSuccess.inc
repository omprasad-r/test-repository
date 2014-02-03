<?php

class AcsfMessageUnitTestSuccess extends AcsfMessage {

  /**
   * Implements AcsfMessage::sendMessage().
   */
  protected function sendMessage($url, $method, $endpoint, $parameters, $username, $password) {
    $response = json_encode(array(
      'url' => $url,
      'method' => $method,
      'endpoint' => $endpoint,
      'parameters' => $parameters,
      'username' => $username,
      'password' => $password,
    ));
    return new AcsfMessageResponseUnitTest($endpoint, 0, $response);
  }

}

