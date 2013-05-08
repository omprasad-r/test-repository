<?php

/**
 * Callback for listing user roles.
 */
function _roleresource_index() {
  return user_roles();
}


/**
 * Callback for retrieving roles for a particular user.
 *
 * @param int $uid
 *   The user ID of the user whose roles are to be retrieved.
 *
 * @return array
 *   An associative array:
 *   - uid: The user ID.
 *   - roles: An associative array of the user's roles, keyed by role ID.
 */
function _roleresource_retrieve($uid) {
  $account = user_load($uid);
  if (!$account) {
    return services_error(t('User @uid does not exist.', array('@uid' => $uid)));
  }
  return array(
    'uid' => $uid,
    'roles' => $account->roles,
  );
}

/**
 * Callback for updating user roles.
 *
 * @param int $uid
 *   The user ID of the user to be updated.
 * @param object $roles
 *   An associative array:
 *   - roles: A comma-delimited string of the role IDs to be applied to the
 *     user.
 *
 * @return array
 *   An associative array:
 *   - uid: The user id of the updated user.
 *   - roles: An associative array of the roles applied to the user, keyed by
 *     role ID.
 */
function _roleresource_update($uid, $roles) {
  // Make sure the account exists.
  $account = user_load($uid);
  if (!$account) {
    return services_error(t('User @uid does not exist.', array('@uid' => $uid)));
  }
  // Make sure all the requested roles exist.
  $roles = explode(',', $roles['roles']);
  $possible_roles = user_roles();
  foreach ($roles as &$rid) {
    $rid = trim($rid);
    if (!isset($possible_roles[$rid])) {
      return services_error(t('Role @rid does not exist.', array('@rid' => $rid)));
    }
  }
  unset($rid);

  // Assemble a list of roles that apply to this user, to be returned for
  // informational purposes. The "authenticated user" role is not stored in
  // users_roles, but it applies to every user, so add it first.
  $return[DRUPAL_AUTHENTICATED_RID] = $possible_roles[DRUPAL_AUTHENTICATED_RID];

  // Update the user's roles.
  db_delete('users_roles')
    ->condition('uid', $uid)
    ->execute();
  $query = db_insert('users_roles')
    ->fields(array('uid', 'rid'));
  foreach ($roles as $rid) {
    if (!in_array($rid, array(DRUPAL_ANONYMOUS_RID, DRUPAL_AUTHENTICATED_RID))) {
      $query->values(array(
        'uid' => $account->uid,
        'rid' => $rid,
      ));
      // Add this role to the list of roles being returned.
      $return[$rid] = $possible_roles[$rid];
    }
  }
  $query->execute();
  cache_clear_all($uid, 'cache_entity_user');

  return array(
    'id' => $uid,
    'roles' => $return,
  );
}
