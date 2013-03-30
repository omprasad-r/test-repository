<?php

/**
 * @file
 * Colorset class.
 */

class Colorset {
  public $id;
  protected $colors;

  public function addColor($hex) {
    throw new ThemebuilderException('Subclass should implement addColor.');
  }

  /**
   * Convert a hex code into a palette index. If the color isn't in the palette,
   * return the original hex code.
   *
   * @param string $hex_code
   * @return string
   */
  public function hexToPaletteIndex($hex_code) {
    $palette_colors = $this->getColorArray();
    $palette_indexes = array_flip($palette_colors);
    $clean_hex = PaletteColor::formatHexCode($hex_code);
    if (isset($palette_indexes[$clean_hex])) {
      $index = $palette_indexes[$clean_hex];
      return "{" . $index . "}";
    }
    return $hex_code;
  }

  /**
   * Convert a palette index into a hex code.
   *
   * @param string $palette_index
   * @return string
   * @return boolean
   */
  public function paletteIndexToHex($palette_index) {
    $palette_colors = $this->getColorArray();
    $clean_index = PaletteColor::cleanIndex($palette_index);
    if (isset($palette_colors[$clean_index])) {
      return $palette_colors[$clean_index];
    }
    return false;
  }



  /**
   * Get an array of hex codes in this colorset, keyed by palette index.
   *
   * @return array
   */
  public function getColorArray() {
    $colors = array();
    if ($this->colors) {
      $color_objects = $this->colors;
      foreach ($color_objects as $key => $palette_color) {
        $colors[$key] = $palette_color->hex;
      }
    }
    return $colors;
  }

  /**
   * Return the type of colorset this object represents. Used in the .info file.
   *
   * @return string
   *   A string representing the colorset type (palette or custom).
   */
  public function getType() {
    $class = get_class($this);
    $type = FALSE;
    switch ($class) {
      case 'Palette':
        $type = 'palette';
        break;
      case 'CustomColorset':
        $type = 'custom';
        break;
    }
    return $type;
  }

  /**
   * Return an array suitable for including in the .info file.
   *
   * @return array
   *   An array representing this object, to include in the .info file.
   */
  public function getInfoArray() {
    throw new ThemebuilderException("Subclass should implement getInfoArray().");
  }

}

