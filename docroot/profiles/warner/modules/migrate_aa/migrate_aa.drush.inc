<?php

/**
 * @file
 * Drush support for the AA migrate module
 */

/**
 * Implements hook_drush_command().
 */
function migrate_aa_drush_command() {
  $items['migrate-aa-fix-passwords'] = array(
    'description' => 'Fix passwords mistakenly double-encrypted',
  );
  return $items;
}

/**
 * gdgfanclub passwords got an extra round of encryption - copy the legacy
 * passwords directly into the destination site.
 */
function drush_migrate_aa_fix_passwords() {
  $connection = array(
    'driver' => 'mysql',
    'database' => 'gdgfanclub_legacy',
    'username' => 'legacy',
    'password' => variable_get('migrate_aa_password', ''),
    'host' => '23.23.120.51',
    'prefix' => '',
  );
  Database::addConnectionInfo('legacy', 'default', $connection);
  $result = Database::getConnection('default', 'legacy')
            ->select('users', 'u')
            ->fields('u', array('pass', 'mail'))
            ->condition('uid', 1, '>')
            ->execute();
  foreach ($result as $row) {
    db_update('users')
      ->fields(array('pass' => $row->pass))
      ->condition('mail', $row->mail)
      ->execute();
  }
}
