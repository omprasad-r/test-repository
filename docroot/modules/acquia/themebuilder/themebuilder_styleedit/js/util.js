/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

ThemeBuilder.styleEditor = ThemeBuilder.styleEditor || {};

/**
 * Make a string into a CSS class identifier (.myclass).
 */
ThemeBuilder.styleEditor.classify = function (x, exclude) {
  if (!x) {
    return '';
  }
  var i;
  for (i = 0; i < exclude.length; i++) {
    x.replace(exclude[i], '');
  }
  return '.' + x.split(/\s+/).join('.');
};

/**
 * Creates a CSS selector for an item by tracing its lineage.
 *
 * @param <object> node
 *
 * @return <string> lineage
 *    the full lineage of a node
 */
ThemeBuilder.styleEditor.getNodePath = function (node) {
  if (!node) {
    return;
  }
  node = jQuery(node);
  var directory = node[0].nodeName.toLowerCase() + (node[0].id ? '#' + node[0].id : '') + ThemeBuilder.styleEditor.classify(node[0].className);
  while ((node = node.parent())) {
    if (!node.length) {
      break;
    }
    directory = node[0].nodeName.toLowerCase() + (node[0].id ? '#' + node[0].id : '') + ThemeBuilder.styleEditor.classify(node[0].className) + ' ' + directory;
  }
  return directory;
};

/**
 * internal function used by matchFullPath
 */
ThemeBuilder.styleEditor.matchOne = function (selector, path) {
  var subs = path.split('.');
  var nid = subs[0].split('#');
  var ssubs = selector.split('.');
  var snid = ssubs[0].split('#');

  if (snid[0].indexOf(':') !== -1) {
    snid[0] = snid[0].split(':')[0];
  }
  if (snid[1]) {
    if (snid[1] !== nid[1]) {
      return false;
    }
  }
  if (snid[0] && snid[0] !== nid[0]) {
    return false;
  }
  var i, e;
  for (i = 1; i < ssubs.length; i++) { // go through class names
    var good = false;
    for (e = 1; e < subs.length; e++) {
      if (ssubs[i] === subs[e]) {
        good = true;
        break;
      }
    }
    if (!good) {
      return false;
    }
  }
  return true;
};

/**
 * Check if full CSS selector matches a node path
 *
 * @param <string>  search
 *    node path
 * @param <string>  selector
 *    css rule selectorText
 * @param <bool>    truncate
 *    truncate selector only to the relevent part
 */
ThemeBuilder.styleEditor.matchFullPath = function (search, selector, truncate) {
  if (!search || !selector) {
    return false;
  }
  var parts = selector.split(/,\s*/g);
  for (var i = 0; i < parts.length; i++) {
    if (ThemeBuilder.styleEditor.matchPath(search, parts[i])) {
      if (truncate) {
        return parts[i];
      }
      else {
        return search;
      }
    }
  }
  return false;
};

/**
 * internal function used by matchFullPath
 */
ThemeBuilder.styleEditor.matchPath = function (selector, path, inherit) {
  if (!selector || !path) {
    return false;
  }
  var parts = selector.split(' ');
  var pparts = path.split(' ');
  var current = 0;
  var good;
  var i;
  for (i = 0; i < parts.length; i++) {
    good = false;
    for (; current < pparts.length; current++) {
      if (ThemeBuilder.styleEditor.matchOne(parts[i], pparts[current])) {
        good = true;
        break;
      }
    }
    if (!good) {
      return false;
    }
  }

  return inherit || current >= pparts.length - 1;
};

/**
 * Convert a css attibute name into a DOM style attribute
 * ex:
 *    background-color   -> backgroundColor
 *    -moz-border-radius -> MozBorderRadius
 */
ThemeBuilder.styleEditor.cdash = function (x) {
  var p = x.split('-');
  var i;
  for (i = 1; i < p.length; i++) {
    p[0] += p[i][0].toUpperCase() + p[i].slice(1);
  }
  return p[0];
};

/**
 * opposite of cdash
 */
ThemeBuilder.styleEditor.uncdash = function (x) {
  return x.replace(/[A-Z]/g, function (x) {
    return '-' + x.toLowerCase();
  });
};

/**
 * Convers an r, g and b component into its hexidecimal equivalent.
 */
ThemeBuilder.styleEditor.rgb_hex = function (r, g, b) {
  return ThemeBuilder.styleEditor.hex(r) + ThemeBuilder.styleEditor.hex(g) + ThemeBuilder.styleEditor.hex(b);
};

/**
 * convert from CSS rgb format to hex.
 *    rgb(255,255,255) -> #FFFFFF
 */
ThemeBuilder.styleEditor.unrgb = function (x) {
  return x.toString().replace(/rgb\((\d+)\s*,\s*(\d+)\s*,\s*(\d+\))/g, function (a, r, g, b) {
    return '#' + ThemeBuilder.styleEditor.rgb_hex(r, g, b);
  });
};

/**
 * Given a number between 0 and 255, convert it to a hexidecimal equivalent.
 */
ThemeBuilder.styleEditor.hex = function (n) {
  var hx = '0123456789ABCDEF';
  n = parseInt(n, 10);
  if (!n || isNaN(n)) {
    return '00';
  }
  n = n < 0 ? 0 : n;
  n = n > 255 ? 255 : n;
  return hx.charAt((n - n % 16) / 16) + hx.charAt(n % 16);
};

/**
 * Returns a getComputedStyle function for a given element.
 *
 * @param {DomObject} element
 *   The element for which the computed style should be calculated.
 */
ThemeBuilder.styleEditor.getComputedStyleFunction = function (element) {
  var style = '';
  if (document.defaultView && document.defaultView.getComputedStyle) {
    style = document.defaultView.getComputedStyle(element, '');
  }
  else if (element.currentStyle) {
    style = element.currentStyle;
  }
  // getComputedStyle is native code in FF, which means we have to clone the
  // CSSStyleDeclaration it returns in order to preserve its current value.
  style = ThemeBuilder.clone(style);
  var cssText = style.cssText;
  if (cssText) {
    // On Safari the property/value pairs are concatenated in the cssText
    // property.  Remove this property and instead add the properties to the
    // style object.
    delete style.cssText;
    var contents = cssText.split(';');
    for (var index = 0; index < contents.length; index++) {
      var line = contents[index];
      var propertyLength = line.indexOf(':');
      if (propertyLength === -1) {
        continue;
      }
      var property = jQuery.trim(line.slice(0, propertyLength));
      var value = jQuery.trim(line.slice(propertyLength + 1));
      if (value.indexOf('repeat ') > -1) {
        // Chrome mis-reports the value of background-repeat on some themes.
        // So far I haven't devised a rule that could make this more generic,
        // so for now I will fix only this particular issue.
        value = value.substr(value.lastIndexOf(' ') + 1);
      }
      style[property] = value;
    }
  }
  return ThemeBuilder.bind(style, ThemeBuilder.styleEditor._getComputedStyle);
};

/**
 * Helper function for ThemeBuilder.styleEditor.getComputedStyleFunction.
 *
 * Returns the computed style for the specified property. The function
 * scope should be a CSSStyleDeclaration for a given DomElement.
 *
 * @private
 *
 * @param {string} property
 *   A CSS property such as "background-color".
 */
ThemeBuilder.styleEditor._getComputedStyle = function (property) {
  // Note: Normally we would use this.getPropertyValue if it were available.
  // However, at least in FF3.5, getPropertyValue() is native code that only
  // works with the original CSSStyleDeclaration object, not a cloned object.
  /*
  if (this.getPropertyValue) {
    value = this.getPropertyValue(property);
    return value;
  }
  */
  var value = this[property];
  if (!value) {
    property = property.replace(/\-(\w)/g, function (match, p1) {
      return  p1.toUpperCase();
    });
    value = this[property];
  }
  return value;
};

/**
 * Returns the size in pixels of the top border and top padding of the
 * specified element.
 *
 * @param {DomElement} element
 *   The element.
 */
ThemeBuilder.styleEditor.getTopOffset = function (element) {
  var properties = ['padding-top', 'border-top-width'];
  var getComputedStyle = ThemeBuilder.styleEditor.getComputedStyleFunction(element);
  var offset = 0;
  for (var i = 0; i < properties.length; i++) {
    offset += parseInt(getComputedStyle(properties[i]), 10);
  }
  return offset;
};

/**
 * Returns the size in pixels of the bottom border and bottom padding
 * of the specified element.
 *
 * @param {DomElement} element
 *   The element.
 */
ThemeBuilder.styleEditor.getBottomOffset = function (element) {
  var properties = ['padding-bottom', 'border-bottom-width'];
  var getComputedStyle = ThemeBuilder.styleEditor.getComputedStyleFunction(element);
  var offset = 0;
  for (var i = 0; i < properties.length; i++) {
    offset += parseInt(getComputedStyle(properties[i]), 10);
  }
  return offset;
};
