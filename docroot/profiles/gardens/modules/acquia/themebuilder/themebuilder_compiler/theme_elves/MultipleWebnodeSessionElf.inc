<?php

/**
 * This theme elf fixes an issue that causes a fatal error for any
 * request of an authenticated user with the 'administer site
 * configuration' role when the themebuilder_session table has
 * multiple webnodes (should always be either 0 or 1).
 */
class MultipleWebnodeSessionElf extends PreprocessThemeElf {
  protected static $block = FALSE;
  protected $okToRun = FALSE;

  /**
   * Indicates whether the theme associated with the specified path is broken because it has multiple instances of the same theme in the themebuilder_themes table.
   *
   * @param {String} $path
   *   The absolute path to the theme's .info file.
   * @return {boolean}
   *   TRUE if the theme is broken, FALSE otherwise.
   */
  public function isBroken($path) {
    $sids = $this->findAbandonedSessions();
    $count = $this->detectMultipleWebnodes();
    return (count($sids) > 0 || $count > 1);
  }

  /**
   * Get the set of webnodes from the themebuilder_session table
   *
   * @param {array} $pathArray
   *   An array that contains absolute paths to every .info file
   *   associated with a custom theme for the associated website.
   *
   * @return {int}
   *   The number of fixes applied, if appropriate.
   */
  public function process($pathArray) {
    $fixed = FALSE;
    $sids = $this->findAbandonedSessions();
    if (count($sids) > 0) {
      // Some sids are associated with abandoned themebuilder sessions.
      $this->removeAbandonedSessions($sids);
      $fixed = TRUE;
    }
    $count = $this->detectMultipleWebnodes();
    if ($count > 1) {
      // More than one webnode is identified in the
      // themebuilder_session table.
      $this->removeAllSessions($count);
      $fixed = TRUE;
    }
    return $fixed;
  }

  /**
   * Finds themebuilder sessions that have been abandoned.
   *
   * This is done by finding sids in the themebuilder_session table
   * for which there is no corresponding Drupal session.
   *
   * @return {Array}
   *   An array of sids representing sessions that have been abandoned.
   */
  private function findAbandonedSessions() {
    $sids = db_query("SELECT ts.sid FROM {themebuilder_session} ts LEFT JOIN {sessions} s ON (ts.sid = s.sid OR ts.sid = s.ssid) WHERE s.sid IS NULL")->fetchCol();
    return $sids;
  }

  /**
   * Determines the number of webnodes that are being used for themebuilder sessions.
   *
   * This is done by counting the unique webnodes in the
   * themebuilder_session table.  There should only ever be 0 or 1
   * webnode in this table; more than that constitutes an error
   * condition.
   *
   * @return {int}
   *   The number of webnodes curently in use in the
   *   themebuilder_session table.
   */
  private function detectMultipleWebnodes() {
    $webnode_count = db_query("SELECT COUNT(DISTINCT(webnode)) FROM {themebuilder_session}")->fetchField();
    return $webnode_count;
  }

  /**
   * Removes themebuilder sessions for the specified sids.
   *
   * @param {Array} $sids
   *   An array of sids representing drupal sessions associated with
   *   themebuilder sessions that must be deleted from the table.
   */
  private function removeAbandonedSessions($sids) {
    db_delete('themebuilder_session')
      ->where('sid IN (:sids) OR webnode IS NULL', array(':sids' => $sids))
      ->execute();
    ThemeBuilderLog()->gardensWarning('AN-25754 - Abandoned themebuilder sessions removed by the MultipleWebnodeSessionElf.',
      'The sids (' . implode(', ', $sids) . ') were in the themebuilder_session table with no corresponding Drupal session.');
  }

  /**
   * Removes all themebuilder sessions.
   *
   * This is necessary when multiple webnodes have been set into the
   * themebuilder_session table.  Only one webnode should ever be used
   * at the same time.
   *
   * @param {int} $count
   *   The number of webnodes found in the themebuilder_session table.
   */
  private function removeAllSessions($count) {
    db_delete('themebuilder_session')
      ->execute();
    ThemeBuilderLog()->gardensWarning('AN-25755 - Too many webnodes in the themebuilder_session table fixed by the MultipleWebnodeSessionElf.',
      'There were ' . $count . ' webnodes referenced in the themebuilder_session table (should be either 0 or 1).  All themebuilder sessions were closed to resolve the issue.');
  }
}
