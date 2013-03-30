<?php

define('ANTIVIRUS_DAEMON_DEFAULT_TIMEOUT', 10);

/**
 * Defines an interface for scanners using a local executable file.
 *
 * @ingroup antivirus_scanners
 */
abstract class AntivirusDaemonScanner extends AntivirusScanner {
  /**
   * The host name of the daemon server.
   *
   * @var string
   */
  public $host;

  /**
   * The port that the daemon server is listening on.
   *
   * @var string
   */
  public $port;

  public function __construct() {
    $this->host = $this->getHost();
    $this->port = $this->getPort();
    $this->timeout = !empty($this->timeout) ? $this->timeout : ANTIVIRUS_DAEMON_DEFAULT_TIMEOUT;
    parent::__construct();
  }

  /**
   * Returns flags set for command-line scanner usage.
   */
  final protected function getFlags() {
    $enabled_flags = array();
    $flags = variable_get('antivirus_scanner_' . $this->name . '_flags', array());

    foreach ($flags as $flag => $enabled) {
      if ($enabled) {
        $enabled_flags[] = $flag;
      }
    }

    return $enabled_flags;
  }

  /**
   * Reports a path.
   *
   * @todo antivirus.admin.inc shouldn't require a getPath() implementation.
   */
  public function getPath() {
    return 'none';
  }

  /**
   * Implements AntivirusScanner::verify();
   */
  public function verify() {
    $messages = array();

    $handler = ($this->host && $this->port) ? @fsockopen($this->host, $this->port, $errno, $errstr, $this->timeout) : FALSE;

    if ($handler) {
      fclose($handler);
      $messages['status'][] = t('Connected to scanner at %host on port %port.', array('%host' => $this->host, '%port' => $this->port));
    }
    else {
      $messages['error'][] = t('Could not connect to scanner at %host on port %port. The daemon reported the error code: %code with the message %message', array('%host' => $this->host, '%port' => $this->port, '%code' => $errno, '%message' => $errstr));
    }

    return $messages;
  }
}

