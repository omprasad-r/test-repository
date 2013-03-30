<?php

/**
 * @file
 *
 * Web UI for importing EOS dumps to Drupal.
 */

ini_set('post_max_size', '255MB');
ini_set('upload_max_filesize', '255MB');

/**
 * Menu callback: Returns a page for importing a WordPress blog into Drupal.
 */
function migrate_warner_import() {
  drupal_set_title(t('EOS import'));
  return drupal_get_form('migrate_warner_import_form');
}

/**
 * Form for uploading the EOS dump.
 */
function migrate_warner_import_form($form, &$form_state) {
  // If an uploaded file exceeds post_max_size, the validate and submit functions
  // never get called (as they will if upload_max_filesize is exceeded but
  // post_max_size isn't). We can detect this case by the presence of an error.
  if ($error = error_get_last()) {
    drupal_set_message(t('File upload failed: !error. Please make sure the file
      you\'re trying to upload is not larger than !limit',
      array('!error' => $error['message'], '!limit' => format_size(file_upload_max_size()))));
  }

  migrate_warner_show_file();

  $form['overview'] = array(
    '#prefix' => '<div>',
    '#markup' => t('Browse and select a ZIP file containing an EOS content dump.
      When you click the Upload button, the file will be uploaded and prepared for
      import, and you will have the opportunity to review the content available
      for import before performing the import.'),
    '#suffix' => '</div>',
  );

  $form['#attributes'] = array('enctype' => 'multipart/form-data');
  $form['zip_file'] = array(
    '#type' => 'file',
    '#title' => t('EOS exported zip file to import into Drupal'),
    '#description' => t('The largest file size you can import is !size',
      array('!size' => format_size(file_upload_max_size()))),
  );

  $form['fan_club'] = array(
    '#type' => 'checkbox',
    '#title' => t('Fan club dump'),
    '#description' => t('Check this box if importing a fan club dump on top of
      the main site. This will ensure that members imported from this dump
      will receive the "fan club member" role.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Upload EOS content for import'),
  );

  return $form;
}

/**
 * Submit callback for the EOS import form.
 */
function migrate_warner_import_form_submit($form, &$form_state) {
  if ($_FILES['files']['error']['zip_file']) {
    form_set_error('zip_file', t('The file could not be uploaded, most likely
      because the file size exceeds the configured limit of !filesize',
      array('!filesize' => format_size(file_upload_max_size()))));
    return;
  }

  // Remember if we last uploaded a fan club dump
  variable_set('warner_fan_club_dump', $form_state['values']['fan_club']);

  $filename = $_FILES['files']['name']['zip_file'];
  // Unlike previously, where we try to upload to the previous warner_private_directory,
  // we will always make sure the directory name matches the zip file name
  preg_match('/^[^_\.]+/', $filename, $matches);
  $subdirectory = $matches[0];
  variable_set('warner_private_directory', $subdirectory);
  $directory = "warner://$subdirectory";

  // Remove any previous uploads
  file_unmanaged_delete_recursive($directory);

  if (!file_prepare_directory($directory, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS)) {
    form_set_error('zip_file', t('Could not prepare directory %directory',
      array('%directory' => $directory)));
    return;
  }

  $tmpfile = $_FILES['files']['tmp_name']['zip_file'];
  if ($tmpfile) {
    // Handle uploaded file
    $destination = $directory . '/' . $filename;
    $saved = file_unmanaged_move($tmpfile, $destination, FILE_EXISTS_REPLACE);
    if (!$saved) {
      form_set_error('zip_file', t('Failed to save file to %filename', array('%filename' => $destination)));
    }
  }
  else {
    form_set_error('zip_file', t('Uploaded file not found'));
  }

  if ($saved) {
    // ZipArchive doesn't seem to use the stream wrappers, so we need
    // to get the local file name.
    $zip = new ZipArchive();
    $stream = new MigrateWarnerStreamWrapper();
    $localpath = $stream->getRealPath($directory) . "/$filename";
    $zip->open($localpath);
    $zip->extractTo($stream->getRealPath($directory));
    $zip->close();

    $form_state['redirect'] = 'admin/content/warner/review';
  }
}

/**
 * Menu callback
 */
function migrate_warner_review() {
  drupal_set_title(t('Eos Import'));
  return drupal_get_form('migrate_warner_review_form');
}

/**
 * Form for reviewing migrations.
 */
function migrate_warner_review_form($form, &$form_state) {
  $build = array();

  migrate_warner_show_file();

  if (!variable_get('warner_private_directory', '')) {
    $build['overview'] = array(
      '#prefix' => '<div>',
      '#markup' => t('No Eos dump has been uploaded yet. Please go to the Upload
        tab and upload a .zip file containing an Eos dump.'),
      '#suffix' => '</div>',
    );
    return $build;
  }
  $build['overview'] = array(
    '#prefix' => '<div>',
    '#markup' => t('For each migration step listed below, you can see:
      <ol>
      <li>TOTAL RECORDS: The number of records available in the dump.</li>
      <li>IMPORTED: The number of records that have been imported.</li>
      <li>UNIMPORTED: The number of records yet to be imported.</li>
      <li>LAST IMPORTED: The date and time the records were last imported.</li>
      </ol>
      The Field migration reflects custom user fields to be added to the user profile.
      Other migrations reflect data records to be imported. Note that the content items
      themselves are divided into two source files for live and non-live content,
      and thus in each case have two separate migration steps - the only difference
      in how they are imported is that the non-live content is imported as unpublished.'),
    '#suffix' => '</div>',
  );

  $header = array(
    'machinename' => array('data' => t('')),
    'importrows' => array('data' => t('Total records')),
    'imported' => array('data' => t('Imported')),
    'unimported' => array('data' => t('Unimported')),
    'lastimported' => array('data' => t('Last imported')),
  );

  $migrations = migrate_migrations();

  $rows = array();
  foreach ($migrations as $migration) {
    $row = array();
    $has_counts = TRUE;
    if (method_exists($migration, 'sourceCount')) {
      $total = $migration->sourceCount();
    }
    else {
      $has_counts = FALSE;
      $total = t('N/A');
    }
    if (method_exists($migration, 'importedCount')) {
      $imported = $migration->importedCount();
    }
    else {
      $has_counts = FALSE;
      $imported = t('N/A');
    }
    if ($has_counts) {
      $unimported = $total - $imported;
    }
    else {
      $unimported = t('N/A');
    }

    $machine_name = $migration->getMachineName();
    $row['machinename'] = $machine_name;
    $row['importrows'] = $total;
    $row['imported'] = $imported;
    $row['unimported'] = $unimported;
    $row['lastimported'] = $migration->getLastImported();
    $rows[$machine_name] = $row;
  }

  $build['dashboard'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No EOS dump has been uploaded yet.'),
  );

  $build['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Options'),
    '#attributes' => array('class' => array('container-inline')),
  );
  $build['options']['update'] = array(
    '#type' => 'checkbox',
    '#title' => t('Update'),
    '#description' => t('Normally, only previously unimported records are imported -
      any records already imported are untouched, even if their source data is
      different in the dump. That is, only records reflected in the UNIMPORTED
      column are imported. If you check this button, in addition to the unimported
      records being imported, any previously-imported records will be overwritten
      with the current data from the source dump.'),
  );
  $build['operations'] = array(
    '#type' => 'fieldset',
    '#title' => t('Operations'),
    '#attributes' => array('class' => array('container-inline')),
  );
  $build['operations']['import'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
    '#submit' => array('migrate_warner_review_import'),
  );
  $build['operations']['rollback'] = array(
    '#type' => 'submit',
    '#value' => t('Rollback'),
    '#submit' => array('migrate_warner_review_rollback'),
  );
  $build['operations']['description'] = array(
    '#prefix' => '<p>',
    '#markup' => t(
      'Clicking Import will run the import process for all migrations listed above.
       Clicking Rollback will reverse the import process - all imported content will
       be deleted, and you will be able to rerun the import process again from scratch.
       The first time you click Import on this site, any pre-existing content of the
       types being imported will be purged (e.g., blog posts will be deleted, but not
       static pages). Rollback only affects imported content - it does not touch
       manually-created nodes.'
    ),
    '#postfix' => '</p>',
  );

  return $build;
}

/**
 * Import callback for the review form.
 */
function migrate_warner_review_import($form, &$form_state) {
  $migrations = migrate_migrations();
  foreach ($migrations as $migration) {
    if ($form_state['values']['update'] && method_exists($migration, 'prepareUpdate')) {
      $migration->prepareUpdate();
    }
    $operations[] = array('migrate_warner_batch',
                          array('import', $migration->getMachineName()));
  }

  $batch = array(
    'operations' => $operations,
    'title' => t('Migration processing'),
    'file' => drupal_get_path('module', 'migrate_warner') .
                  '/migrate_warner.pages.inc',
    'init_message' => t('Starting import process'),
    'progress_message' => t(''),
    'error_message' => t('An error occurred. Some or all of the import processing has failed.'),
    'finished' => 'migrate_warner_batch_finish',
  );
  batch_set($batch);
}

/**
 * Rollback callback for the review form.
 */
function migrate_warner_review_rollback($form, &$form_state) {
  $migrations = migrate_migrations();
  $machine_names = array();
  foreach ($migrations as $migration) {
    $machine_names[] = $migration->getMachineName();
  }

  // Rollback in reverse order.
  $machine_names = array_reverse($machine_names);
  foreach ($machine_names as $machine_name) {
    $operations[] = array('migrate_warner_batch', array('rollback', $machine_name));
  }

  $batch = array(
    'operations' => $operations,
    'title' => t('Migration processing'),
    'file' => drupal_get_path('module', 'migrate_warner') . '/migrate_warner.pages.inc',
    'init_message' => t('Starting rollback process'),
    'progress_message' => t(''),
    'error_message' => t('An error occurred. Some or all of the rollback processing has failed.'),
    'finished' => 'migrate_warner_batch_finish',
  );
  batch_set($batch);
}

/**
 * Process all enabled migration processes in a browser, using the Batch API
 * to break it into manageable chunks.
 *
 * @param $operation
 *  Operation to perform - 'import', 'rollback', 'stop', or 'reset'.
 * @param $machine_name
 *  Machine name of migration to process.
 * @param $context
 *  Batch API context structure
 */
function migrate_warner_batch($operation, $machine_name, &$context) {
  // If we got a stop message, skip everything else
  if (isset($context['results']['stopped'])) {
    $context['finished'] = 1;
    return;
  }

  $migration = Migration::getInstance($machine_name);

  // Messages generated by migration processes will be captured in this global
  global $_migrate_messages;
  $_migrate_messages = array();
  $migration->setDisplayFunction('migrate_warner_capture_message');

  // Perform the requested operation
  switch ($operation) {
    case 'import':
      $result = $migration->processImport();
      break;
    case 'rollback':
      $result = $migration->processRollback();
      break;
  }

  switch ($result) {
    case Migration::RESULT_INCOMPLETE:
      // Default to half-done, in case we can't get a more precise fix
      $context['finished'] = .5;
      if (method_exists($migration, 'sourceCount')) {
        $total = $migration->sourceCount();
        if ($total && method_exists($migration, 'importedCount')) {
          $imported = $migration->importedCount();
          switch ($operation) {
            case 'import':
              $context['finished'] = $imported/$total;
              break;
            case 'rollback':
              $context['finished'] = ($total-$imported)/$total;
              break;
          }
        }
      }
      break;
    case MigrationBase::RESULT_SKIPPED:
      $_migrate_messages[] = t('Skipped !name due to unfulfilled dependencies',
        array('!name' => $machine_name));
      $context['finished'] = 1;
      break;
    case MigrationBase::RESULT_STOPPED:
      $context['finished'] = 1;
      // Skip any further operations
      $context['results']['stopped'] = TRUE;
      break;
    default:
      $context['finished'] = 1;
      break;
  }

  // Add any messages generated in this batch to the cumulative list
  foreach ($_migrate_messages as $message) {
    $context['results'][] = $message;
  }

  // While in progress, show the cumulative list of messages
  $full_message = '';
  foreach ($context['results'] as $message) {
    $full_message .= $message . '<br />';
  }
  $context['message'] = $full_message;
}

/**
 * Batch API finished callback - report results
 *
 * @param $success
 *  Ignored
 * @param $results
 *  List of results from batch processing
 * @param $operations
 *  Ignored
 */
function migrate_warner_batch_finish($success, $results, $operations) {
  unset($results['stopped']);
  foreach ($results as $result) {
    drupal_set_message($result);
  }
}

function migrate_warner_capture_message($message, $level) {
  if ($level != 'debug') {
    global $_migrate_messages;
    $_migrate_messages[] = $message;
  }
}

function migrate_warner_show_file() {
  $subdirectory = variable_get('warner_private_directory', '');
  if ($subdirectory) {
    $directory = "warner://$subdirectory";
    foreach (file_scan_directory($directory, '/.*\.zip/') as $key => $file) {
      drupal_set_message(t('Current dump file for import is !file',
                         array('!file' => $key)));
    }
    if (variable_get('warner_fan_club_dump', 0)) {
      drupal_set_message(t('This is a fan club dump'));
    }
    else {
      drupal_set_message(t('This is not a fan club dump'));
    }
  }
}
