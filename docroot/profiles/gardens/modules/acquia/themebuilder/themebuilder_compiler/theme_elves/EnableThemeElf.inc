<?php

/**
 * This theme elf looks at all of the themes being used (the default
 * theme and any themes currently being edited) and ensures they are
 * enabled.
 */
class EnableThemeElf extends PostprocessThemeElf {
  /**
   * Get the set of themes that should be enabled, and make sure they
   * are enabled.
   *
   * @param {array} $pathArray
   *   An array that contains absolute paths to every .info file
   *   associated with a custom theme for the associated website.
   *
   * @return {int}
   *   The number of fixes applied, if appropriate.
   */
  public function process($pathArray) {
    // Make sure we rebuild the theme data here.  Theme .info filenames
    // may have changed during the theme fix operation.
    system_rebuild_theme_data();
    $themes = $this->findThemesInUse();
    $fixed = $this->enableThemes($themes);
    return $fixed;
  }

  /**
   * Discovers the set of themes that should be in use.
   *
   * @return {array)
   *   An array of theme names that are currently in use.
   */
  private function findThemesInUse() {
    $themes = array();
    $defaultTheme = variable_get('theme_default', NULL);
    if (!empty($defaultTheme)) {
      $themes[] = $defaultTheme;
    }

    // Get the themes associated with individual users...
    $editThemes = db_query('SELECT theme FROM {users} u')->fetchCol();
    $count = count($editThemes);
    for ($i = 0; $i < $count; $i++) {
      if (!empty($editThemes[$i])) {
        $themes[] = $editThemes[$i];
      }
    }
    return $themes;
  }

  /**
   * Ensures the specified themes are enabled.
   *
   * @param {array} $themes
   *   The names of the themes that should be enabled.
   * @return {int}
   *   The number of themes from the specified list that were not
   *   enabled when this method was invoked.
   */
  private function enableThemes($themes) {
    $fixed = 0;
    $count = count($themes);
    for ($i = 0; $i < $count; $i++) {
      $enabled = db_query("SELECT status FROM {system} WHERE type = :type AND name = :name",
        array(':type' => 'theme', ':name' => $themes[$i]))->fetchField();
      if ($enabled !== 1 && $enabled !== '1') {
        // Found a problem.  One of our themes being used is disabled.
        // Fix that here.  I am not enabling the theme using standard
        // Drupal techniques because the theme elves are working on
        // themes that may be horrifically broken and in many cases
        // cause Drupal to white screen or throw errors.  In general,
        // when fixing the themes, we should avoid those.
        db_update('system')
          ->fields(
            array(
              'status' => '1',
            )
          )
          ->condition('name', $themes[$i])
          ->execute();
        // TODO: We probably don't need this level of information long
        // term, but it would be good to understand how often these
        // things happen.
        ThemeBuilderLog()->gardensWarning('AN-22436 - Theme fixed by EnableThemeElf.', 'The theme "' . $themes[$i] . '" was not enabled in the database, but that has been resolved with the EnableThemeElf.');
        $fixed++;
      }
    }
    return $fixed;
  }
}
