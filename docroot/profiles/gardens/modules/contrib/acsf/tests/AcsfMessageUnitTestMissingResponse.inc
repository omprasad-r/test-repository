<?php

class AcsfMessageUnitTestMissingResponse extends AcsfMessage {

  /**
   * Implements AcsfMessage::sendMessage().
   */
  protected function sendMessage($url, $method, $endpoint, $parameters, $username, $password) {
    return new AcsfMessageResponseUnitTest('unit.test.endpoint', 0, NULL);
  }

}

