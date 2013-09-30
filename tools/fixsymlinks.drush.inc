<?php

function fixsymlinks_drush_command() {
  $items = array();
  $items['fix-symlinks'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
      'sitegroup' => 'The hosting sitegroup (e.g. tangle001).',
      'environment' => 'The environment (e.g. prod).',
      'db-role' => 'The Gardens site database name/role (e.g. g123).',
    ),
    'required-arguments' => TRUE,
  );
  return $items;
}

function drush_fixsymlinks_fix_symlinks($site, $env, $gardens_site_id) {
  $new_location = "/mnt/files/{$site}.{$env}/sites/g/files/{$gardens_site_id}";
  $old_location = "/mnt/files/{$site}.{$env}/gardens-sites/{$gardens_site_id}";

  if (!file_exists($new_location) && !file_exists($old_location)) {
    drush_set_error("Error: Neither $new_location nor $old_location exists on the filesystem.");
    return FALSE;
  }

  if (symlink_ok($new_location, $old_location)) {
    drush_print("Symlink status: OK");
  }
  else {
    drush_print("Attempting site directory move and symlink:");

// Log current directory and link status.
    drush_print("Old location: " . $old_location);
    ls($old_location);
    drush_print("New location: " . $new_location);
    ls($old_location);

    $success = TRUE;

    drush_print("----- Fix backwards symlinks -----");
    if (is_link($new_location)) {
      drush_print("$new_location is a backwards symlink. Attempting to remove it.");
      $command = 'rm -f ' . $new_location;
      run($command);
    }
    else {
      drush_print("$new_location is not a symlink:");
      ls($new_location);
    }

    drush_print("----- Move site directory from gardens-sites to sites/g/files -----");
    if (!is_dir($new_location) && file_exists($old_location)) {
      drush_print("$new_location is empty, and there are files at $old_location. Attempting to move the files directory.");
      $command = 'mv ' . $old_location . ' ' . $new_location;
      $exit_code = run($command);
      if ($exit_code === 0) {
        // The move succeeded. Wait 30 seconds for the directory to show up.
        $time = time();
        while(!file_exists($new_location) && time() < $time + 30) {
          clearstatcache(TRUE);
          drush_print('Waiting for site directory move...');
          sleep(2);
        }
      }
    }
    else {
      drush_print("Cannot move files from $old_location to $new_location:");
      ls($new_location);
      ls($old_location);
    }

// Create a symlink from gardens-sites to sites/g/files.
    drush_print("----- Create a symlink from gardens-sites to sites/g/files -----");
    if (!is_link($new_location) && is_dir($new_location) && !file_exists($old_location)) {
      drush_print("$new_location is a target directory, and there is nothing at $old_location. Attempting to symlink.");
      $command = 'ln -s ' . $new_location .' ' . $old_location;
      $exit_code = run($command);
      if ($exit_code === 0) {
        // The symlink was created. Wait for it to show up.
        $time = time();
        while(!is_link($old_location) && time() < $time + 30) {
          clearstatcache(TRUE);
          drush_print('Waiting for site directory symlink...');
          sleep(2);
        }
      }
    }
    else {
      drush_print("Cannot create symlink at $old_location to $new_location:");
      ls($new_location);
      ls($old_location);
    }

    sleep(2);
    if (symlink_ok($new_location, $old_location, TRUE)) {
      drush_print("Symlink fix succeeded.");
    }
    else {
      drush_set_error('symlink', sprintf('Symlink fix failed for site id %s.', $gardens_site_id));
      return FALSE;
    }
  }

// If the symlink fix succeeded, attempt the files -> f conversion.
  $files = $new_location . '/files';
  $f = $new_location . '/f';
  if (files_ok($files, $f)) {
    drush_print("Files status: OK");
  }
  else {
    drush_print("----- files -> f conversion -----");
    drush_print("----- Move files to f -----");
    if (is_dir($files) && !is_link($files) && !file_exists($f)) {
      $command = "mv $files $f";
      $exit_code = run($command);
      if ($exit_code === 0) {
        $time = time();
        while(!is_dir($f) && time() < $time + 30) {
          clearstatcache();
          drush_print('Waiting for files directory move...');
          sleep(2);
        }
      }
    }
    else {
      drush_print("Could not move files to f:");
      ls($files);
      ls($f);
    }

    drush_print("----- Symlink f to files -----");
    if (!file_exists($files) && is_dir($f)) {
      $command = "ln -s $f $files";
      $exit_code = run($command);
      if ($exit_code === 0) {
        $time = time();
        while(!is_link($files) && time() < $time + 30) {
          clearstatcache();
          drush_print('Waiting for files directory symlink...');
          sleep(2);
        }
      }
    }
    else {
      drush_print("Could not symlink f to files:");
      ls($files);
      ls($f);
    }

    sleep(2);
    if (files_ok($files, $f, TRUE)) {
      drush_print(sprintf('Files -> f conversion succeeded for site id %s', $gardens_site_id));
    }
    else {
      drush_set_error('files', sprintf('Files -> f conversion failed for site id %s.', $gardens_site_id));
    }

  }
}

/**
 * Run a shell command and output its information.
 *
 * @param $command
 * @return int
 *   Exit code.
 */
function run($command) {
  $output = array();
  $exit = 0;

  drush_print('Command: ' . $command);
  exec($command, $output, $exit);
  if (!empty($output)) {
    drush_print("Output: ");
    foreach($output as $line) {
      drush_print($line);
    }
  }
  drush_print('Exit code: ' . $exit);
  clearstatcache(TRUE);
  return $exit;
}

/**
 * ls a directory or symlink and output its information.
 *
 * @param $path
 */
function ls($path) {
  run("ls -ldh " . escapeshellarg($path));
}

function symlink_ok($new_location, $old_location, $output = FALSE) {
  clearstatcache(TRUE);
  $new_link_exists = is_link($new_location);
  $new_link_is_dir = is_dir($new_location);
  $old_link_exists = is_link($old_location);
  $old_link_is_dir = is_dir($old_location);
  $success = !$new_link_exists && $new_link_is_dir && $old_link_exists && $old_link_is_dir;
  if ($output) {
    if ($success) {
      drush_print('Symlink status: OK');
    }
    else {
      drush_print('Symlink status: error');
      drush_print("$new_location is symlink: " . ($new_link_exists ? 'TRUE' : 'FALSE') . ' (should be FALSE)');
      drush_print("$new_location is directory: " . ($new_link_is_dir ? 'TRUE' : 'FALSE') . ' (should be TRUE)');
      drush_print("$old_location is symlink: " . ($old_link_exists ? 'TRUE' : 'FALSE') . ' (should be TRUE)');
      drush_print("$old_location is directory: " . ($old_link_is_dir ? 'TRUE' : 'FALSE') . ' (should be TRUE)');
    }
  }
  return $success;
}

function files_ok($files, $f, $output = FALSE) {
  clearstatcache(TRUE);
  $f_is_dir = is_dir($f);
  $files_is_link = is_link($files);
  $success = $f_is_dir && $files_is_link;
  if ($output) {
    if ($success) {
      drush_print("Files status: OK");
    }
    else {
      drush_print("Files status: error");
      drush_print("$f is directory: " . ($f_is_dir ? "TRUE" : "FALSE") . ' (should be TRUE)');
      drush_print("$files is symlink: " . ($files_is_link ? "TRUE" : "FALSE") . ' (should be TRUE)');
    }
  }
  return $success;
}
