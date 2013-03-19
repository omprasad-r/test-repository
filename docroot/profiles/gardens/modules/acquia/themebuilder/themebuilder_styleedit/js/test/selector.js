
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true assertEquals: true*/

/**
 * @ignore
 */
function setUp() {
  // Create an interesting DOM hierarchy to try.
  var $ = jQuery;
  var topElement = $('<div id="styleedit_test" class="one two three"><div><div><table><tr><td><h3><img id="styleedit_test_find_me" class="four five six">');
  $('body').append(topElement);
}

/**
 * @ignore
 */
function tearDown() {
  var $ = jQuery;
  $('#styleedit_test').remove();
}

/**
 * @ignore
 */
function getTestElement() {
  return document.getElementById('styleedit_test_find_me');
}

/**
 * This is a simple listener object that detects whether the selectorChanged
 * method has been called.
 * @class
 */
var SelectorListener = ThemeBuilder.initClass();

SelectorListener.prototype.initialize = function () {
  this._selectorChangedListenerCalled = false;
  this._selectorElementChangedListenerCalled = false;
};

SelectorListener.prototype.resetSelectorChanged = function () {
  this._selectorChangedListenerCalled = false;
};

SelectorListener.prototype.getSelectorChanged = function () {
  return this._selectorChangedListenerCalled;
};

SelectorListener.prototype.selectorChanged = function (selector) {
  if (selector) {
    this._selectorChangedListenerCalled = true;
  }
};

SelectorListener.prototype.resetSelectorElementChanged = function () {
  this._selectorElementChangedListenerCalled = false;
};

SelectorListener.prototype.getSelectorElementChanged = function () {
  return this._selectorElementChangedListenerCalled;
};

SelectorListener.prototype.selectorElementChanged = function (selector) {
  if (selector) {
    SelectorListener._selectorElementChangedListenerCalled = true;
  }
};

/**
 * @ignore
 */
function test_path_constructor() {
  // Pass the element to the Selector class.
  var element = getTestElement();
  var selector = new ThemeBuilder.styles.Selector();
  selector.setElement(element);
  var css_selector = selector.getCssSelector();
  assertEquals('html.js body div#styleedit_test.one.two.three div div table tbody tr td h3 img#styleedit_test_find_me.four.five.six', css_selector);
}

/**
 * @ignore
 */
function test_get_element() {
  var element = getTestElement();
  var selector = new ThemeBuilder.styles.Selector();
  selector.setElement(element);
  assertEquals(element, selector.getElement());
}

/**
 * Make sure the getSelectedElement method works.  This is supposed to return
 * the element closest to the actual element selected by the user, but matches
 * the currently configured selector.
 * @ignore
 */
function test_getSelectedElement() {
  var element = getTestElement();
  var selector = new ThemeBuilder.styles.Selector();
  selector.setElement(element);
  assertEquals(element, selector.getElement());

  // Disable the last PathElement, simulating the user deselecting that node.
  var path = selector.path;
  path[path.length - 1].setEnabled(false);
  assertEquals(element.parentNode, selector.getSelectedElement());

  path[path.length - 2].setEnabled(false);
  assertEquals(element.parentNode.parentNode, selector.getSelectedElement());
}

/**
 * @ignore
 */
function test_selectorChanged() {
  var element = getTestElement();
  var selector = new ThemeBuilder.styles.Selector();
  selector.setElement(element);
  var selectorListener = new SelectorListener();
  selector.addSelectorListener(selectorListener);
  assertEquals(false, selectorListener.getSelectorChanged());
  selector.selectorChanged();
  assertEquals(true, selectorListener.getSelectorChanged());
}

/**
 * @ignore
 */
function test_selectorElementChanged() {
  var element = getTestElement();
  var selector = new ThemeBuilder.styles.Selector();
  selector.setElement(element);
  var selectorListener = new SelectorListener();
  selector.addSelectorListener(selectorListener);
  assertEquals(false, selectorListener.getSelectorChanged());
  selector.selectorChanged();
  assertEquals(true, selectorListener.getSelectorChanged());
}

/**
 * @ignore
 */
function test_getHumanReadableSelector() {
  var $ = jQuery;
  var selector = new ThemeBuilder.styles.Selector();
  selector.setElement($('body').get(0));
  assertEquals('All site background in all html', selector.getHumanReadableSelector());
  var path = selector.path;
  path[0].setEnabled(false);
  assertEquals('All site background', selector.getHumanReadableSelector());
}
