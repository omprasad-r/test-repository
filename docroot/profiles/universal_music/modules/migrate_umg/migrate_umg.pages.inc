<?php
/**
 * @file
 * Configuration forms for UMG migration.
 */
 

/**
 * Menu callback
 */
function migrate_umg_configure() {
  return drupal_get_form('migrate_umg_configure_form');
}

/**
 * Form for reviewing migrations.
 */
function migrate_umg_configure_form($form, &$form_state) {
  $build = array();

  $build['description'] = array(
     '#prefix' => '<div>',
     '#markup' => t('Configure the source database for artist imports. This
      information will be saved in encrypted form.'),
     '#suffix' => '</div>',
  );

  $connection = migrate_umg_connection();

  $build['host'] = array(
    '#type' => 'textfield',
    '#title' => t('MySQL host'),
    '#description' => t('The domain or IP address of the MySQL server holding the
      source database'),
    '#default_value' => $connection['host'],
  );
  $build['username'] = array(
    '#type' => 'textfield',
    '#title' => t('MySQL username'),
    '#description' => t('The MySQL username (should be select-only)'),
    '#default_value' => $connection['username'],
  );
  $build['password'] = array(
    '#type' => 'password',
    '#title' => t('MySQL password'),
    '#description' => t('The password of the above MySQL account'),
    '#default_value' => $connection['password'],
  );
  $build['database'] = array(
    '#type' => 'textfield',
    '#title' => t('MySQL database'),
    '#description' => t('The database holding the source data'),
    '#default_value' => $connection['database'],
  );
  $build['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
    '#submit' => array('migrate_umg_configure_submit'),
    '#validate' => array('migrate_umg_configure_validate'),
  );
  return $build;
}

/**
 * Validate callback for the configuration form handler fieldset.
 */
function migrate_umg_configure_validate($form, &$form_state) {
  $connection = array(
    'host' => $form_state['values']['host'],
    'username' => $form_state['values']['username'],
    'password' => $form_state['values']['password'],
    'database' => $form_state['values']['database'],
    'driver' => 'mysql',
  );
  try {
    Database::addConnectionInfo('umg_legacy', 'default', $connection);
    $artist_count = Database::getConnection('default', 'umg_legacy')
                    ->select('artists', 'a')
                    ->countQuery()
                    ->execute()
                    ->fetchField();
  }
  catch (Exception $e) {
    form_set_error('host', $e->getMessage());
    return;
  }
  if (!$artist_count) {
    form_set_error('host', t('Could not open connection to source database, please
      check the address and credentials'));
  }
}

/**
 * Submit callback for the configuration form handler fieldset.
 */
function migrate_umg_configure_submit($form, &$form_state) {
  $connection = array(
    'host' => $form_state['values']['host'],
    'username' => $form_state['values']['username'],
    'password' => $form_state['values']['password'],
    'database' => $form_state['values']['database'],
    'driver' => 'mysql',
  );
  variable_set('migrate_umg_connection', migrate_umg_encrypt($connection));
  drupal_set_message(t('Your configuration changes have been saved.'));
  $form_state['redirect'] = 'admin/content/import/umg_add';
}

/**
 * Menu callback
 */
function migrate_umg_add_artist() {
  return drupal_get_form('migrate_umg_add_artist_form');
}

/**
 * Form for adding migration for an artist.
 */
function migrate_umg_add_artist_form($form, &$form_state) {
  $build = array();

  $connection_array = migrate_umg_connection();
  if (empty($connection_array['host'])) {
    drupal_set_message(t('You must set up the source database connection before
      you can add an artist for import.'));
    drupal_goto('admin/content/import/umg_config');
  }
  Database::addConnectionInfo('umg_legacy', 'default', $connection_array);

  $build['description'] = array(
     '#prefix' => '<div>',
     '#markup' => t('Set up the import of a given artist\'s content into this site.'),
     '#suffix' => '</div>',
  );

  $artist_options = array();
  $connection = Database::getConnection('default', 'umg_legacy');
  if (!$connection) {
    drupal_set_message(t('To import artist content, a connection named ' .
      '<em>umg_legacy</em> must be defined in the site settings.php file.'));
  }
  else {
    $result = $connection->select('artists', 'a')
              ->fields('a', array('ArtistID', 'FName', 'LName'))
              ->condition('Status', 2)
              ->execute();
    foreach ($result as $row) {
      $first = trim($row->FName);
      $last = trim($row->LName);
      if ($first) {
        $name = $first . ' ' . $last;
      }
      else {
        $name = $last;
      }
      $name .= ' (' . $row->ArtistID . ')';
      $artist_options[$row->ArtistID] = $name;
    }
    natcasesort($artist_options);
  }

  $build['artist'] = array(
    '#type' => 'select',
    '#options' => $artist_options,
    '#title' => t('Artist name'),
    '#description' => t('Choose the artist whose content is to be imported into this ' .
                       'Gardens site. The unique ArtistID of each artist is in parentheses.'),
  );

  $user_options = array();
  $query = db_select('users', 'u')
           ->fields('u', array('uid', 'name'));
  $query->innerJoin('users_roles', 'ur', 'u.uid=ur.uid');
  $query->innerJoin('role', 'r', 'ur.rid=r.rid');
  $query->condition('r.name', array('administrator', 'content manager', 'editor', 'site maintainer'), 'IN');
  $result = $query->execute();
  foreach ($result as $row) {
    $user_options[$row->uid] = $row->name;
  }

  // The starter site has a default editor account with uid 241 - use that as
  // the default content author, but allow another admin-type account to be
  // selected instead.
  if (isset($user_options[241])) {
    $default_value = 241;
  }
  else {
    $default_value = 1;
  }

  $build['default_author'] = array(
    '#type' => 'select',
    '#options' => $user_options,
    '#title' => t('Default author'),
    '#default_value' => $default_value,
    '#description' => t('Choose the user account that will serve as the author of ' .
      'imported content. All accounts that have one of the roles <em>administrator</em>, ' .
      '<em>content manager</em>, <em>editor</em>, or <em>site maintainer</em> are ' .
      'listed.'),
  );

  $build['youtube_feed_username1'] = array(
    '#type' => 'textfield',
    '#title' => t('Youtube feed usernames'),
    '#description' => t('Enter the Youtube username for this artist\'s video ' .
      'feed. For example, the feed for Fergie is ' .
      'https://gdata.youtube.com/feeds/api/users/FergieVEVO/uploads, so enter ' .
      '<em>FergieVEVO</em>. Leave this field empty to skip video migration for ' .
      'this artist.'),
  );
  $build['youtube_feed_username2'] = array(
    '#type' => 'textfield',
    '#description' => t('Additional Youtube username.')
  );
  $build['youtube_feed_username3'] = array(
    '#type' => 'textfield',
    '#description' => t('Additional Youtube username.')
  );

  $build['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add artist'),
    '#submit' => array('migrate_umg_add_artist_submit'),
  );

  return $build;
}

/**
 * Submit callback for the configuration form handler fieldset.
 */
function migrate_umg_add_artist_submit($form, &$form_state) {
  $arguments['artist_id'] = $form_state['values']['artist'];
  $arguments['default_author_id'] = $form_state['values']['default_author'];

  // Make sure video usernames are valid.
  $youtube_usernames = array();
  for ($i = 1; $i <= 3; $i++) {
    $field_name = "youtube_feed_username$i";
    $youtube_feed_username = trim($form_state['values'][$field_name]);
    if ($youtube_feed_username) {
      $url =
        "http://gdata.youtube.com/feeds/api/users/$youtube_feed_username/uploads";
      $response = drupal_http_request($url);
      if ($response->code != 200) {
        form_set_error($field_name, t('Could not fetch the feed URL ' .
          '!url, please check the Youtube feed username and try again.',
          array('!url' => $url)));
        return;
      }
      $youtube_usernames[] = $youtube_feed_username;
    }
  }

  $connection = Database::getConnection('default', 'umg_legacy');
  $data = $connection->select('artists', 'a')
            ->fields('a', array('FName', 'LName'))
            ->condition('ArtistID', $arguments['artist_id'])
            ->execute()
            ->fetchAssoc();
  $first = trim($data['FName']);
  $last = trim($data['LName']);
  if ($first) {
    $artist_name = $first . ' ' . $last;
  }
  else {
    $artist_name = $last;
  }

  $group_machine_name = preg_replace('|[^a-z0-9_]|i', '', $artist_name);
  MigrateGroup::register($group_machine_name, $artist_name, array());
  $arguments['group_name'] = $group_machine_name;

  $migrations = array(
    'News' => 'UMGNewsMigration',
    'NewsStoryType' => 'UMGNewsStoryTypeMigration',
    'Photos' => 'UMGPhotoMigration',
    'Release' => 'UMGReleaseMigration',
    'Track' => 'UMGTrackMigration',
    'Galleries' => 'UMGGalleryMigration',
  );

  foreach ($migrations as $machine_name => $class_name) {
    Migration::registerMigration($class_name, $group_machine_name . $machine_name,
                                 $arguments);
  }

  // Only register the video migration if we have a username.
  foreach ($youtube_usernames as $youtube_feed_username) {
    $arguments['youtube_feed_username'] = $youtube_feed_username;
    Migration::registerMigration('UMGVideoMigration',
      'YT' . $youtube_feed_username, $arguments);
  }

  menu_rebuild();
  drupal_set_message(t('An import job for !artist has been created.',
                     array('!artist' => $artist_name)));
  $form_state['redirect'] = 'admin/content/import';
}
