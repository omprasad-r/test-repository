<?php

class ThemeChecker {

  public function __construct($argv) {
    // This belongs in the tools directory at the top of the workspace.
    $pathinfo = pathinfo($argv[0]);
    // Traverse up one directory from tools to get to the docroot.
    $dir = realpath($pathinfo['dirname']) . '/../docroot/themes/acquia';
    if (!file_exists($dir)) {
      throw new Exception("Directory $dir does not exist.");
    }
    if (!is_dir($dir)) {
      throw new Exception("$dir is not a directory.");
    }
    $this->dir = $dir;
  }

}
