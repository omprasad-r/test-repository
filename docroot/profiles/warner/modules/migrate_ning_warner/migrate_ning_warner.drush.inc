<?php

/**
 * Implements hook_drush_command().
 */
function migrate_ning_warner_drush_command() {
  $items['migrate-ning-warner-fix-usernames'] = array(
    'description' => 'Repopulate core usernames from field_member_name, with deduping',
    'drupal dependencies' => array('migrate'),
  );
  return $items;
}

function drush_migrate_ning_warner_fix_usernames() {
  $result = db_select('field_data_field_member_name', 'n')
            ->fields('n', array('entity_id', 'field_member_name_value'))
            ->execute();
  foreach ($result as $row) {
    $deduped = migrate_ning_warner_handle_dedupe($row->field_member_name_value);
    db_update('users')
      ->fields(array('name' => $deduped))
      ->condition('uid', $row->entity_id)
      ->execute();
  }
}

function migrate_ning_warner_handle_dedupe($original) {
  $i = 1;
  $candidate = $original;
  while ($candidate_found = db_select('users', 'u')
                    ->fields('u', array('name'))
                    ->range(0, 1)
                    ->condition('name', $candidate)
                    ->execute()
                    ->fetchField()) {
    // We already have the candidate value. Find a non-existing value.
    $i++;
    $candidate = $original . '_' . $i;
  }
  return $candidate;
}
