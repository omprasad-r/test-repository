<?php

class UnitTestDummyHandlerInterrupt extends AcsfEventHandler {
  public function handle() {
    $this->event->dispatcher->interrupt();
  }
}

