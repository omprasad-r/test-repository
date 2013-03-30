<?php
$allowed_severity = array(
  WATCHDOG_EMERGENCY => WATCHDOG_EMERGENCY,
  WATCHDOG_ALERT => WATCHDOG_ALERT,
  WATCHDOG_CRITICAL => WATCHDOG_CRITICAL,
  WATCHDOG_ERROR => WATCHDOG_ERROR,
  WATCHDOG_WARNING => 0,
  WATCHDOG_NOTICE => 0,
  WATCHDOG_INFO => 0,
  WATCHDOG_DEBUG => 0,
);

variable_set('dblog_allowed_severity', $allowed_severity);
variable_set('syslog_allowed_severity', $allowed_severity);
?>


