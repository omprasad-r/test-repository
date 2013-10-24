<?php

/**
 * @file
 * Provides basic statistics on APC status.
 */

if (function_exists('apc_cache_info')) {
  $data = array();
  // Set $limited = TRUE on all these calls to omit details.
  $data['system'] = apc_cache_info('', TRUE);
  $data['user'] = apc_cache_info('user', TRUE);
  $data['sma'] = apc_sma_info(TRUE);
  header('Content-type: application/json');
  print json_encode($data);
  exit;
}
