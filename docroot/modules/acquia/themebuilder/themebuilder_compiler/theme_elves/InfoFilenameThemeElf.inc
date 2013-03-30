<?php

/**
 * This theme elf is responsible for detecting and fixing the issue that sometimes occurs in which the session theme is saved but the "_session" part of the theme name is not removed.
 *
 * In this scenario, he .info file must be renamed, and the
 * template.php file rewritten with the correct function names.
 */
class InfoFilenameThemeElf extends ThemeElf {
  /**
   * Indicates whether the theme associated with the specified path is broken because the name of the .info file does not match that of the containing directory.
   *
   * @param {String} $path
   *   The absolute path to the theme's .info file.
   * @return {boolean}
   *   TRUE if the theme is broken, FALSE otherwise.
   */
  public function isBroken($path) {
    // Check to ensure the filename of the .info file matches the name
    // of the containing directory.
    $themeFileInfo = $this->getThemeFileInfo($path);

    return $themeFileInfo['theme'] !== $themeFileInfo['dir'];
  }

  /**
   * Attempts to fix the theme by renaming the .info file and rewriting the template.php with method names that match the theme name.
   *
   * @param {String} $path
   *   The absolute path to the theme's .info file.
   * @return {boolean}
   *   TRUE if the theme was fixed, FALSE otherwise.
   */
  public function fix($path) {
    $fixed = FALSE;
    $themeFileInfo = $this->getThemeFileInfo($path);
    $themeName = $themeFileInfo['theme'];
    $dirName = $themeFileInfo['dir'];
    if ($themeName !== $dirName) {
      if (TRUE === $this->renameInfoFile($path, $dirName)) {
        // Great.  Now we have to rewrite the template.php file.
        if (TRUE === $this->modifyFunctionNames($themeFileInfo['path'], $themeName, $dirName)) {
          $fixed = TRUE;
          ThemeBuilderLog()->gardensWarning('AN-22437 - Theme fixed by the InfoFilenameThemeElf.', '  The theme "' . $dirName . '" had an info file named "' . $themeName . '.info", but that has been resolved with the InfoFilenameThemeElf.');
        }
        else {
          ThemeBuilderLog()->gardensError('The InfoFilenameThemeElf failed to fix a theme when changing template.php.', "Attempted to fix theme $path with the InfoFilenameThemeElf, but changing the code in template.php file failed.");
        }
      }
      else {
        ThemeBuilderLog()->gardensError('The InfoFilenameThemeElf failed to fix a theme when renaming the .info file.', "Attempted to fix theme $path with the InfoFilenameThemeElf, but renaming the .info file failed.");
      }
    }
    else {
      // This would only happen if the controlling code called fix on
      // a theme that wasn't broken.
      ThemeBuilderLog()->gardensError('InfoFilenameThemeElf->fix called to fix a theme that did not have an incorrect .info filename.', "Called on path $path and the associated theme wasn't actually broken.  Check the logic in ThemeCorrector->iterate.");
    }
    return $fixed;
  }

  /**
   * Returns file info from the specified path.
   *
   * The file info reveals the filename (no extension) of the theme
   * .info file and the name of the containing directory.  This
   * provides all of the information necessary to determine if a theme
   * is broken because the .info filename doesn't match the direcory
   * name.
   *
   * @param {String} $path
   *   The absolute path of the theme .info file.
   * @return {Array}
   *   An associative array containing the name of the theme, the name
   *   of the containing directory, and the full path to the theme
   *   directory.
   */
  private function getThemeFileInfo($path) {
    $fileInfo = pathinfo($path);
    $themeName = $fileInfo['filename'];
    $dir = $fileInfo['dirname'];

    $dirInfo = pathinfo($fileInfo['dirname']);
    $dirName = $dirInfo['filename'];
    return array('theme' => $themeName, 'path' => $dir, 'dir' => $dirName);
  }

  /**
   * Renames the info file.
   *
   * @param {String} $path
   *   The absolute path to the .info file.
   * @param {String} $newName
   *   The theme name that the .info file should be changed to.
   * @return {boolean}
   *   TRUE if the rename was successful; FALSE otherwise.
   */
  private function renameInfoFile($path, $newName) {
    $fileInfo = pathinfo($path);
    $newPath = $fileInfo['dirname'] . "/$newName.info";
    $result = rename($path, $newPath);
    return $result;
  }

  /**
   * Modifies the template.php, changing the function names from the old theme name to the new theme name.
   *
   * @param {String} $path
   *   The absolute path to the theme directory.
   * @param {String} $oldTheme
   *   The current name of the theme.
   * @param {String} $newTheme
   *   The name of the new theme.
   * @return {boolean}
   *   TRUE if the modification was successful; FALSE otherwise.
   */
  private function modifyFunctionNames($path, $oldTheme, $newTheme) {
    $result = FALSE;
    try {
      $filename = "$path/template.php";
      $code = file_get_contents($filename);

      // Replace all instances of the old theme name with the new
      // theme name.  Note that this is the same regex used in the
      // FunctionAssetNew->renameFunctions() method.
      $modifiedCode = preg_replace('/([ \t=])([_])*(' . $oldTheme . ')([_])/m',
        '${1}${2}' . $newTheme . '${4}', $code);

      $bytes = file_put_contents($filename, $modifiedCode);
      if ($bytes !== FALSE) {
        $result = TRUE;
      }
    }
    catch (Exception $e) {
    }
    return $result;
  }
}
