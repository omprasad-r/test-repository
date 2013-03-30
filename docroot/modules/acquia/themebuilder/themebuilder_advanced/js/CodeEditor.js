/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true window: true*/

var ThemeBuilder = ThemeBuilder || {};

/**
 * Singleton class that manages the advanced CSS subtab.  This class
 * is a singleton because it manages a particular textarea in the DOM
 * and is written in such a way that there can only be one such
 * textarea.
 * @class
 */
ThemeBuilder.CodeEditor = ThemeBuilder.initClass();

/**
 * Static method to retrieve the singleton instance of the CodeEditor.
 *
 * @return
 *   The ThemeBuilder.CodeEditor instance.
 */
ThemeBuilder.CodeEditor.getInstance = function () {
  if (!ThemeBuilder.CodeEditor._instance) {
    ThemeBuilder.CodeEditor._instance = new ThemeBuilder.CodeEditor();
  }
  return ThemeBuilder.CodeEditor._instance;
};

/**
 * Constructor for the ThemeBuilder.CodeEditor class.  This
 * constructor should not be called directly, but instead the
 * getInstance static method should be used.
 */
ThemeBuilder.CodeEditor.prototype.initialize = function () {
  if (ThemeBuilder.CodeEditor._instance) {
    throw "ThemeBuilder.CodeEditor is a singleton that has already been instantiated.";
  }
  var $ = jQuery;
  this.panes = {};
  this.modifications = {};
  this.history = ThemeBuilder.History.getInstance();
};

/**
 * Initializes the UI of the advanced tab. Retrieves CSS from the
 * server, initializes the CSS textarea, and sets up event handlers.
 */
ThemeBuilder.CodeEditor.prototype.init = function () {
  var $ = jQuery;
  this.panes.css = new ThemeBuilder.EditPane($('#themebuilder-advanced-css textarea'), this);

  // Allow modifications to preview the CSS live in the browser by creating
  // an empty stylesheet and a way to update it on the fly.
  var stylesheet = ThemeBuilder.styles.Stylesheet.getInstance('advanced.css.live');
  this.updateStylesheet = ThemeBuilder.bind(this, this._updateStylesheet, stylesheet);

  // Handle the custom 'update' jQuery event.
  this.panes.css.editor.bind('update', ThemeBuilder.bind(this, this.handleUpdate));

  // TODO: AN-25510 - Enable the custom scroll bar.  The markup for
  // the textarea needs to change, moving the scrollpane class to an
  // enclosing div, and we need to force the height of the textarea
  // element such that it will not have its own scrollbar.
  // $('#themebuilder-wrapper .scrollpane').jScrollPane();
  // $('#themebuilder-advanced-css .scrollpane').bind('keyup', ThemeBuilder.bind(this, this.keyUp));

  // Create the palette cheat sheet, and update it when the palette changes.
  this.loadPalette();
  $('#themebuilder-style').bind('paletteChange', ThemeBuilder.bind(this, this.loadPalette));

  $('#advanced-update-button').click(ThemeBuilder.bind(this, this.updateButtonPressed));
  ThemeBuilder.addModificationHandler(ThemeBuilder.codeEditorModification.TYPE, this);
  this.loadAdvancedCss();
};

// Not used yet, in preparation for custom themed scrollbars.
ThemeBuilder.CodeEditor.prototype.keyUp = function (event) {
  var $ = jQuery;
  var $textarea = $('#themebuilder-advanced-css textarea');
  $textarea.css('height', '1px');
  $textarea.css('height', (String(25 + $textarea.attr('scrollHeight') + 'px')));
  $('#themebuilder-advanced-css .scrollpane').data('jsp').reinitialise();
};

/**
 * Causes the advanced.css file to be loaded, and its contents to be placed in
 * the editor.
 */
ThemeBuilder.CodeEditor.prototype.loadAdvancedCss = function () {
  var $ = jQuery;
  $.get(Drupal.settings.basePath + Drupal.settings.themeEditorPaths[Drupal.settings.currentTheme] + '/advanced.css',
    ThemeBuilder.bind(this, this.advancedCssLoaded));
};

/**
 * A callback function that puts the specified css text into the editor and
 * initializes the Modification instance for undo purposes.
 *
 * @param {String} cssText
 *   The css text to put into the editor.
 */
ThemeBuilder.CodeEditor.prototype.advancedCssLoaded = function (cssText) {
  var $ = jQuery;
  $('#themebuilder-advanced-css textarea').val(cssText);
  this.modifications.css = new ThemeBuilder.codeEditorModification('css');
  this.modifications.css.setPriorState(cssText);
};

/**
 * Handle the custom 'update' event.
 *
 * Note that we only trigger the update event when the user has deliberately
 * made a change to the textarea, by typing or clicking on a palette swatch.
 * It is not automatically triggered when the textarea value changes
 * programmatically (such as after the user clicks 'undo').
 *
 * @param {object} event
 *   The event that carries the new value in the text area.
 */
ThemeBuilder.CodeEditor.prototype.handleUpdate = function (event) {
  event.stopPropagation();
  this.modifications.css.setNewState(event.currentTarget.value);
  // Disable undo until the user has sent their changes to the server.
  if (this.isDirty()) {
    if (!this.statusKey) {
      this.statusKey = ThemeBuilder.undoButtons.disable();
    }
  }
  else if (this.statusKey) {
    ThemeBuilder.undoButtons.clear(this.statusKey);
    delete this.statusKey;
  }
  this.setUpdateButtonState();
  this.preview(this.modifications.css.getNewState());
};

/**
 * Updates the stylesheet to match the specified CSS document.
 *
 * @private
 * @param {string} cssText
 *   The text representing the CSS document to apply.
 * @param {object} stylesheet
 *   The stylesheet to which the modifications will be applied.
 */
ThemeBuilder.CodeEditor.prototype._updateStylesheet = function (cssText, stylesheet) {
  stylesheet.setCssText(cssText);
};

/**
 * Invoked when a different tab is selected.
 */
ThemeBuilder.CodeEditor.prototype.hide = function () {
  return this.select();
};

/**
 * Invoked when the Advanced tab is selected.  This function is used to cause
 * the advanced css text to be loaded and an appropriate modification object
 * initialized representing the advanced css prior state.
 */
ThemeBuilder.CodeEditor.prototype.show = function () {
  var $ = jQuery;
  $('#themebuilder-wrapper #themebuilder-advanced .palette-cheatsheet').removeClass('hidden');
  if ($('#themebuilder-wrapper #themebuilder-advanced .layout-cheatsheet')) {
    $('#themebuilder-wrapper #themebuilder-advanced .layout-cheatsheet').addClass('hidden');
  }
  this.setUpdateButtonState();
};

/**
 * Invoked when the Custom CSS subtab is clicked, before the panel is
 * shown.  This callback is used to check to see if the textarea is
 * dirty, and if so, prompt the user to save or lose changes.
 *
 * @return {Boolean}
 *   Always returns true, indicating it is ok to move off of the tab.
 */
ThemeBuilder.CodeEditor.prototype.select = function () {
  var updateChanges = false;
  var $ = jQuery;
  if (this.isDirty()) {
    updateChanges = confirm(Drupal.t('Would you like to commit your changes?'));
  }
  if (updateChanges) {
    this.updateButtonPressed();
  }
  else if (this.modifications.css) {
    // The user chose not to save the changes.  Revert the changes.
    this.preview(this.modifications.css.getPriorState());
    $('#themebuilder-advanced-css textarea').val(this.modifications.css.getPriorState().code);
  }
  if (this.statusKey) {
    // Allow undo and redo buttons to be used again.
    ThemeBuilder.undoButtons.clear(this.statusKey);
    delete this.statusKey;
  }
  this.setUpdateButtonState();
  return true;
};

/**
 * Preview the changes.
 *
 * @param {object} state
 *   The state of the modification to preview.
 * @param {Modification} modification
 *   Optional parameter that provides the modification object.  This will be
 *   provided when undoing or redoing a change.
 */
ThemeBuilder.CodeEditor.prototype.preview = function (state, modification) {
  // Replace non-breaking space with whitespace.
  state.code = state.code.replace(/\u00a0/g, ' ');

  if (this.panes[state.selector].editor.val() !== state.code) {
    this.panes[state.selector].changed = true;
    this.panes[state.selector].buffer = state.code;
  }

  this.updateStylesheet(state.code);

  // The modification needs to be reset if this is the result of an
  // undo.
  if (modification) {
    this.panes[state.selector].editor.val(state.code);
    this.modifications.css = new ThemeBuilder.codeEditorModification('css');
    this.modifications.css.setPriorState(state.code);
    this.modifications.css.setNewState(state.code);
    this.setUpdateButtonState();
  }
};

/**
 * Load the current palette colors into the palette cheatsheet.
 */
ThemeBuilder.CodeEditor.prototype.loadPalette = function () {
  var $ = jQuery;
  var colorManager = ThemeBuilder.getColorManager();
  if (!colorManager.isInitialized()) {
    // Cannot initialize yet.  We need the color manager to be fully initialized
    // first.
    setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this, this.loadPalette), 50);
    return;
  }
  this.addColorSwatches($('#themebuilder-advanced .palette-cheatsheet table.palette-colors'), colorManager.getPalette().mainColors, 'palette-swatch-');
  this.addColorSwatches($('#themebuilder-advanced .palette-cheatsheet table.custom-colors'), colorManager.getCustom().colors, 'custom-swatch-');
};

/**
 * Adds the specified color swatches to the specified table.
 *
 * @param {jQuery} $table
 *   The jQuery object representing the table to add the color swatches to.
 * @param {Object} colors
 *   An object containing the set of colors to add.
 * @param {String} classPrefix
 *   The prefix to use for the classname for each of the color items.
 */
ThemeBuilder.CodeEditor.prototype.addColorSwatches = function ($table, colors, classPrefix) {
  var $ = jQuery;
  $table.html('');
  var current_row = $('<tr></tr>').appendTo($table);
  var i = 0;
  var key, td, hex;
  for (key in colors) {
    if (typeof(key) === 'string') {
      // Create a table cell with a palette color swatch in it.
      td = $('<td class="index"><div class="color-swatch palette-swatch-' + i + '" style="background-color:#' + colors[key].hex + '"></div><div class="color-swatch-label ' + classPrefix + i + '">#' + colors[key].hex + '</div></td>\n');
      td.appendTo(current_row);
/*
      // Update the textarea when the palette swatch is clicked.
      hex = '#' + colors[key].hex;
      $('.' + classPrefix + i).bind('click', ThemeBuilder.bindIgnoreCallerArgs(this, this.insertText, hex, 'css'));
*/
      // Add another table row if necessary.
      if (i % 2 !== 0) {
        current_row = $('<tr></tr>').appendTo($table);
      }
      i++;
    }
  }
};

/**
 * Insert text into one of the editPane instances.
 *
 * @param {string} text
 *   The text to be inserted.
 * @param {string} type
 *   The type of editPane (e.g. 'css') into which to insert the text.
 */
ThemeBuilder.CodeEditor.prototype.insertText = function (text, type) {
  if (this.panes[type]) {
    var pane = this.panes[type];
    pane.insertAtCursor(text);
    pane.editor.trigger('update');
  }
};

/**
 * Invoked when the update button is clicked.  This method will commit any
 * changes to the server.
 */
ThemeBuilder.CodeEditor.prototype.updateButtonPressed = function () {
  var prior = this.modifications.css.getPriorState().code;
  var next = this.modifications.css.getNewState().code;
  if (prior !== next) {
    ThemeBuilder.applyModification(this.modifications.css);
    this.modifications.css = this.modifications.css.getFreshModification();
    this.modifications.css.setNewState(next);
  }
  if (this.statusKey) {
    // The user just committed a change.  It is ok for them to undo that
    // change now.  You cannot undo during active edit though.  Only when
    // there are no uncommitted changes.
    ThemeBuilder.undoButtons.clear(this.statusKey);
    delete this.statusKey;
  }
  this.setUpdateButtonState();
};

/**
 * Enables or disables the update button depending on if the editor contents
 * have been changed.
 */
ThemeBuilder.CodeEditor.prototype.setUpdateButtonState = function () {
  var $ = jQuery;
  if (this.isDirty()) {
    $('#advanced-update-button').removeClass('disabled');
  }
  else {
    $('#advanced-update-button').addClass('disabled');
  }
};

/**
 * Indicates if the current editor contents differ from the last saved contents.
 * This is useful in knowing whether the undo / redo buttons should be enabled
 * or if we should hassle the user about updating before traversing away from
 * the advanced editor tab.
 *
 * @return {boolean}
 *   True if the editor contents have not been committed to the server; false
 *   otherwise.
 */
ThemeBuilder.CodeEditor.prototype.isDirty = function () {
  if (this.modifications.css) {
    var $ = jQuery;

    var textAreaValue = $('#themebuilder-advanced-css textarea').val();
    textAreaValue = this._standardizeLineEndings(textAreaValue);

    var priorState = this.modifications.css.getPriorState().code;
    priorState = this._standardizeLineEndings(priorState);

    return textAreaValue !== priorState;
  }
  return false;
};

/**
 * Convert all line endings in the given text to Unix newlines (\n).
 *
 * @private
 * @param {string} text
 *   The text to standardize.
 *
 * @return {string}
 *   The text with Unix line endings.
 */
ThemeBuilder.CodeEditor.prototype._standardizeLineEndings = function (text) {
  // Windows line endings are carriage return + line feed (\r\n). See if
  // a carriage return exists in the text.
  var textAreaValueIndex = text.indexOf('\r');
  // Convert Windows line endings to Unix line endings (\n) by removing any
  // carriage returns.
  if (textAreaValueIndex !== -1) {
    var strReplace = text;
    while (textAreaValueIndex !== -1) {
      strReplace = strReplace.replace('\r', '');
      textAreaValueIndex = strReplace.indexOf('\r');
    }
    text = strReplace;
  }
  return text;
};
