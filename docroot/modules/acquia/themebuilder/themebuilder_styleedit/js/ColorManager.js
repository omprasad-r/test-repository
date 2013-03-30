/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

var ThemeBuilder = ThemeBuilder || {};

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * The ColorManager object is a single interface for dealing with colors.
 * Normally you wouldn't need to instantiate this class; rather, you would
 * interact with the ThemeBuilder.colorManager instance.
 * @class
 */
ThemeBuilder.styles.ColorManager = ThemeBuilder.initClass();

/**
 * The constructor for the ColorManager class.
 */
ThemeBuilder.styles.ColorManager.prototype.initialize = function () {
  ThemeBuilder.getApplicationInstance().addApplicationInitializer(ThemeBuilder.bind(this, this.setPaletteInfo));
  this.initialized = false;
  ThemeBuilder.addModificationHandler(ThemeBuilder.PaletteModification.TYPE, this);
  ThemeBuilder.addModificationHandler(ThemeBuilder.SwatchModification.TYPE, this);
};

ThemeBuilder.styles.ColorManager.prototype.setPaletteInfo = function (appData) {
  var info = appData.palette_info;
  var paletteId = info.current_palette;
  this.palette = new ThemeBuilder.styles.Palette(paletteId);
  this.custom = new ThemeBuilder.styles.CustomColorset(info.customColors);
  this.paletteIndexes = info.indexes;
  this.mainPaletteIndexes = info.mainIndexes;
  this._createPalettes(info.palettes);
  this.initialized = true;
};

ThemeBuilder.styles.ColorManager.prototype.isInitialized = function () {
  return this.initialized;
};

ThemeBuilder.styles.ColorManager.prototype._createPalettes = function (paletteInfo) {
  var palettes = [];
  var i, info;
  for (i in paletteInfo) {
    if (i) {
      info = paletteInfo[i];
      palettes[i] = new ThemeBuilder.styles.Palette(info.id);
    }
  }
  this.palettes = palettes;
};

/**
 * Looks up a palette or custom index and returns a hex code.
 */
ThemeBuilder.styles.ColorManager.prototype.hexToPaletteIndex = function (hex) {
  var index;
  hex = hex.replace(/^#/g, '');
  // Check the palette.
  index = this.palette.hexToPaletteIndex(hex);
  if (index === false) {
    index = this.custom.hexToPaletteIndex(hex);
  }
  return index;
};

ThemeBuilder.styles.ColorManager.prototype.paletteIndexToHex = function (paletteIndex) {
  var cleanIndex = paletteIndex.toString().replace(/\{|\}/g, "");
  var index = this.palette.paletteIndexToHex(cleanIndex);
  if (index) {
    return index;
  }
  else {
    index = this.custom.paletteIndexToHex(cleanIndex);
    if (index) {
      return index;
    }
  }
  // No matches. We have bogus input.
  return false;
};

/**
 * Applies the specified modification description to the client side only.
 * This allows the user to preview the modification without committing it
 * to the theme.
 *
 * @param {Object} state
 *   The modification description.  To get this value, you should pass in
 *   the result of Modification.getNewState() or Modification.getPriorState().
 * @param {Modification} modification
 *   The modification that represents the change in the current state that
 *   should be previewed.
 */
ThemeBuilder.styles.ColorManager.prototype.preview = function (state, modification) {
  switch (state.type) {
  case ThemeBuilder.PaletteModification.TYPE:
    this.palettePreview(state);
    break;
  case ThemeBuilder.SwatchModification.TYPE:
    this.swatchPreview(state);
    break;
  default:
    throw "Unexepected modification type: " + state.type;
  }
};

/**
 * Preview method for the PaletteModification class.
 */
ThemeBuilder.styles.ColorManager.prototype.palettePreview = function (state) {
  var newPalette = new ThemeBuilder.styles.Palette(state.paletteId);
  ThemeBuilder.styles.Stylesheet.getInstance('palette.css').replacePalette(this.palette, newPalette);
  // There may be copies of color-related properties in border.css as well. See
  // AN-12796.
  ThemeBuilder.styles.Stylesheet.getInstance('border.css').replacePalette(this.palette, newPalette);
  this.palette = new ThemeBuilder.styles.Palette(state.paletteId);
  jQuery('#themebuilder-style').trigger({type: 'paletteChange', paletteId: state.paletteId});
};

/**
 * Preview method for the SwatchModification class.
 */
ThemeBuilder.styles.ColorManager.prototype.swatchPreview = function (state) {
  // Determine whether the swatch is palette or custom.
  var palette = this[state.selector];
  if (palette) {
    var existingHex = this.paletteIndexToHex(state.index);
    if (!existingHex) {
      if (state.hex) {
        // Add a new color to the palette.
        palette.addColor(state.index, state.hex);
      } else {
        // Remove a color from the palette.
        palette.removeColor(state.index);
      }
    } else {
      // This color already exists in the palette. Not sure what the desired
      // behavior is.
    }
    // Let everyone know there's been a swatch change.
    jQuery('#themebuilder-style').trigger({type: 'swatchPreview', 'index': state.index});
  }
};

/**
 * Remove {} from a palette index.
 */
ThemeBuilder.styles.ColorManager.prototype.cleanIndex = function (paletteIndex) {
  return paletteIndex.toString().replace(new RegExp('^{'), '').replace(new RegExp('}$'), '');
};

/**
 * Remove # from a hex code.
 */
ThemeBuilder.styles.ColorManager.prototype.cleanHex = function (hex) {
  return ThemeBuilder.styles.PaletteColor.cleanHex(hex);
};

/**
 * Check a hex code for validity.
 */
ThemeBuilder.styles.ColorManager.prototype.isHex = function (hex) {
  return ThemeBuilder.styles.PaletteColor.isHex(hex);
};

/**
 * Determine if a given index represents a palette or custom color.
 */
ThemeBuilder.styles.ColorManager.prototype.isValidIndex = function (palette_index) {
  var indexes = this.getIndexes('palette');
  var i;
  for (i in indexes) {
    if (indexes[i] === palette_index) {
      return 'palette';
    }
  }
  var custom = this.getIndexes('custom');
  for (i in custom) {
    if (parseInt(custom[i], 10) === parseInt(palette_index, 10)) {
      return 'custom';
    }
  }
  return false;
};

ThemeBuilder.styles.ColorManager.prototype.getPalette = function () {
  return this.palette;
};

ThemeBuilder.styles.ColorManager.prototype.getPalettes = function () {
  return this.palettes;
};

ThemeBuilder.styles.ColorManager.prototype.getIndexes = function (indexType) {
  if (!this.palette) {
    var data = ThemeBuilder.getApplicationInstance().getData();
    this.setPaletteInfo(data);
  }
  if (indexType === 'palette') {
    return this.palette.getIndexes();
  }
  else if (indexType === 'palette main') {
    return this.palette.getMainIndexes();
  }
  else if (indexType === 'custom') {
    return this.custom.getIndexes();
  }
  else {
    var paletteIndexes = this.palette.getIndexes();
    var customIndexes = this.custom.getIndexes();
    return paletteIndexes.concat(customIndexes);
  }
};

ThemeBuilder.styles.ColorManager.prototype.getCustom = function () {
  return this.custom;
};

ThemeBuilder.styles.ColorManager.prototype.getNextCustomIndex = function () {
  var indexes = this.getIndexes('custom');
  var highestIndex = indexes.last() || 0;
  return parseInt(highestIndex, 10) + 1;
};

/**
 * Safely add a hash to a hex code for output in CSS.
 *
 * @param {string} hex
 *   A hex code, or 'transparent', with or without a #.
 * @return {string}
 *   A color suitable for inclusion in a CSS rule.
 */
ThemeBuilder.styles.ColorManager.prototype.addHash = function (hex) {
  if (hex === 'transparent') {
    return hex;
  }
  else if (hex.indexOf('#') === 0) {
    return hex;
  }
  else {
    return '#' + hex;
  }
};





/**
 * A single instance of ThemeBuilder.styles.ColorManager.
 */
ThemeBuilder.getColorManager = function () {
  ThemeBuilder._colorManager = ThemeBuilder._colorManager || new ThemeBuilder.styles.ColorManager();
  return ThemeBuilder._colorManager;
};
