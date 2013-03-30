<?php

/**
 * @file
 * CustomColorset class.
 */

class CustomColorset extends Colorset {
  public $theme;

  public function load() {
    
  }

  public static function createFromTheme($theme) {
    themebuilder_log_trace_enter(__METHOD__);
    $themes_info = themebuilder_compiler_get_theme_data();
    $theme_name = $theme->getName();
    $theme_info = $themes_info[$theme_name];
    $result = CustomColorset::createFromThemeInfo($theme_info->info);
    themebuilder_log_trace_exit(__METHOD__);
    return $result;
  }

  public static function createFromThemeInfo($info) {
    themebuilder_log_trace_enter(__METHOD__);
    $result = new CustomColorset();
    // TODO: Consider parsing $theme->baseStylesheets to get existing colors.
    if (isset($info['custom'])) {
      $palette_info = $info['custom'];
      foreach ($palette_info as $index => $color) {
        $result->colors[$index] = new PaletteColor($result, '', $color['hex'], $index);
      }
    }
    themebuilder_log_trace_exit(__METHOD__);
    return $result;
  }

  /**
   * Add a new custom color to the theme.
   *
   * The caller is responsible for making sure this color doesn't already exist.
   * (TODO: Is this true?)
   *
   * @param <string> $hex
   *   The hex code of the new custom color.
   */
  public function addColor($hex, $index = NULL) {
    if (is_null($index)) {
      // Determine the new color's index.
      $highestIndex = max(array_keys($this->colors));
      $index = $highestIndex + 1;
    }
    if (isset($this->colors[$index])) {
      // This should not happen. If it does, something's likely wrong on the
      // client side; it's responsible for requesting a safe custom color index.
      throw new ThemebuilderException("The custom color index $index already exists.");
    }
    $this->colors[$index] = new PaletteColor($this, '', $hex, $index);
  }

  /**
   * Determine whether an index represents a custom color.
   *
   * @param <string> $index
   *   The index, without enclosing braces.
   * @return <boolean>
   */
  public static function isCustomIndex($index) {
    if ((string) $index === (string) (int) $index) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

  /**
   * Return an array suitable for including in the .info file.
   *
   * @return array
   *   An array representing this object, to include in the .info file.
   */
  public function getInfoArray() {
    $colors = $this->getColorArray();
    $infoArray = array();
    foreach ($this->colors as $color) {
      $infoArray[$color->index]['hex'] = $color->hex;
    }
    return $infoArray;
  }


}

