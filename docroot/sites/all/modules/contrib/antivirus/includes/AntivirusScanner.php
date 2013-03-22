<?php

/**
 * @file
 * Definition of AntivirusScanner.
 */

/**
 * @defgroup antivirus_scanners Antivirus Scanners
 * @{
 * Various implementations of antivirus scanners.
 * @}
 */

/**
 * Defines a common interface for an antivirus provider.
 *
 * @ingroup antivirus_scanners
 */
abstract class AntivirusScanner {

  /**
   * Defines the human-readable name of this scanner.
   *
   * @var string
   */
  public $name;

  public function __construct() {
    $this->name = $this->getName();
  }

  /**
   * Reports the status of the scanner to drupal_set_message().
   */
  public function status() {
    $messages = $this->verify();

    if (!empty($messages)) {
      foreach ($messages as $type => $list) {
        foreach ($list as $message) {
          drupal_set_message($message, $type);
        }
      }
    }
    else {
      drupal_set_message(t('Scanner did not report its status.'), 'warning');
    }
  }

  /**
   * Allow each scanner to report the version for debugging.
   */
  public abstract function getVersion();

  /**
   * Allows each scanner to report its name for admin purposes.
   */
  public abstract function getName();

  /**
   * Allows scanners to verify that they are set up correctly.
   *
   * @return array
   *   An array containing two arrays keyed by 'status' and 'error', each
   *   of which contain a list of strings.
   */
  public abstract function verify();

  /**
   * Scans a file.
   *
   * @param $file
   *   The file object to be scanned.
   * @param $options
   *   An array of options for this scan.
   *
   * @return
   *   - ANTIVIRUS_SCAN_OK if no viruses found
   *   - ANTIVIRUS_SCAN_FOUND if viruses are found
   *   - ANTIVIRUS_SCAN_ERROR if the scan failed
   */
  public abstract function scan($file, $options = array());

  /**
   * Defines the configuration form.
   *
   * @param array $form
   *   The Form API array to configure this scanner.
   */
  public abstract function configure(&$form);

  /**
   * Saves custom values from the configuration form.
   *
   * @param array $values
   *   The Form API array of submitted values.
   */
  public abstract function save($values);

}

