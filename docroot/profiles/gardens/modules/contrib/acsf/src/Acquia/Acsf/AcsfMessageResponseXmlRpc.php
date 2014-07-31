<?php

/**
 * @file
 * Defines a response from AcsfMessageXmlRpc.
 */

namespace Acquia\Acsf;

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
