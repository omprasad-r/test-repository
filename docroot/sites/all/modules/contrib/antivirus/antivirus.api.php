<?php

/**
 * @file
 * API documentation for the antivirus module.
 */

/**
 * Defines an available antivirus scanners.
 *
 * This hook allows a module to define one or more scanners that can then be
 * enabled/disabled and configured from within the UI.
 *
 * @return
 *   An array of information defining the antivirus scanners. The array contains
 *   a sub-array for each scanner type, with the machine-readable type name as
 *   the key. Each sub-array has up to 10 attributes.
 *
 *   Possible attributes:
 *   - "name": the human-readable name of the node type. Required.
 *   - "class": the class name used to construct the scanner object. Required.
 *   - "link": a URL to find more information about the antivirus scanner.
 *   - "download": a URL where the antivirus scanner can be downloaded from.
 *   - "hidden": a boolean flag to hide the scanner from administrative screens.
 */
function hook_antivirus_scanner_info() {
  $defaults = array('module' => 'antivirus');

  return array(
    'test' => $defaults + array(
      'name' => t('Test Scanner'),
      'class' => 'TestScanner',
      'hidden' => TRUE,
    ),
  );
}

