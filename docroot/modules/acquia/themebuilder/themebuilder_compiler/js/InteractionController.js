
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

/**
 * This class is provided to simplify complex interactions that
 * involve multiple steps.  An example is collecting a theme name from
 * the user.  In such an interaction, the user must be presented with
 * a dialog that allows them to type the theme name, and depending on
 * the users response and whether the theme already exists, the name
 * may be accepted or the user may be asked for the theme name again
 * or the application may ask for confirmation whether it is ok to
 * overwrite the existing theme.
 * 
 * This sort of interaction is best removed from the client code if
 * possible to keep the entire sequence together.  Further, if we
 * normalize these sorts of interactions we create a repeatable
 * pattern and we should be able to reduce the total amount of code
 * that must be maintained.
 * 
 * This InteractionController is based on a simple finite state
 * machine, which allows the interaction flowchart to be easily
 * understood just by looking at the interaction table rather than
 * sifting through all of the code.  Also the interaction can be
 * modified by adjusting the table.
 * @class
 * @constructor
 */
ThemeBuilder.InteractionController = ThemeBuilder.initClass();

/**
 * Constructor for the InteractionController.
 */
ThemeBuilder.InteractionController.prototype.initialize = function () {
  this.callbacks = {};
  this.table = {};
  
  this.setInteractionTable({
    // Complete the interaction
    interactionDone: 'done',
    interactionFailed: 'fail',
    interactionCanceled: 'cancel'
  });
};

/**
 * Sets the interaction table that drives the behavior of the
 * InteractionController instance.
 * 
 * The table consists of an object in which the key is the name of a
 * state within the interaction and the value is the name of the
 * method that should be called when that state is entered.
 * 
 * Example:
 *  this.setInteractionTable({
 *    // Show the name dialog
 *    begin: 'showNameDialog',
 *    nameAccepted: 'verifyName',
 *    nameCanceled: 'cancel',
 *
 *    // Verify the theme name
 *    nameAlreadyUsed: 'showOverwriteDialog',
 *    nameOk: 'done',
 *
 *    // Theme already exists
 *    overwriteTheme: 'done',
 *    doNotOverwrite: 'showNameDialog'
 *  });
 * 
 * Note that the value is a string and not a function.
 * 
 * Each method that is part of the interaction must have the same
 * method signature:
 * example.showNameDialog = function (data) {
 * ... in which the data parameter is an object that provides
 * information to the function.
 * 
 * Also, note that the methods enumerated in the interaction table do
 * not return values, but rather cause the state to change by calling
 * the event method.
 * 
 * @param {Object} table
 *   The interaction table in which the key is the state name and the
 *   value is a string that contains the name of the method that is
 *   called when the interaction reaches that state.
 */
ThemeBuilder.InteractionController.prototype.setInteractionTable = function (table) {
  if (table) {
    this.table = ThemeBuilder.merge(this.table, table);
  }
};

/**
 * Sets the callbacks for this interaction.  The caller can register
 * methods for 'done' and 'cancel' which will be called when the
 * interaction completes.
 * 
 * @param {Object} callbacks
 *   The callbacks object in which the key is 'done' and/or 'cancel'
 *   with the corresponding value(s) being the callback function.
 */ 
ThemeBuilder.InteractionController.prototype.setCallbacks = function (callbacks) {
  if (callbacks) {
    this.callbacks = ThemeBuilder.merge(this.callbacks, callbacks);
  }
};

/**
 * Starts the interaction.
 * 
 * @param {Object} data
 * An optional argument that can be passed to the method associated
 * with the 'begin' action.
 */
ThemeBuilder.InteractionController.prototype.start = function (data) {
  this.event(data, 'begin');
};

/**
 * Returns the current state of the interaction.
 * 
 * @return {String}
 *   The current state.
 */
ThemeBuilder.InteractionController.prototype.getCurrentState = function () {
  return this.currentState;
};

/**
 * Called when an event occurs, which causes the state of the
 * interaction to change.
 * 
 * Note that the data and eventName parameters must be the last
 * arguments in the list.  This choice was made because this method is
 * often called as an event callback, so the arguments passed directly
 * from the calling code will always come first.
 * 
 * @param {Array} map
 *  (Optional) An array containing property names that will be used to
 *  set the caller parameters into the data object.  This only works
 *  if the map size exactly matches the number of unnamed parameters.
 *  If there are unnamed parameters and no map, the unnamed parameters
 *  will be inserted into an array named 'callData' which is set into
 *  the data object.
 * @param {Object} data
 *   (Optional) Data associated with the event.  If this object is not
 *   provided, an empty object will be used instead.
 * @param {String} eventName
 *   The name of the event.
 */
ThemeBuilder.InteractionController.prototype.event = function (/* The arguments are assigned dynamically - do not specify them here. */) {
  var arglen = arguments.length, map, i, len, data;
  if (arguments.length < 1) {
    throw 'InteractionController.event called without an event name.';
  }
  var eventName = arguments[arglen - 1];

  // Determine whether an argument map is provided
  if (arglen >= 3 && jQuery.isArray(arguments[arglen - 3]) && arguments[arglen - 3].length === arglen - 3) {
    // An argument map has been provided.
    map = arguments[arglen - 3];
  }

  // Determine whether a data object was provided.  If not, use an empty object.
  data = arglen >= 2 ? arguments[arglen - 2] : {};

  // If there are unnamed parameters, add those to the data object.
  if (arglen > 2) {
    if (map) {
      // A map was provided to allow us to populate the resulting
      // object with named properties.
      for (i = 0, len = arglen - 3; i < len; i++) {
        data[map[i]] = arguments[i];
      }
    }
    else {
      // This is suboptimal, but provide the additional arguments as an array.
      data.callData = [];
      for (i = 0, len = arglen - 2; i < len; i++) {
        data.callData.push(arguments[i]);
      }
    }
  }

  // Invoke the action associated with the event name.
  var action = this.table[eventName];
  if (!action) {
    throw 'Could not find a transition associated with ' + eventName + '.';
  }
  if (!this[action]) {
    throw 'Missing function ' + action + ' associated with event ' + eventName + '.';
  }
  this.currentState = eventName;
  this[action](data);
};

/**
 * Helper method that creates an event callback.  Whenever an event
 * occurs, the InteractionController needs to manage the event and call the appropriate
 * methods so the state can be maintained and the flow through the
 * states can be governed by the table.
 * 
 * @param {String} eventName
 *   The name of the event.
 * @param {Object} data
 *   (Optional) The data associated with the event.
 */
ThemeBuilder.InteractionController.prototype.makeEventCallback = function (eventName, data) {
  if (data) {
    return ThemeBuilder.bind(this, this.event, data, eventName);
  }
  else {
    // This is probably being connected with a redirect, so the data
    // will be a callback-time parameter.
    return ThemeBuilder.bind(this, this.event, eventName);
  }
};

/**
 * The callback associated with the final state of this interaction.
 * 
 * @param {Object} data
 *   The data associated with the event.
 */
ThemeBuilder.InteractionController.prototype.done = function (data) {
  if (this.callbacks && this.callbacks.done) {
    this.callbacks.done(data);
  }
};

/**
 * The callback associated with the final state of this interaction.
 * 
 * @param {Object} data
 *   The data associated with the event.
 */
ThemeBuilder.InteractionController.prototype.fail = function (data) {
  if (this.callbacks && this.callbacks.fail) {
    this.callbacks.fail(data);
  }
};

/**
 * The callback associated with the final state of this interaction.
 * 
 * @param {Object} data
 *   The data associated with the event.
 */
ThemeBuilder.InteractionController.prototype.cancel = function (data) {
  if (this.callbacks && this.callbacks.cancel) {
    this.callbacks.cancel(data);
  }
};
