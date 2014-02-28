<?php

/**
 * @file
 * Drush scripts for Acquia Gardener broadcasted task handling.
 */

/**
 * Implements hook_drush_command().
 */
function gardens_broadcasting_client_drush_command() {
  $items = array();

  $items['gardens-broadcasting-client-tasks-get'] = array(
    'description' => 'Pulls task list from associated Acquia Gardener and stores the new tasks in a file.',
  );

  return $items;
}

/**
 * Pulls and stores new tasks from Acquia Gardener.
 */
function drush_gardens_broadcasting_client_tasks_get() {
  // When $cluster is empty then it's an SMB site.
  $site = AcsfSite::load();
  $cluster = isset($site->client_name) ? $site->client_name : 'smb';

  // Tangle info.
  $tangle = variable_get('acquia_hosting_site_info', '');
  $tangle = $tangle ? $tangle['name'] : '';

  // We do not know which site's DB will be used for this script so we can not
  // depend on variables other than ones that we do not expect to change. When
  // contacting the Gardener to provide a list of tasks we do not want to get
  // the full list, just those which are newer than the last one we got. Ideally
  // this would be stored in a variable, but as described above this might not
  // be available all the time. Instead check the file and take the last entry
  // from it. If the file is not yet present then this might be a new tangle in
  // which case we'll just read back a certain number of days.
  $tasks_filepath = gardens_broadcasting_client_tasks_filepath_get();
  $last_tasks = array();
  if (file_exists($tasks_filepath) && ($last_tasks = gardens_broadcasting_client_file_tasks_get())) {
    $last_task = array_pop($last_tasks);
    $timestamp = (int) $last_task['created'];
  }
  else {
    $timestamp = strtotime(variable_get('gardens_broadcasting_client_prepopulate_range', '-14 days'));
  }

  // Get new tasks.
  try {
    $message = new AcsfMessageXmlRpc('GET', 'acquia.gardens.broadcasting.task.get', array($cluster, $timestamp, $tangle));
    $message->send();
    $new_tasks = $message->getResponseBody();
  }
  catch (Exception $e) {
    watchdog('gardens_broadcasting', $e->getMessage(), array(), WATCHDOG_ERROR);
  }

  // It's either an array or NULL, so have to double check to make sure an array
  // is being fed to the loop.
  if ($new_tasks) {
    // Since the tasks are being stored in file and the task is defined as an
    // array it needs serialization.
    foreach ($new_tasks as $key => $data) {
      $new_tasks[$key]['task'] = serialize($data['task']);
    }
    // Add and write out the new tasks.
    gardens_broadcasting_client_file_tasks_save($new_tasks);
  }
}
