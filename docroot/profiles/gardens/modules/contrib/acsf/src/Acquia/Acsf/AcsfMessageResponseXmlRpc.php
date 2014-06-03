<?php

namespace Acquia\Acsf;

/**
 * @file
 * Defines a response from AcsfMessageXmlRpc.
 */

class AcsfMessageResponseXmlRpc extends AcsfMessageResponse {

  /**
   * Implements AcsfMessageResponse::failed().
   */
  public function failed() {
    if ($this->code) {
      return TRUE;
    }
    return FALSE;
  } 
}
