<?php
// $Id$

/**
 * @file
 * Builds placeholder replacement tokens for Gardens-related data.
 */

/**
 * Implements hook_token_info().
 */
function gardens_misc_token_info() {
  $data = array();

  $data['tokens']['site']['file-public-path'] = array(
    'name' => t('Public file path'),
    'description' => t('Relative path to the uploaded public files.'),
  );

  return $data;
}

/**
 * Implements hook_tokens().
 */
function gardens_misc_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();

  if ($type == 'site' && isset($tokens['file-public-path'])) {
    $replacements[$tokens['file-public-path']] = variable_get('file_public_path');
  }

  return $replacements;
}
