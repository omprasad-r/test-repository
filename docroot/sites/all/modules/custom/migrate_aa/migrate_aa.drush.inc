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
  $items['migrate-aa-delete-stub-users'] = array(
    'description' => 'Remove dangling user stub accounts',

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

/**
 * Delete any stub user accounts.
 */
function drush_migrate_aa_delete_stub_users() {
  $count = 0;
  $result = db_select('users', 'u')
            ->fields('u', array('uid'))
            ->condition('status', 0)
            ->condition('name', 'Stub\_%', 'LIKE')
            ->execute();
  foreach ($result as $row) {
    user_delete($row->uid);
    db_delete('migrate_map_user')
      ->condition('destid1', $row->uid)
      ->execute();
    $count++;
  }
  drush_print("$count stub users deleted");
}
