
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true */

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * An abstract class to represent a group of colors (either palette or custom).
 * @class
 */
ThemeBuilder.styles.Colorset = ThemeBuilder.initClass();
ThemeBuilder.styles.Colorset.prototype.initialize = function () {
};
ThemeBuilder.styles.Colorset.prototype.paletteIndexToHex = function (index) {
  if (this.colors[index]) {
    return this.colors[index].hex;
  }
  else {
    return false;
  }
};

ThemeBuilder.styles.Colorset.prototype.hexToPaletteIndex = function (hex) {
  // TODO: This is O(n). Add a lookup table.
  var i;
  for (i in this.colors) {
    if (this.colors[i].hex && this.colors[i].hex.toLowerCase() === hex.toLowerCase()) {
      return i;
    }
  }
  return false;
};

/**
 * Return a list of indexes for all colors in the palette.
 */
ThemeBuilder.styles.Colorset.prototype.getIndexes = function () {
  return this.getIndexesOfType('all');
};

/**
 * Return a list of indexes for the main colors in a palette.
 */
ThemeBuilder.styles.Colorset.prototype.getMainIndexes = function () {
  return this.getIndexesOfType('main');
};

/**
 * Helper function to return a list of indexes in a palette, for either all
 * palette colors or only the main palette colors (depending on how the
 * function is called).
 */
ThemeBuilder.styles.Colorset.prototype.getIndexesOfType = function (indexType) {
  if (indexType === 'main') {
    var colors = this.mainColors;
  }
  else if (indexType === 'all') {
    colors = this.colors;
  }
  else {
    throw 'Colorset getIndexesOfType function was called with an unexpected index type ' + indexType;
  }
  // TODO: Pretty sure this isn't safe. Revisit.
  var i;
  var indexes = [];
  for (i in colors) {
    if (ThemeBuilder.util.isNumeric(i) || typeof(i) === 'string') {
      indexes.push(i);
    }
  }
  return indexes;
};

ThemeBuilder.styles.Colorset.prototype.addColor = function (index, hex, name) {
  this.colors[index] = new ThemeBuilder.styles.PaletteColor(hex, name);
};

ThemeBuilder.styles.Colorset.prototype.removeColor = function (index) {
  throw 'Removing colors from palettes not fully implemented.';
  // TODO: Test to make sure delete works as expected.
  //delete this.colors[index];
};







/**
 * A class to represent a color palette.
 * @class
 * @extends ThemeBuilder.styles.Colorset
 */
ThemeBuilder.styles.Palette = ThemeBuilder.initClass();
ThemeBuilder.styles.Palette.prototype = new ThemeBuilder.styles.Colorset();

/**
 * Constructor.
 */
ThemeBuilder.styles.Palette.prototype.initialize = function (palette_id) {
  this.id = palette_id;
  var app = ThemeBuilder.getApplicationInstance();
  var data = app.getData();
  if (data) {
    this.colorDataLoaded(data);
  }
  else {
    app.addApplicationInitializer(ThemeBuilder.bind(this, this.colorDataLoaded));
  }
};

/**
 * Called when the color data has been loaded.  This occurs very early in the
 * initialization process through a request sent by the Application instance.
 *
 * @param data
 *   The application initialization data.
 */
ThemeBuilder.styles.Palette.prototype.colorDataLoaded = function (data) {
  // Store the complete list of palette colors (including tints).
  this.indexes = data.palette_info.indexes;
  this.colors = {};
  var palette = data.palette_info.palettes[this.id];
  var i;
  for (i = 0; i < this.indexes.length; i++) {
    var index = this.indexes[i];
    var hex = palette[index];
    this.colors[index] = new ThemeBuilder.styles.PaletteColor(hex, '');
  }
  // Store the list of main palette colors (i.e., the primary colors in the
  // palette).
  this.mainIndexes = data.palette_info.mainIndexes;
  this.mainColors = {};
  for (i = 0; i < this.mainIndexes.length; i++) {
    index = this.mainIndexes[i];
    hex = palette[index];
    this.mainColors[index] = new ThemeBuilder.styles.PaletteColor(hex, '');
  }
  // Store other palette information.
  this.name = palette.name;
};







/**
 * @class
 * @extends ThemeBuilder.styles.Colorset
 */
ThemeBuilder.styles.CustomColorset = ThemeBuilder.initClass();
ThemeBuilder.styles.CustomColorset.prototype = new ThemeBuilder.styles.Colorset();

ThemeBuilder.styles.CustomColorset.prototype.initialize = function (customColorInfo) {
  this.colors = {};
  var i;
  for (i in customColorInfo) {
    if (ThemeBuilder.util.isNumeric(i)) {
      this.colors[i] = new ThemeBuilder.styles.PaletteColor(customColorInfo[i], '');
    }
  }
};








/**
 * @class
 */
ThemeBuilder.styles.PaletteColor = ThemeBuilder.initClass();

ThemeBuilder.styles.PaletteColor.prototype.initialize = function (hex, name) {
  var cleanHex = ThemeBuilder.styles.PaletteColor.cleanHex(hex);
  if (cleanHex) {
    this.hex = cleanHex;
    this.name = name;
  }
};

/**
 * Remove # from a hex code.
 *
 * @param {String} hex
 * @return {String|Boolean}
 */
ThemeBuilder.styles.PaletteColor.cleanHex = function (hex) {
  if (this.isHex(hex)) {
    return hex.replace(/^#/g, '');
  }
  else {
    return false;
  }
};

/**
 * Check a hex code for validity. Note that 'transparent' is also allowed.
 *
 * @param {String} hex
 *   The potential hex code.
 * @return {Boolean}
 */
ThemeBuilder.styles.PaletteColor.isHex = function (hex) {
  if (hex === 'transparent') {
    return true;
  }
  var regex = /^#?([a-f]|[A-F]|[0-9]){3}(([a-f]|[A-F]|[0-9]){3})?$/;
  return regex.test(hex);
};

