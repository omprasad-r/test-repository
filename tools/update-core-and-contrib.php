#!/usr/bin/php -q
<?php

/**
 * @file
 * This script takes code from a Gardens workspace (built using Drush Make) and
 * copies it to the docroot.
 *
 * WARNING: This script can destroy any work in progress or extra files that
 * you have in the same directory as your local Gardens SVN checkout. Use it
 * with caution!
 *
 * To use this script, you'll first need to build a Gardens codebase. Run a
 * command like the following from the top level of your SVN checkout of the
 * Gardens repository:
 *
 * ./drush/drush make gardens.make make-workspace/gardens --no-patch-txt
 *
 * (TODO: Make this script run the above command automatically as its first
 * step)
 *
 * Then, run the current script. There are a few possible options, including:
 *
 * # Update Drupal core:
 * ./tools/update-core-and-contrib.php --core
 *
 * # Update a contrib module named "views" located in sites/all/modules:
 * ./tools/update-core-and-contrib.php views
 *
 * # Update all contrib modules (not well-supported yet):
 * ./tools/update-core-and-contrib.php --contrib
 *
 * # Update the entire Gardens workspace (not well-supported yet):
 * ./tools/update-core-and-contrib.php --all
 *
 * Note that you can also use the --exclude parameter to avoid overwriting
 * certain local files or directories within your SVN checkout. For example,
 * many people run their local sites off of http://gardens.dev and thus have a
 * sites/gardens.dev directory in their local checkout that they don't want
 * blown away. In that case, you can modify the above commands to be similar to
 * this:
 *
 * ./tools/update-core-and-contrib.php --core --exclude sites/gardens.dev
 *
 * Note that you can put as many entries as you want after the "--exclude"
 * parameter:
 *
 * ./tools/update-core-and-contrib.php --core --exclude sites/gardens.dev path/to/some/other/file/that/should/not/be/overwritten
 *
 * Finally, when you are done running this script, you'll likely have some
 * files in your local checkout that were added or deleted, and thus need to be
 * added via "svn add" or removed via "svn rm" before committing the new
 * version of the module (or Drupal core) to SVN. The script will do that for
 * you in some cases, but you'll likely need to do some of it on your own. For
 * that purpose, you might find the "add_to_svn" and "remove_from_svn" scripts
 * (in the same directory as this file) to be useful.
 */

// Collect parameters.
if (!isset($argv[1])) {
  print "ERROR: This script requires the first parameter be one of the following:\n";
  print "  1. The name of a contributed module to update\n";
  print "  2. \"--core\" (to update Drupal core)\n";
  print "  3. \"--contrib\" (to update all contrib modules)\n";
  print "  4. \"--all\" (to update the entire codebase)\n";
  exit;
}
if (isset($argv[2]) && $argv[2] == '--exclude') {
  $exclude_from_copy = array_slice($argv, 3);
}
else {
  $exclude_from_copy = array();
}

// Call the function that was requested.
$module_to_update = $argv[1];
if ($module_to_update == '--core') {
  update_core($exclude_from_copy);
}
elseif ($module_to_update == '--contrib') {
  update_contrib($exclude_from_copy);
}
elseif ($module_to_update == '--all') {
  update_all($exclude_from_copy);
}
else {
  update_module($module_to_update, $exclude_from_copy);
}

/**
 * Copy Drupal core from the make workspace into the docroot.
 *
 * @param $exclude_from_copy
 *   Optional array of directories, files, or glob patterns (in the docroot)
 *   that should not be overwritten.
 */
function update_core($exclude_from_copy = array()) {
  $exclude_from_copy = array_merge($exclude_from_copy, array(
    'favicon.ico',
    'sites/dev.example',
    'sites/default/settings.php',
    'modules/acquia',
    'themes/acquia',
    'profiles/gardens*',
    'profiles/florida_hospital',
    'profiles/warner',
    'sites/all/modules/*/',
    'sites/all/themes/*/',
    'sites/all/libraries',
  ));
  _update_from_workspace_directory(dirname( __FILE__ ) . '/../make-workspace/gardens', $exclude_from_copy);
}

/**
 * Copy all contrib modules from the make workspace into the docroot.
 *
 * All modules found within sites/all/modules will be copied.
 *
 * @param $exclude_from_copy
 *   Optional array of directories, files, or glob patterns (in the docroot)
 *   that should not be overwritten.
 */
function update_contrib($exclude_from_copy = array()) {
  $contrib = glob(dirname ( __FILE__ ) . '/../make-workspace/gardens/sites/all/modules/*', GLOB_ONLYDIR);
  foreach ($contrib as $make_path) {
    _update_from_workspace_directory($make_path, $exclude_from_copy, TRUE);
  }
}

/**
 * Copy the entire codebase from the make workspace into the docroot.
 *
 * @param $exclude_from_copy
 *   Optional array of directories, files, or glob patterns (in the docroot)
 *   that should not be overwritten.
 */
function update_all($exclude_from_copy = array()) {
  _update_from_workspace_directory(dirname( __FILE__ ) . '/../make-workspace/gardens', $exclude_from_copy);
}

/**
 * Copy a particular module from the make workspace into the docroot.
 *
 * The module must live in sites/all/modules in order to be copied.
 *
 * @param $module
 *   The name of the module to copy.
 * @param $exclude_from_copy
 *   Optional array of directories, files, or glob patterns (in the docroot)
 *   that should not be overwritten.
 */
function update_module($module, $exclude_from_copy = array()) {
  _update_from_workspace_directory(dirname( __FILE__ ) . "/../make-workspace/gardens/sites/all/modules/$module", $exclude_from_copy, TRUE);
}

/**
 * Copy code from the make workspace into the docroot.
 *
 * @param $make_path
 *   The path within the make workspace to copy code from. The code will be
 *   copied to an equivalent path in the docroot.
 * @param $exclude_from_copy
 *   Optional array of directories, files, or glob patterns (in the docroot)
 *   that should not be overwritten.
 * @param $cleanup_directories
 *   (optional) Set this to TRUE to use "svn rm" to clean up .git, CVS, and
 *   translations directories within the destination directory. Defaults to
 *   FALSE.
 */
function _update_from_workspace_directory($make_path, $exclude_from_copy = array(), $cleanup_directories = FALSE) {
  $docroot_path = str_replace('/make-workspace/gardens', '/docroot', $make_path);
  svn_safe_copy($make_path, $docroot_path, $exclude_from_copy);
  if ($cleanup_directories) {
    svn_remove_git_cvs_translation_directories($docroot_path);
  }
}

/**
 * Copy a directory, leaving .svn subdirectories in the destination intact.
 *
 * @param $path
 *   The directory to copy from.
 * @param $destination
 *   The directory to copy to.
 * @param $exclude_from_copy
 *   Optional array of directories, files, or glob patterns (in the docroot)
 *   that should not be overwritten. Any .svn subdirectories will be
 *   automatically added to this list.
 */
function svn_safe_copy($path, $destination, $exclude_from_copy = array()) {
  $exclude_from_copy[] = '.svn';
  $command = "rsync --verbose -a --delete ";
  foreach ($exclude_from_copy as $path_to_exclude) {
    $command .= "--exclude=$path_to_exclude ";
  }
  $command .= "$path/ $destination";
  print "Running: $command\n";
  $output = array();
  $return = 0;
  exec($command, $output, $return);
}

/**
 * Remove any outdated .git, CVS, or "translations" directories from a path.
 *
 * @param $path
 *   The path to remove the directories from.
 */
function svn_remove_git_cvs_translation_directories($path) {
  // TODO: ignore errors
  exec("svn rm $path/.git; svn rm `find $path -name CVS`; svn rm `find $path -name translations`");
}
