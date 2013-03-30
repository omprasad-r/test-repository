<?php

class ProjectInfoChecker {
  public $files = array();
  private $current_git_project;
  private $current_git_branch_or_tag;

  public function __construct($drupal_root, $directory) {
    // Load files required by this class from the Drupal installation.
    require_once $drupal_root . '/includes/bootstrap.inc';
    require_once $drupal_root . '/includes/common.inc';

    // Store an ordered array of all files underneath the provided directory.
    $this->files = array($directory);
    if (version_compare(PHP_VERSION, '5.3.0') == -1) {
      // We need to hardcode the flags value because the class constant SKIP_DOTS
      // was not available prior to php 5.3
      $flags = 4096;
    }
    else {
      $flags = RecursiveDirectoryIterator::SKIP_DOTS;
    }
    $file_objects = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($directory, $flags), RecursiveIteratorIterator::SELF_FIRST);
    foreach ($file_objects as $file => $object) {
      $this->files[] = $file;
    }
  }

  /**
   * Checks all relevant info files and returns an array of errors.
   */
  public function checkFiles() {
    $errors = array();

    foreach ($this->getInfoFileVersions() as $file => $version) {
      $info = drupal_parse_info_file($file);
      // These should always be present.
      foreach (array('core', 'project', 'version') as $property) {
        if (!isset($info[$property])) {
          $errors[] = "$file is missing the '$property' property in its info file.";
        }
      }
      // The version should always be valid.
      if (isset($info['version'])) {
        // If we're on HEAD, we don't know what branch it should be, but at
        // least make sure it's dev.
        if ($version == 'HEAD') {
          if (!$this->isDevVersion($info['version'])) {
            $errors[] = "$file is on HEAD/master but is not using a dev version in the info file ({$info['version']}).";
          }
        }
        else {
          // The versions must match, but if we're supposed to be on a dev
          // version we allow the version in the info file to either have or
          // not have "-dev" at the end. The latter is to support modules that
          // are running off of a d.o. sandbox and therefore don't actually
          // have -dev tarballs.
          if ($info['version'] != $version && (!$this->isDevVersion($version) || $info['version'] != substr($version, 0, -4))) {
            $errors[] = "$file is on $version but the info file says it's on {$info['version']}";
          }
          // If we're supposed to be on a dev version, check for a datestamp.
          if ($this->isDevVersion($version) && empty($info['datestamp'])) {
            $errors[] = "$file is on a dev version ($version) but does not have a datestamp in the info file.";
          }
        }
      }
    }

    return $errors;
  }

  /**
   * Returns an array mapping info files to their expected versions.
   */
  public function getInfoFileVersions() {
    $info_file_versions = array();
    foreach ($this->files as $file) {
      if ($this->isExportable($file) && $this->readyForDrupal7($file)) {
        if (is_dir($file)) {
          $this->storeCurrentGitProperties($file);
        }
        if ($this->isInfoFile($file)) {
          $branch_or_tag = $this->getBranchOrTag($file);
          // Projects that don't have a branch or tag (i.e., aren't in Gardens
          // via a git or CVS checkout) get skipped for now.
          if (isset($branch_or_tag)) {
            // Depending on how the project was checked out from git, we could
            // end up with a ref name like remotes/origin/7.x-.2.x, in which
            // case we just want the 7.x-2.x part.
            if (strpos($branch_or_tag, '/') !== FALSE) {
              $parts = explode('/', $branch_or_tag);
              $branch_or_tag = end($parts);
            }
            $info_file_versions[$file] = $branch_or_tag;
          }
        }
      }
    }
    return $info_file_versions;
  }

  /**
   * Determines if a file lives in a directory that we expect to be exportable.
   */
  public function isExportable($file) {
    // The themes in docroot/themes/acquia and sites/[url]/themes/mythemes are
    // exportable, but they use the 'version' key in the .info file differently
    // than Drupal normally does, and they aren't from Git or CVS anyway, so we
    // treat them as non-exportable also.
    return strpos($file, 'docroot/profiles/gardens') === FALSE && strpos($file, 'docroot/modules/acquia') === FALSE && strpos($file, 'docroot/themes/acquia') === FALSE && strpos($file, '/themes/mythemes/') === FALSE;
  }

  /**
   * Determines if a file belongs to a project that is ready for Drupal 7.
   */
  public function readyForDrupal7($file) {
    // Special case; simpleviews for D7 doesn't actually exist on drupal.org.
    // The version we have in Gardens (which we're going to remove soon anyway)
    // is a heavily patched D6 version, so just skip it.
    return strpos($file, 'sites/all/modules/simpleviews/') === FALSE;
  }

  /**
   * Determines if a version string represents a -dev version of a project.
   */
  public function isDevVersion($version) {
    return substr($version, -4) == '-dev';
  }

  /**
   * As we traverse the directories, this function keeps track of the current
   * git project and branch/tag we're traversing (if there is one).
   */
  private function storeCurrentGitProperties($directory) {
    // If this directory is not underneath the previously found git project,
    // reset the variable that tracks which project we're on.
    if (isset($this->current_git_project) && strpos($directory, $this->current_git_project . '/') !== 0) {
      $this->current_git_project = NULL;
    }

    // Since sites/all/modules and sites/all/themes are part of Drupal core
    // (which is managed by Git) but have subdirectories that can be separate
    // Git checkouts, we need to separately track when we enter or exit these
    // directories and manipulate the Git properties accordingly.
    static $subdirs_entered = array(
      '/sites/all/modules/' => FALSE,
      '/sites/all/themes/' => FALSE,
    );
    static $subdirs_in = array(
      '/sites/all/modules/' => FALSE,
      '/sites/all/themes/' => FALSE,
    );
    static $core_git_directory, $core_git_branch_or_tag;
    foreach ($subdirs_entered as $subdir => $entered) {
      if (!$entered && strpos($directory, $subdir) !== FALSE) {
        // When we enter the subdirectory for the first time, stop keeping
        // track of the core Git branch, but remember it for future use.
        if (isset($this->current_git_project)) {
          $core_git_directory = $this->current_git_project;
          $core_git_branch_or_tag = $this->current_git_branch_or_tag;
          $this->current_git_project = NULL;
        }
        $subdirs_entered[$subdir] = TRUE;
        $subdirs_in[$subdir] = TRUE;
      }
    }
    foreach ($subdirs_in as $subdir => $in) {
      // When we exit the subdirectory, restore the core Git branch as our
      // current one.
      if ($in && strpos($directory, $subdir) === FALSE) {
        $this->current_git_project = $core_git_directory;
        $this->current_git_branch_or_tag = $core_git_branch_or_tag;
        $subdirs_in[$subdir] = FALSE;
      }
    }

    // If this directory contains a git project, remember that, and also find
    // its branch or tag.
    if (is_dir("$directory/.git")) {
      $this->current_git_project = $directory;
      $this->current_git_branch_or_tag = $this->getGitBranchOrTag($directory);
    }
  }

  /**
   * Find the current git branch or tag for a directory.
   *
   * Note that the directory is assumed to have already been checked to verify
   * that it is under git control.
   */
  public function getGitBranchOrTag($directory) {
    chdir($directory);

    // First check if we are on a tag.
    $current_git_branch_or_tag = trim(shell_exec("git name-rev --name-only --tags HEAD"));

    // If not, check if we are on a branch.
    if ($current_git_branch_or_tag == 'undefined') {
      $current_git_branch_or_tag = trim(shell_exec("git name-rev --name-only HEAD"));
    }

    // For whatever reason, sometimes the tags have "^0" appended. Remove that.
    if (substr($current_git_branch_or_tag, -2) == '^0') {
      $current_git_branch_or_tag = substr($current_git_branch_or_tag, 0, -2);
    }

    return $current_git_branch_or_tag;
  }

  /**
   * Find the current commit for a given git directory.
   *
   * @param $directory
   *   The directory of the git project (or a file within that directory).
   *
   * @return
   *   A SHA1 hash representing the most recent commit on the current branch.
   */
  public function getGitCommit($directory) {
    chdir(dirname($directory));
    $commit = trim(shell_exec("git rev-parse HEAD"));
    // Make sure the return value looks like a SHA1 hash.
    if (preg_match('/^[0-9a-f]{40}$/i', $commit)) {
      return $commit;
    }
    return FALSE;
  }

  /**
   * Find the origin URL of a given git directory.
   *
   * @param $directory
   *   The directory of the git project (or a file within that directory).
   *
   * @return
   *   The origin URL (i.e. http://git.drupal.org/project/x.git).
   */
  public function getGitOrigin($directory) {
    chdir(dirname($directory));
    $origin = trim(shell_exec('git config --get remote.origin.url'));
    // Make sure the return value looks like a URL.
    if (strpos($origin, 'http') === 0) {
      return $origin;
    }
    return FALSE;
  }

  /**
   * Determines if a file is a Drupal .info file.
   */
  public function isInfoFile($file) {
    return is_file($file) && preg_match('/\.info$/', $file);
  }

  /**
   * Determines the actual CVS/git branch or tag associated with a file.
   *
   * This function is private because it relies on the git properties at the
   * current state of the directory traversal.
   */
  private function getBranchOrTag($file) {
    // If we are in a git directory, use the information from that.
    if (isset($this->current_git_project)) {
      $branch_or_tag = $this->current_git_branch_or_tag;
    }
    // Otherwise, look for a CVS branch or tag. If the directory is not managed
    // by CVS, this will return NULL.
    else {
      $directory = dirname($file);
      $branch_or_tag = $this->getCVSBranchOrTag($directory);
    }

    // Cleanup; make sure we use a consistent string for HEAD/master, and
    // append "-dev" to dev versions (since that is how it will appear in the
    // download URL)
    if ($branch_or_tag == 'master' || $branch_or_tag == 'head') {
      $branch_or_tag = 'HEAD';
    }
    if (substr($branch_or_tag, -1) == 'x') {
      $branch_or_tag .= '-dev';
    }

    return $branch_or_tag;
  }

  /**
   * Returns the CVS branch or tag for the directory, or NULL if there is none.
   */
  public function getCVSBranchOrTag($directory) {
    if (is_dir("$directory/CVS")) {
      if (is_file("$directory/CVS/Tag")) {
        $raw_branch_or_tag = trim(file_get_contents("$directory/CVS/Tag"));
        $branch_or_tag = $this->parseCVSTag($raw_branch_or_tag);
      }
      elseif (is_file("$directory/CVS/Entries")) {
        $branch_or_tag = 'HEAD';
      }
      return $branch_or_tag;
    }
  }

  /**
   * Parses CVS Tag file contents to return the actual branch or tag name.
   */
  public function parseCVSTag($raw_tag) {
    // Remove the leading "N".
    $tag = substr($raw_tag, 1);

    // Deal with core (DRUPAL-7-0 becomes 7.0).
    if (preg_match('/^DRUPAL\-[0-9]+\-[0-9]+/', $tag)) {
      $tag = preg_replace('/^DRUPAL\-([0-9]+)\-([0-9]+)/', '$1.$2', $tag);
    }
    // Deal with contrib.
    else {
      // A leading DRUPAL-7 becomes 7.x.
      $tag = preg_replace('/^DRUPAL\-([0-9]+)\-/', '$1.x', $tag);
      // Tag: DRUPAL-7--1-0 becomes 7.x-1.0.
      if (preg_match('/[0-9]+\-[0-9]+/', $tag)) {
        $tag = preg_replace('/([0-9]+)\-([0-9]+)/', '$1.$2', $tag);
      }
      // Branch: DRUPAL-7--1 becomes 7.x-1.x.
      else {
        $tag = preg_replace('/\-([0-9]+)$/', '-$1.x', $tag);
      }
    }

    // Lowercase the tag; so e.g. "BETA1" becomes "beta1", which is how it will
    // appear in download URLs.
    $tag = strtolower($tag);

    return $tag;
  }

  /**
   * Print a set of errors, normally those returned by checkFiles(). Or print a
   * success message if there are none.
   */
  public function printErrors($errors) {
    if (!empty($errors)) {
      foreach ($errors as $error) {
        print "$error\n";
      }
    }
    else {
      print "No info file problems found.\n";
    }
  }
}
