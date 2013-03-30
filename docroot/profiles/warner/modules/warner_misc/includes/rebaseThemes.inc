<?php

/**
 * A utility to change the base theme of acq_ subthemes.
 */
function warner_misc_rebase_themes($base = null) {
  if (!empty($base)) {
    // Destroy all ThemeBuilder sessions
    // This is destructive! Please verify with end-users that they have closed
    // any ThemeBuilder sessions before running this code.
    if (db_table_exists('themebuilder_session')) {
      db_truncate('themebuilder_session')
        ->execute();
    }
    // Acquire a semphore to prevent simultaneous editing of a theme.
    $semaphore = themebuilder_compiler_get_semaphore();
    // Get a list of themes straight from the database. We will use the database
    // representation of a theme to determine if it needs to be rebased. This
    // saves us the overhead of reading files from the filesystem before we know
    // if they need to be acted on.
    $themes = list_themes(TRUE);
    foreach ($themes as $name => $theme) {
      // We only want subthemes of Acquia's base themes.
      $isAcquia = strpos($name, 'acq_');
      if ($isAcquia === 0) {
        $info = $theme->info;
        $rebase = FALSE;
        foreach ($info['regions'] as $name => $region) {
          // If the theme has a region named coda, it needs to be rebased to the
          // wmg base theme.
          if ($name === 'coda') {
            $rebase = TRUE;
            break;
          }
        }
        // Rebase the theme
        if ($rebase) {
          $filename = $theme->filename;
          $path = DRUPAL_ROOT . '/' . $filename;
          if (file_exists($path)) {
            // Read the info file into an array.
            $infofile = file($path, FILE_IGNORE_NEW_LINES);
          }
          if (!empty($infofile)) {
            // Find the base_theme entry and replace it with wmg.
            foreach ($infofile as $key => $value) {
              // Find the base_theme configuration in the theme's info file.
              if (preg_match('/^base_theme/', $value) === 1) {
                // If the value does not have the base in it, then act.
                if (preg_match('/' . $base . '/', $value) === 0) {
                  $infofile[$key] = "base_theme = '{$base}'";
                  $contentString = implode("\n", $infofile);
                  file_put_contents($path, $contentString, LOCK_EX);
                  break;
                }
              }
            }
          }
        }
      }
    }
    themebuilder_compiler_release_semaphore($semaphore);
    system_rebuild_theme_data();
  }
}
