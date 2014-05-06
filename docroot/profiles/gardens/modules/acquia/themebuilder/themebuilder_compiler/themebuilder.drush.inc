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
  $items['themebuilder-check-diff'] = array(
    'dependencies' => array('themebuilder_compiler'),
    'arguments' => array(
      'name' => dt('Name of the theme to check'),
    ),
    'description' => 'Checks whether a given theme has modifications from its base.',
  );
  $items['themebuilder-revert-theme'] = array(
    'dependencies' => array('themebuilder_compiler'),
    'arguments' => array(
      'name' => dt('Name of the theme to revert to its base theme'),
    ),
    'description' => 'Reverts a subtheme to its base.',
  );
  $items['themebuilder-list-themes'] = array(
    'dependencies' => array('themebuilder_compiler'),
    'description' => 'Lists all themes known to themebuilder and their bases. Output is only available in JSON format.',
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

/**
 * Lists all themes known to the themebuilder and their bases.
 *
 * Outputs as JSON an associative array (object) with theme names as keys and
 * corresponding base theme names as values.
 */
function drush_themebuilder_list_themes() {
  $all_themes = themebuilder_compiler_get_theme_list();
  $result = array();
  foreach($all_themes['mythemes'] as $name => $info) {
    $theme = themebuilder_compiler_get_theme_by_name($name);
    $result[$name] = $theme->getBaseThemeName();
  }

  // Ensure that the top of the list is the published theme to ensure that if
  // and when we pull in the subtheme's block settings, the published theme
  // takes precedence.
  $published = variable_get('theme_default', 'bartik');
  if (isset($result[$published])) {
    $first = array($published => $result[$published]);
    unset($result[$published]);
    $result = $first + $result;
  }

  drush_print(drupal_json_encode($result));
}

/**
 * Command callback: check a given theme for modifications.
 *
 * Exits with error status (!= 0) if there are any detectable modifications in
 * the local theme from the base theme.  A zero exit status implies no such
 * modifications were detected.
 *
 * @param $name
 *   The [sub]theme name to compare against its inferred base.
 */
function drush_themebuilder_check_diff($name) {
  $theme = themebuilder_compiler_get_theme_by_name($name);
  $base_name = $theme->getBaseThemeName();
  if (empty($base_name)) {
    throw new Exception(sprintf('Theme %s has no base theme (most likely not a sub-theme)', $name));
  }
  $base = themebuilder_compiler_get_theme_by_name($base_name);

  // Both of these files typically have at least one empty line, so the only
  // reliable way to test "emptiness" is grab content and trim whitespace.
  $custom = trim(file_get_contents($theme->getPath() . '/custom.css'));
  if (strlen($custom) > 0) {
    return drush_set_error('THEME_IS_MODIFIED', 'custom.css has content');
  }
  $advanced = trim(file_get_contents($theme->getPath() . '/advanced.css'));
  if (strlen($advanced) > 0) {
    return drush_set_error('THEME_IS_MODIFIED', 'advanced.css has content');
  }

  $info = $theme->assets_new['info']->getContents();
  $base_info = $base->assets_new['info']->getContents();

  // Remove some properties from consideration that just create noise (name is
  // more or less always unique, and base_palette is added to every subtheme as
  // a copy of the palette of the base theme).
  unset($info['base_palette']);
  unset($info['name']);
  unset($base_info['name']);

  $local_modifications = array_diff_recursive($info, $base_info, TRUE);
  if (count($local_modifications)) {
    $message = sprintf('Some elements in the theme (%s) did not match the base theme (%s): %s', $name, $base_name, print_r($local_modifications, TRUE));
    return drush_set_error('THEME_IS_MODIFIED', $message);
  }

  // Elements present in the base theme which are not present in the subtheme -
  // we don't expect this to happen much, but conceivably could happen if there
  // were updates to the base theme that were not updated in the subtheme.
  $base_modifications = array_diff_recursive($base_info, $info, TRUE);
  if (count($base_modifications)) {
    $message = sprintf('Some elements in the base theme (%s) did not match the theme (%s).  This should not happen, but might imply a theme that was not updated to the latest: %s', $base_name, $name, print_r($base_modifications, TRUE));
    return drush_set_error('THEME_IS_MODIFIED', $message);
  }

  drush_print('No modifications to advanced and custom CSS files, or the .info file.');
}

/**
 * Reverts a custom theme to its corresponding base theme.
 *
 * @param string $name
 *
 */
function drush_themebuilder_revert_theme($name) {
  $theme = themebuilder_compiler_get_theme_by_name($name);
  $base_name = $theme->getBaseThemeName();
  theme_enable(array($base_name));
  $base = themebuilder_compiler_get_theme_by_name($base_name);
  if ($name == variable_get('theme_default', 'bartik')) {
    variable_set('theme_default', $base_name);
  }

  $blocks_modified = db_query('SELECT 1 FROM {block} WHERE theme = :theme', array(
    ':theme' => $base_name,
  ))->fetchField(0);
  // We only try to import the blocks settings once per base theme. The list has
  // been ordered to ensure the published theme is at the top, so that we ensure
  // that blocks settings from the most important theme take precedence.
  // Thereafter, blocks settings are imported exactly once for each base theme.
  if (!$blocks_modified) {
    // Merge block settings from the custom theme into the base theme. The
    // expectation here is that there will be no rows for the base theme when a
    // theme is first reverted (because the base theme would never have been
    // enabled prior to this theme revert).
    db_query('UPDATE {block} SET theme = :base WHERE theme = :theme', array(
      ':base' => $base_name,
      ':theme' => $name,
    ));
  }

  // Delete the themebuilder custom theme ... for now, we will soft-delete
  // themes, meaning they will cease to show in the themebuilder's list, but
  // a full delete would only require uncommenting this line:
  // themebuilder_compiler_delete_theme($theme, $name);

  // Soft-delete themebuilder theme:
  $soft_deleted = variable_get('themebuilder_soft_deleted_themes', array());
  $soft_deleted[] = $name;
  variable_set('themebuilder_soft_deleted_themes', $soft_deleted);
  cache_clear_all('themebuilder_theme_list', 'cache');
}

/**
 * Compares 2 arrays recursively.
 *
 * @param $array1
 *   First input array for comparison
 * @param $array2
 *   Second input array for comparison
 * @param bool $ci
 *   Set to TRUE for case insensitive comparison of strings
 *
 * @return array
 *   A nested array containing those parts of that are not contained in
 */
function array_diff_recursive($array1, $array2, $ci = FALSE) {
  $return = array();

  foreach ($array1 as $key => $value) {
    if (array_key_exists($key, $array2)) {
      if (is_array($value)) {
        $diff = array_diff_recursive($value, $array2[$key], $ci);
        if (count($diff)) {
          $return[$key] = $diff;
        }
      }
      else {
        // Only attempt case insensitive comparison if all values are strings.
        if ($ci && is_string($value) && is_string($array2[$key])) {
          if (strtolower($value) != strtolower($array2[$key])) {
            $return[$key] = $value;
          }
        }
        else {
          if ($value != $array2[$key]) {
            $return[$key] = $value;
          }
        }
      }
    }
    else {
      $return[$key] = $value;
    }
  }

  return $return;
}
