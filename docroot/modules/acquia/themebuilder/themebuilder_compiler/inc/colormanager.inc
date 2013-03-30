<?php
// $Id:$

/**
 * @file
 * ColorManager class.
 */

/**
 * The ColorManager for a theme is the public interface for all color operations.
 */
class ColorManager {
  const PALETTE_INDEX = 'PALETTE_INDEX';
  const CUSTOM_INDEX = 'CUSTOM_INDEX';

  public function __construct($arg = null) {
    if ($arg != null) {
      throw new Exception("ColorManager instantiated with argument.");
    }
  }

  public static function createFromTheme($theme) {
    themebuilder_log_trace_enter(__METHOD__);
    $colormanager = new ColorManager();
    $colormanager->theme = $theme;
    $colormanager->palette = Palette::CreateFromTheme($theme);
    $colormanager->customColorset = CustomColorset::createFromTheme($theme);
    themebuilder_log_trace_exit(__METHOD__);
    return $colormanager;
  }

  public static function createFromThemeInfo($theme_info) {
    $colormanager = new ColorManager();
    $colormanager->palette = Palette::createFromThemeInfo($theme_info);
    $colormanager->customColorset = CustomColorset::createFromThemeInfo($theme_info);
    return $colormanager;
  }

  public static function createFromPaletteInfo($palette_info) {
    $colormanager = new ColorManager();
    $colormanager->palette = Palette::createFromPaletteInfo($palette_info);
    $colormanager->customColorset = new CustomColorset();
    return $colormanager;
  }

  public function getPalettes() {
    return $this->palette->getAvailablePalettes();
  }

  public function getCustomColorArray() {
    return $this->customColorset->getColorArray();
  }

  /**
   * Determine whether a color is a palette index, a custom color index, or
   * neither.
   *
   * @param <string> $color
   * @return <boolean>
   */
  public function indexType($color) {
    $index = PaletteColor::cleanIndex($color);
    if (Palette::isPaletteIndex($index)) {
      return self::PALETTE_INDEX;
    }
    else if (CustomColorset::isCustomIndex($index)) {
      return self::CUSTOM_INDEX;
    }
    else {
      return FALSE;
    }
  }

  public function getPaletteId() {
    return $this->palette->id;
  }

  /**
   * Add a color to the given palette, or modify an existing color.
   *
   * @param string $paletteId
   *   The palette to which to add the new color. May be a pid from
   *   themebuilder_palettes, or 'custom' for the theme's custom palette.
   * @param <type> $state
   *   The new color, with index and hex properties.
   *
   */
  public function setColor($paletteId, $state) {
    $return = FALSE;
    if ($paletteId !== 'custom') {
      throw new ThemebuilderException("Adding colors to palettes not yet implemented.");
    }
    else {
      $hex = $state->hex;
      $index = $state->index;
      if (($hex || $hex === '000000')) {
        //$existing_index = $this->indexType();// fix this - it doesn't make any sense
        if (empty($existing_index)) {
          $this->customColorset->addColor($hex, $index);
          $return = $this->customColorset;
        } else {
          // This color already exists in the palette.
          throw new ThemebuilderException("Trying to add a custom color that already exists.");
        }
      }
      else {
        throw new ThemebuilderException("Tried to add an invalid hex code to the custom colorset.");
      }
    }
  return $return;
  }

  /**
   * Set the palette associated with this color manager.
   *
   * @param <mixed> $newPalette
   *   Either a Palette object, or a palette id.
   */
  public function setPalette($newPalette) {
    if (!is_a($newPalette, 'Palette')) {
      $newPalette = Palette::createFromPaletteId($newPalette);
    }
    if ($newPalette) {
      $this->palette = $newPalette;
    }
    else {
      throw new ThemebuilderException("Palette set failed.");
    }
  }

  public function paletteIndexToHex($index) {
    $paletteIndex = $this->palette->paletteIndexToHex($index);
    if ($paletteIndex !== FALSE) {
      return $paletteIndex;
    } else {
      $customIndex = $this->customColorset->paletteIndexToHex($index);
      if ($customIndex !== FALSE) {
        return $customIndex;
      }
    }
    return FALSE;
  }

  public function hexToPaletteIndex($hex) {
    $validHex = $this->palette->hexToPaletteIndex($hex);
    if ($validHex !== FALSE) {
      return $validHex;
    } else {
      $validHex = $this->customColorset->hexToPaletteIndex($hex);
      if ($validHex !== FALSE) {
        return $validHex;
      }
    }
    return FALSE;
  }

  /**
   * Safely add a hash to a hex code for output in CSS.
   *
   * @param string $hex
   *   A hex code, or 'transparent', with or without a #.
   * @return string
   *   A color suitable for inclusion in a CSS rule.
   */
  public function addHash($hex) {
    if ($hex === 'transparent' || strpos($hex, '#') === 0) {
      return $hex;
    }
    else {
      return '#' . $hex;
    }
  }

}

