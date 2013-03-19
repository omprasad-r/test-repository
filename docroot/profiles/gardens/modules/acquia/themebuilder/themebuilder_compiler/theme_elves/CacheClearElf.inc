<?php

/**
 * This class fixes themebuilder issues for which a cache clear is all
 * that is required.
 */
class CacheClearElf extends PostprocessThemeElf {
  /**
   * Clear the cache.
   *
   * @param {array} $pathArray
   *   An array that contains absolute paths to every .info file
   *   associated with a custom theme for the associated website.
   *
   * @return {int}
   *   The number of fixes applied, if appropriate.
   */
  public function process($pathArray) {
    drupal_flush_all_caches();
    return 0;
  }

  /**
   * Returns the weight of this elf.
   *
   * Clearing the cache should be the last thing that is done.
   */
  public function getWeight() {
    return 1000;
  }
}
