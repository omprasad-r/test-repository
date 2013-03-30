<?php
/**
 * @file
 *   Themebuilder drush commands.
 */

/**
 * Implementation of hook_drush_command().
 *
 * In this hook, you specify which commands your
 * drush module makes available, what it does and
 * description.
 *
 * Notice how this structure closely resembles how
 * you define menu hooks.
 *
 * @See drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function themebuilder_drush_command() {
  $items = array();
  $items['tb-seed-cache'] = array(
    'callback' => 'themebuilder_seed_cache',
    'dependencies' => array('themebuilder_compiler'),
    'description' => 'Seeds the base themes with a cached version of palette-related properties.',
  );
  return $items;
}

/**
 * Implementation of hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function themebuilder_drush_help($section) {
  switch ($section) {
    case 'drush:themebuilder seed cache':
      return dt("Seeds the base themes with a cached version of palette-related properties.");
  }
}

/**
 * Runs the seedCache() method on each of the base Acquia themebuilder themes.
 */
function themebuilder_seed_cache() {
  $args = func_get_args();
  $exists = is_writable('themes/acquia/broadway');
  $site_theme = variable_get('theme_default', NULL);

  $available_themes = list_themes();
  foreach ($available_themes as $name => $available_theme) {
    if (strpos($available_theme->filename, 'themes/acquia') === 0) {
      drush_print('Seeding cache for ' . $name . '...');
      variable_set('theme_default', $name);
      $theme = themebuilder_compiler_get_current_theme();
      $theme->seedCache();
    }
  }
  // Put the site's original theme default back the way it was.
  if ($site_theme) {
    variable_set('theme_default', $site_theme);
  }
}
