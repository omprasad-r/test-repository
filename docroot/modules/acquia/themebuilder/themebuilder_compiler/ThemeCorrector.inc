<?php

/**
 * The ThemeCorrector class is responsible for detecting theme issues and fixing them if possible.
 *
 * The ThemeCorrector makes use of tiny "theme elves" that fix themes
 * when you are not looking.  Each elf knows how to detect and correct
 * a particular type of theme corruption.  As we run into theme issues
 * in the future, we should be able to add a new elf to address them.
 */
class ThemeCorrector {
  private $elves;

  /**
   * Constructor.
   *
   * The constructor is responsible for finding and loading all the
   * elves.  The elves are sorted to run in an appropriate order,
   * which can be controlled by assigning a phase to the elf
   * ('before', 'during', or 'after'), and also by adjusting the elf
   * weight, which affects the order of execution within the
   * associated phase.
   *
   * @param {String} $path
   *   The absolute path to the custom theme directory
   * @param {array} $elves
   *   If an array of elf classnames is specified here, only those
   *   specified elves will be instantiated and used to fix themes.
   */
  public function __construct($path, $elves = array()) {
    $this->infoFiles = $this->findAllThemeInfoFiles($path);
    $this->elves = $this->loadThemeElves($elves);
    $this->sortElves();
  }

  /**
   * Loads the elves.
   *
   * @param {array} $requestedElves
   *   If an array of elf classnames is specified here, only those
   *   specified elves will be loaded, otherwise all available elves
   *   will be loaded..
   */
  private function loadThemeElves($requestedElves = array()) {
    $elves = array('before' => array(), 'during' => array(), 'after' => array());
    $path = DRUPAL_ROOT . '/' . drupal_get_path('module', 'themebuilder_compiler');
    $elf_path = "$path/theme_elves/";
    $command = "find $elf_path -name \"*.inc\"";
    @exec($command, $paths, $exitVal);
    if ($exitVal == 0) {
      $path_len = count($paths);
      for ($i = 0; $i < count($paths); $i++) {
        $info = pathinfo($paths[$i]);
        $classname = $info['filename'];
        if (empty($requestedElves) || in_array($classname, $requestedElves)) {
          require_once($paths[$i]);
          $elf = new $classname();
          $elves[$elf->getPhase()][] = $elf;
        }
      }
    }
    return $elves;
  }

  /**
   * Sorts the elves.
   */
  private function sortElves() {
    usort($this->elves['before'], array('ThemeCorrector', 'compareElves'));
    usort($this->elves['during'], array('ThemeCorrector', 'compareElves'));
    usort($this->elves['after'], array('ThemeCorrector', 'compareElves'));
  }

  /**
   * Utility function used during the sorting process.  Unfortunately
   * this has to be public.
   *
   * @param {ThemeElf} $a
   *   The first elf.
   * @param {ThemeElf} $b
   *   The second elf.
   * @return {int}
   *   0 if the elves have the same weight, -1 if the first elf has a
   *   lower weight than the second, 1 otherwise.
   */
  public static function compareElves($a, $b) {
    $weightA = 0;
    $weightB = 0;
    if (method_exists($a, 'getWeight')) {
      $weightA = $a->getWeight();
    }
    if (method_exists($b, 'getWeight')) {
      $weightB = $b->getWeight();
    }
    if ($weightA === $weightB) {
      return 0;
    }
    return $weightA < $weightB ? -1 : 1;
  }

  /**
   * Attempts to fix any custom themes associated with the site.
   *
   * @return {int}
   *   The number of theme fixes that were successfully applied.
   */
  public function fix() {
    $fixCount = 0;
    $fixCount += $this->iterate('before');
    $fixCount += $this->iterate('during');
    $fixCount += $this->iterate('after');
    return $fixCount;
  }

  /**
   * Iterates over all of the themes in the specified phase and
   * attempts to detect and fix issues using all of the elves
   * available.
   *
   * @param {String} $phase
   *   One of 'before', 'during', or 'after'.
   * @return {int}
   *   The number of theme fixes that were successfully applied.
   */
  private function iterate($phase) {
    $brokenCount = 0;
    $fixCount = 0;
    $elves = $this->elves[$phase];
    $elfCount = count($elves);
    for ($elfIndex = 0; $elfIndex < $elfCount; $elfIndex++) {
      $elf = $elves[$elfIndex];
      if ($phase == 'before' || $phase == 'after') {
        $count = $elf->process($this->infoFiles);
        if ($count) {
          gardens_statsd_send('theme_elf.success.' . get_class($elf), $count, GARDENS_STATSD_COUNTER);
        }
        $fixCount += $count;
      }
      else if ($phase == 'during') {
        $pathCount = count($this->infoFiles);
        for ($pathIndex = 0; $pathIndex < $pathCount; $pathIndex++) {
          if ($elf->isBroken($this->infoFiles[$pathIndex])) {
            // The theme is broken.  Try to fix it.
            $brokenCount++;
            if ($elf->fix($this->infoFiles[$pathIndex])) {
              gardens_statsd_send('theme_elf.success.' . get_class($elf), 1, GARDENS_STATSD_COUNTER);
              $fixCount++;
            }
            else {
              gardens_statsd_send('theme_elf.fail.' . get_class($elf), 1, GARDENS_STATSD_COUNTER);
              // If the fix failed, the individual elves will log an
              // appropriate message.
            }
          }
        }
      }
    }
    return $fixCount;
  }

  /**
   * Gets a list of all theme info files, which will be used to identify broken themes.
   *
   * @param {String} $path
   *   The absolute path to the custom theme directory.
   */
  private function findAllThemeInfoFiles($path) {
    $command = 'find -L ' . escapeShellArg($path) . ' -name "*.info"';
    @exec($command, $output, $exitVal);
    return $exitVal == 0 ? $output : NULL;
  }
}

/**
 * The ThemeElf is the base class for all theme elf implementations.
 */
class ThemeElf {
  /**
   * Returns the weight of this elf instance.
   *
   * @return {int}
   *   The weight.
   */
  public function getWeight() {
    return 0;
  }

  /**
   * Returns the phase of this elf instance.
   *
   * @return {String}
   *   'before', 'during', or 'after', depending on when during the
   *   round of theme fixing this elf should be executed.
   */
  public function getPhase() {
    return 'during';
  }

  /**
   * Indicates whether the theme associated with the specified path is broken with the type of issues this elf is able to detect and fix.
   *
   * @param {String} $path
   *   The absolute path to the theme's .info file.
   * @return {boolean}
   *   TRUE if the theme is broken, FALSE otherwise.
   */
  public function isBroken($path) {
    return FALSE;
  }

  /**
   * Attempts to fix the theme.
   *
   * @param {String} $path
   *   The absolute path to the theme's .info file.
   * @return {boolean}
   *   TRUE if the theme was fixed, FALSE otherwise.
   */
  public function fix($path) {
    return FALSE;
  }
}

/**
 * The PreprocessThemeElf is the base class of all theme elf implementations that are meant to execute before the theme elves that work on individual themes.
 *
 * If there is any preprocessing logic that needs to occur before the
 * bulk of the elves are invoked, use a subclass of PreprocessThemeElf
 * to accomplish that.
 */
class PreprocessThemeElf extends ThemeElf {
  /**
   * Returns the phase of this elf instance.
   *
   * @return {String}
   *   'before', 'during', or 'after', depending on when during the
   *   round of theme fixing this elf should be executed.
   */
  public function getPhase() {
    return 'before';
  }

  /**
   * Invoke the fix, in whatever form makes sense.
   *
   * @param {array} $pathArray
   *   An array that contains absolute paths to every .info file
   *   associated with a custom theme for the associated website.
   *
   * @return {int}
   *   The number of fixes applied, if appropriate.
   */
  public function process($pathArray) {
    return 0;
  }
}

/**
 * The PostprocessThemeElf is the base class of all theme elf implementations that are meant to execute after the theme elves that work on individual themes.
 *
 * If there is any postprocessing logic that needs to occur after the
 * bulk of the elves are invoked, use a subclass of PostprocessThemeElf
 * to accomplish that.
 *
 * Examples include setting the 'status' flag in the system table for
 * all themes that should be enabled and clearing the cache.
 */
class PostprocessThemeElf extends PreprocessThemeElf {
  /**
   * Returns the phase of this elf instance.
   *
   * @return {String}
   *   'before', 'during', or 'after', depending on when during the
   *   round of theme fixing this elf should be executed.
   */
  public function getPhase() {
    return 'after';
  }
}
