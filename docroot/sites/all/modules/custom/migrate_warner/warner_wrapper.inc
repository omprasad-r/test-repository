<?php

/**
 * Warner Migrate (warner://) stream wrapper class.
 *
 * Provides support for storing uploaded EOS dumps where they won't be seen by
 * anyone but the import code.
 *
 * Extends DrupalLocalStreamWrapper.
 */
class MigrateWarnerStreamWrapper extends DrupalLocalStreamWrapper {
  /**
   * Implements abstract public function getDirectoryPath()
   */
  public function getDirectoryPath() {
    if (file_exists('/mnt/files/tangle001/files-private')) {
      $directory = '/mnt/files/tangle001/files-private/eos_import';
    }
    else {
      $directory = '/mnt/files/tangle002/files-private/eos_import';
    }
    file_prepare_directory($directory, FILE_CREATE_DIRECTORY);
    return $directory;
  }

  /**
   * Overrides getExternalUrl().
   *
   * Return the HTML URI of a private file.
   */
  function getExternalUrl() {
    $path = str_replace('\\', '/', $this->getTarget());
    return url('system/files/' . $path, array('absolute' => TRUE));
  }

  public function getRealPath($uri) {
    return $this->getLocalPath($uri);
  }
}
