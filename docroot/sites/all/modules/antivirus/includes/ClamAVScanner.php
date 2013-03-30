<?php

/**
 * @file
 * Definition of ClamAVScanner.
 */

/**
 * Implements the Clam AntiVirus scanner.
 *
 * @ingroup antivirus_scanners
 */
class ClamAVScanner extends AntivirusExecutableScanner {

  public function __construct() {
    parent::__construct();
  }

  /**
   * Implements AntivirusScanner::getName().
   */
  public function getName() {
    return 'name';
  }

  /**
   * Implements AntivirusScanner::version().
   */
  public function versionFlags() {
    return array('--version' => '');
  }

  /**
   * Implements AntivirusScanner:getPath().
   */
  public function getPath() {
    global $conf;

    if (isset($conf['antivirus_clamav_path'])) {
      return $conf['antivirus_clamav_path'];
    }
    else {
      $search_paths = array(
        // Linux/Unix paths.
        '/usr/bin/clamscan',
        // Mac paths.
        '/usr/local/bin/clamscan',
        '/Applications/ClamXav.app/Contents/Resources/ScanningEngine/bin/clamscan',
        // Windows paths.
      );

      foreach ($search_paths as $path) {
        if (file_exists($path) && is_executable($path)) {
          return $path;
        }
      }
    }

    return NULL;
  }

  /**
   * Implements AntivirusScanner::scan().
   */
  public function scan($file, $options = array(), $debug = FALSE) {
    $flags = $this->getFlags();
    if (isset($options['flags'])) {
      $flags += $options['flags'];
    }
    array_push($flags, escapeshellarg($file));

    $ret = $this->execute($flags, $output, $error, $debug);

    if ($debug) {
      debug($output, t('Antivirus output'));
      debug($error, t('Antivirus error output'));
    }

    switch ($ret) {
      case ANTIVIRUS_SCAN_OK:
        drupal_set_message(t('No viruses found in %file.', array('%file' => $file)));
        return ANTIVIRUS_SCAN_OK;

      case ANTIVIRUS_SCAN_FOUND:
        drupal_set_message(t('Virus found in %file.', array('%file' => $file), 'warning'));
        return ANTIVIRUS_SCAN_FOUND;

      case ANTIVIRUS_SCAN_ERROR:
        drupal_set_message(t('An error occurred while scanning %file.', array('%file' => $file)), 'error');
        return ANTIVIRUS_SCAN_ERROR;
    }
  }

  /**
   * Implements AntivirusScanner::configure().
   */
  public function configure(&$form) {
    $form['scanner_flags']['#options']['--quiet'] = t('Quiet mode (only print error messages)');
    $form['scanner_flags']['#options']['-i'] = t('Only print infected files');
    $form['scanner_flags']['#options']['-v'] = t('Be verbose');
  }

  /**
   * Implements AntivirusScanner::save().
   */
  public function save($values) {
    variable_set('antivirus_settings_clamav', array(
      'path' => $values['scanner_path'],
      'flags' => $values['scanner_flags'],
    ));
  }

}

