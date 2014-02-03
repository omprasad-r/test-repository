<?php

class AcsfMessageUnitTestFailureException extends AcsfMessage {

  /**
   * Implements AcsfMessage::sendMessage().
   */
  protected function sendMessage($url, $method, $endpoint, $parameters, $username, $password) {
    throw new AcsfMessageFailureException('error message');
  }

}

