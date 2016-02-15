<?php
/**
 * @file
 * Define module permissions.
 */

/**
 * Implements hook_permission().
 */
function environment_libraries_permission() {
  return array(
    'administer environment_libraries settings' => array(
      'title' => t('Administer Environment Libraries settings'),
      'description' => t('Perform administration tasks for environment_libraries.'),
      'restrict access' => TRUE,
    ),
    'view non-production environment_libraries' => array(
      'title' => t('View Non-production libraries'),
      'description' => t('Users with this role will have access to view non-production libraries when in one of those environments. Othwerwise only production will be used.'),
      'restrict access' => FALSE,
    ),
  );
}

/**
 * Implements hook_user_default_permissions().
 */
function environment_libraries_user_default_permissions() {
  $permissions = array();
  // Only admins have access to settings by default.
  $permissions['administer environment_libraries settings'] = array(
    'name' => 'administer environment_libraries settings',
    'roles' => array(
      'administrator',
    ),
    'module' => 'environment_libraries',
  );

  $permissions['view non-production environment_libraries'] = array(
    'name' => 'view non-production environment_libraries',
    'roles' => array(
      'administrator',
    ),
    'module' => 'environment_libraries',
  );
  return $permissions;
}
