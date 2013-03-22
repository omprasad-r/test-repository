<?php
/**
 * @file
 * Summary
 */

/**
 * Implementation of hook_migrate_api.
 */
function migrate_diggy_migrate_api() {
  $api = array(
    'api' => 2,
  );
  return $api;
}
