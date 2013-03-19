#!/usr/bin/php
<?php

/**
 * Creates semaphores without deleting them until no more semaphores can be created.
 */
function consumeAllSemaphores() {
  global $semaphores;
  if (!isset($semaphores)) {
    $semaphores = array();
  }

  $offset = 999;
  $index = 0;
  do {
    $semaphores[$index] = @sem_get($offset + $index);
    $result = $semaphores[$index];
    $index++;
  } while ($result);
  return $index;
}

consumeAllSemaphores();
print("All semaphores are consumed.  Ctrl-C to exit.");

// Hold the process open.  This makes it impossible for the
// themebuilder to create a semaphore.  Unless the semaphore has
// already been created, this will cause the themebuilder to hang
// until this process is stopped.
while (TRUE) {
  sleep(15);
}
