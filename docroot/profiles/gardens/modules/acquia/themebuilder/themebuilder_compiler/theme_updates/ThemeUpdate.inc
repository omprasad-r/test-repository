<?php

/**
 * The ThemeUpdate class is the superclass of all theme updates.  The update
 * logic goes in this class while the specifics of a particular update belong
 * in the subclass.
 *
 * Subclasses have names of the form {base_theme_name}Update{index} and reside
 * in a file name theme_updates/{base_theme_name}Update{index].inc, where
 * base_theme_name is the name of the base theme, and index is a number
 * starting from one, and incrementing once for each update for the associated
 * theme.  The idea is that for any base theme being updated, all updates
 * could be identified as easily as trying to instantiate every update class
 * for that base theme.
 *
 * The subclass must have protected member variables that indicate the theme
 * version that the update takes as input and the theme version the update
 * outputs.  These member variables are '$oldVersion' and '$newVersion',
 * respectively.  When updating a theme, each theme update class that matches
 * the version range of the theme being updated will instantiate correctly.
 * All themes outside of the update range will fail with an exception,
 * essentially keeping themes outside the version range required for the
 * update from being used.
 *
 * The subclass must also contain an associative array that maps old selector
 * chunks to new selector chunks.  The selector chunks are pieces of selectors
 * that are space separated.  Each chunk within a selector could potentially
 * be replaced by the theme update.  Note that if no selector chunks need to
 * be replaced for a theme update, a ThemeUpdate implementation is
 * unnecessary.  A basic update includes replacing all of the theme templates,
 * regenerating the .info file, and recreating the template.php file, as well
 * as refreshing any other files that come with the base theme.  Creating one
 * of these update classes should be the exception, not the rule.
 *
 * For an example of an actual theme update implementation, see
 * broadwayUpdate1.inc.
 */
class ThemeUpdate {

  protected $selectorMap = array();
  protected $paletteMap = array();
  /**
   * Creates a new instance of this theme updater.  If the specified update
   * does not meet the condition that this updater responds to, an exception
   * should be thrown at this point so this instance is not added to the list
   * of updates that will run.
   */
  public function __construct($theme, $oldVersion, $newVersion) {
    $this->valid = false;
    if (isset($this->themes) && !in_array($theme, $this->themes)) {
      // This class does not apply to the current theme.
      // I would prefer to use an exception, but exceptions thrown from the
      // constructor are handled via the error_handler (see set_error_handler)
      // rather than the enclosing try/catch blocks.  This flattens exception
      // handling to the point that it isn't very useful, but seems to only be
      // a problem for exceptions thrown from a constructor.
      return;
    }
    if ($this->versionLessOrEqual($oldVersion, $this->oldVersion) &&
      $this->versionLessOrEqual($this->newVersion, $newVersion)) {
      // This update class applies to the current update.
      $this->valid = true;
    }
    else {
      // This class doesn't apply to the current theme update.  The theme
      // version range for the update is outside of the version range
      // applicable for this class.  Again, I would prefer to throw an
      // exception here.
      return;
    }
  }

  /**
   * Indicates whether this update class should be used for the current
   * update.
   */
  public function isValid() {
    return $this->valid === true;
  }

  /**
   * Determines whether the first specified version number is less than or
   * equal to the second version number.
   *
   * @param {String} $ver1
   *   The first version.
   * @param {String} $ver2
   *   The second version.
   * @return
   *   true if $ver1 is less than or equal to $ver2; false otherwise.
   */
  protected function versionLessOrEqual($ver1, $ver2) {
    $ver1Sections = explode('.', $ver1);
    $ver2Sections = explode('.', $ver2);
    $ver1Len = count($ver1Sections);
    $ver2Len = count($ver2Sections);
    $limit = min(count($ver1Sections), count($ver2Sections));
    for ($index = 0; $index < $limit; $index++) {
      $diff = (int)$ver2Sections[$index] - (int)$ver1Sections[$index];
      if ($diff != 0) {
        return $diff > 0;
      }
    }
    if ($ver1Len != $ver2Len) {
      return $ver1Len < $ver2Len;
    }
    else {
      // Equality
      return true;
    }
  }

  /**
   * Creates customizations based on the specified css rules.  The css rules
   * passed in should be of the form $selector => array($property1 => $value1,
   * $property2 => $value2, ...).  This is the heart of the update class, as
   * it breaks the selector into chunks and then determines whether any of
   * those chunks have a replacement in the new theme.
   *
   * @param {Array} $css
   *   The original custom css.
   * @return
   *   A new array containing the css rules that should be used in the new
   *   theme.  These new rules may contain modified selectors, but everything
   *   else will be the same.
   */
  public function updateCustomizedCss($css) {
    $newCss = array();
    foreach ($css as $selector => $rules) {
      $newSelector = $this->updateSelector($selector);
      if (!empty($newSelector)) {
        $newCss[$newSelector] = $css[$selector];
      }
    }
    return $newCss;
  }

  /**
   * This method figures out the new selector that should replace the
   * specified selector when the theme is updated.
   *
   * @param $selector
   *   The original selector.
   * @return
   *   A string containing the new selector.
   */
  protected function updateSelector($selector) {
    $selector_parts = explode(' ', $selector);
    $newSelectorParts = array();
    foreach($selector_parts as $part) {
      if (array_key_exists($part, $this->selectorMap)) {
        $newPart = $this->selectorMap[$part];
        if (is_string($newPart) && !in_array($newPart, $newSelectorParts)) {
          $newSelectorParts[] = $newPart;
        }
      }
      else if (!in_array($part, $newSelectorParts)) {
        $newSelectorParts[] = $part;
      }
    }
    $newSelector = implode(' ', $newSelectorParts);
    $this->validateSelector($newSelector);
    return $newSelector;
  }

  /**
   * Validate the selector.  This method looks for duplicate ids in the same
   * selector, which would clearly represent a broken selector.  If a problem
   * is found, an exception is thrown.
   *
   * @param {String} $selector
   *   The selector.
   */
  public function validateSelector($selector) {
    $countMap = $this->buildSelectorCountMap($selector);
    $len = count($countMap);
    foreach ($countMap as $chunk => $count) {
      if ($this->selectorChunkIsId($chunk) && $count > 1) {
        // Found a problem.  Multiple ids.
        throw new Exception('Invalid selector detected.  Multiple instances of ' . $chunk . ' in selector ' . $selector);
      }
    }
  }

  /**
   * Creates a data structure that indicates how many instances of each part
   * of a selector exist.  This is useful for validating selectors.
   *
   * @param {String} $selector
   *   The selector.
   * @return
   *   An associative array that indicates how many instances of each piece of
   *   the selector occur in the specified selector.
   */
  private function buildSelectorCountMap($selector) {
    // Build a map that has the selector parts and the number of references.
    // Mostly interested in ids, because if there are multiple of the same ids
    // in the same selector it is clearly broken.
    $counts = array();
    $selectorParts = explode(' ', $selector);
    $partsLen = count($selectorParts);
    for ($index = 0; $index < $partsLen; $index++) {
      $selectors = explode(':', $selectorParts[$index]);
      for ($si = 0; $si < count($selectors); $si++) {
        $counts[$selectors[$si]] = (int)($selectors[$si]) + 1;
      }
    }
    return $counts;
  }

  /**
   * Returns true if the specified selector chunk represents an id.
   *
   * @param {String} $selectorChunk
   *   A part of a selector.
   * @return
   *   true if the chunk represents an id; false otherwise.
   */
  private function selectorChunkIsId($selectorChunk) {
    return substr($selectorChunk, 1) == '#';
  }

  /**
   * Updates the given palette info according to the palette map.
   *
   * @param <Array> $paletteInfo
   *   The palette section of the .info file array.
   * @return <Array>
   *   An updated version of the original palette info.
   */
  public function updatePaletteInfo($paletteInfo) {
    $newInfo = $paletteInfo;
    foreach ($this->paletteMap as $oldKey => $newKey) {
      $newInfo[$newKey] = $paletteInfo[$oldKey];
    }
    return $newInfo;
  }
}
