<?php
/**
 * @file
 * Configuration forms for Roadrunner migration.
 */
 

/**
 * Menu callback
 */
function migrate_rr_configure() {
  return drupal_get_form('migrate_rr_configure_form');
}

/**
 * Form for reviewing migrations.
 */
function migrate_rr_configure_form($form, &$form_state) {
  $build = array();

  $build['description'] = array(
     '#prefix' => '<div>',
     '#markup' => t('Configure the source database for artist imports. This
      information will be saved in encrypted form.'),
     '#suffix' => '</div>',
  );

  $connection = migrate_rr_connection();

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
    '#submit' => array('migrate_rr_configure_submit'),
    '#validate' => array('migrate_rr_configure_validate'),
  );
  return $build;
}

/**
 * Validate callback for the configuration form handler fieldset.
 */
function migrate_rr_configure_validate($form, &$form_state) {
  $connection = array(
    'host' => $form_state['values']['host'],
    'username' => $form_state['values']['username'],
    'password' => $form_state['values']['password'],
    'database' => $form_state['values']['database'],
    'driver' => 'mysql',
  );
  try {
    Database::addConnectionInfo('rr_legacy', 'default', $connection);
    $artist_count = Database::getConnection('default', 'rr_legacy')
                    ->select('fanclubs', 'f')
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
function migrate_rr_configure_submit($form, &$form_state) {
  $connection = array(
    'host' => $form_state['values']['host'],
    'username' => $form_state['values']['username'],
    'password' => $form_state['values']['password'],
    'database' => $form_state['values']['database'],
    'driver' => 'mysql',
  );
  variable_set('migrate_rr_connection', migrate_rr_encrypt($connection));
  drupal_set_message(t('Your configuration changes have been saved.'));
  $form_state['redirect'] = 'admin/content/import/rr_add';
}

/**
 * Menu callback
 */
function migrate_rr_add_artist() {
  return drupal_get_form('migrate_rr_add_artist_form');
}

/**
 * Form for adding migration for an artist.
 */
function migrate_rr_add_artist_form($form, &$form_state) {
  $build = array();

  $connection_array = migrate_rr_connection();
  if (empty($connection_array['host'])) {
    drupal_set_message(t('You must set up the source database connection before
      you can add an artist for import.'));
    drupal_goto('admin/content/import/rr_config');
  }
  Database::addConnectionInfo('rr_legacy', 'default', $connection_array);

  $build['description'] = array(
     '#prefix' => '<div>',
     '#markup' => t('Set up the import of a given artist\'s content into this site.'),
     '#suffix' => '</div>',
  );

  $artist_options = array();
  $connection = Database::getConnection('default', 'rr_legacy');
  if (!$connection) {
    drupal_set_message(t('To import artist content, a connection named ' .
      '<em>rr_legacy</em> must be defined in the site settings.php file.'));
  }
  else {
    $result = $connection->select('fanclubs', 'f')
              ->fields('f', array('id', 'name'))
              ->condition('is_active', 1)
              ->orderBy('name')
              ->execute();
    foreach ($result as $row) {
      $artist_options[$row->id] = $row->name;
    }
  }

  $build['artist'] = array(
    '#type' => 'select',
    '#options' => $artist_options,
    '#title' => t('Artist name'),
    '#description' => t('Choose the artist whose content is to be imported into this ' .
                       'Gardens site. The unique id of each artist is in parentheses.'),
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

  $build['default_author'] = array(
    '#type' => 'select',
    '#options' => $user_options,
    '#title' => t('Default author'),
    '#default_value' => 1,
    '#description' => t('Choose the user account that will serve as the author of ' .
      'imported tourdate content. All accounts that have one of the roles ' .
      '<em>administrator</em>, <em>content manager</em>, <em>editor</em>, or ' .
      '<em>site maintainer</em> are listed.'),
  );

  $build['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add artist'),
    '#submit' => array('migrate_rr_add_artist_submit'),
  );

  return $build;
}

/**
 * Submit callback for the configuration form handler fieldset.
 */
function migrate_rr_add_artist_submit($form, &$form_state) {
  $arguments['artist_id'] = $form_state['values']['artist'];
  $arguments['default_author_id'] = $form_state['values']['default_author'];

  $connection = Database::getConnection('default', 'rr_legacy');
  $data = $connection->select('fanclubs', 'f')
            ->fields('f', array('name'))
            ->condition('id', $arguments['artist_id'])
            ->execute()
            ->fetchObject();
  $artist_name = $data->name;

  $group_machine_name = preg_replace('|[^a-z0-9_]|i', '', $artist_name);
  MigrateGroup::register($group_machine_name, $artist_name, array());
  $arguments['group_name'] = $group_machine_name;

  $migrations = array(
    'Picture' => 'RRPictureMigration',
    'User' => 'RRUserMigration',
    'Tags' => 'RRTagsMigration',
    'Event' => 'RREventMigration',
    'ForumCategory' => 'RRForumCategoryMigration',
    'ForumTopic' => 'RRForumTopicMigration',
    'Forum' => 'RRForumMigration',
    'ForumComment' => 'RRForumCommentMigration',
    'News' => 'RRNewsMigration',
    'NewsComment' => 'RRNewsCommentMigration',
  );

  foreach ($migrations as $machine_name => $class_name) {
    Migration::registerMigration($class_name, $group_machine_name . $machine_name,
                                 $arguments);
  }

  menu_rebuild();
  drupal_set_message(t('An import job for !artist has been created.',
                     array('!artist' => $artist_name)));
  $form_state['redirect'] = 'admin/content/import';
}
