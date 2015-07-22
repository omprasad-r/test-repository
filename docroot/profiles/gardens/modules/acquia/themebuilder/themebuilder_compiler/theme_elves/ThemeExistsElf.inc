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

        // Attempt to restore the theme using the backup directory created
        // during Themebuilder save.
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

        // Check if the theme can be found on the ephemeral disk.
        $ephemeral_dir = $this->getThemeEphemeralDir($themeName);
        if ($ephemeral_dir && file_exists($ephemeral_dir)) {
          // Recreate the symbolic link hoping that it would fix the Gluster
          // issue.
          $theme_path = $this->getThemeDir($themeName);
          $command = sprintf('[ ! -e %2$s ] && ln -s %1$s %2$s', escapeshellarg($ephemeral_dir), escapeshellarg($theme_path));
          exec($command, $output, $exit_code);
          if ($exit_code === 0 || $this->doesThemeExist($themeName)) {
            if ($exit_code === 0) {
              ThemeBuilderLog()->gardensWarning('Theme fixed by ThemeExistsElf.', sprintf('The default theme "%s" did not exist on Gluster but was found on the ephemeral disk and the symbolic link was recreated.', $themeName));
            }
            $fixCount++;
            return $fixCount;
          }
          else {
            $message = sprintf('The default theme "%s" did not exist on Gluster but was found on ephemeral. Could not recreate the symbolic link.', $themeName);
            ThemeBuilderLog()->gardensError('ThemeExistsElf tried to fix a theme using the ephemeral, but the operation failed.', $message);
            // Exit early because letting this theme elf proceed would mean
            // getting the active theme overridden and the customer will just
            // think that their theme got lost.
            throw new Exception($message);
          }
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

  /**
   * Returns the theme's supposed location on the ephemeral disk.
   *
   * @param string $theme_name
   *   The name of the theme to look for.
   *
   * @return string
   *   The path to the theme on the ephemeral disk if it can be found.
   */
  private function getThemeEphemeralDir($theme_name) {
    $path = NULL;
    $site = getenv('AH_SITE_GROUP');
    $env = getenv('AH_SITE_ENVIRONMENT');
    // In the theme repository on the ephemeral disk, the sites are organized in
    // a directory that relates to the name of the site's docroot pair.
    // Unfortunately this name has not been consistently generated, and it is
    // also not being stored in the site, so we can not use anything more
    // concrete here (example names being: tangle001, heartland1_01, wmg_04 or
    // on dev tangle_<sitegroup>). Lowering the scope of the search by:
    // - including the site's directory in the path after the asterisk,
    // - restricting the search to just directories (type -d),
    // - restricting the depth it is allowed to go (-maxdepth 1).
    // Note: maxdepth needs to come before the name option!
    // Note: escaping the directory path where the find should happen will
    // result in an error that the directory could not be found.
    $command = sprintf('find /mnt/tmp/%s.%s/theme_repo/live/themebuilder/*/%s -maxdepth 1 -name %s -type d', $site, $env, gardens_client_get_gardens_db_name(), escapeshellarg($theme_name));
    exec($command, $output, $exit_code);
    // The $output will be an array, and each value will be one line of the find
    // command's output. If $output is empty then the theme directory was not
    // found. If it contains more than one directory then we do not know which
    // one to take, so again, do not do anything.
    if ($exit_code === 0 && count($output) === 1) {
      $path = array_pop($output);
    }
    return $path;
  }

}
