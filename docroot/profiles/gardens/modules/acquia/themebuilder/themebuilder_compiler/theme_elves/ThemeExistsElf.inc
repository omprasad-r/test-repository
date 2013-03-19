<?php

/**
 * This theme elf checks the default theme and makes certain that it
 * actually exists.  If not, an appropriate default theme will be set,
 * allowing the site owner to select whatever theme they wish without
 * interacting with support.
 */
class ThemeExistsElf extends PostprocessThemeElf {
  /**
   * Returns the weight of this elf instance.
   *
   * This theme elf should run before the cache clear and before the
   * check to make sure themes are enabled in the system table.
   *
   * @return {int}
   *   The weight.
   */
  public function getWeight() {
    return -20;
  }

  /**
   * Do the actual check to see that the default theme is set and exists.
   *
   * @param {array} $pathArray
   *   An array containing paths to every custom theme associated with
   *   this site.
   * @return {int}
   *   The number of themes fixed (either 0 or 1).
   */
  public function process($pathArray) {
    $fixCount = 0;
    $themeName = variable_get('theme_default', NULL);
    if (!$this->doesThemeExist($themeName)) {
      if (!empty($themeName)) {
        $this->logGlusterIssues($themeName);
        try {
          $backupPath = $this->getBackupDir($themeName);
          $themePath = $this->getThemeDir($themeName);
          if (!file_exists($themePath)) {
            @mkdir($themePath);
          }
          // Copy the actual files.
          $command = "cp -Rfp ${backupPath}/* " . escapeshellarg($themePath);
          exec($command, $output, $result);
          if ($result === 0) {
            // The copy was successful.
            ThemeBuilderLog()->gardensWarning('Theme fixed by ThemeExistsElf.',
              "The default theme was set to \"${themeName}\", which did not exist.  The backup ${backupPath} was used to fix the theme.");
            $fixCount++;
            return $fixCount;
          }
          ThemeBuilderLog()->gardensWarning('ThemeExistsElf tried to fix a theme using the theme backup, but the operation failed.',
            "The default theme was set to \"${themeName}\", which did not exist.  Tried to use theme backup from ${backupPath} to fix the theme but failed with message:" . implode('; ', $output));
        }
        catch (Exception $e) {
          // Fall through to last resort code below.
        }
      }
      else {
        // The default theme is not set.  This should never happen,
        // but if it does it will fall through and be fixed by setting
        // a base theme as the default.
      }

      // The selected theme is missing and no backup exists.
      variable_set('theme_default', 'builderbase');

      // Note that we would definitely want to flush the cache after
      // changing the theme.  This happens via the CacheClearElf,
      // which is weighted to execute after everything else has been
      // done to fix themes.
      ThemeBuilderLog()->gardensError('AN-22435 - Theme fixed by ThemeExistsElf.',
        "The default theme was set to \"${themeName}\", which does not exist.  An appropriate default theme has been set by the ThemeExistsElf.");
      $fixCount++;
    }
    return $fixCount;
  }

  /**
   * Figures out if the default theme exists.
   *
   * @param {String} $themeName
   *   The name of the theme.
   * @return {boolean}
   *   TRUE if the default theme is set and exists, FALSE otherwise.
   */
  private function doesThemeExist($themeName) {
    $result = FALSE;
    if (!empty($themeName)) {
      $dir = $this->getThemeDir($themeName);
      // We are not checking specifically for a directory because this
      // could be a symbolic link, in the case of an edit session.
      if(file_exists($dir)) {
        $infoFilePath = $dir . "/$themeName.info";
        if (file_exists($infoFilePath)) {
          $result = TRUE;
        }
      }
    }
    return $result;
  }

  /**
   * Try to detect gluster client problems and log the results.
   *
   * Sometimes this elf detects the selected theme has no .info file and
   * replaces it with a base theme.  We have seen occasionally the glusterfs
   * client showing permissions on the containing directory as question marks.
   * Simply restarting the gluster client fixes this issue with apparently no
   * data loss.
   *
   * In order to help diagnose the issue I want to detect this situation and
   * log it to alert ops and gardens engineering so we can confirm this is the
   * underlying cause, or at least to get information about the underlying
   * cause.
   *
   * The problem is that I'm not sure exactly what the permissions value is
   * that would cause ls to show question marks.  I suspect it is an illegal
   * combination of the file type bits within the permissions.  Only 7 of 16
   * combinations are legal, so on this premise I will go about the business
   * of detecting the gluster problem.
   *
   * @param {String} $themeName
   *   The name of the theme.
   */
  private function logGlusterIssues($themeName) {
    if (!empty($themeName)) {
      $dir = $this->getThemeDir($themeName);
      $filename = "${dir}/${themeName}.info";
      $existing_filename = $filename;
      while (!file_exists($existing_filename)) {
        $existing_filename = dirname($existing_filename);
      }
      if (!empty($existing_filename)) {
        $perms = fileperms($existing_filename);
        if (!$this->fileTypeOk($perms)) {
          ThemeBuilderLog()->gardensError('DG-814 - ThemeExistsElf found a file with bad permissions', 'Filename: ' . $existing_filename . ', file permissions: ' . decoct($perms));
        }
        elseif ($existing_filename != $filename) {
          ThemeBuilderLog()->gardensError('DG-815 - ThemeExistsElf identified a missing theme info file', 'Filename: ' . $filename . ', first existing parent directory: ' . $existing_filename . ', file permissions for the directory: ' . decoct($perms));
        }
      }
    }
  }

  /**
   * Takes the specified permissions value and determines whether the
   * file type is legal.  We don't actually care what the file type
   * is, as long as it is a legal file type.  This is used to
   * determine whether the gluster client has become corrupt, in which
   * case the permissions are reported with question marks, indicating
   * the permissions do not make sense.  I believe that the file type
   * is the only set of bits in the permissions value that have
   * illegal values.
   *
   * @param {int} $perms
   *   The file permissions to check.
   * @return {boolean}
   *   TRUE if the file type is ok; FALSE otherwise.
   */
  private function fileTypeOK($perms) {
    $result = FALSE;
    $type = ($perms & 0xF000) >> 12;
    switch ($type) {
    case 0xC:
      // Socket
    case 0xA:
      // Symlink
    case 0x8:
      // Regular file
    case 0x6:
      // Block special
    case 0x4:
      // Directory
    case 0x2:
      // Character special
    case 0x1:
      // FIFO
      $result = TRUE;
      break;
    default:
      // An unknown combination.
      $result = FALSE;
    }
    return $result;
  }

  /**
   * Gets the directory of the specified theme.
   *
   * @param {String} $themeName
   *   The name of the theme.
   * @return {String}
   *   The theme directory.
   */
  private function getThemeDir($themeName) {
    if (strpos($themeName, "acq_") === 0) {
      $path = DRUPAL_ROOT . '/' . conf_path() . "/themes/mythemes/$themeName";
    }
    else {
      $path = DRUPAL_ROOT . "/themes/acquia/$themeName";
    }
    return $path;
  }

  /**
   * Returns the backup directory for the specified theme.
   *
   * If no backup directory exists, an exception is thrown.
   *
   * @param {String} $themeName
   *   The name of the theme.
   * @return {String}
   *   The fully-qualified path to the backup directory for the specified theme.
   */
  private function getBackupDir($themeName) {
    $path = DRUPAL_ROOT . '/' . conf_path() . '/files/backup';
    $command = 'find ' . escapeshellarg($path) . ' -name ' . escapeshellarg("${themeName}.info");
    exec($command, $output, $result);
    if ($result !== 0 || count($output) === 0) {
      throw new Exception('Could not find the backup directory');
    }

    // Find the most recent backup of the specified theme.
    $ctime = 0;
    for ($i = 0, $len = count($output); $i < $len; $i++) {
      $info = lstat($output[$i]);
      if ($info['ctime'] > $ctime) {
        $themePath = $output[$i];
        $ctime = $info['ctime'];
      }
    }

    // Remove the .info filename from the themePath.
    $themePath = substr($themePath, 0, max(0, strlen($themePath) - (strlen("${themeName}.info") + 1)));
    return $themePath;
  }
}
