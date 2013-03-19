
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true */

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * @class
 */
ThemeBuilder.styles.Editor = ThemeBuilder.initClass();
ThemeBuilder.styles.Editor.prototype.initialize = function (elementPicker) {
  this.elementPicker = elementPicker;
  this.isVisible = false;
};

/**
 * This method is called when the corresponding tab is selected.
 */
ThemeBuilder.styles.Editor.prototype.tabSelectionChanged = function (tabName) {
  var $ = jQuery;
  if (this.tabName === tabName) {
    this.isVisible = true;
    this.tabSelected();
  }
  else if (this.isVisible) {
    this.isVisible = false;
    this.tabDeselected();
  }
};

ThemeBuilder.styles.Editor.prototype.tabSelected = function () {
};

ThemeBuilder.styles.Editor.prototype.tabDeselected = function () {
};

ThemeBuilder.styles.Editor.prototype.disableInputs = function () {
};

/**
 * Simple private function that simply returns the passed value.  This is
 * used in the case that one or more slider methods are not provided.
 *
 * @param {mixed} value
 *   The value that should be returned from this function.
 */
ThemeBuilder.styles.Editor.prototype._return = function (value) {
  return value;
};

/**
 * Attempts to determine the value associated with the specified attribute
 * name.  The value is determined by placing a new element at an appropriate
 * place in the DOM such that the specified selector would apply, and then
 * query for the specified value.
 *
 * @param {String} selector
 *   The selector.
 * @param {String} attr
 *   The attribute name.
 *
 * @return {String}
 *   The value of the specified property, or 'undefined' if it could not be
 *   determined.
 */
ThemeBuilder.styles.Editor.prototype.getPropertyValue = function (selector, attr) {
  var items = selector.split(' ');
  var node = 'body';
  var first = null;
  for (var it = 0; it < items.length; it++) {
    var parts = items[it].split('.');
    if (parts[0] === '' || parts[0][0] === '#') {
      node = jQuery('<div></div>').appendTo(node);
    }
    else {
      node = jQuery('<' + parts[0] + '></' + parts[0] + '>').appendTo(node);
    }
    if (!first) {
      first = node;
    }
    node.css('display', 'none');
    if (parts[0][0] === '#') {
      node.attr('id', parts[0]);
    }
    for (var i = 1; i < parts.length; i++) {
      node.addClass(parts[i]);
    }
  }
  var val = undefined;
  if (attr) {
    val = node.css(attr);
  }
  first.remove();
  return val;
};

/**
 * Returns the selected element.  This represents the element that the user
 * selected or a parent of the element the user selected based on the current
 * state of the Selector.  The idea is that we want to initialize values of
 * the properties based on the element the user selected (not, for example,
 * the first element in an array of elements that match the current selector).
 *
 * @return {DomElement}
 *   The element.  If the user has not yet selected an element, the body
 *   element is returned.
 */
ThemeBuilder.styles.Editor.prototype.getSelectedElement = function () {
  var $ = jQuery;
  var selector = this.elementPicker.path_selector;
  var element;
  if (selector) {
    element = selector.getSelectedElement();
  }
  else {
    element = $('body').get(0);
  }
  return element;
};
