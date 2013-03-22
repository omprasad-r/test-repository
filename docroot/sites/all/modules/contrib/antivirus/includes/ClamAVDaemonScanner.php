<?php

/**
 * @file
 * Definition of ClamAVDaemonScanner.
 */

/**
 * Defines the default host for clamd.
 */
define('ANTIVIRUS_CLAMAVDAEMON_DEFAULT_HOST', 'localhost');

/**
 * Defines the default port for clamd.
 */
define('ANTIVIRUS_CLAMAVDAEMON_DEFAULT_PORT', '3310');

/**
 * Implements the Clam AntiVirus scanner for a daemon process.
 *
 * @ingroup antivirus_scanners
 */
class ClamAVDaemonScanner extends AntivirusDaemonScanner {
  protected $settings;

  public function __construct() {
    $this->settings = variable_get('antivirus_settings_clamavdaemon', array());
    parent::__construct();
  }

  /**
   * Implements AntivirusScanner::getName().
   */
  public function getName() {
    return 'clamavdaemon';
  }

  /**
   * Implements AntivirusDaemonScanner::getHost().
   */
  public function getHost() {
    return isset($this->settings['host']) ? $this->settings['host'] : ANTIVIRUS_CLAMAVDAEMON_DEFAULT_HOST;
  }

  /**
   * Implements AntivirusDaemonScanner::getPort().
   */
  public function getPort() {
    return isset($this->settings['port']) ? $this->settings['port'] : ANTIVIRUS_CLAMAVDAEMON_DEFAULT_PORT;
  }

  /**
   * Implementation of AntivirusScanner::getVersion().
   */
  public function getVersion() {
    $handler = ($this->host && $this->port) ? @fsockopen($this->host, $this->port, $errno, $errstr, $this->timeout) : FALSE;

    if ($handler) {
      fwrite($handler, "VERSION");
      $version = fgets($handler);
      fclose($handler);
      return $version;
    }
  }

  /**
   * Implementation of AntivirusScanner::scan().
   */
  public function scan($file, $options = array(), $debug = FALSE) {
    $settings = variable_get('antivirus_settings_clamavdaemon', array());

    $flags = $this->getFlags();
    if (isset($options['flags'])) {
      $flags += $options['flags'];
    }
    array_push($flags, escapeshellarg($file));

    // Try to open a socket to the ClamAV Deamon.
    $handler = ($this->host && $this->port) ? @fsockopen($this->host, $this->port, $errno, $errstr, $this->timeout) : FALSE;

    if (!$handler) {
      watchdog('antivirus', 'The clamav module can not connect to the ClamAV Daemon. The uploaded file %file could not be scanned. The daemon reported the error code: %code with the message %message', array('%file' => $file, '%code' => $errno, '%message' => $errstr), WATCHDOG_WARNING);

      return ANTIVIRUS_SCAN_ERROR;
    }

    // Request a scan from the daemon.
    $filehandler = fopen($file, 'r');
    if ($filehandler) {
      // Open a request with the daemon to stream file data.
      fwrite($handler, "zINSTREAM\0");
      $bytes = filesize($file);
      if ($bytes > 0) {
        // Tell the daemon how many bytes of data we're sending.
        fwrite($handler, pack("N", $bytes));
        // Send the file data.
        stream_copy_to_stream($filehandler, $handler);
      }
      // Send a zero-length block to indicate that we're done sending file data.
      fwrite($handler, pack("N", 0));
      $response = fgets($handler);
      fclose($filehandler);
      fclose($handler);
      $response = trim($response);

      if ($debug) {
        watchdog('antivirus', 'ClamAV Daemon response for %file: %response', array('%file' => $file, '%response' => $response), WATCHDOG_NOTICE);
      }
    }
    else {
      watchdog('antivirus', 'Uploaded file %file could not be scanned: failed to open file handle.', array('%file' => $file), WATCHDOG_WARNING);

      return ANTIVIRUS_SCAN_ERROR;
    }

    // The clamd daemon returns a string response in the format:
    //   stream: OK
    //   stream: <name of virus> FOUND
    //   stream: <error string> ERROR
    if (preg_match('/^stream: OK$/', $response)) {
      // Log the message to watchdog, if verbose mode is used.
      if (in_array('-v', $flags)) {
        watchdog('antivirus', 'File %file scanned by ClamAV Daemon and found clean.', array('%file' => $file), WATCHDOG_INFO);
      }

      return ANTIVIRUS_SCAN_OK;
    }
    elseif (preg_match('/^stream: (.*) FOUND$/', $response, $matches)) {
      $virus_name = $matches[1];
      watchdog('antivirus', 'Virus detected in uploaded file %file. ClamAV Daemon reported the virus:<br/>@virus_name', array('%file' => $file, '@virus_name' => $virus_name), WATCHDOG_CRITICAL);

      return ANTIVIRUS_SCAN_FOUND;
    }
    else {
      // Try to extract the error message from the response.
      preg_match('/^stream: (.*) ERROR$/', $response, $matches);
      $error_string = $matches[1]; // the error message given by the daemon
      watchdog('antivirus', 'Uploaded file %file could not be scanned. ClamAV Daemon reported:<br/>@error_string', array('%file' => $file, '@error_string' => $error_string), WATCHDOG_WARNING);

      return ANTIVIRUS_SCAN_ERROR;
    }
  }

  /**
   * Implements AntivirusScanner::configure().
   */
  public function configure(&$form) {
    $settings = variable_get('antivirus_settings_clamavdaemon', array());

    // @todo Flags aren't really appropriate here, move verbose to something global?
    $form['scanner_flags']['#options']['-v'] = t('Be verbose');

    $form['scanner_info']['daemon_host'] = array(
      '#title' => t('ClamAV Daemon host'),
      '#type' => 'textfield',
      '#default_value' => $settings['host'],
    );

    $form['scanner_info']['daemon_port'] = array(
      '#title' => t('ClamAV Daemon port'),
      '#type' => 'textfield',
      '#default_value' => $settings['port'],
    );

    $form['scanner_info']['daemon_timeout'] = array(
      '#title' => t('ClamAV Daemon Timeout'),
      '#type' => 'textfield',
      '#default_value' => $settings['timeout'],
    );
  }

  /**
   * Implements AntivirusScanner::save().
   */
  public function save($values) {
    variable_set('antivirus_settings_clamavdaemon', array(
      'host' => $values['daemon_host'],
      'port' => $values['daemon_port'],
      'timeout' => $values['daemon_timeout'],
    ));
    $this->host = $this->settings['host'] = $values['daemon_host'];
    $this->port = $this->settings['port'] = $values['daemon_port'];
    $this->timeout = $this->settings['timeout'] = $values['daemon_timeout'];
  }

}

