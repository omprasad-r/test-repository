
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * This class represents a CSS declaration (such as "border-width: 1px;").
 * @class
 */
ThemeBuilder.styles.Declaration = ThemeBuilder.initClass();

/**
 * The constructor for the Declaration class.
 *
 * @param {string} declarationText
 *   The declaration text ("background-image: url(https://domain/image.jpg);").
 */
ThemeBuilder.styles.Declaration.prototype.initialize = function (declarationText) {
  if (declarationText) {
    var pieces = declarationText.split(":");
    // The property is the section to the left of the first colon. Example:
    // background-image
    this.property = jQuery.trim(pieces.shift());
    // The entire remaining string, including any colons that were originally
    // in it, is the value. Example:
    //   url(https://domain:8080/image.jpg);
    this.value = pieces.join(':');
    // Remove the trailing semicolon and any stray whitespace remaining in the
    // value. Example:
    // url(https://domain:8080/image.jpg)
    var re = new RegExp('\\s*(.+);\\s*$');
    this.value = this.value.replace(re, '$1');
  }
  else {
    this.property = this.value = "";
  }
  this.toRemove = false;
};

/**
 * Static method to split a rule's declaration block into Declarations.
 */
ThemeBuilder.styles.Declaration.getDeclarations = function (rule) {
  var ruleText = jQuery.rule(rule).text();
  // Split the rule into individual declaration lines, without semicolons.
  var declarations = ruleText.replace(/;$/, '').split(";");
  var i;
  var declarationObjects = [];
  for (i = 0; i < declarations.length; i++) {
    declarationObjects[i] = new ThemeBuilder.styles.Declaration(declarations[i]);
  }
  return declarationObjects;
};

/**
 * Static method to join declarations into one declaration block.
 *
 * @return string
 *   A CSS declaration block ("border-width: 2px; border-color: #000000;").
 */
ThemeBuilder.styles.Declaration.joinDeclarations = function (declarations) {
  var i;
  var declarationText = "";
  for (i = 0; i < declarations.length; i++) {
    declarationText += declarations[i].toString();
  }
  return declarationText;
};

/**
 * Set a delete flag on the Declaration object.
 */
ThemeBuilder.styles.Declaration.prototype.remove = function () {
  this.toRemove = true;
};

/**
 * Return a declaration as text that can be added to a CSS rule.
 *
 * @return string
 *   A CSS declaration in text form ("border-width: 2px;").
 */
ThemeBuilder.styles.Declaration.prototype.toString = function () {
  if (this.toRemove) {
    return "";
  }
  else {
    return this.property + ": " + this.value + ";";
  }
};

/**
 * Set the declaration's property ("border-width").
 */
ThemeBuilder.styles.Declaration.prototype.setProperty = function (property) {
  this.property = jQuery.trim(property);
};

/**
 * Set the declaration's value ("2px").
 */
ThemeBuilder.styles.Declaration.prototype.setValue = function (value) {
  if (value instanceof String !== true) {
    value = "" + value;
  }
  this.value = jQuery.trim(value);
};

/**
 * Convert a declaration from one palette to another.
 * 
 * @param {Palette} oldPalette
 * @param {Palette} newPalette
 */
ThemeBuilder.styles.Declaration.prototype.replaceColor = function (oldPalette, newPalette) {
  if (this.property.toLowerCase().indexOf('color') !== -1) {
    var colorManager = ThemeBuilder.getColorManager();
    var hex;
    if (this.value === 'transparent') {
      hex = 'transparent';
    }
    else {
      if (this.value.indexOf('rgb') !== -1) {
        // Standards-compliant browsers.
        var matches = this.value.match(new RegExp('rgb\\((.+),\\s?(.+),\\s?(.+)\\)'));
        hex = ThemeBuilder.styleEditor.RGBToHex({
          r: parseInt(matches[1], 10),
          g: parseInt(matches[2], 10),
          b: parseInt(matches[3], 10)
        });
      }
      else if (this.value.indexOf('#') === 0) {
        // IE.
        hex = this.value.substr(1, 6);
      }
      else {
        // This doesn't appear to be a real color-related declaration.
        // Do nothing to change it.
        return;
      }
    }
    var paletteIndex = oldPalette.hexToPaletteIndex(hex);
    if (paletteIndex) {
      var newHex = newPalette.paletteIndexToHex(paletteIndex);
      newHex = colorManager.addHash(newHex);
      // Replace the declaration's value.
      this.setValue(newHex);
    }
    else {
      // If the old color wasn't in the old palette, it didn't belong in
      // palette.css in the first place. Don't rewrite it.
    }
  }

};
