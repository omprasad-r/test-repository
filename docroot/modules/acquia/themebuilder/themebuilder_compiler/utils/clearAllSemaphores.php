<?php

include_once('../SemaphoreManager.inc');

$sm = new SemaphoreManager();
$semaphores = $sm->getAllSemaphores();
$sm->deleteSemaphores($semaphores);

// Remove the delete semaphore also...
$delete = $sm->getSemaphoreInfo(THEMEBUILDER_COMPILER_DELETE_SEMAPHORE);
if ($delete) {
  exec('ipcrm -s ' .escapeShellArg($delete->semid));
}
print("Semaphores cleared.\n");
