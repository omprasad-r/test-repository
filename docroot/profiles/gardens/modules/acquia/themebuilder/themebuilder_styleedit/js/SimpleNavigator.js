
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true ThemeBuilder: true */

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * The SimpleNavigator isn't really a navigator at all.  It is the original
 * highlight mechanism we used to employ to show the user which element(s)
 * would be affected by their changes.  This has been abstracted out of the
 * ElementPicker so we could implement other more interesting strategies and
 * bind the strategy to the version of theme that it was meant to work with.
 * @class
 */
ThemeBuilder.styles.SimpleNavigator = ThemeBuilder.initClass();
ThemeBuilder.styles.SimpleNavigator.prototype.initialize = function () {
  this.highlighter = ThemeBuilder.styles.Stylesheet.getInstance('highlighter.css');
  this.advanced = false;
};

/**
 * Causes the element(s) identified by the specified selector to be highlighted.
 *
 * @param {String} selector
 *   The selector that describes the set of selected elements.
 */
ThemeBuilder.styles.SimpleNavigator.prototype.highlightSelection = function (selector) {
  this.unhighlightSelection();
  if (selector) {
    this.highlighter.setRule(selector, 'background', 'rgba(0, 0, 255, 0.2) !important');
  }
};

/**
 * Causes the entire navigator to be removed from the dom.
 */
ThemeBuilder.styles.SimpleNavigator.prototype.deleteNavigator = function () {
  this.unhighlightSelection();
};

/**
 * Remove the highlight from the elements identified by the current selector.
 */
ThemeBuilder.styles.SimpleNavigator.prototype.unhighlightSelection = function () {
  this.highlighter.clear();
};

/**
 * Highlight the selected element.  This method is provided for more
 * sophisticated navigators that allow the user to move through the DOM.
 *
 * @param {jQuery element} $element
 *   The selected element.
 */
ThemeBuilder.styles.SimpleNavigator.prototype.highlightClicked = function ($element) {
};
