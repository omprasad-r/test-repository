<?php

/**
 * @file
 * Defines a response from AcsfMessageRest.
 */

class AcsfMessageResponseRest extends AcsfMessageResponse {

  /**
   * Implements AcsfMessageResponse::failed().
   */
  public function failed() {
    if ($this->code >= 400) {
      return TRUE;
    }
    return FALSE;
  } 
}
