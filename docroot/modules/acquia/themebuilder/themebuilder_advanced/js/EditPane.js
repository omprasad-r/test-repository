/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true */

var ThemeBuilder = ThemeBuilder || {};

/**
 * The editPane class represents a textarea in which users can enter code.
 * @class
 */
ThemeBuilder.EditPane = ThemeBuilder.initClass();

/**
 * Initializes a new code editing pane.
 *
 * @param {jQuery} editingTextArea
 *   A jQuery object representing the textarea.
 * @param {CodeEditor} codeEditor
 *   The ThemeBuilder.CodeEditor object associated with this textarea.
 */
ThemeBuilder.EditPane.prototype.initialize = function (editingTextArea, codeEditor) {
  this.codeEditor = codeEditor;
  this.editor = editingTextArea; //This is the actual textarea we are operating on.
  this.buffer = null;
  this.changed = false;
  this.timer = 0;
  this.editor.bind('keyup', ThemeBuilder.bind(this, this.handleKeyUp));
  this.editor.bind('change', ThemeBuilder.bind(this, this.update));
  this.editor.keydown(ThemeBuilder.bind(this, this.handleKeyDown));
};

/**
 * Handle the keyup event.
 */
ThemeBuilder.EditPane.prototype.handleKeyUp = function () {
  clearTimeout(this.timer); // Clear the timeout if a keypress occurs before the timer callback fired
  var keyPressUpdate = ThemeBuilder.bindIgnoreCallerArgs(this, this.update);
  this.timer = setTimeout(keyPressUpdate, 450); // Update the page after 450ms
};

/**
 * Handler for the keydown event. Inserts two spaces if the tab key is pressed.
 *
 * @param {event} e
 *   The keydown event.
 *
 * @return {boolean}
 *   False if the tab key was pressed, true if any other key was pressed.
 */
ThemeBuilder.EditPane.prototype.handleKeyDown = function (e) {
  if (e.keyCode === 9) {
    this.insertAtCursor('  ');
    // Prevent the default tab key event from propagating (we don't want to
    // move to the next tabindex).
    return false;
  }
  return true;
};

/**
 * Called after every keyUp; triggers the custom 'update' event on the textarea.
 */
ThemeBuilder.EditPane.prototype.update = function () {
  if (this.buffer === this.editor.val()) {
    // No change, so nothing to do.
    return true;
  }

  this.changed = true;
  this.buffer = this.editor.value;
  this.editor.trigger('update');
};

/**
 * Insert text into the textarea at the current cursor location.
 *
 * @param {string} text
 *    The text to be inserted.
 */
ThemeBuilder.EditPane.prototype.insertAtCursor = function (text) {
  var textarea = this.editor.get(0);
  var top = textarea.scrollTop;
  var where = this._caret();
  if (document.selection) {
    textarea.focus();
    var sel = document.selection.createRange();
    sel.text = text;
  }
  else if (textarea.selectionStart || textarea.selectionStart === '0') {
    var startPos = textarea.selectionStart;
    var endPos = textarea.selectionEnd;
    textarea.value = textarea.value.substring(0, startPos) + text + textarea.value.substring(endPos, textarea.value.length);
  } else {
    textarea.value += text;
  }
  textarea.scrollTop = top;
  this._setSelRange(where + text.length, where + text.length);
};

/**
 * Determine the current location of the cursor.
 *
 * @private
 */
ThemeBuilder.EditPane.prototype._caret = function () {
  var node = this.editor.get(0);
  if (node.selectionStart) {
    return node.selectionStart;
  }
  else if (!document.selection) {
    return 0;
  }
  var c = String.fromCharCode(1);
  var sel = document.selection.createRange();
  var dul = sel.duplicate();
  dul.moveToElementText(node);
  sel.text = c;
  var len = (dul.text.indexOf(c));
  sel.moveStart('character', -1);
  sel.text = "";
  return len;
};

/**
 * Set the current selection in the textarea.
 *
 * @param selStart
 *   The beginning of the desired selection.
 * @param selEnd
 *   The end of the desired selection.
 *
 * @private
 */
ThemeBuilder.EditPane.prototype._setSelRange = function (selStart, selEnd) {
  var inputEl = this.editor.get(0);
  if (inputEl.setSelectionRange) {
    inputEl.focus();
    inputEl.setSelectionRange(selStart, selEnd);
  } else if (inputEl.createTextRange) {
    var range = inputEl.createTextRange();
    range.collapse(true);
    range.moveEnd('character', selEnd);
    range.moveStart('character', selStart);
    range.select();
  }
};
