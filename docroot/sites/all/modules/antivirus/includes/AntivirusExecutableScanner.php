<?php

/**
 * Defines an interface for scanners using a local executable file.
 *
 * @ingroup antivirus_scanners
 */
abstract class AntivirusExecutableScanner extends AntivirusScanner {
  /**
   * The path to the executable antivirus program.
   *
   * @var string
   */
  public $path;

  public function __construct() {
    $this->path = $this->getPath();
    parent::__construct();
  }

  /**
   * Executes a scan command.
   *
   * @param string $command
   *   The virus scanning command to run.
   * @param array $flags
   *   An array of flags to append to the command.
   * @param string $output
   *   The output of the executed command.
   *
   * @return integer
   *   The exit code of executed command.
   */
  final protected function execute($flags, &$stdout = '', &$stderr = '', $debug = FALSE) {
    $descriptorspec = array(
      0 => array('pipe', 'r'),
      1 => array('pipe', 'w'),
      2 => array('pipe', 'w'),
    );

    $flag_list = array();
    foreach ($flags as $key => $value) {
      if (empty($value)) {
        $flag_list[] = $key;
      }
      elseif (is_numeric($key)) {
        $flag_list[] = $value;
      }
      else {
        $flag_list[] = $key . '=' . $value;
      }
    }

    $cmd = $this->getPath() . ' ' . implode(' ', $flag_list);

    if ($debug) {
      debug($cmd, 'Antivirus');
    }

    $process = proc_open(escapeshellcmd($cmd), $descriptorspec, $pipes);
    if (!is_resource($process)) {
      drupal_set_message(t('Unable to execute antivirus command: %cmd', array('%cmd' => $cmd)), 'error');
      return;
    }

    // Save stderr to output variable.
    $stdout = stream_get_contents($pipes[1]);
    $stderr = stream_get_contents($pipes[2]);

    // Close all open pipes to avoid deadlock when closing proc.
    fclose($pipes[0]);
    fclose($pipes[1]);
    fclose($pipes[2]);

    // Capture return code when closing proc.
    return proc_close($process);
  }

  /**
   * Returns the flags set for command-line scanner usage.
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
   * Returns the current version of the antivirus scanner.
   */
  public final function getVersion() {
    if ($flags = $this->versionFlags()) {
      $output = '';
      $ret = $this->execute($this->versionFlags(), $output);
      return $output;
    }

    return NULL;
  }

  /**
   * Implements AntivirusScanner::verify();
   */
  public function verify() {
    $messages = array();

    $path = $this->getPath();
    $version = $this->getVersion();

    if (!empty($path) && !empty($version)) {
      $messages['status'][] = t('Scanner path found and tested at %path.', array('%path' => $path));
    }
    if (empty($path)) {
      $messages['error'][] = t('Scanner path not found at %path.', array('%path' => $path));
    }
    elseif (empty($version)) {
      $messages['error'][] = t('Scanner path could not be tested at %path.', array('%path' => $path));
    }

    return $messages;
  }

  /**
   * Allows each scanner to decide the logic in discovering the path to the
   * antivirus executable.
   */
  public abstract function getPath();

  /**
   * Returns the list of flags required to get the current version of the
   * antivirus scanner.
   */
  protected abstract function versionFlags();

}

