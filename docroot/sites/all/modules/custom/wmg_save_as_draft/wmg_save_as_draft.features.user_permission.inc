<?php
/**
 * @file
 * wmg_save_as_draft.features.user_permission.inc
 */

/**
 * Implements hook_user_default_permissions().
 */
function wmg_save_as_draft_user_default_permissions() {
  $permissions = array();

  // Exported permission: 'bypass node access'.
  $permissions['bypass node access'] = array(
    'name' => 'bypass node access',
    'roles' => array(
      'administrator' => 'administrator',
      'site maintainer' => 'site maintainer',
    ),
    'module' => 'node',
  );

  return $permissions;
}
