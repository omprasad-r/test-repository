/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global ThemeBuilder: true debug: true */

/**
 * The UndoStatusSwitch allows any code in the system to file a reason for
 * which the undo/redo buttons should be disabled and determines when the
 * buttons should be enabled again.
 *
 * The idea is pretty simple.  Code that has a need to disable the buttons
 * can call an instance of this class to disable the buttons, and receive
 * an opaque key in return.  When that reason for disabling the buttons
 * is resolved (for example, the user dismisses a dialog), the reason key
 * can be cleared with this object.  When there are no reasons for the buttons
 * to be disabled, the buttons will again become enabled.
 *
 * This is necessarily more complex than simply causing the buttons to be
 * disabled and enabled at certain points within the code because of the
 * asynchronous nature of the themebuilder.  The undo/redo buttons should be
 * disabled while the user is interacting with the color dialog box.  It should
 * also be disabled between the time the user clicks the undo button and the
 * time the server response is received.  If such events overlap, it is not
 * feasible to handle the enabling and disabling of these buttons inline.
 * @class
 */
ThemeBuilder.UndoStatusSwitch = ThemeBuilder.initClass();

/**
 * Initializes the UndoStatusSwitch instance.
 */
ThemeBuilder.UndoStatusSwitch.prototype.initialize = function () {
  // This is a set of reasons for the undo / redo buttons to be disabled.  When
  // the set is empty, the buttons should be enabled, provided there are
  // modifications in each of the stacks.
  this._reasons = {};
  this._listeners = [];
};

/**
 * Indicates the current status of undo and redo functionality.  A true result
 * indicates the buttons and functionality should be enabled, and false indicates
 * disabled.
 *
 * @return {boolean}
 *   true indicates enabled; false disabled.
 */
ThemeBuilder.UndoStatusSwitch.prototype.getStatus = function () {
  return !this._hasReasons();
};

/**
 * Registers a reason for disabling the undo and redo buttons.  If there are
 * no other reasons registered, this call will cause event listeners to be
 * called, which will result in the buttons being disabled.
 *
 * @return {string}
 *   A unique, opaque key that must be used to clear the disabling later.
 */
ThemeBuilder.UndoStatusSwitch.prototype.disable = function () {
  var key = this._generateUniqueKey();
  var wasEmpty = !this._hasReasons();
  this._reasons[key] = true;
  if (wasEmpty) {
    this.notifyListeners(false);
  }
  return key;
};

/**
 * Clears the reason associated with the specified key.  this key is returned
 * from the disable method.  If this key represents the last reason to disable
 * the undo and redo buttons, the event listeners will be notified that the buttons
 * should now be enabled.
 *
 * @param {string} key
 *   The key returned from the disable method that represents the reason for
 *   disabling the buttons.
 */
ThemeBuilder.UndoStatusSwitch.prototype.clear = function (key) {
  delete this._reasons[key];
  if (!this._hasReasons()) {
    this.notifyListeners(true);
  }
};

/*
 * @param listener object
 *   An object with a  method.
 */
ThemeBuilder.UndoStatusSwitch.prototype.addStatusChangedListener = function (listener) {
  this._listeners.push(listener);
};

/**
 * Removes the specified listener from the switch.
 *
 * @param listener object
 *   The listener to remove.
 */
ThemeBuilder.UndoStatusSwitch.prototype.removeStatusChangedListener = function (listener) {
  var listeners = [];
  for (var i = 0; i < this._listeners.length; i++) {
    if (this._listeners[i] !== listener) {
      listeners.push(this._listener[i]);
    }
  }
  this._listeners = listeners;
};

/**
 * Notifies the listeners that a change to the status of the undo and redo
 * buttons has occurred.
 *
 * @param {boolean} status
 *   If true, the listeners are notified that the buttons are going from the
 *   disabled state to the enabled state. false indicates the opposite.
 */
ThemeBuilder.UndoStatusSwitch.prototype.notifyListeners = function (status) {
  for (var i = 0; i < this._listeners.length; i++) {
    this._listeners[i].undoStatusChanged(status);
  }
};

/**
 * Indicates whether or not there are registered reasons for not enabling the
 * undo and redo buttons.
 *
 * @return {boolean}
 *   true if there are registered reasons for not enabling the buttons; false
 *   otherwise.
 */
ThemeBuilder.UndoStatusSwitch.prototype._hasReasons = function () {
  for (var i in this._reasons) {
    if (this.hasOwnProperty(i)) {
      return true;
    }
  }
  return false;
};

/**
 * Generates a unique key that represents a reason.  This key will be used
 * to clear the reason from this switch when the reason no longer applies.
 */
ThemeBuilder.UndoStatusSwitch.prototype._generateUniqueKey = function () {
  var key = 'reason_key_' + Math.floor(Math.random() * 25000);
  while (this._reasons[key]) {
    key = 'reason_key_' + Math.floor(Math.random() * 25000);
  }
  return key;
};
ThemeBuilder.undoButtons = new ThemeBuilder.UndoStatusSwitch();
