<?php

/**
 * @file
 * Contains AcsfDuplicationScrubUserHandler.
 */

/**
 * Handles the scrubbing of Drupal users.
 */
class AcsfDuplicationScrubUserHandler extends AcsfEventHandler {

  /**
   * Implements AcsfEventHandler::handle().
   */
  public function handle() {
    drush_print(dt('Entered @class', array('@class' => get_class($this))));

    $options = $this->event->context['scrub_options'];
    $user_batch = $options['user_batch'] ? $options['user_batch'] : 1000;

    if ($options['retain_users']) {
      return;
    }

    if ($options['avoid_oom']) {
      if ($uids = self::getItems($user_batch)) {
        self::deleteUsers($uids);
        $this->event->dispatcher->interrupt();
      }
    }
    else {
      do {
        $uids = self::getItems($user_batch);
        if (empty($uids)) {
          break;
        }
        self::deleteUsers($uids);
      } while (TRUE);
    }
  }

  /**
   * Reassigns files owned by the given user ID to the anonymous user.
   *
   * Prior to deleting the user, re-assign {file_managed}.uid to anonymous.
   * Re-assign files only: allow nodes and comments to be deleted. It would be
   * more proper to call file_load_multiple(), iterate each loaded file entity,
   * set its uid property, and call file_save() (see comment_user_cancel() for a
   * similar example for comments). It would be even more proper if file.module
   * implemented hook_user_cancel(), so we could just call that hook. But for
   * performance, we just update the {file_managed} table directly.
   *
   * @param int $uid
   *   The user ID for which to reassign files.
   *
   * @todo Figure out if this still necessary now that the scrub is ordered.
   */
  protected function reassignFiles($uid) {
    db_update('file_managed')
      ->fields(array(
        'uid' => 0,
      ))
      ->condition('uid', $uid)
      ->execute();
  }

  /**
   * Deletes a given list of users.
   *
   * @param array $uids
   *   An indexed array of user IDs to delete.
   */
  protected function deleteUsers($uids = array()) {
    foreach ($uids as $uid) {
      $account = user_load($uid, TRUE);
      if (!gardens_duplication_user_is_openid_admin($account)) {
        self::reassignFiles($account->uid);
        user_delete($account->uid);
      }
    }
  }

  /**
   * Gets a list of Open ID admins.
   *
   * @return array
   *   An indexed array of user IDs representing Open ID admins.
   */
  protected function getOpenIdAdmins() {
    static $admin_roles;
    if (!isset($admin_roles)) {
      $admin_roles = array_filter(array(
        variable_get('user_admin_role'),
        variable_get('gardens_site_owner_role'),
      ));
    }

    $openid_admins = array();
    $uids = db_query('SELECT uid FROM {authmap} WHERE module = :module', array(':module' => 'openid'))->fetchCol();
    foreach ($uids as $uid) {
      $account = user_load($uid);
      $roles = array_intersect(array_keys($account->roles), $admin_roles);
      if (!empty($roles)) {
        $openid_admins[] = $uid;
      }
    }
    return $openid_admins;
  }

  /**
   * Gets a list of user IDs to preserve.
   *
   * @return array
   *   An indexed array of user IDs to preserve.
   */
  public function getPreservedUsers() {
    $preserve = self::getOpenIdAdmins(); // Preserve Open ID admins.
    $preserve[] = 0; // Preserve the anonymous user.
    $preserve[] = 1; // Preserve UID 1.

    // @todo: Provide a way for other modules to preserve users.
    if (module_exists('mast')) {
      if ($mast_uid = variable_get('mast_app_uid')) {
        $preserve[] = $mast_uid;
      }
    }
    return array_unique($preserve);
  }

  /**
   * Counts the remaining users excluding those marked as preserved.
   *
   * @return int
   *   The number of items remaining in the table that do not need to be
   *   preserved.
   */
  public function countRemaining() {
    return db_query('SELECT COUNT(*) FROM {users} WHERE uid NOT IN (:uids)', array(':uids' => self::getPreservedUsers()))->fetchField();
  }

  /**
   * Gets a range of user IDs excluding those marked as preserved.
   *
   * @param int $limit
   *   The number of records to retrieve.
   *
   * @return array
   *   An indexed array containing the relevant UIDs, or an empty array if there
   *   is no result set.
   */
  protected function getItems($limit = 5000) {
    return db_query_range('SELECT uid FROM {users} WHERE uid NOT IN (:uids)', 0, $limit, array(':uids' => self::getPreservedUsers()))->fetchCol();
  }

}

