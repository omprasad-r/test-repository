<?php

/**
 * This theme elf looks for info file corruption.  We have had issues
 * in which entire HTML responses have been inserted into the .info
 * file as the value for the site logo or the favicon.  This elf knows
 * what to do.
 */
class CorruptedInfoFileThemeElf extends ThemeElf {
  /**
   * Determines whether the theme is broken because of a corrupted
   * .info file.
   *
   * @param {String} $path
   *   The absolute path to the theme's .info file.
   * @return {boolean}
   *   TRUE if the theme is broken, FALSE otherwise.
   */
  public function isBroken($path) {
    $contents = file($path, FILE_IGNORE_NEW_LINES);
    $result = $this->brokenLine($contents);
    return $result !== -1;
  }

  /**
   * Fixes the theme associated with the specified .info file.
   *
   * This should only ever be called on a theme with the corrupted
   * .info file.
   *
   * @param {String} $path
   *   The absolute path to the theme's .info file.
   * @return {int}
   *   TRUE if the theme has been fixed, FALSE otherwise.
   */
  public function fix($path) {
    $contents = file($path, FILE_IGNORE_NEW_LINES);
    $fixed = FALSE;
    $line = $this->brokenLine($contents);
    if ($line !== -1) {
      $newContents = array_slice($contents, 0, $line);
      $contentString = implode("\n", $newContents);
      $bytes = file_put_contents($path, $contentString);
      if ($bytes !== FALSE) {
        ThemeBuilderLog()->gardensWarning('AN-22432 - Theme fixed by CorruptedInfoFileThemeElf.',
          'The info file "' . $path . '" had theme corruption on line ' . $line . '.  The problem has been resolved.');
        $fixed = TRUE;
      }
      else {
        // Write failed.
        ThemeBuilderLog()->gardensError('Theme fix attempt by CorruptedInfoFileThemeElf failed.', "Attempted to fix info file $path with the CorruptedInfoFileThemeElf, but writing the new contents of the .info file failed.");
      }
    }
    else {
      // Theme wasn't broken
      ThemeBuilderLog()->gardensError('CorruptedInfoFileThemeElf was called to fix a theme that had no corruption in the .info file.', "CorruptedInfoFileThemeElf->fix called on path $path and the associated theme wasn't actually broken.  Check the logic in ThemeCorrector->iterate.");
    }
    return $fixed;
  }

  /**
   * Determines on which line the .info file is broken.
   *
   * This determination is made with a regular expression that is
   * applied to each line, therefore the line above the detected
   * corruption is actually the line at fault.  The return value takes
   * this into account.
   *
   * @param {String} $contents
   *   The contents of the .info file.
   * @return {int}
   *   The line number of the corrupted line, or -1 if no corruption
   *   was detected.
   */
  private function brokenLine($contents) {
    // This regular expression ascertains the correctness of each line
    // within the .info file.  By way of description, it goes like
    // this: Accept any line that is empty or contains a word followed
    // by any number of [] pairs (with or without contents) followed
    // by an equals sign and value.  If we find any line that does not
    // match this pattern, we have found trouble.
    $pattern = '/^\s*(\w+(\[[^\]]*\])*\s*=.+)?\s*$/';

    $len = count($contents);
    for ($i = 0; $i < $len; $i++) {
      if (!preg_match($pattern, $contents[$i])) {
        return max(0, $i - 1);
      }
    }
    // No corruption found.
    return -1;
  }
}
