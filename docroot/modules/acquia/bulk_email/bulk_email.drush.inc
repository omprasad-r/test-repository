<?php

/**
 * @file
 * Drush integration for the bulk_email module.
 */

/**
 * Implements hook_drush_command().
 */
function bulk_email_drush_command() {
  $items = array();

  $items['bulk-email-generate'] = array(
    'description' => dt('Set up a site for batch emailing by populating a temporary table with users.'),
    'aliases' => array('beg'),
  );

  $items['generate-email-users'] = array(
    'description' => 'Create users from a file of email addresses.',
    'arguments' => array(
      'email_filename' => 'Filename containing email addresses, 1 per line',
    ),
    'options' => array(
      'kill' => 'Delete all users before generating new ones.',
    ),
    'aliases' => array('genemu'),
  );

  return $items;
}

/**
 * Command handler. Populate the bulk_email table and
 * present the url for batch processing to the end user
 */
function drush_bulk_email_generate() {
  if(!drush_confirm(dt('All previous mail queues will be deleted. Proceed?'))) {
      return drush_user_abort();
  }
  // If our table exists, ensure it's empty
  if (db_table_exists('bulk_email')) {
    db_delete('bulk_email')
      ->execute();
  // Otherwise create the table
  }
  else {
    include_once(drupal_get_path('module', 'bulk_email') . '/bulk_email.install');
    $schema = bulk_email_schema();
    db_create_table('bulk_email', $schema['bulk_email']);
  }
  // Build the SELECT query.
  $query = db_select('users', 'u');
  // Add the fields we want.
  $query->addField('u','uid');
  // Add a condition to not include specific users.
  $query->condition('uid', 0, '!=');
  $query->condition('uid', 1, '!=');

  // Perform the insert.
  db_insert('bulk_email')
    ->from($query)
    ->execute();
  $key = variable_get('cron_key');
  $path = url('bulk-email-process', array('absolute' => TRUE, 'query' => array('cron_key' => $key)));
  drush_print(dt("The queue has been populated with users, use the following url to process a batch of emails."));
  drush_print($path);
}

/**
 * Drush command to generate users from a file of email addresses.
 *
 * Adapted from devel_create_users, removed user picture and field handling.
 *
 * @param $filename
 */
function drush_bulk_email_generate_email_users($filename) {
  // Just set this default for our purposes.  Devel has an option for age.
  $age = 0;

  if (empty($filename)) {
    return drush_set_error('DEVEL_GENERATE_INVALID_INPUT', dt('Missing filename'));
  }

  $kill = drush_get_option('kill', FALSE);

  if ($kill) {
    $uids = db_select('users', 'u')
            ->fields('u', array('uid'))
            // Gardens-specific - 2 default users.
            ->condition('uid', 2, '>')
            ->execute()
            ->fetchAllAssoc('uid');
    user_delete_multiple(array_keys($uids));
    drupal_set_message(format_plural(count($uids), '1 user deleted', '@count users deleted.'));
  }

  if (!$fp = fopen($filename, 'r')) {
    return drush_set_error('DEVEL_GENERATE_INVALID_INPUT', dt('Input file does not exist'));
  }

  $num = 0;
  while (($buffer = fgets($fp)) !== false) {
    if ($email = trim($buffer)) {
      $name = _bulk_email_devel_generate_word(mt_rand(6, 12));

      if (empty($roles)) {
        $roles = array(DRUPAL_AUTHENTICATED_RID);
      }

      $edit = array(
        'uid'     => NULL,
        'name'    => $name,
        'pass'    => NULL, // No password avoids user_hash_password() which is expensive.
        'mail'    => $email,
        'status'  => 1,
        'created' => REQUEST_TIME - mt_rand(0, $age),
        'roles' => drupal_map_assoc($roles),
        'devel_generate' => TRUE // A flag to let hook_user_* know that this is a generated user.
      );

      $account = user_save(drupal_anonymous_user(), $edit);
      ++$num;
    }
  }

  fclose($fp);
  drupal_set_message(t('!num_users created.', array('!num_users' => format_plural($num, '1 user', '@count users'))));
}

/**
 * Copy of devel_generate_word to remove devel dependency
 *
 * @param $length
 *   Int length of word required.
 * @return
 *   Word of the given length.
 */
function _bulk_email_devel_generate_word($length){
  mt_srand((double)microtime()*1000000);

  $vowels = array("a", "e", "i", "o", "u");
  $cons = array("b", "c", "d", "g", "h", "j", "k", "l", "m", "n", "p", "r", "s", "t", "u", "v", "w", "tr",
  "cr", "br", "fr", "th", "dr", "ch", "ph", "wr", "st", "sp", "sw", "pr", "sl", "cl", "sh");

  $num_vowels = count($vowels);
  $num_cons = count($cons);
  $word = '';

  while(strlen($word) < $length){
    $word .= $cons[mt_rand(0, $num_cons - 1)] . $vowels[mt_rand(0, $num_vowels - 1)];
  }

  return substr($word, 0, $length);
}
