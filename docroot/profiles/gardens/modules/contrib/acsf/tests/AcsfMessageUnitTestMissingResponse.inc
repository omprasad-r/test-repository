<?php

require_once __DIR__ . '/../vendor/autoload.php';

class AcsfMessageUnitTestMissingResponse extends \Acquia\Acsf\AcsfMessage {

  /**
   * Implements AcsfMessage::sendMessage().
   */
  protected function sendMessage($url, $method, $endpoint, $parameters, $username, $password) {
    return new AcsfMessageResponseUnitTest('unit.test.endpoint', 0, NULL);
  }

}

