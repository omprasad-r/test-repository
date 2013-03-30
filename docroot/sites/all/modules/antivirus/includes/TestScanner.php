<?php

/**
 * @file
 * Definition of TestScanner.
 */

/**
 * Implements a test scanner.
 *
 * Simple test to verify scanning is taking place and results are evaluated.
 * This "scanner" only tests whether a filename has the word "virus" in the
 * name.
 *
 * @ingroup antivirus_scanners
 */
class TestScanner extends AntivirusExecutableScanner {

  public function __construct() {
    parent::__construct();
  }

  /**
   * Implements AntivirusScanner::version().
   */
  public function versionFlags() {
    return NULL;
  }

  /**
   * Implements AntivirusScanner:getPath().
   */
  public function getPath() {
    return NULL;
  }

  /**
   * Implements AntivirusScanner:getName().
   */
  public function getName() {
    return 'test';
  }

  /**
   * Implements AntivirusScanner:verify().
   */
  public function verify() {
  }

  /**
   * Implements AntivirusScanner::scan().
   */
  public function scan($file, $options = array(), $debug = FALSE) {
    $result = preg_match('/virus/i', basename($file));

    if ($result > 0) {
      drupal_set_message(t('Virus found in file %file.', array('%file' => $file)), 'warning');
      return ANTIVIRUS_SCAN_FOUND;
    }
    else {
      drupal_set_message(t('No viruses found in %file.', array('%file' => $file)));
      return ANTIVIRUS_SCAN_OK;
    }
  }

  /**
   * Implements AntivirusScanner::configure().
   */
  public function configure(&$form) {
    $form = NULL;
  }

  /**
   * Implements AntivirusScanner::save().
   */
  public function save($values) {
  }

}

