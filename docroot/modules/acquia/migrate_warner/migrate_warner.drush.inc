<?php

/**
 * Implementation of hook_drush_command()
 */
function migrate_warner_drush_command() {
  $items = array();
  $items['migrate-warner-video-unpublish'] = array(
    'description' => "Unpublish fan videos which are no longer valid",
  );
  $items['migrate-warner-validate-usernames'] = array(
    'description' => "Identify bad usernames",
    'options' => array(
      'fix' => 'Strip bad characters from usernames',
    ),
  );
  return $items;
}

function drush_migrate_warner_video_unpublish() {
  $query = db_select('node', 'n')
           ->fields('n', array('nid'));
  $query->innerJoin('field_data_field_is_pro', 'ip', 'n.vid=ip.revision_id');
  $query->innerJoin('field_data_field_legacy_url', 'lu', 'n.vid=lu.revision_id');
  $query->fields('lu', array('field_legacy_url_value'));
  $query->condition('n.type', 'video')
        ->condition('ip.field_is_pro_value', 0);
  $result = $query->execute();
  $total = $unpublished = 0;
  foreach ($result as $row) {
    $total++;
    $asset = $row->field_legacy_url_value;
    $data = oembedcore_oembed_data($asset);
    if ($data->type == 'link') {
      $node = node_load($row->nid);
      $node->status = NODE_NOT_PUBLISHED;
      node_save($node);
      $unpublished++;
    }
  }
  drush_print(dt('Unpublished !unpublished of !total total fan videos',
    array('!unpublished' => $unpublished, '!total' => $total)));
}

function drush_migrate_warner_validate_usernames() {
  $fix = drush_get_option('fix');
  if (!$fix) {
    $fix = 0;
  }
  $num_bad = 0;
  $result = db_select('users', 'u')
            ->fields('u', array('uid', 'mail', 'name', 'login'))
            ->condition('uid', 1, '>')
            ->execute();
  foreach ($result as $row) {
    if ($message = user_validate_name($row->name)) {
      $num_bad++;
      drush_print(dt('UID @uid, mail @mail, name \'@name\': @message',
        array('@uid' => $row->uid, '@mail' => $row->mail,
              '@name' => $row->name, '@message' => $message)));
      $original = trim($row->name);
      // Patterns are from user_validate_name
      $name = preg_replace('/[^\x{80}-\x{F7} a-z0-9@_.\'-]/i', '',  $original);
      $name = preg_replace('/[\x{80}-\x{A0}' .         // Non-printable ISO-8859-1 + NBSP
                    '\x{AD}' .                // Soft-hyphen
                    '\x{2000}-\x{200F}' .     // Various space characters
                    '\x{2028}-\x{202F}' .     // Bidirectional text overrides
                    '\x{205F}-\x{206F}' .     // Various text hinting characters
                    '\x{FEFF}' .              // Byte order mark
                    '\x{FF01}-\x{FF60}' .     // Full-width latin
                    '\x{FFF9}-\x{FFFD}' .     // Replacement characters
                    '\x{0}-\x{1F}]/u',        // NULL byte and control characters
                    '', $name);
      $name = preg_replace('/ +/', ' ', $name);
      $name = trim($name);
      $new_name = drush_migrate_warner_dedupe($name, $row->uid, $fix);
      db_merge('migrate_warner_username_fix')
        ->key(array('uid' => $row->uid))
        ->fields(array(
          'original_username' => $row->name,
          'fixed_username' => $new_name,
          'email' => $row->mail,
          'login' => $row->login,
          'reason' => $message,
          'fixed' => $fix,
        ))
        ->execute();
    }
  }
  drush_print(dt('@count bad usernames found', array('@count' => $num_bad)));
}

function drush_migrate_warner_dedupe($original, $uid, $fix) {
  $i = 1;
  // All characters stripped from original, take email as a candidate
  if (!$original) {
    $email = db_select('users', 'u')
                        ->fields('u', array('mail'))
                        ->range(0, 1)
                        ->condition('uid', $uid)
                        ->execute()
                        ->fetchField();
    $original = strtok($email, '@');
    if (!$original) {
      $original = $email;
    }
  }
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

  if ($fix) {
    drush_print("Changing $uid username to $candidate");
    db_update('users')
      ->fields(array('name' => $candidate))
      ->condition('uid', $uid)
      ->execute();
  }
  return $candidate;
}
