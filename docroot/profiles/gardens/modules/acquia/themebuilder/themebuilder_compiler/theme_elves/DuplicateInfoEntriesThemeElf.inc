<?php

/**
 * This theme elf is responsible for detecting and fixing the issue
 * that occured upon theme update on the 1.00 release.  Duplicate
 * lines appeared in the .info file, and some values were converted to
 * arrays, causing the block configuration page to behave poorly.
 *
 * In this scenario, he .info file must be renamed, and the
 * template.php file rewritten with the correct function names.
 */
class DuplicateInfoEntriesThemeElf extends ThemeElf {
  /**
   * Indicates whether the theme associated with the specified path is broken because the .info file contains duplicate entries.
   *
   * @param {String} $path
   *   The absolute path to the theme's .info file.
   * @return {boolean}
   *   TRUE if the theme is broken, FALSE otherwise.
   */
  public function isBroken($path) {
    $duplicates = $this->getDuplicateLines($path);
    return count($duplicates) > 0;
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
    $fixed = FALSE;

    $contents = file_get_contents($path);
    $new_contents = $this->removeDuplicateLines($contents);

    // Remove the [] from all regions, configurations, selectorMap,
    // and settings keys.
    $keys = array('regions', 'configurations', 'selectorMap', 'settings');
    foreach ($keys as $key) {
      $new_contents = $this->removeArrayFromKey($key, $new_contents);
    }

    // Write the results if needed.
    if ($new_contents != $contents) {
      $bytes = file_put_contents($path, $new_contents);
      if ($bytes !== FALSE) {
        ThemeBuilderLog()->gardensWarning('AN-24141 - Theme fixed by DuplicateEntriesThemeElf.',
          'The info file "' . $path . '" had theme in the form of duplicate key/value pairs and some values converted to arrays.  The problem has been resolved.');
        $fixed = TRUE;
      }
      else {
        // Write failed.
        ThemeBuilderLog()->gardensError('Theme fix attempt by DuplicateInfoEntriesThemeElf failed.', "Attempted to fix info file $path with the DuplicateInfoEntriesThemeElf, but writing the new contents of the .info file failed.");
      }
    }
    return $fixed;
  }

  /**
   * Returns any duplicate lines in the specified file.
   *
   * @param {String} $path
   *   The path to the .info file.
   * @return {array}
   *   The duplicate lines.
   */
  private function getDuplicateLines($path) {
    // Find all duplicate lines in the .info file.
    $contents = file_get_contents($path);
    $count = preg_match_all('/^(.*)(\r?\n\1)+$/m', $contents, $matches);
    return $matches[1];
  }

  /**
   * Removes duplicate lines in the specified contents.
   *
   * Only removes duplicates that appear one directly after the other
   * that are exactly the same.
   *
   * @param {String} $contents
   *   The contents of the .info file.
   * @return {String}
   *   The new contents with the duplicates removed.
   */
  private function removeDuplicateLines($contents) {
    $new_contents = preg_replace('/^(.*)(\r?\n\1)+$/m', '${1}', $contents);
    return $new_contents;
  }

  /**
   * Removes the array from the specified key within the specified contents.
   *
   * Turns a string like
   *   selectorMap[.active .active][] = '.tabs .tabs .active a'
   * into
   *   selectorMap[.active .active] = '.tabs .tabs .active a'
   *
   * @param {String} $key
   *   The key.
   * @param {String} $contents
   *   The contents of the .info file.
   * @return {String}
   *   The new contents with the array removed.
   */
  private function removeArrayFromKey($key, $contents) {
    $exp = "/^(${key}.*)(\[\] =)/m";
    $new_contents = preg_replace($exp, '${1} =', $contents);
    return $new_contents;
  }
}
