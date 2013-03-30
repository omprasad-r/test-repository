<?php
// $Id$

/**
 * @file
 * Palette-related classes.
 */

require_once('lib/csstidy/class.csstidy.php');

/**
 * Palette class
 */
class Palette extends Colorset {
  private $name;
  private $declared_in_theme = '';
  private static $palettes = NULL;
  private static $palette_name_map = array();

  /**
   * The indexes of the main colors in a palette.
   */
  public static $main_indexes = array('a', 'b', 'c', 'd', 'e');

  /**
   * An array of supplemental colors in a palette.
   *
   * If the palette defines colors for any of the keys in this array, that
   * color will be used rather than the standard definition supplied here.
   */
  public static $supplemental_colors = array(
    'black' => '000000',
    'white' => 'FFFFFF',
    'transparent' => 'transparent',
  );

  /**
   * An array of multipliers used when calculating tints.
   *
   * The keys can be either "white" or "black" (technically, any of the main
   * indexes or supplemental colors above, but white and black are the useful
   * ones), which represent the color that will be mixed with each palette
   * color when making a tint. The values are arrays representing the
   * multipliers used in the mixing.
   */
  public static $tint_multipliers = array('white' => array('0.33', '0.67'));

  /**
   * Prevent a Palette from being instantiated by external code.  It should
   * instead be created via one of the "createFrom" methods.
   */
  private function __construct() {
  }

  /**
   * Creates a Palette instance from the specified theme.  The palette colors
   * will be loaded from the theme's .info file.
   *
   * @param {Theme} $theme
   *   The theme for which the Palette should be instantiated.  The Palette
   *   will contain the colors associated with the theme.
   *
   * @return
   *   A new Palette instance with colors loaded from the theme's .info file.
   */
  public static function createFromTheme($theme) {
    themebuilder_log_trace_enter(__METHOD__);

    $theme_name = $theme->getName();
    // Read the theme's info file and get the palette information from it.
    // NOTE: This is a Drupal API function. If we're not going to rely on it,
    // the $theme object needs to parse its own .info file and make that
    // information available.
    // $theme->assets['info'] is not parsed.
    timer_start(__METHOD__ . '_read');
    $theme_info = themebuilder_compiler_load_theme_info($theme_name);
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_read'));

    if (!isset($theme_info['palette'])) {
      // If there's no palette information for this theme, load the palette
      // information from builderbase.
      if (Palette::getPaletteByName($theme_name)) {
        return Palette::getPaletteByName($theme_name);
      }
      return Palette::getPaletteByName('Minima');
    }

    $palette = Palette::createFromThemeInfo($theme_info);
    $palette->declared_in_theme = $theme_name;
    themebuilder_log_trace_exit(__METHOD__);

    return $palette;
  }

  /**
   * Creates a Palette instance from the specified palette id.  This id
   * matches the themebuilder_palettes table's pid field, which is unique.
   * The palette data will simply be loaded from the database.
   *
   * @param {int} $palette_id
   *   The palette id.
   *
   * @return
   *   A new Palette instance with colors loaded from the database.
   */
  public static function createFromPaletteId($palette_id) {
    themebuilder_log_trace_enter(__METHOD__);
    $palettes = Palette::getAvailablePalettes();
    themebuilder_log_trace_exit(__METHOD__);
    if (isset($palettes[$palette_id])) {
      return $palettes[$palette_id];
    }
    return NULL;
  }
  
  /**
   * Creates a palette from the full contents of a theme's .info file.
   *
   * @param {array} $theme_info
   *   An array containing the full .info file of a theme.
   *
   * @return
   *   A new Palette instance containing the associated colors.
   */
  public static function createFromThemeInfo($theme_info) {
    themebuilder_log_trace_enter(__METHOD__);
    $palette_info = $theme_info['palette'];
    $palette = Palette::createFromPaletteInfo($palette_info);
    $palette->declared_in_theme = $theme_info['name'];
    themebuilder_log_trace_exit(__METHOD__);
    return $palette;
  }

  /**
   * Create a full palette based on an array of palette information (usually
   * provided from a theme's .info file).
   *
   * The palette must define the main, official palette colors (currently "a"
   * through "e") and can optionally define the allowed supplemental colors
   * (e.g., black and white). If other colors are defined, we ignore them.
   *
   * Tints of each of the above colors are automatically created.
   *
   * @param <array> $palette_info
   *   The "palette" section of the theme's .info file.
   *
   * @return
   *   A new Palette instance popuplated with the colors in the specified
   *   array.
   */
  public static function createFromPaletteInfo($palette_info) {
    themebuilder_log_trace_enter(__METHOD__);
    // Set up the primary palette colors.
    $palette = new Palette();
    $palette->name = $palette_info['name'];
    
    // Get the id of the palette.
    if (isset($palette_info['id'])) {
      $palette->id = $palette_info['id'];
    }
    else {
      // If the palette doesn't supply one, look it up by name.
      $found_pal = Palette::getPaletteByName($palette->name);
      if (isset($found_pal->id)) {
        $palette->id = $found_pal->id;
      }
      // If it can't be found by name either, then something is definitely wrong.
      else {
        throw new Exception(t("The selected palette '%name' does not have a valid id.", array('%name' => $palette->name)));
      }
    }
    
    $palette->author = $palette_info['author'];
    $palette->colors = array();
    timer_start(__METHOD__ . '_for1');
    foreach (Palette::$main_indexes as $index) {
      if (isset($palette_info[$index])) {
        $color_info = $palette_info[$index];
        $color_info['name'] = isset($color_info['name']) ? $color_info['name'] : '';
        $palette->colors[$index] = new PaletteColor($palette, $color_info['name'], $color_info['hex'], $index);
      }
    }
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_for1'));

    // Set up the supplemental colors. These can optionally be defined in the
    // input palette array, but don't have to be.
    timer_start(__METHOD__ . '_for2');
    foreach (Palette::$supplemental_colors as $index => $hex) {
      if (isset($palette_info[$index])) {
        // Use the theme override, if there is one.
        $color_info = $palette_info[$index];
        $color_info['name'] = isset($color_info['name']) ? $color_info['name'] : '';
        $palette->colors[$index] = new PaletteColor($palette, $color_info['name'], $color_info['hex'], $index);
      }
      else {
        // Otherwise use the standard values for this color.
        $palette->colors[$index] = new PaletteColor($palette, ucwords($index), $hex, $index);
      }
    }
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_for2'));

    // Now add tints of the above colors.
    timer_start(__METHOD__ . '_tints');
    $palette->colors += $palette->calculateTints($palette->colors);
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_tints'));
    // Make sure the palette exists in the database and that we have a valid
    // palette id.
    themebuilder_log_trace_exit(__METHOD__);
    
    Palette::$palettes[$palette->id] = $palette;
    Palette::$palette_name_map[$palette->name] = $palette->id;
    return $palette;
  }

  /**
   * Get palettes by name.
   * 
   * This should only be needed for themes that don't have UUIDs (themes
   * produced before sprint 38,) or if we attempt to load a palette from
   * a theme that does actually contain a palette.
   */
  public function getPaletteByName($name) {
    if (empty(Palette::$palettes)) {
      Palette::getAvailablePalettes();
    }
    
    if (isset(Palette::$palette_name_map[$name])) {
      $id = Palette::$palette_name_map[$name];
      if (isset(Palette::$palettes[$id])) {
        return Palette::$palettes[$id];
      }
    };
    return NULL;
  }

  /**
   * Calculate tints for an array of palette colors.
   *
   * @param $colors
   *   An array of color objects whose tints will be calculated.
   * @return
   *   An array of color objects representing the tints.
   */
  public function calculateTints($colors) {
    return $this->_getTints($colors);
  }

  /**
   * Calculate the expected tint indexes for an array of palette indexes.
   */
  private function getTintIndexes($palette_indexes) {
    return self::_getTints(array_flip($palette_indexes), TRUE);
  }

  /**
   * Returns either the actual tints for an array of palette colors, or their
   * expected indexes.
   *
   * TODO: This is ugly, but better than code duplication...
   */
  private function _getTints($colors, $indexes_only = FALSE) {
    $tints = array();
    $color_indexes = array_keys($colors);
    foreach (self::$tint_multipliers as $mix_color => $multipliers) {
      // We require that the mix color be defined.
      if (isset($colors[$mix_color])) {
        foreach ($multipliers as $multiplier) {
          foreach ($color_indexes as $palette_color) {
            // Do not mix a color with itself. Also, do not mix "transparent",
            // since it isn't really a color.
            if ($mix_color != $palette_color && $palette_color != 'transparent') {
              $percentage = 100 * $multiplier;
              $index = self::getTintIndex($palette_color, $percentage, $mix_color);
              if ($indexes_only) {
                $tints[] = $index;
              }
              else {
                $first_color = $colors[$palette_color]->hex;
                $second_color = $colors[$mix_color]->hex;
                $hex = PaletteColor::mix($first_color, $second_color, $multiplier);
                $tints[$index] = new PaletteColor($this, $index, $hex, $index);
              }
            }
          }
        }
      }
    }
    return $tints;
  }

  /**
   * Returns the (CSS-friendly) index that is used to represent a tint.
   *
   * The index is of the form 'b-33-white', when palette color b is mixed with
   * 33% of white.
   */
  public static function getTintIndex($palette_color, $percentage, $mix_color) {
    return implode('-', array($palette_color, $percentage, $mix_color));
  }

  /**
   * Returns all indexes in a palette (including tints).
   */
  public static function getIndexes() {
    // First define the colors that will have their tints calculated, then add
    // their expected tints.
    $indexes = array_merge(self::$main_indexes, array_keys(self::$supplemental_colors));
    $indexes = array_merge($indexes, self::getTintIndexes($indexes));
    return $indexes;
  }

  /**
   * Returns the main color indexes in a palette (i.e., the primary palette
   * colors).
   */
  public static function getMainIndexes() {
    return self::$main_indexes;
  }

  public function __get($name) {
    return $this->$name;
  }

  /**
   * Get an array of hex codes in this palette, keyed by palette index.
   *
   * @return array
   */
  public function getColorArray() {
    $colors = array();
    $color_objects = $this->colors;
    foreach ($color_objects as $key => $palette_color) {
      $colors[$key] = $palette_color->hex;
    }
    return $colors;
  }

  public function getBaseColorArray() {
    $colors = array();
    $allColors = $this->getColorArray();
    $indexes = Palette::getMainIndexes();
    foreach ($allColors as $index => $color) {
      if (in_array($index, $indexes)) {
        $colors[$index] = $color;
      }
    }
    return $colors;
  }

  /**
   * Returns all palettes currently available to the themebuilder.
   */
  public static function getAvailablePalettes() {
    themebuilder_log_trace_enter(__METHOD__);
    timer_start(__METHOD__ . '_load1');
    if (Palette::$palettes == NULL) {
      Palette::$palettes = array();
      $palettes = themebuilder_compiler_get_palette_info();
      foreach ($palettes as $index => $value) {
        $value['name'] = $index;
        Palette::createFromPaletteInfo($value);
      }
    }
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_load1'));
    timer_start(__METHOD__ . '_load2');

    // Now load the current theme.    
    $theme = themebuilder_compiler_get_current_theme();
    Palette::createFromTheme($theme);
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_load1'));
    themebuilder_log_trace_exit(__METHOD__);

    return Palette::$palettes;
  }

  /**
   * Determine whether an index represents a palette color.
   *
   * @param <string> $index
   *   The index, without enclosing braces.
   * @return <boolean>
   */
  public static function isPaletteIndex($index) {
    $indexes = self::getIndexes();
    if (in_array($index, $indexes)) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

  public function addColor($hex) {
    throw new ThemebuilderException("Adding colors to a palette is not yet supported.");
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
    $infoArray['id'] = $this->id;
    $infoArray['name'] = $this->name;
    $infoArray['author'] = $this->author;

    $color_names = array('white', 'black', 'a', 'b', 'c', 'd', 'e');
    foreach ($color_names as $name) {
      $infoArray[$name]['hex'] = $this->colors[$name]->hex;
      $infoArray[$name]['name'] = $this->colors[$name]->name;
    }
    return $infoArray;
  }
}

/**
 * PaletteColor class
 */
class PaletteColor {

  private $name;
  private $hex;
  private $index;
  private $palette;

  public function __construct($palette, $name, $hex, $index) {
    $this->palette = $palette;
    $this->name = $name;
    $this->index = $index;
    $this->hex = self::formatHexCode($hex);
  }

  public function __get($name) {
    return $this->$name;
  }

  /**
   * Clean up a hex color code.
   *
   * @param string $code
   * @return string
   *   The formatted hex code, without a leading "#".
   */
  public static function formatHexCode($unformatted_code) {
    if ($unformatted_code === 'transparent') {
      return 'transparent';
    } else {
      $code = trim($unformatted_code);
      $code = str_replace('#', '', $code);

      // Handle triple hex codes from CSS.
      if (strlen($code) == 3) {
        // RGB gets expanded to RRGGBB.
        $letters = str_split($code);
        $code = $letters[0] . $letters[0] . $letters[1] . $letters[1] . $letters[2] . $letters[2];
      }

      // Validate the hex code. Invalid colors will end up as white.
      $decimal_code = abs(hexdec($code));
      if ($decimal_code > 16777215) { // 16777215 = FFFFFF
        $decimal_code = 0;
      }
      $code = sprintf("%06x", $decimal_code);

      return $code;
    }
  }

  /**
   * Mix two colors.
   *
   * @param $hex1
   *   A string representing the hex value of the first color.
   * @param $hex2
   *   A string representing the hex value of the second color.
   * @param $multiplier
   *   The multiplier which is used to mix the colors. The mix is calculated
   *   separately for red, green, and blue, and in each case, simply represents
   *   the fractional distance that one "travels" between the first and second
   *   colors to get the R, G, or B value of the return color.
   * @return
   *   A string representing the hex value of the mixed color.
   */
  public static function mix($hex1, $hex2, $multiplier = 0.5) {
    // Separate out red, green, and blue.
    $rgb1 = self::splitRGB($hex1);
    $rgb2 = self::splitRGB($hex2);

    // Fancy (or not-so-fancy) color blender math.
    $mixed_rgb = array();
    foreach (array_keys($rgb1) as $rgb_index) {
      $color1 = hexdec($rgb1[$rgb_index]);
      $color2 = hexdec($rgb2[$rgb_index]);
      $mixed_rgb[] = $color1 * (1 - $multiplier) + $color2 * $multiplier;
    }

    // Convert back to hex.
    $hex = '';
    foreach ($mixed_rgb as $decimal_value) {
      $hex .= dechex(round($decimal_value));
    }
    return self::formatHexCode($hex);
  }

  public static function splitRGB($hex_code) {
    $hex_code = self::formatHexCode($hex_code);
    $parts = str_split($hex_code, 2);
    return $parts;
  }

  public static function colorType($color) {
    if (strpos($color, '{') === 0) {
      return 'index';
    }
    if (strpos($color, '#') === 0) {
      return 'hex';
    }
    return false;
  }

  public static function cleanIndex($color) {
    $index = trim($color, " {}");
    return $index;
  }

}

/**
 * An asset representing the palette.css file.
 */
class PaletteCssAsset extends CssAsset {

  private $palette;
  private $tidy;
  public $asset_name;
  public $theme;
  public $colorManager;

  public function __construct($theme) {
    parent::__construct($theme);
    $this->asset_name = 'palette.css';
    $this->palette = Palette::createFromTheme($theme);
    $this->theme = $theme;
    $this->colorManager = $theme->getColorManager();
  }

  public function __get($name) {
    return $this->$name;
  }

  /**
   * Set the palette for this asset.
   *
   * @param <mixed> $palette
   *   Either a Palette object, or a palette ID.
   */
  public function setPalette($palette) {
    themebuilder_log_trace_enter(__METHOD__);
    if (is_a($palette, 'Palette')) {
      $new_palette = $palette;
    }
    else {
      $new_palette = Palette::createFromPaletteId($palette);
    }
    if ($new_palette) {
      $this->palette = $new_palette;
    }
    themebuilder_log_trace_exit(__METHOD__);
    return $new_palette;
  }

  /**
   * Converts a color to a palette index string.
   *
   * @param {string} $value
   *   A string containing the color value.
   * @param {ColorManager} $colorManager
   *   (Optional) The ColorManager instance configured with the palette.  If
   *   not specified, the ColorManager associated with this Palette instance
   *   will be used.
   */
  public function colorToIndex($value, $colorManager = null) {
    if (!$colorManager) {
      $colorManager = $this->colorManager;
    }
    // If the passed-in $value is a palette index, just save it.
    if ($colorManager->indexType($value)) {
      return $value;
    }
    // The passed-in $value was not a palette index. See if it's a hex code that
    // maps to a palette index.
    else {
      $palette_value = $colorManager->hexToPaletteIndex($value);
      // If we didn't manage to match this color up with a palette index, it
      // probably means the theme designer used a color outside the defined
      // palette, and we've run into it while parsing style.css.
      // For now, just ignore the error; that color won't react to palette
      // switching, and that's okay.
      if ($palette_value == $value) {
        return null;
      }
      return $palette_value;
    }
  }

  public function setProperty($selector, $property_name, $value, $generated = self::CUSTOM) {
    themebuilder_log_trace_enter(__METHOD__);
    // If the passed-in $value is a palette index, just save it.
    if ($this->colorManager->indexType($value)) {
      parent::setProperty($selector, $property_name, $value, $generated);
    }
    // The passed-in $value was not a palette index. See if it's a hex code that
    // maps to a palette index.
    else {
      $palette_value = $this->colorManager->hexToPaletteIndex($value);
      // If we didn't manage to match this color up with a palette index, it
      // probably means the theme designer used a color outside the defined
      // palette, and we've run into it while parsing style.css.
      // For now, just ignore the error; that color won't react to palette
      // switching, and that's okay.
      if ($palette_value == $value) {
        // throw new ThemeBuilderException('PaletteCssAsset only handles palette colors.');
      }
      // If we did get a palette index out of the hex code, save the index.
      parent::setProperty($selector, $property_name, $palette_value, $generated);
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  public function renderString() {
    themebuilder_log_trace_enter(__METHOD__);
    $propertiesRendered = 0;
    $white = $this->colorManager->addHash($this->colorManager->paletteIndexToHex('{white}'));
    $black = $this->colorManager->addHash($this->colorManager->paletteIndexToHex('{black}'));
    $css = $this->getSortedProperties();
    $output = '';
    foreach ($css as $type => $section) {
      $output .= "\n";
      foreach ($section as $rule => $properties) {
        $output .= $rule ." {\n";
        foreach ($properties as $name => $value) {
          // Only write to palette.css if the property is a palette index with {}.
          if ($this->colorManager->indexType($value)) {
            // Translate the palette index to a hex code.
            $hex_code = $this->colorManager->paletteIndexToHex($value);
            $hex_code = $this->colorManager->addHash($hex_code);
            $output .= '  ' .$name .': ' .$hex_code .";\n";
            if ($hex_code !== $white && $hex_code !== $black) {
              // Here we have written an interesting color, so we are likely
              // writing a valid palette.css file.  Failure to write any
              // interesting colors most likely means that the themebuilder
              // palette code has been broken.
              $propertiesRendered++;
            }
          }
        }
        $output .= "}\n\n";
      }
    }
    if ($propertiesRendered === 0) {
      // No non-white or non-black colors were written into palette.css.  It
      // is likely the palette has been corrupted.
      $theme = themebuilder_compiler_get_current_theme();
      $theme_version = themebuilder_compiler_get_theme_version($theme->getName());
      ThemeBuilderLog()->gardensError('AN-19976 - Rendered a palette.css file with no colors.', 'Theme is ' . $theme->getBaseThemeName() . ' version ' . $theme_version);
    }
    themebuilder_log_trace_exit(__METHOD__);
    return $output;
  }

  public function load() {
    themebuilder_log_trace_enter(__METHOD__);
    timer_start(__METHOD__ . '_query');
    $exists_in_db = db_query('SELECT 1 FROM {themebuilder_css} WHERE aid = :aid', array(':aid' => $this->aid))->fetchField();
    themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_query'));
    if (!$exists_in_db) {
      $this->loadProperties();
    }
    else {
      parent::load();
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Set several properties at once.
   * 
   * @param {array} $properties
   *   An associative array of CSS properties that belong in palette.css, in
   *   the same format as self::$properties.
   */
  private function setProperties($properties) {
    themebuilder_log_trace_enter(__METHOD__);
    foreach ($properties as $generated => $css) {
      foreach ($css as $selector => $rules) {
        foreach ($rules as $property_name => $value) {
          $this->setProperty($selector, $property_name, $value, $generated);
        }
      }
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Checks the theme's stylesheets for palette-related properties.
   */
  public function loadProperties() {
    themebuilder_log_trace_enter(__METHOD__);
    $stylesheets = $this->theme->getStylesheets();
    foreach ($stylesheets as $stylesheet) {
      // Determine the stylesheet's path, relative to the Drupal root.
      $filename = $this->theme->getPath() . "/" . $stylesheet;
      if (file_exists($filename)) {
        // Get a cached version of its palette-related properties, if available.
        $generatedProperties = $this->getPaletteCache($filename);
        if (!$generatedProperties) {
          // If there's no cache of the palette properties, parse the stylesheet
          // and grab any CSS properties with colors.
          $generatedProperties = $this->parse($filename);
          $this->setPaletteCache($filename, $generatedProperties);
        }
        // Save the palette-related properties to the asset object.
        $this->setProperties(array(self::GENERATED => $generatedProperties));
      }
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  /**
   * Get the cached version of a stylesheet's palette properties, if it exists.
   *
   * @param {string} $filename
   * @return {mixed}
   *   An associative array of the cached properties, or FALSE if no cached
   *   properties were found.
   */
  function getPaletteCache($filename) {
    themebuilder_log_trace_enter(__METHOD__);
    // Determine if we have a cached version of this stylesheet's color-related
    // properties, in any location where they are allowed to reside.
    $cachefiles = $this->getCacheFileNames($filename);
    foreach ($cachefiles as $cachefile) {
      if (file_exists($cachefile)) {
        // Load the palette cache from the file.
        $properties = unserialize(file_get_contents($cachefile));
        if (is_array($properties)) {
          themebuilder_log_trace_exit(__METHOD__);
          return $properties;
        }
      }
    }
    // Either no cache files exist, or none of them contained a correctly
    // serialized array.
    themebuilder_log_trace_exit(__METHOD__);
    return FALSE;
  }

  /**
   * Save a cached version of a stylesheet's palette properties.
   *
   * @param {string} $filename
   *   The path to a file (must be known to exist).
   * @param {array} $properties
   *   An associative array to be saved to the cache.
   */
  private function setPaletteCache($filename, $properties) {
    themebuilder_log_trace_enter(__METHOD__);
    $cachefiles = $this->getCacheFileNames($filename);
    foreach ($cachefiles as $cachefile) {
      // Write to the first potential cache file that we're able to.
      if (is_writable(dirname($cachefile))) {
        $result = file_put_contents($cachefile, serialize($properties));
        themebuilder_log_trace_exit(__METHOD__);
        return $result;
      }
      // We always hope we can write to the preferred cache location, so log a
      // warning whenever we can't.
      ThemeBuilderLog()->warning("A potential palette cache file location ($cachefile) is not writable.");
    }
    // None of the potential cache files are writable, which means this site
    // will experience unnecessary slowdowns.
    $all_cachefiles = implode(' and ', $cachefiles);
    ThemeBuilderLog()->gardensError('No Jira ID - No palette cache file locations are writable', "(tried $all_cachefiles).");
    themebuilder_log_trace_exit(__METHOD__);
    return FALSE;
  }

  /**
   * Given a stylesheet filename, return an array of possible cache file names
   * associated with it, in the preferred order of their use.
   *
   * Since each potential cache filename is determined from a unique hash of
   * the contents of the passed-in stylesheet file, any of them are OK to use,
   * but we return them in a preferred order for performance reasons on
   * multisite installations.
   *
   * @param {string} $filename
   *   The stylesheet filename.
   * @return {array}
   *   An array of names of possible associated cache files.
   */
  private function getCacheFileNames($filename) {
    $checksum = md5_file($filename);
    $cachefile_basename = basename($filename) . '.palette.' . $checksum;
    return array(
      // If there is a top-level files directory in this Drupal installation,
      // we always want to use that - this is shared between multisites, and
      // therefore only one of them needs to take the performance hit of
      // generating the cache.
      'files/' . $cachefile_basename,
      // Otherwise, try to write into the theme's directory itself.
      // THIS HAS BEEN REMOVED. We used to do this, but there's no point, since
      // the 'files' directory will always be available on production machines.
      // So all this really resulted in was extra files getting randomly
      // written on local development setups, where under some conditions (such
      // as command line installs), file permissions are such that this code
      // could result in base themes within the themes/acquia directory getting
      // written to.
      // dirname($filename) . '/' . $cachefile_basename,
    );
  }

  /**
   * Get an instance of cssTidy for parsing stylesheets.
   *
   * @return {cssTidy}
   *   An instance of cssTidy.
   */
  private function getTidy() {
    if (!$this->tidy) {
      $this->tidy = new csstidy();
      $this->tidy->set_cfg('compress_colors', 0);
      $this->tidy->set_cfg('optimise_shorthands', 0);
      $this->tidy->set_cfg('merge_selectors', 0);
      $this->tidy->load_template('low_compression');
    }
    return $this->tidy;
  }

  /**
   * Parse a stylesheet and return all its palette-related properties.
   *
   * @param {string} $filename
   *   The stylesheet's filename.
   * @return {array}
   *   An associative array containing any palette-related properties from the
   *   stylesheet.
   */
  private function parse($filename) {
    themebuilder_log_trace_enter(__METHOD__);
    $tidy = $this->getTidy();
    $css = file_get_contents($filename);
    $tidy->parse($css);
    $properties = array();
    // Check each stylesheet for color-related rules.
    foreach ($tidy->css as $sheet) {
      foreach ($sheet as $selector => $rules) {
        foreach ($rules as $property_name => $value) {
          // If this rule has to do with color, create a property for it.
          if (in_array($property_name, $GLOBALS['csstidy']['color_values'])) {
            $properties[$selector][$property_name] = $value;
          }
        }
      }
    }
    themebuilder_log_trace_exit(__METHOD__);
    return $properties;
  }

  /**
   * Updates the css rules that use the palette with changes found in the new
   * base theme.  The use case is that you have a customized theme and the
   * original base theme was updated (presumably to fix bugs or make it more
   * robust).  This function updates the palette.css by replacing the part of
   * the file that was generated from the style.css with a new generated part
   * created from the new style.css.
   *
   * @param {Theme} $basetheme
   *   The base theme from which the palette.css file should be updated.
   * @param {ThemeUpdate array} $updateFilters
   *   The list of ThemeUpdate instances that should be applied to the custom
   *   theme to affect the desired update.
   */
  public function updatePalette($basetheme, $updateFilters) {
    $CPC = $this->getCustomizedRules();
    foreach ($updateFilters as $filter) {
      if (method_exists($filter, 'updateCustomizedCss')) {
        $CPC = $filter->updateCustomizedCss($CPC);
      }
    }
    $BPG = $basetheme->getPaletteAsset()->getGeneratedRules();
    // Clear the properties associated with this asset.
    $this->properties = array(self::GENERATED => array(), self::CUSTOM => array());
    $this->setProperties(array(self::GENERATED => $BPG, self::CUSTOM => $CPC));
  }

  /**
   * Returns an array containing the custom css rules pertaining to palette
   * colors.  This set will not include the css rules that were generated from
   * the original style.css file.
   *
   * @return
   *   An array containing all of the customized CSS rules.  This array will
   *   have values in palette index form rather than actual colors.
   */
  private function getCustomizedRules() {
    // Get an array representing the version of palette.css that the theme
    // started out with, before any customizations. It has palette indexes,
    // and it's based on the original palette.
    $CPG = $this->getGeneratedRules();
    $parser = new StylesheetParser();
    // Get an array representing the customized palette.css.
    $properties = $parser->getProperties($this->theme->getPath(), array('palette.css'));
    // Convert the colors in the customized properties array to the indexes
    // corresponding to the palette appearing in the theme's .info file.
    $custom = $this->convertColorsToIndexes($properties);

    // Now create a result array that contains only the custom part.
    //
    //  The colors should evaluate the same in the customized palette
    //  properties as in the generated palette properties if no changes were
    //  made because it is using the palette index rather than the actual
    //  color.  Go through the custom theme generated rules (CPG) and remove
    //  matching rules from the custom ruleset to get only the rules added by
    //  user customization.
    foreach ($CPG as $selector => $rules) {
      foreach ($rules as $property_name => $value) {
        if (!empty($custom[$selector]) && $custom[$selector][$property_name]) {
          $customValue = $custom[$selector][$property_name];
          if ($customValue == $value) {
            // The color was not customized.  Remove this from the customized
            // array
            unset($custom[$selector][$property_name]);
            if (count($custom[$selector]) == 0) {
              // The last property was removed from the rule.  Remove the
              // entire rule.
              unset($custom[$selector]);
            }
          }
        }
      }
    }
    // Now go through CPC and identify any multi-selector rules for removal.
    // It is currently impossible to create a custom rule with multiple
    // selectors, so this condition indicates the rule was part of selector
    // aggregation that was done in the initial beta release of gardens.
    foreach ($custom as $selector => $rules) {
      if (strpos($selector, ',') !== false) {
        unset($custom[$selector]);
      }
    }
    // TODO: Verify whether this step is needed - $custom should already be in
    // index form.
    $result = $this->convertColorsToIndexes($custom);
    return $result;
  }

  /**
   * Returns an array containing the css rules pertaining to palette colors
   * that were generated from the original style.css file.  This is key
   * information for doing a theme update, in which we will replace the
   * generated rules in the palette.css file with the new generated rules from
   * the new version of the base theme.
   *
   * @return
   *   An array containing all of the generated CSS rules.  This array will
   *   have values in palette index form rather than actual colors.
   */
  private function getGeneratedRules() {
    $parser = new StylesheetParser();
    $properties = $parser->getProperties($this->theme->getPath(), $this->theme->getStylesheets());
    // Now get the palette info.  There is a base_palette name in the .info
    // file.
    $base_palette = $this->theme->getBasePalette();
    $colormanager = ColorManager::createFromPaletteInfo($base_palette);
    $result = $this->convertColorsToIndexes($properties, $colormanager);
    return $result;
  }

  /**
   * Converts all of the colors in the specified array from literal color
   * values to palette indexes.
   *
   * @param {Array} $css
   *   An array containing CSS rules.
   * @param {ColorManager} $colorManager
   *   (Optional) The ColorManager instance configured with the palette.  If
   *   not specified, the ColorManager associated with this Palette instance
   *   will be used.
   */
  function convertColorsToIndexes($css, $colorManager = null) {
    themebuilder_log_trace_enter(__METHOD__);
    $result = array();
    foreach ($css as $selector => $rules) {
      $generatedRule = array();
      foreach ($rules as $property_name => $value) {
        $index = $this->colorToIndex($value, $colorManager);
        if ($index) {
          $generatedRule[$property_name] = $index;
        }
      }
      if (count($generatedRule) > 0) {
        $result[$selector] = $generatedRule;
      }
    }
    themebuilder_log_trace_exit(__METHOD__);
    return $result;
  }
}

/**
 * This class is used for parsing stylesheets.  Specifically it finds rules
 * that contain property settings where the value is a color and provides an
 * array of such rules for processing.
 */
class StylesheetParser {

  public $tidy;

  /**
   * Returns an array containing the color related properties from the
   * specified stylesheets of the theme.  The colors will be in the format
   * they appear in the actual css files, not as palette indexes.
   *
   * @param {String} $path
   *   The path to the theme files.
   * @param {array} $stylesheets
   *   An array containing the names of the stylesheets that should be
   *   processed to get the color related properties.
   *
   * @return
   *   An array containing the selectors, properties, and values from the
   *   specified stylesheets.  Only the color related properties are included.
   */
  public function getProperties($path, $stylesheets) {
    $properties = array();
    foreach ($stylesheets as $stylesheet) {
      // Determine the stylesheet's path, relative to the Drupal root.
      $filename = $path . "/" . $stylesheet;
      if (file_exists($filename)) {
        // Can't use the cached version because the older cache files merged
        // the selectors, making the comparisons fail.
        $parsed_properties = $this->parse($filename);
        $properties = array_merge($properties, $parsed_properties);
      }
    }
    return $properties;
  }


  /**
   * Parse a stylesheet and return all its palette-related properties.
   *
   * @param {string} $filename
   *   The stylesheet's filename.
   * @return {array}
   *   An associative array containing any palette-related properties from the
   *   stylesheet.
   */
  private function parse($filename) {
    $tidy = $this->getTidy();
    $css = file_get_contents($filename);
    $tidy->parse($css);
    $properties = array();
    // Check each stylesheet for color-related rules.
    foreach ($tidy->css as $sheet) {
      foreach ($sheet as $selector => $rules) {
        foreach ($rules as $property_name => $value) {
          // If this rule has to do with color, create a property for it.
          if (in_array($property_name, $GLOBALS['csstidy']['color_values'])) {
            $properties[$selector][$property_name] = $value;
          }
        }
      }
    }
    return $properties;
  }

  /**
   * Get an instance of cssTidy for parsing stylesheets.
   *
   * @return {cssTidy}
   *   An instance of cssTidy.
   */
  private function getTidy() {
    if (!$this->tidy) {
      $this->tidy = new csstidy();
      $this->tidy->set_cfg('compress_colors', 0);
      $this->tidy->set_cfg('optimise_shorthands', 0);
      $this->tidy->set_cfg('merge_selectors', 0);
      $this->tidy->load_template('low_compression');
    }
    return $this->tidy;
  }
}  
