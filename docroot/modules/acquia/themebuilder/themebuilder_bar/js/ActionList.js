/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global window : true jQuery: true Drupal: true ThemeBuilder: true*/

ThemeBuilder = ThemeBuilder || {};

ThemeBuilder.ui = ThemeBuilder.ui || {};

/**
 * The ActionList wraps a message in a jquery ui ActionList with hooks for the OK and Cancel button callbacks
 * @class
 */
ThemeBuilder.ui.ActionList = ThemeBuilder.initClass();

/**
 * The constructor of the ActionList class.
 *
 * @param {DomElement} element
 *   The element is a pointer to the jQuery object that will be wrapped in the 
 *   ActionList.
 * @param {Object} options
 *   Options for the ActionList. May contain the following optional properties:
 *   - text: Text for the message displayed in the ActionList. HTML or plain text
 *     can be passed in. Defaults to 'Continue?'.
 *   - actionButton: Text displayed in the action button. Defaults to 'OK'.
 *   - cancelButton: Text displayed in the cancellation button. Defaults to
 *     'Cancel'.
 * @param {Object} callbacks
 *   Button callback functions for the ActionList. May contain the following
 *   optional properties (if either of these are not set, the ActionList will
 *   simply be closed when the corresponding button is clicked):
 *   - action: Callback to invoke when the action button is clicked.
 *   - cancel: Callback to invoke when the cancellation button is clicked.
 * @return {Boolean}
 *   Returns true if the ActionList initializes.
 */
ThemeBuilder.ui.ActionList.prototype.initialize = function (options) {
  
  if (!options && !options.actions) {
    return null;
  }
  
  var $ = jQuery;
  this._$pointer = null;
  this._actions = options.actions;
  this._wrapperId = '';
  this._wrapperClasses = [];
  if (options.wrapper) {
    this._wrapperId = (options.wrapper.id) ? options.wrapper.id : '';
    this._wrapperClasses = (options.wrapper.classes) ? options.wrapper.classes : [];
  }
  this._type = 'ActionList';
  
  // Build the DOM element
  this._build();
  
  return this;
};

ThemeBuilder.ui.ActionList.prototype._build = function () {
  var $ = jQuery;
  // Make a wrapper for the UI element
  var wrapperClasses = ['actions'];
  $.merge(wrapperClasses, this._wrapperClasses);
  var $wrapper = $('<ul>', {
    id: ((this._wrapperId) ? this._wrapperId : "themebuilder-actionlist")
  }).addClass(wrapperClasses.join(' '));
  // Loop through the actions and create action items for each one
  for (var i = 0; i < this._actions.length; i++) {
    var itemClasses = [];
    var linkClasses = ['action'];
    var handler = ThemeBuilder.bind(this, this._respond, this._actions[i].action);
    var label = this._actions[i].label;
    if (!label) {
      $.error(Drupal.t('No label provided for ') + this._actions[i]);
    }
    // Create a class name from the label and add to the linkClasses
    $.merge(linkClasses, [ThemeBuilder.util.getSafeClassName(label)]);
    // Check for item linkClasses
    if (this._actions[i].linkClasses) {
      $.merge(linkClasses, this._actions[i].linkClasses);
    }
    // Check for itemClasses
    if (this._actions[i].itemClasses) {
      $.merge(itemClasses, this._actions[i].itemClasses);
    }
    var $item = $('<li>', {
      html: $('<a>', {
        html: $('<span>', {
          html: label
        }),
        click: handler
      }).addClass(linkClasses.join(' '))
    }).addClass(itemClasses.join(' '));
    $item.appendTo($wrapper);
  }
  // Save a pointer to the jQuery object
  this._$pointer = $wrapper;
};

/**
 * Destroys the ActionList DOM element
 */
ThemeBuilder.ui.ActionList.prototype._close = function () {
  this._$pointer.remove();
};

/**
 * Returns the form data to the interaction control that instantiated this ActionList
 * 
 * @param {Event} event
 *   The ActionList button click event
 * @param {function} action
 *   The callback associated with the button, defined by the instantiating 
 *   interaction control.
 */
ThemeBuilder.ui.ActionList.prototype._respond = function (event, action) {
  action(event);
};

/**
 * Returns a jQuery pointer to this object
 *
 * @return {object}
 *   Returns null if the carousel has no pointer.
 */
ThemeBuilder.ui.ActionList.prototype.getPointer = function () {
  if (this._$pointer) {
    return this._$pointer;
  } 
  else {
    return null;
  }
};

/**
 * Utility function to remove 'px' from calculated values.  The function assumes that
 * that unit 'value' is pixels.
 *
 * @param {String} value
 *   The String containing the CSS value that includes px.
 * @return {int}
 *   Value stripped of 'px' and casted as a number or NaN if 'px' is not found in the string.
 */
ThemeBuilder.ui.ActionList.prototype._stripPX = function (value) {
  var index = value.indexOf('px');
  if (index === -1) {
    return NaN;
  }
  else {
    return Number(value.substring(0, index));
  }
};
