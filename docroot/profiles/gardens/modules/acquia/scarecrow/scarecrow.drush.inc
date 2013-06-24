<?php

/**
 * @file
 * Drush commands for the Scarecrow module.
 */

/**
 * Implements hook_drush_command().
 */
function scarecrow_drush_command() {
  $items['scarecrow-assign-permission'] = array(
    'description' => 'Assigns a permission to a role.',
    'arguments' => array(
      'permission' => 'The machine-readable name of the permission to assign.',
      'role' => 'The name of the role to assign the permission to.',
    ),
  );

  $items['scarecrow-revoke-permission'] = array(
    'description' => 'Revokes a permission from a role.',
    'arguments' => array(
      'permission' => 'The machine-readable name of the permission to revoke.',
      'role' => 'The name of the role to revoke the permission from.',
    ),
  );

  return $items;
}

/**
 * Drush command callback; assigns a permission to a role.
 */
function drush_scarecrow_assign_permission($permission, $role) {
  if (!_scarecrow_validate_drush_permission_role($permission, $role)) {
    return;
  }

  $role_object = user_role_load_by_name($role);
  drush_op('user_role_grant_permissions', $role_object->rid, array($permission));
  // Clear the cached pages and blocks, to mimic what happens when the
  // permissions page form is submitted.
  drush_op('cache_clear_all');
  drush_log(dt('Permission "@permission" was assigned to role "@role".', array('@permission' => $permission, '@role' => $role)), 'success');
}

/**
 * Drush command callback; revokes a permission from a role.
 */
function drush_scarecrow_revoke_permission($permission, $role) {
  if (!_scarecrow_validate_drush_permission_role($permission, $role)) {
    return;
  }

  $role_object = user_role_load_by_name($role);
  drush_op('user_role_revoke_permissions', $role_object->rid, array($permission));
  // Clear the cached pages and blocks, to mimic what happens when the
  // permissions page form is submitted.
  drush_op('cache_clear_all');
  drush_log(dt('Permission "@permission" was revoked from role "@role".', array('@permission' => $permission, '@role' => $role)), 'success');
}

/**
 * Validate the permission and role before assigning or revoking it via Drush.
 *
 * The basic idea here is to forbid more or less anything that cannot be done
 * via the permissions page user interface on the Gardens site (since the Drush
 * commands in this file are ultimately intended to be triggered by a user
 * interface on the Gardener).
 *
 * @return
 *   TRUE if the permission/role combination is valid, or FALSE if it is not.
 */
function _scarecrow_validate_drush_permission_role($permission, $role) {
  if (in_array($permission, scarecrow_forbidden_permissions())) {
    drush_set_error(NULL, dt('The permission @permission is not allowed to be assigned or revoked.', array('@permission' => $permission)));
    return FALSE;
  }

  $all_permissions = array_keys(module_invoke_all('permission'));
  if (!in_array($permission, $all_permissions)) {
    drush_set_error(NULL, dt('The permission "@permission" is not defined by an enabled module.', array('@permission' => $permission)));
    return FALSE;
  }

  $role_object = user_role_load_by_name($role);
  if (empty($role_object)) {
    drush_set_error(NULL, dt('No role named "@role" exists on the site.', array('@role' => $role)));
    return FALSE;
  }

  if (($site_owner_rid = variable_get('gardens_site_owner_role')) && $role_object->rid == $site_owner_rid) {
    drush_set_error(NULL, dt('The "@role" role automatically gets all permissions in Gardens, so it cannot have additional permissions assigned or revoked.', array('@role' => $role)));
    return FALSE;
  }

  return TRUE;
}

