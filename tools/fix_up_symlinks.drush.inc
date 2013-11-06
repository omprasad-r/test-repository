<?php

function fix_up_symlinks_drush_command() {
  $items = array();
  $items['fix-up-symlinks'] = array(
    'description' => dt('Ensure that legacy sites (created prior to 2.01) have a symlink from the up tangle to live site directory.'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
      'sitegroup' => 'The hosting sitegroup (e.g. tangle001).',
      'environment' => 'The environment (e.g. prod).',
      'db-role' => 'The Gardens site database name/role (e.g. g123).',
      'update-env' => 'The update environment corresponding to the live environment in the "environment" arg',
    ),
    'options' => array(
      'run' => dt('Set to 1 to actually attempt to make changes on the filesystem.'),
    ),
    'required-arguments' => TRUE,
  );
  return $items;
}

function drush_fix_up_symlinks($site, $env, $gardens_site_id, $up_env) {
  $new_location = "/mnt/gfs/{$site}.{$env}/sites/g/files/{$gardens_site_id}";
  $old_location = "/mnt/gfs/{$site}.{$env}/gardens-sites/{$gardens_site_id}";
  $up_tangle_link = "/mnt/gfs/{$site}.{$up_env}/sites/g/files/{$gardens_site_id}";

  // Handful of checks before running the symlinking - only check points that
  // are critical for the operation of this script.
  if (!file_exists($new_location) || !is_dir($new_location)) {
    drush_set_error("Error: $new_location does not exist on the filesystem.");
    return FALSE;
  }

  if (!is_link($new_location)) {
    drush_print("Directory: OK");
  }
  else {
    drush_set_error('symlink', sprintf('Site directory (new location sites/g/files) for site id %s is a symlink. Attempting to add up tangle symlink anyway.', $gardens_site_id));
  }

  $files = $new_location . '/files';
  $f = $new_location . '/f';
  if (files_ok($files, $f)) {
    drush_print("Files status: OK");
  }
  else {
    drush_print("Files status: ERROR, can proceed anyway.");
  }

  if (is_link($up_tangle_link) && realpath($up_tangle_link) == realpath($new_location)) {
    drush_print("*UP* tangle symlink status: OK");
  }
  else {
    if (is_link($up_tangle_link) && realpath($up_tangle_link) != realpath($new_location)) {
      drush_set_error('symlink', sprintf('Up tangle symlink points to the wrong place for site id %s. Link path is %s, target should be %s', $gardens_site_id, $up_tangle_link, $new_location));
      return FALSE;
    }
    elseif (file_exists($up_tangle_link) && !is_link($up_tangle_link)) {
      drush_set_error('symlink', sprintf('Up tangle link for site id %s is not a symlink. Link path is %s, target should be %s', $gardens_site_id, $up_tangle_link, $new_location));
      return FALSE;
    }

    if (!is_writable(dirname($up_tangle_link))) {
      drush_set_error('symlink', sprintf('Up tangle sites dir is not writable!!! All symlink operations will fail. %s', dirname($up_tangle_link)));
      return FALSE;
    }

    $command = "ln -s $new_location $up_tangle_link";
    if (drush_get_option('run')) {
      drush_print("Executing: $command");
      $exit_code = run($command);

      if ($exit_code === 0) {
        $time = time();
        while(!is_link($up_tangle_link) && time() < $time + 30) {
          clearstatcache();
          drush_print('Waiting for up tangle symlink...');
          sleep(2);
        }
        if (!is_link($up_tangle_link)) {
          drush_set_error('symlink', sprintf('Up tangle symlink was not present after waiting 30 sec. %s', $up_tangle_link));
          return FALSE;
        }
      }

    }
    else {
      drush_print("Dry run: $command");
    }
  }
}
