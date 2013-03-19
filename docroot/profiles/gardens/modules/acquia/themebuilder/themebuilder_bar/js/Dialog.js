/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global window : true jQuery: true Drupal: true ThemeBuilder: true*/

ThemeBuilder = ThemeBuilder || {};

/**
 * ThemeBuilder.ui is a namespace for all User Interface wrappers that augment
 * the basic functionality of ThemeBuilder components.
 * It includes UI widgets such as Sliders, Tabs and Carousels
 * @namespace
 */
ThemeBuilder.ui = ThemeBuilder.ui || {};

/**
 * The dialog wraps a message in a jquery ui dialog with hooks for the OK and Cancel button callbacks
 * @class
 */
ThemeBuilder.ui.Dialog = ThemeBuilder.initClass();

/**
 * The constructor of the Dialog class.
 *
 * @param {DomElement} element
 *   The element is a pointer to the jQuery object that will be wrapped in the
 *   dialog.
 * @param {Object} options
 *   Options for the dialog. May contain the following optional properties:
 *   - text: Text for the message displayed in the dialog. HTML or plain text
 *     can be passed in. Defaults to 'Continue?'.
 *   - actionButton: Text displayed in the action button. Defaults to 'OK'.
 *   - cancelButton: Text displayed in the cancellation button. Defaults to
 *     'Cancel'.
 * @param {Object} callbacks
 *   Button callback functions for the dialog. May contain the following
 *   optional properties (if either of these are not set, the dialog will
 *   simply be closed when the corresponding button is clicked):
 *   - action: Callback to invoke when the action button is clicked.
 *   - cancel: Callback to invoke when the cancellation button is clicked.
 * @return {Boolean}
 *   Returns true if the dialog initializes.
 */
ThemeBuilder.ui.Dialog.prototype.initialize = function (element, options) {

  // This UI element depends on jQuery Dialog
  if (!jQuery.isFunction(jQuery().dialog)) {
    return false;
  }

  if (!options.buttons) {
    return false;
  }

  var $ = jQuery;
  this._$element = (element) ? element : $('body');
  this._$dialog = null;
  this._buttons = options.buttons;
  this._html = options.html;
  this._type = 'Dialog';
  if (options.defaultAction) {
    this._default = options.defaultAction;
  }
  else if (options.buttons.length === 1) {
    // If there is only one option, use that as the default.
    this._default = options.buttons[0].action;
  }
  // Build the DOM element
  this._build();

  return true;
};

ThemeBuilder.ui.Dialog.prototype._build = function () {
  var $ = jQuery;
  var $wrapper = $('<div>', {
    id: "themebuilder-confirmation-dialog",
    html: this._html
  }).addClass('message');
  var buttons = {};
  for (var i = 0; i < this._buttons.length; i++) {
    buttons[this._buttons[i].label] = ThemeBuilder.bind(this, this._respond, this._buttons[i].action);
  }
  $wrapper.appendTo(this._$element);
  var dialogOptions = {
    bgiframe: true,
    autoOpen: true,
    dialogClass: 'themebuilder-dialog tb',
    modal: true,
    overlay: {
      backgroundColor: '#000',
      opacity: 0.5
    },
    position: 'center',
    width: 335,
    buttons: buttons
  };
  if (this._default) {
    dialogOptions.beforeclose = this._default;
  }
  this._$dialog = $wrapper.dialog(dialogOptions);
};

/**
 * Destroys the dialog DOM element
 */
ThemeBuilder.ui.Dialog.prototype._close = function () {
  this._$dialog.remove();
};

/**
 * Returns the form data to the interaction control that instantiated this dialog
 *
 * @param {Event} event
 *   The dialog button click event
 * @param {function} action
 *   The callback associated with the button, defined by the instantiating
 *   interaction control.
 */
ThemeBuilder.ui.Dialog.prototype._respond = function (event, action) {
  var $ = jQuery,
      data = {},
      $form = $(event.target).closest('.ui-dialog').find('.ui-dialog-content');
  // Scrape all of the form data out of the dialog and store as key/value pairs
  // Input elements
  $(':input', $form).each(function (index, item) {
    var $this = $(this),
        name = $this.attr('name');
    if (name) {
      data[name] = $this.val();
    }
  });
  this._close();
  action(data);
};

/**
 * Returns a jQuery pointer to this object
 *
 * @return {object}
 *   Returns null if the carousel has no pointer.
 */
ThemeBuilder.ui.Dialog.prototype.getPointer = function () {
  if (this._$dialog) {
    return this._$dialog;
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
ThemeBuilder.ui.Dialog.prototype._stripPX = function (value) {
  var index = value.indexOf('px');
  if (index === -1) {
    return NaN;
  }
  else {
    return Number(value.substring(0, index));
  }
};
