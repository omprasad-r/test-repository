<?php

/**
 * Implements hook_drush_command()
 */
function acsf_init_drush_command() {
  return array(
    'acsf-init' => array(
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_ROOT,
      'callback' => 'drush_acsf_init',
      'description' => dt('Installs/updates the non-drupal components for this Drupal repository to be compatible with Acquia Site Factory. This command will update in place, so there is no harm in running it multiple times.'),
      'options' => array(
        'relative-path' => dt('The path from the drupal root to acsf, e.g. "sites/all/modules/acquia", defaults to "sites/all/modules"'),
      ),
    ),
  );
}

/**
 * Callback: executes the required changes to this repository.
 */
function drush_acsf_init() {
  drush_print('Installing');
  $conf = array();
  $relative_path = trim(drush_get_option('relative-path', 'sites/all/modules'), '/');
  // @todo remove this cruft.
  $conf['sdf_path'] = $relative_path;
  $relative_path = str_replace('/acsf', '', $relative_path);
  $relative_path_reverse = preg_replace('/[^\/]+/', '..', $relative_path);

  if (!file_exists(sprintf('%s/%s/acsf/acsf.module', DRUPAL_ROOT, $relative_path))) {
    drush_set_error(dt('Could not locate the acsf module. Please check your --relative-path argument.'));
    exit(1);
  }

  $post_db_copy_path = sprintf('%s/../hooks/common/post-db-copy', DRUPAL_ROOT);
  $library_path = sprintf('%s/../library', DRUPAL_ROOT);
  $drush_path = sprintf('%s/sites/all/drush', DRUPAL_ROOT);

  if (!file_exists($post_db_copy_path)) {
    mkdir($post_db_copy_path, 0755, TRUE);
  }
  if (!file_exists($library_path)) {
    mkdir($library_path, 0755, TRUE);
  }
  if (!file_exists($drush_path)) {
    mkdir($drush_path, 0755, TRUE);
  }

  chdir(sprintf('%s/../', DRUPAL_ROOT));

  $from_root_to_lib = sprintf('docroot/%s/acsf/acsf_init/lib', $relative_path);

  $from_sites_to_lib = str_replace('docroot/sites/', '', $from_root_to_lib);

  acsf_init_place_symlink(sprintf('%s/sites/sites.php', $from_sites_to_lib), 'docroot/sites/sites.php');
  acsf_init_place_symlink(sprintf('%s/sites/g', $from_sites_to_lib), 'docroot/sites/g');
  acsf_init_place_symlink(sprintf('%s/hosting-drush', $from_root_to_lib), 'hosting-drush');
  acsf_init_place_symlink(sprintf('../../../%s/cloud_hooks/000-acquia_required_scrub.php', $from_root_to_lib), 'hooks/common/post-db-copy/000-acquia_required_scrub.php');
  acsf_init_place_symlink(sprintf('../../%s/drush/drushrc.php', $from_sites_to_lib), 'docroot/sites/all/drush/drushrc.php');

  acsf_write_ini($conf);
}

/**
 * Places a symlink, removing the previous link in case of a new target.
 *
 * @param String $target
 *   The link target.
 * @param String $name
 *   The link name.
 */
function acsf_init_place_symlink($target, $name) {
  if (is_link($name)) {
    drush_log(dt('Removing old symlink for !name', array('!name', $name)));
    unlink($name);
  }
  symlink($target, $name);
}

/**
 * Writes the specified configuration to an ini file.
 */
function acsf_write_ini($conf) {
  $ini = "[acsf]\n";
  foreach ($conf as $key => $value) {
    $ini .= "$key = \"$value\"\n";
  }

  file_put_contents(sprintf('%s/../acsf.ini', DRUPAL_ROOT), $ini);
}
