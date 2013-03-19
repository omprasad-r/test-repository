
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true AjaxUpload: true ThemeBuilder: true*/

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * The RadioButton class turns a set of divs into radio button behavior,
 * allowing the user to toggle one of the divs on and providing the
 * associated selected value to the application for processing.  Each
 * div that represents a button must have a class associated with it,
 * which must be specified in the constructor.  Also, each button div
 * must have an element id which is the concatenation of the button
 * class and "-<value>".  In this way the id is made unique and it identifies
 * the value associated with the button.
 *
 * @class
 */
ThemeBuilder.styles.RadioButton = ThemeBuilder.initClass();

/**
 * The constructor of the RadioButton class.  Currently the markup for the
 * buttons must be created before this call is made.  The constructor
 * identifies the set of buttons being managed by this RadioButton instance
 * and it sets the default state and wires the buttons to an appropriate
 * event handler.
 *
 * @param {String} parentSelector
 *   The selector used to identify the container that serves as the parent
 *   to all of the buttons being managed by this RadioButton instance.
 * @param {String} buttonClass
 *   The classname common to all buttons in this RadioButton set.  This is
 *   also the id prefix and the property name.
 * @param {String} defaultValue
 *   The default value.  The associated button will be set into the enabled
 *   state.
 */
ThemeBuilder.styles.RadioButton.prototype.initialize = function (parentSelector, buttonClass, defaultValue) {
  this.parentSelector = parentSelector;
  this.buttonClass = buttonClass;
  this.defaultValue = defaultValue;

  this.listeners = [];
  this.buttonPressed = ThemeBuilder.bind(this, this._buttonPressed);
  this.buttons = this.findButtons();
  this.setEnabledButton(defaultValue);
  this.addClickHandlers();
};

/**
 * Returns the property name associated with this RadioButton instance.
 *
 * @return {String}
 *   The property name.
 */
ThemeBuilder.styles.RadioButton.prototype.getPropertyName = function () {
  return this.buttonClass;
};

/**
 * Finds all of the buttons that should be managed by this RadioButton
 * instance.
 *
 * @return {object}
 *   An object that represents a mapping between the value that a button
 *   represents and the button's id, for each button being managed by
 *   this RadioButton instance.
 */
ThemeBuilder.styles.RadioButton.prototype.findButtons = function () {
  var $ = jQuery;
  var buttons = {};
  var children = $(this.parentSelector + ' .' + this.buttonClass);
  for (var i = 0; i < children.length; i++) {
    // Determine the value associated with the button.
    var value = children[i].id.substr((this.buttonClass + '-').length);
    buttons[value] = {value: value, id: '#' + children[i].id};
  }
  return buttons;
};

/**
 * Enables the button associated with the specified value.  If the specified
 * value does not have an associated button, an exception is thrown.
 *
 * @param {String} buttonValue
 *   The value identifying the button that should be enabled.
 */
ThemeBuilder.styles.RadioButton.prototype.setEnabledButton = function (buttonValue) {
  var $ = jQuery;
  var element = this.valueToElement(buttonValue);
  if (element) {
    $('.' + this.buttonClass).removeClass('enabled');
    element.addClass('enabled');
    return;
  }
  throw 'Radio button (' + this.buttonClass + ') set to unknown value "' + buttonValue + '"';
};

/**
 * Causes an appropriate click handler to be associated with each button
 * being managed by this RadioButton instance.
 */
ThemeBuilder.styles.RadioButton.prototype.addClickHandlers = function () {
  var $ = jQuery;
  for (var value in this.buttons) {
    if (this.buttons[value]) {
      $(this.buttons[value].id).click(this.buttonPressed);
    }
  }
};

/**
 * Sets the associated value.  If the value doesn't have an associated button
 * being managed by this RadioButton instance, nothing changes.  Change
 * listeners are called when the value is successfully changed.
 *
 * @param {String} newValue
 *   The value to set.
 */
ThemeBuilder.styles.RadioButton.prototype.setValue = function (newValue) {
  try {
    var oldValue = this.getValue();
    if (oldValue !== newValue) {
      this.setEnabledButton(newValue);
      this.notifyListeners(oldValue, newValue);
    }
  }
  catch (e) {
  }
};

/**
 * Returns the current value represented by the state of this Radio Button
 * instance.
 *
 * @return {String}
 *   The current value.
 */
ThemeBuilder.styles.RadioButton.prototype.getValue = function () {
  var $ = jQuery;
  var selected = $(this.parentSelector + ' .' + this.buttonClass + '.enabled');
  if (selected.length !== 1) {
    throw ('Radio button is in a bad state - ' + selected.length +
      ' buttons are selected.');
  }
  return this.elementToValue(selected);
};

/**
 * Retrieves the value associated with the specified element.
 *
 * @param {DomObject} element
 *   The element for which the associated value should be determined.
 * @return {String}
 *   The value associated with the specified element.
 */
ThemeBuilder.styles.RadioButton.prototype.elementToValue = function (element) {
  if (element.length === 1) {
    element = element[0];
  }
  for (var value in this.buttons) {
    if (value && this.buttons[value].id === '#' + element.id) {
      return value;
    }
  }
  return undefined;
};

/**
 * Retrieves the element associated with the specified value.  If no element
 * is associated with the specified value within this RadioButton instance,
 * the value 'undefined' is returned instead.
 *
 * @param {String} value
 *   The value associated with the desired element.
 * @return {DomObject}
 *   The associated DOM element, or undefined if it doesn't exist.
 */
ThemeBuilder.styles.RadioButton.prototype.valueToElement = function (value) {
  var $ = jQuery;
  var id = '#' + this.buttonClass + '-' + value;
  var element = $(id);
  return element.length === 1 ? element : undefined;
};

/**
 * The event handler that responds when the user clicks one of the buttons.
 * This handler simply enables the associated button, and all other buttons
 * being managed by this RadioButton instance are disabled as a result.
 *
 * @param {DomEvent} event
 *   The click event.
 */
ThemeBuilder.styles.RadioButton.prototype._buttonPressed = function (event) {
  var value = this.elementToValue(event.currentTarget);
  this.setValue(value);
};

/**
 * Adds a change listener that will be notified when the user selects a
 * new value in this RadioButton instance.
 *
 * @param {Object} listener
 *   The change listener to add.
 */
ThemeBuilder.styles.RadioButton.prototype.addChangeListener = function (listener) {
  this.listeners.push(listener);
};

/**
 * Notifies all listeners that the value represented by this RadioButton
 * instance has changed to the specified value.
 *
 * @param {String} oldValue
 *   The original value.
 * @param {String} newValue
 *   The newly selected value.
 */
ThemeBuilder.styles.RadioButton.prototype.notifyListeners = function (oldValue, newValue) {
  for (var i = 0; i < this.listeners.length; i++) {
    if (this.listeners[i].valueChanged) {
      this.listeners[i].valueChanged(this.getPropertyName(), oldValue, newValue);
    }
  }
};
