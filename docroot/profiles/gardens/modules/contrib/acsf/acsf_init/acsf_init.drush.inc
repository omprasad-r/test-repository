<?php

/**
 * Implements hook_drush_command()
 */
function acsf_init_drush_command() {
  return array(
    'acsf-init' => array(
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
      'callback' => 'drush_acsf_init',
      'description' => dt('Installs/updates the non-standard Drupal components for this repository to be compatible with Acquia Site Factory. This command will update in place, so there is no harm in running it multiple times.'),
    ),
    'acsf-uninstall' => array(
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
      'callback' => 'drush_acsf_uninstall',
      'description' => dt('Uninstalls components for this Drupal repository to be compatible with Acquia Site Factory.'),
    ),
  );
}

/**
 * Callback: executes the required changes to this repository.
 */
function drush_acsf_init() {
  $path = dirname(dirname(dirname(__FILE__)));
  $relative_path = str_replace(DRUPAL_ROOT . '/', '', $path);
  $repo_root = str_replace('/docroot', '', DRUPAL_ROOT);
  $lib_path = sprintf('%s/%s/acsf/acsf_init/lib', DRUPAL_ROOT, $relative_path);

  drush_print('Installing ACSF requirements.');

  // Create the required directories.
  foreach (acsf_init_get_required_dirs($repo_root) as $name => $dir) {
    drush_print(dt('Creating directory for !name at !dir', array('!name' => $name, '!dir' => $dir)));
    if (!file_exists($dir)) {
      if (mkdir($dir, 0755, TRUE)) {
        drush_log(dt('Success'), 'success', 'success');
      }
      else {
        drush_log(dt('Error'), 'error', 'error');
      }
    }
    else {
      drush_log(dt('Already Exists'), 'success', 'ok');
    }
  }

  // Copy the required files.
  foreach (acsf_init_get_required_files($repo_root) as $file => $location) {
    $source = sprintf('%s/%s/%s', $lib_path, $location['source'], $file);
    $dest = sprintf('%s/%s', $location['dest'], $file);
    $mod = isset($location['mod']) ? $location['mod'] : FALSE;
    drush_print(dt('Copying !file to !dest.', array('!file' => $source, '!dest' => $dest)));
    if (file_exists($dest)) {
      $confirm = drush_confirm(dt('Destination file exists, continue?'));
      if ($confirm === FALSE) {
        continue;
      }
    }
    // Copy the file into the destination.
    if (copy($source, $dest)) {
      drush_log(dt('Copy Success: !file', array('!file' => $file)), 'success', 'success');
    }
    else {
      drush_log(dt('Copy Error: !file', array('!file' => $file)), 'error', 'error');
    }
    // Chmod the file if required.
    if ($mod && chmod($dest, $mod)) {
      drush_log(dt('Chmod Success: !file', array('!file' => $file)), 'success', 'success');
    }
    elseif ($mod) {
      drush_log(dt('Chmod Error: !file', array('!file' => $file)), 'error', 'error');
    }
  }
}

/**
 * Uninstalls ACSF files from the code base.
 */
function drush_acsf_uninstall() {
  drush_print('Removing ACSF requirements.');

  foreach (acsf_init_get_required_files($repo_root) as $file => $location) {
    $dest = sprintf('%s/%s', $location['dest'], $file);
    if (file_exists($dest)) {
      $confirm = drush_confirm(dt('Delete !file?', array('!file' => $dest)));
      if ($confirm === FALSE) {
        continue;
      }
      if (unlink($dest)) {
        drush_log(dt('Success'), 'success', 'success');
      }
      else {
        drush_log(dt('Error'), 'error', 'error');
      }
    }
  }
}

/**
 * Lists all required directories to create.
 */
function acsf_init_get_required_dirs($repo_root) {
  return array(
    'cloud hooks' => sprintf('%s/hooks/common/post-db-copy', $repo_root),
    'acquia hook dir' => sprintf('%s/hooks/acquia', $repo_root),
    'cloud hook samples' => sprintf('%s/hooks/samples', $repo_root),
    'site config logic' => sprintf('%s/sites/g', DRUPAL_ROOT),
  );
}

/**
 * Lists all required files to create/delete.
 */
function acsf_init_get_required_files($repo_root) {
  return array(
    'README.md' => array(
      'source' => 'cloud_hooks',
      'dest' => sprintf('%s/hooks', $repo_root),
    ),
    '000-acquia_required_scrub.php' => array(
      'source' => 'cloud_hooks/common/post-db-copy',
      'dest' => sprintf('%s/hooks/common/post-db-copy', $repo_root),
      'mod' => 0751,
    ),
    'db_connect.php' => array(
      'source' => 'cloud_hooks/acquia',
      'dest' => sprintf('%s/hooks/acquia', $repo_root),
    ),
    'uri.php' => array(
      'source' => 'cloud_hooks/acquia',
      'dest' => sprintf('%s/hooks/acquia', $repo_root),
    ),
    'acquia-cloud-site-factory-post-db.sh' => array(
      'source' => 'cloud_hooks/samples',
      'dest' => sprintf('%s/hooks/samples', $repo_root),
    ),
    'hello-world.sh' => array(
      'source' => 'cloud_hooks/samples',
      'dest' => sprintf('%s/hooks/samples', $repo_root),
    ),
    'post-code-deploy.tmpl' => array(
      'source' => 'cloud_hooks/samples',
      'dest' => sprintf('%s/hooks/samples', $repo_root),
    ),
    'sites.php' => array(
      'source' => 'sites',
      'dest' => sprintf('%s/sites', DRUPAL_ROOT),
    ),
    'apc_rebuild.php' => array(
      'source' => 'sites/g',
      'dest' => sprintf('%s/sites/g', DRUPAL_ROOT),
    ),
    'apcstat.php' => array(
      'source' => 'sites/g',
      'dest' => sprintf('%s/sites/g', DRUPAL_ROOT),
    ),
    '.gitignore' => array(
      'source' => 'sites/g',
      'dest' => sprintf('%s/sites/g', DRUPAL_ROOT),
    ),
    'settings.php' => array(
      'source' => 'sites/g',
      'dest' => sprintf('%s/sites/g', DRUPAL_ROOT),
    ),
    'sites.inc' => array(
      'source' => 'sites/g',
      'dest' => sprintf('%s/sites/g', DRUPAL_ROOT),
    ),
  );

}
