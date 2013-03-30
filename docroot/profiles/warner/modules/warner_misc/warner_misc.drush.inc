<?php

/**
 * Implements hook_drush_command().
 */
function warner_misc_drush_command() {
  $items = array();

  $items['warner-consolidate-vocabularies'] = array(
    'description' => "A one-time script to consolidate Pro and Fan categories.",
    'options' => array(
      'pro-video' => 'The vocabulary machine name of the Pro video category.',
      'fan-video' => 'The vocabulary machine name of the Fan video category.',
      'pro-photo' => 'The vocabulary machine name of the Pro photo category.',
      'fan-photo' => 'The vocabulary machine name of the Fan photo category.',
    ),
    'examples' => array(
      'drush warner-consolidate-vocabularies --pro-video=video_category --fan-video=fan_video_category --pro-photo=pro_photo_category --fan-photo=fan_photo_category' => 'Consolidate the pro and fan video and photo vocabularies.',
    ),
  );

  $items['warner-stats-run'] = array(
    'description' => 'Compile stats.',
    'arguments' => array(
      'timestamp' => 'The time of the request.',
    ),
  );

  $items['warner-stats-flush'] = array(
    'description' => 'Flush stats.',
    'arguments' => array(
      'day' => 'The day number for the log file - php date("N")',
      'nid' => 'The gardener nid of this site.',
      'domain' => 'The domain name of this site.',
    ),
  );


  return $items;
}

/**
 * Compile the number of currently active sessions.
 *
 * @param int $timestamp
 *  The timestamp of the common request.
 */
function drush_warner_misc_warner_stats_run($timestamp = REQUEST_TIME) {
  // 'active' sessions can only be so stale.
  $window = variable_get('warner_active_session_window', 300);

  // Get all currently active sessions;
  $session_count = db_select('sessions', 's')
    ->condition('uid', 0, '>')
    ->condition('timestamp', $timestamp - $window, '>')
    ->countQuery()
    ->execute()
    ->fetchField();

  // Track the count.
  db_insert('warner_session_count')
    ->fields(array(
      'timestamp' => $timestamp,
      'count' => $session_count
    ))
    ->execute();
}

/**
 * Writes all session counts to a specified log file and deletes
 * anything older than 48 hours.
 *
 * @param int $day
 *  The day that is being flushed - as in date("N").
 * @param int $nid
 *  The Gardener nid for this site.
 * @param string $domain
 *  The domain name for this site.
 */
function drush_warner_misc_warner_stats_flush($day = 1, $nid, $domain) {
  if (!is_numeric($day) || $day < 1 || $day > 7) {
    exit(1);
  }

  // Get all session counts in the last 24 hours.
  $result = db_select('warner_session_count', 'w')
    ->fields('w', array('timestamp', 'count'))
    ->orderBy('timestamp', 'asc')
    ->groupBy('timestamp')
    ->execute();

  // Clean out entries in the session counter table.
  db_delete('warner_session_count')
    ->execute();

  // Set up log directory.
  $dir = '/var/log/warner';
  if (!file_exists($dir)) {
    mkdir($dir);
  }

  // Set up log file.
  $file = 'warner_stats_' . $day . '.log';
  $fp = fopen($dir . '/' . $file, 'a+');

  // Write each row to CSV.
  foreach ($result as $row) {
    fputcsv($fp, array($nid, $domain, $row->timestamp, $row->count));
  }

  fclose($fp);
}

/**
 * Implements drush_COMMAND_init().
 *
 * This script was originally written with the assumption that the vocabulary
 * names would vary from site to site. In practice, they are uniform, and
 * some shortcuts were taken in the body of the script that rely on these
 * options not changing. If you decide to pass in options to this script, make
 * sure you resolve the TODOs below.
 */
function drush_warner_consolidate_vocabularies_init() {
  $default_options = array(
    'pro-video' => 'pro_video_category',
    'fan-video' => 'fans_video_category',
    'pro-photo' => 'pro_photo_category',
    'fan-photo' => 'fans_photo_category',
  );
  foreach ($default_options as $option_name => $vocabulary_name) {
    $option = drush_get_option($option_name);
    if (empty($option)) {
      drush_set_option($option_name, $vocabulary_name);
    }
  }
}

function drush_warner_misc_warner_consolidate_vocabularies() {
  if (!module_exists('tac_alt_ui')) {
    return drush_set_error('tac_alt_ui required', 'The tac_alt_ui module is required.');
  }

  $vocabulary_identifiers = array(
    'pro_video' => 'pro-video',
    'fan_video' => 'fan-video',
    'pro_photo' => 'pro-photo',
    'fan_photo' => 'fan-photo',
  );
  $vocabularies = array();

  // Basic verification: Make sure we have four valid vocabularies.
  foreach ($vocabulary_identifiers as $variable_name => $option_name) {
    $machine_name = drush_get_option($option_name);
    // Verify that all four vocabularies exist on the site.
    $vocabulary = taxonomy_vocabulary_machine_name_load($machine_name);
    if (!$vocabulary) {
      return drush_set_error('missing vocabulary', dt('The @machine_name vocabulary does not exist.', array('@machine_name' => $machine_name)));
    }
    $vocabularies[$variable_name] = $vocabulary;
    $machine_names[$machine_name] = $vocabulary;
  }

  // Make sure the site is set up the way we expect.
  // - There must be an is_pro boolean field to attach to the new vocabs.
  $is_pro_field = field_info_field('field_is_pro');
  if (empty($is_pro_field) || $is_pro_field['type'] !== 'list_boolean') {
    return drush_set_error('missing_boolean_field', dt('The site must have a boolean is_pro field, so it can be added to the new vocabularies.'));
  }
  // - There must not be an existing "Photo category" or "Video category" vocab.
  $existing_vocab = db_query("SELECT COUNT(1) FROM {taxonomy_vocabulary} WHERE name = 'Photo category' OR name = 'Video category' OR machine_name = 'photo_category' OR machine_name = 'video_category'")->fetchField();
  if ($existing_vocab > 0) {
    return drush_set_error('existing_vocab', dt('The site already has a vocabulary named "Photo category" or "Video category", or with a machine name of photo_category or video_category.  Make sure you have renamed video_category to pro_video_category and photo_category to pro_photo_category before running the script.'));
  }
  // No nodes may have terms from both pro and fan vocabularies.
  $field_info = field_info_fields();
  $vocabulary_fields = array();
  foreach ($field_info as $field_name => $field) {
    if (!empty($field['settings']['allowed_values'][0]['vocabulary'])) {
      if (in_array($field['settings']['allowed_values'][0]['vocabulary'], array_keys($machine_names))) {
        $vocabulary_fields[$field['settings']['allowed_values'][0]['vocabulary']] = $field;
      }
    }
  }
  // TODO: These table names are hard-coded, and therefore the script will not
  // work if the vocabulary names are passed in. Parameterize.
  $overlapping_photo_terms = db_query("SELECT COUNT(pro.entity_id) from field_data_field_photo_category pro join field_data_field_fan_photo_category fan on pro.entity_id = fan.entity_id")->fetchField();
  $overlapping_video_terms = db_query("SELECT COUNT(pro.entity_id) from field_data_field_video_category pro join field_data_field_fan_video_category fan on pro.entity_id = fan.entity_id")->fetchField();
  if ($overlapping_photo_terms || $overlapping_video_terms) {
    return drush_set_error('overlapping_terms', dt('Error: There are @nodes node(s) with terms from both pro and fan vocabularies.', array('@nodes' => $overlapping_photo_terms + $overlapping_video_terms)));
  }

  // Create the two new vocabularies, add an is_pro flag, and make them
  // access-controlled.
  $vocabulary = new stdClass();
  $vocabulary->name = 'Photo category';
  $vocabulary->machine_name = 'photo_category';
  $vocabulary->hierarchy = 0;
  taxonomy_vocabulary_save($vocabulary);
  $vocabulary = taxonomy_vocabulary_machine_name_load($vocabulary->machine_name);
  $photo_category_vocabulary = $vocabulary;
  tac_alt_ui_enable_vocabulary($vocabulary->machine_name);
  tac_redirect_403_enable_vocabulary($vocabulary->machine_name);
  $instance = array(
    'field_name' => 'field_is_pro',
    'entity_type' => 'taxonomy_term',
    'bundle' => $vocabulary->machine_name,
    'label' => 'Is pro',
    'widget' => array(
      'type' => 'options_onoff',
      'module' => 'options',
      'active' => 1,
      'settings' => array('display_label' => 1),
    ),
    'required' => 0,
    'display' => array(
      'default' => array(
        'label' => 'above',
        'type' => 'list_default',
        'settings' => array(),
        'module' => 'list',
        'weight' => 0,
      ),
    ),
    'default_value' => array(
      0 => array(
        'value' => 0,
      ),
    ),
  );
  field_create_instance($instance);

  $vocabulary = new stdClass();
  $vocabulary->name = 'Video category';
  $vocabulary->machine_name = 'video_category';
  $vocabulary->hierarchy = 0;
  taxonomy_vocabulary_save($vocabulary);
  $vocabulary = taxonomy_vocabulary_machine_name_load($vocabulary->machine_name);
  $video_category_vocabulary = $vocabulary;
  tac_alt_ui_enable_vocabulary($vocabulary->machine_name);
  tac_redirect_403_enable_vocabulary($vocabulary->machine_name);
  $instance['bundle'] = $vocabulary->machine_name;
  field_create_instance($instance);

  // Move term reference field data from the fan field into the pro field.
  // TODO: These table names are hard-coded, and therefore the script will not
  // work if the vocabulary names are passed in. Parameterize.
  try {
    db_query("insert into field_data_field_photo_category select * from field_data_field_fan_photo_category");
    db_query("insert into field_data_field_video_category select * from field_data_field_fan_video_category");
    db_query("insert into field_revision_field_photo_category select * from field_revision_field_fan_photo_category");
    db_query("insert into field_revision_field_video_category select * from field_revision_field_fan_video_category");
  }
  catch (Exception $e) {
    return drush_set_error('overlapping_terms', 'There were overlapping terms and the script did not complete. Revert site to backup, correct overlapping terms, and try again.');
  }

  // Move terms from each existing vocabulary to the new vocabularies.
  foreach ($vocabularies as $identifier => $vocabulary) {
    $is_pro = ($identifier == 'pro_video' || $identifier == 'pro_photo') ? 1 : 0;
    $destination_vocabulary = ($identifier == 'pro_video' || $identifier == 'fan_video') ? $video_category_vocabulary : $photo_category_vocabulary;
    $tids = db_query("SELECT tid FROM {taxonomy_term_data} WHERE vid = :vid", array(':vid' => $vocabulary->vid))->fetchCol();

    if ($is_pro) {
      // Move the tac_redirect_403 field from the old pro bundles into the new bundles.
      db_query("UPDATE field_data_tac_redirect_403 SET bundle = :machine_name WHERE bundle = :old_machine_name", array(":machine_name" => $destination_vocabulary->machine_name, ":old_machine_name" => $vocabulary->machine_name));
      db_query("UPDATE field_revision_tac_redirect_403 SET bundle = :machine_name WHERE bundle = :old_machine_name", array(":machine_name" => $destination_vocabulary->machine_name, ":old_machine_name" => $vocabulary->machine_name));
    }

    // Move terms to the new vocabulary by updating taxonomy_term_data.
    db_update('taxonomy_term_data')
      ->condition('vid', $vocabulary->vid)
      ->fields(array('vid' => $destination_vocabulary->vid))
      ->execute();

    // Set the is_pro flag for every term in the vocabulary.
    $field_data_query = db_insert('field_data_field_is_pro')
      ->fields(array('entity_type', 'bundle', 'deleted', 'entity_id', 'revision_id', 'language', 'delta', 'field_is_pro_value'));
    $field_revision_query = db_insert('field_revision_field_is_pro')
      ->fields(array('entity_type', 'bundle', 'deleted', 'entity_id', 'revision_id', 'language', 'delta', 'field_is_pro_value'));
    foreach ($tids as $tid) {
      $values = array(
        'entity_type' => 'taxonomy_term',
        'bundle' => $destination_vocabulary->machine_name,
        'deleted' => 0,
        'entity_id' => $tid,
        'revision_id' => $tid,
        'language' => LANGUAGE_NONE,
        'delta' => 0,
        'field_is_pro_value' => $is_pro,
      );
      $field_data_query->values($values);
      $field_revision_query->values($values);
    }
    $field_data_query->execute();
    $field_revision_query->execute();

    // Handle existing term reference fields pointing to the old vocabs.
    foreach ($vocabulary_fields as $machine_name => $field) {
      if ($vocabulary->machine_name == $machine_name) {
        // Change the vocabulary on existing pro term reference fields.
        if ($is_pro) {
          $field['settings']['allowed_values'][0]['vocabulary'] = $destination_vocabulary->machine_name;
          field_update_field($field);
        }
        // Get rid of fan term reference fields.
        else {
          field_delete_field($field_name);
        }
      }
    }

    // Delete the old vocabulary.
    // TODO: Put this back in when the metatag bug is fixed.
    //taxonomy_vocabulary_delete($vocabulary->vid);
  }

  // Set a flag so that the migration script knows the vocabularies have been
  // combined, and can change its mapping accordingly.
  variable_set('warner_combined_taxonomy', TRUE);
  drush_print("Vocabularies were successfully combined.");
}
