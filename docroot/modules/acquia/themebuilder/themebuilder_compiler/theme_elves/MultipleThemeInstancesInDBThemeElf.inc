<?php

/**
 * This theme elf is responsible for detecting and fixing the issue in
 * which each save would result in a new copy of the theme, the
 * theme's assets, and all of the custom css associated with the
 * theme.  At some point the tables would grow so large that the theme
 * couldn't even be saved before the request would time out.
 */
class MultipleThemeInstancesInDBThemeElf extends ThemeElf {
  /**
   * Indicates whether the theme associated with the specified path is broken because it has multiple instances of the same theme in the themebuilder_themes table.
   *
   * @param {String} $path
   *   The absolute path to the theme's .info file.
   * @return {boolean}
   *   TRUE if the theme is broken, FALSE otherwise.
   */
  public function isBroken($path) {
    // SELECT COUNT(*) FROM themebuilder_themes WHERE theme_name = 'theme'; count > 1 indicates breakage.
    $themeName = $this->getThemeName($path);
    $count = intval(db_select('themebuilder_themes', 't')
      ->condition('theme_name', $themeName)
      ->countQuery()
      ->execute()
      ->fetchField());
    return $count > 1;
  }

  /**
   * Attempts to fix the theme by removing the duplicate entries and removes the arrays that were added to the 'regions', 'configurations', 'selectorMap', and 'settings' keys.
   *
   * @param {String} $path
   *   The absolute path to the theme's .info file.
   * @return {boolean}
   *   TRUE if the theme was fixed, FALSE otherwise.
   */
  public function fix($path) {
    $themeName = $this->getThemeName($path);
    $oldTids = db_select('themebuilder_themes', 't')
      ->fields('t', array('tid'))
      ->condition('theme_name', $themeName)
      ->orderBy('t.tid', 'DESC')
      ->range(1, 1000)
      ->execute()
      ->fetchCol();
    $this->removeOldThemes($oldTids);
    $count = db_select('themebuilder_themes', 't')
      ->condition('theme_name', $themeName)
      ->countQuery()
      ->execute()
      ->fetchField();
    return $count == 1;
  }

  private function removeOldThemes($tids) {
    $this->removeOldAssets($tids);
    db_delete('themebuilder_themes')
      ->condition('tid', $tids, 'IN')
      ->execute();
  }

  private function removeOldAssets($tids) {
    $aids = db_select('themebuilder_assets', 'a')
      ->fields('a', array('aid'))
      ->condition('tid', $tids, 'IN')
      ->execute()
      ->fetchCol();
    $this->removeOldCss($aids);
    db_delete('themebuilder_assets')
      ->condition('tid', $tids)
      ->execute();
  }
  private function removeOldCss($aids) {
    db_delete('themebuilder_css')
      ->condition('aid', $aids, 'IN')
      ->execute();
  }

  private function getThemeName($path) {
    $fileInfo = pathinfo($path);
    $themeName = $fileInfo['filename'];
    return $themeName;
  }
}
