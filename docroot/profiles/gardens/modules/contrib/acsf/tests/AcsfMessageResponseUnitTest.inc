<?php

class AcsfMessageResponseUnitTest extends AcsfMessageResponse {

  public function failed() {
    if ($this->code) {
      return TRUE;
    }
    return FALSE;
  }

}

