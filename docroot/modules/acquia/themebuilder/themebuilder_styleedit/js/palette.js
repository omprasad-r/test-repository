/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true */

var ThemeBuilder = ThemeBuilder || {};
ThemeBuilder.styleEditor = ThemeBuilder.styleEditor || {};

ThemeBuilder.styleEditor.getPaletteStylesheet = function () {
  if (jQuery.rule) {
    return jQuery('link[href*=palette.css]').sheet();
  } else {
    return false;
  }
};

ThemeBuilder.styleEditor.changePalette = function (new_palette_id) {
  console.warn("ThemeBuilder.styleEditor.changePalette is deprecated. Use a PaletteModification instead.");
  // Validate palette_id.
  if (!ThemeBuilder.styleEditor.palettes[new_palette_id]) {
    alert("That palette does not exist.");
    return false;
  }

  ThemeBuilder.styleEditor.previewNewPalette(new_palette_id, ThemeBuilder.styleEditor.currentPalette);

  // Let Javascript know what the new palette id is.
  ThemeBuilder.styleEditor.currentPalette = new_palette_id;
  // Let the server know what the new palette id is.
  ThemeBuilder.postBack(Drupal.settings.styleSetPalettePath,
    {'palette_id': new_palette_id});
};



/**
 * Converts HSB to RGB.
 *
 * Taken from colorpicker.js.
 *
 * @param <object> hsb
 * @return <object>
 */
ThemeBuilder.styleEditor.HSBToRGB = function (hsb) {
  var rgb = {};
  var h = Math.round(hsb.h);
  var s = Math.round(hsb.s * 255 / 100);
  var v = Math.round(hsb.b * 255 / 100);
  if (s === 0) {
    rgb.r = rgb.g = rgb.b = v;
  } else {
    var t1 = v;
    var t2 = (255 - s) * v / 255;
    var t3 = (t1 - t2) * (h % 60) / 60;
    if (h === 360) {
      h = 0;
    }
    if (h < 60) {
      rgb.r = t1;
      rgb.b = t2;
      rgb.g = t2 + t3;
    }
    else if (h < 120) {
      rgb.g = t1;
      rgb.b = t2;
      rgb.r = t1 - t3;
    }
    else if (h < 180) {
      rgb.g = t1;
      rgb.r = t2;
      rgb.b = t2 + t3;
    }
    else if (h < 240) {
      rgb.b = t1;
      rgb.r = t2;
      rgb.g = t1 - t3;
    }
    else if (h < 300) {
      rgb.b = t1;
      rgb.g = t2;
      rgb.r = t2 + t3;
    }
    else if (h < 360) {
      rgb.r = t1;
      rgb.g = t2;
      rgb.b = t1 - t3;
    }
    else {
      rgb.r = 0;
      rgb.g = 0;
      rgb.b = 0;
    }
  }
  return { r: Math.round(rgb.r),
           g: Math.round(rgb.g),
           b: Math.round(rgb.b) };
};

/**
 * Converts RGB to a hex value.
 *
 * Taken from colorpicker.js.
 *
 * @param <object> rgb
 * @return <string>
 *   The hex color value (i.e. FFFFFF).
 */
ThemeBuilder.styleEditor.RGBToHex = function (rgb) {
  if (typeof(rgb.r) === 'undefined') {
    if (rgb.indexOf('(') !== -1) {
      rgb = rgb.replace(/^rgb\(/g, '').replace(/\)$/g, '').split(/, */g);
      rgb = {
        r: parseInt(rgb[0], 10),
        g: parseInt(rgb[1], 10),
        b: parseInt(rgb[2], 10)
      };
    }
  }
  var hex = [
    rgb.r.toString(16),
    rgb.g.toString(16),
    rgb.b.toString(16)
  ];
  jQuery.each(hex, function (nr, val) {
    if (val.length === 1) {
      hex[nr] = '0' + val;
    }
  });
  return hex.join('');
};

/**
 * Converts HSB to a hex value.
 *
 * Taken from colorpicker.js.
 *
 * @param <object> hsb
 * @return <string>
 *   The hex color value (i.e. FFFFFF).
 */
ThemeBuilder.styleEditor.HSBToHex = function (hsb) {
  return ThemeBuilder.styleEditor.RGBToHex(ThemeBuilder.styleEditor.HSBToRGB(hsb));
};
