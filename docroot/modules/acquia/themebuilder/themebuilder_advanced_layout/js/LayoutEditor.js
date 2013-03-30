/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true window: true*/

var ThemeBuilder = ThemeBuilder || {};

/**
 * Singleton class that manages the advanced layout subtab.  This class
 * is a singleton because it manages a particular textarea in the DOM
 * and is written in such a way that there can only be one such
 * textarea.
 * @class
 */
ThemeBuilder.AdvancedLayoutEditor = ThemeBuilder.initClass();

/**
 * Static method to retrieve the singleton instance of the AdvancedLayoutEditor.
 *
 * @return
 *   The ThemeBuilder.AdvancedLayoutEditor instance.
 */
ThemeBuilder.AdvancedLayoutEditor.getInstance = function () {
  if (!ThemeBuilder.AdvancedLayoutEditor._instance) {
    ThemeBuilder.AdvancedLayoutEditor._instance = new ThemeBuilder.AdvancedLayoutEditor();
  }
  return ThemeBuilder.AdvancedLayoutEditor._instance;
};

/**
 * Constructor for the ThemeBuilder.AdvancedLayoutEditor class.  This
 * constructor should not be called directly, but instead the
 * getInstance static method should be used.
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.initialize = function () {
  if (ThemeBuilder.AdvancedLayoutEditor._instance) {
    throw "ThemeBuilder.AdvancedLayoutEditor is a singleton that has already been instantiated.";
  }
  var $ = jQuery;
  this.panes = {};
  this.modifications = {};
  this.history = ThemeBuilder.History.getInstance();
};

/**
 * Initializes the UI of the advanced tab. Retrieves layout information from the
 * server, initializes the textarea, and sets up event handlers.
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.init = function () {
  var $ = jQuery;
  this.panes.layout = new ThemeBuilder.EditPane($('#themebuilder-advanced-layout textarea'), this);

  // Handle the custom 'update' jQuery event.
  this.panes.layout.editor.bind('update', ThemeBuilder.bind(this, this.handleUpdate));

  $('#advanced-layout-update-button').click(ThemeBuilder.bind(this, this.updateButtonPressed));
  ThemeBuilder.addModificationHandler(ThemeBuilder.layoutEditorModification.TYPE, this);
  this.loadAdvancedLayout();
};

/**
 * Causes the advanced layout information to be loaded, and its contents to be
 * placed in the editor.
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.loadAdvancedLayout = function () {
  var $ = jQuery;
  $.get(Drupal.settings.basePath + 'themebuilder-advanced-layout-load',
    ThemeBuilder.bind(this, this.advancedLayoutLoaded));
};

/**
 * A callback function that puts the specified layout text into the editor and
 * initializes the Modification instance for undo purposes.
 *
 * @param {String} layoutText
 *   The layout text to put into the editor.
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.advancedLayoutLoaded = function (layoutText) {
  var $ = jQuery;
  $('#themebuilder-advanced-layout textarea').val(layoutText);
  // This needs to be a code editor.
  this.modifications.layout = new ThemeBuilder.codeEditorModification('layout');
  this.modifications.layout.setPriorState(layoutText);
};

/**
 * Handle the custom 'update' event.
 *
 * Note that we only trigger the update event when the user has deliberately
 * made a change to the textarea, by typing. It is not automatically triggered
 * when the textarea value changes programmatically (such as after the user
 * clicks 'undo').
 *
 * @param {object} event
 *   The event that carries the new value in the text area.
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.handleUpdate = function (event) {
  var $ = jQuery;
  event.stopPropagation();
  this.modifications.layout.setNewState(event.currentTarget.value);
  // @TODO Undo is disabled. Disable undo until the user navigates away from this tab.
  //this.statusKey = ThemeBuilder.undoButtons.disable();
  // @TODO We're creating a new control veil...bad, bad, bad.
  if ($('#themebuilder-temp-veil').length === 0) {
    $('<div>', {
      id: 'themebuilder-temp-veil'
    })
    .css({
      position: 'absolute',
      top: 0,
      left: 0,
      height: '34px',
      width: '74px',
      'z-index': 500
    })
    .prependTo($('#themebuilder-wrapper #themebuilder-save'))
    .parent()
    .css({
      position: 'relative'
    });
  }
  this.setUpdateButtonState();
  // @TODO Preview is disabled while this feature is in development.
  // this.preview(this.parseModifications());
};

/**
 * Required callback - called when this subtab is deselected
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.hide = function() {
  return this.select();
}

/**
 * Required callback - called when this subtab is selected
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.show = function() {
  var $ = jQuery;
  $('#themebuilder-wrapper #themebuilder-advanced .palette-cheatsheet').addClass('hidden');
  $('#themebuilder-wrapper #themebuilder-advanced .layout-cheatsheet').removeClass('hidden');
  this.setUpdateButtonState();
}

/**
 * Invoked when another subtab is clicked, before the panel is
 * shown. This callback is used to check to see if the textarea is
 * dirty, and if so, prompt the user to save or lose changes.
 *
 * @return {Boolean}
 *   Always returns true, indicating it is ok to move off of the tab.
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.select = function () {
  var updateChanges = false;
  var $ = jQuery;
  if (this.isDirty()) {
    updateChanges = confirm(Drupal.t('Would you like to commit your changes?'));
  }
  if (updateChanges) {
    this.updateButtonPressed();
  }
  else if (this.modifications.layout) {
    // The user chose not to save the changes.  Revert the changes.
    // @TODO Preview is disabled in this dev release.
    //this.preview(this.modifications.layout.getPriorState());
    $('#themebuilder-advanced-layout textarea').val(this.modifications.layout.getPriorState().code);
  }

  // @TODO We're directly manipulating the control veil...bad, bad, bad.
  $('#themebuilder-temp-veil').remove();

  this.setUpdateButtonState();
  return true;
};

/**
 * @TODO Preview is disabled while this module is under development.
 *
 * Applies the specified modification description to the client side only.
 * This allows the user to preview the modification without committing it
 * to the theme.
 *
 * @param {Object} desc
 *   The modification description.  To get this value, you should pass in
 *   the result of Modification.getNewState() or Modification.getPriorState().
 * @param {Modification} modification
 *   The modification that represents the change in the current state that
 *   should be previewed.
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.preview = function (state, modification) {
  // Deal with
//  var $ = jQuery;
//  // Get the layout name...
//  var name = this.getPageLayoutName();
//  var layoutClass = this.layoutNameToClass(name);
//  var newName = desc.layout.split('body-layout-')[1];
//
//  // Highlight the appropriate image in the layout selector.
//  var screenshot = $('#themebuilder-main .layout-' + newName);
//  var scope = desc.selector === '<global>' ? 'all' : 'single';
//  $('#themebuilder-main .layout-shot.' + scope).removeClass(scope);
//  if (desc.selector === this.currentPage) {
//    $('#themebuilder-main .layout-shot.single').removeClass('single');
//    screenshot.addClass('single');
//    Drupal.settings.layoutIndex = newName;
//  }
//  else if (desc.selector === '<global>') {
//    $('#themebuilder-main .layout-shot.all').removeClass('all');
//    screenshot.addClass('all');
//    Drupal.settings.layoutGlobal = newName;
//  }
//  else {
//    //not handling yet
//  }
//
//  // Fix the body class to set the new layout.
//  $('body', parent.document).removeClass(layoutClass);
//  if (desc.layout) {
//    $('body', parent.document).addClass(desc.layout);
//    this.shuffleRegionMarkup(this.classToLayoutName(desc.layout));
//  }
//  else {
//    $('body', parent.document).addClass(this.layoutNameToClass(Drupal.settings.layoutGlobal));
//    this.shuffleRegionMarkup(Drupal.settings.layoutGlobal);
//  }
};

/**
 * Invoked when the update button is clicked.  This method will commit any
 * changes to the server.
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.updateButtonPressed = function () {
  var prior = this.modifications.layout.getPriorState().code;
  var next = this.modifications.layout.getNewState().code;
  if (prior !== next) {
    // Break the text into individual modifications of a group so they can be
    // applied to the in discrete units through $theme->setLayout()
    var modifications = this.parseModifications();
    ThemeBuilder.applyModification(modifications);
    this.modifications.layout = this.modifications.layout.getFreshModification();
    this.modifications.layout.setNewState(next);
  }
  // @TODO Undo is disabled while this module is under development
  // this.statusKey = ThemeBuilder.undoButtons.disable();
  this.setUpdateButtonState();
};

/**
 * Enables or disables the update button depending on if the editor contents
 * have been changed.
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.setUpdateButtonState = function () {
  var $ = jQuery;
  if (this.isDirty()) {
    $('#advanced-layout-update-button').removeClass('disabled');
  }
  else {
    $('#advanced-layout-update-button').addClass('disabled');
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
ThemeBuilder.AdvancedLayoutEditor.prototype.isDirty = function () {
  if (this.modifications.layout) {
    var $ = jQuery;

    var textAreaValue = $('#themebuilder-advanced-layout textarea').val();
    textAreaValue = this._standardizeLineEndings(textAreaValue);

    var priorState = this.modifications.layout.getPriorState().code;
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
ThemeBuilder.AdvancedLayoutEditor.prototype._standardizeLineEndings = function (text) {
  // @TODO This check for text is hiding an error and should be investigated
  // @TODO (AE) - think we can shorten this function, just needs replace(/\r/g, ...)? test and replace ...
  if (text) {
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
  }
  return text;
};

/**
 * Parse textarea contents from the states of the advanced layout editor into
 * a GroupedModification object that can be passed to
 * ThemeBuilder.applyModification. This function returns the delta of changes
 * to be applied to the theme's layout configuration.
 *
 * * @return {object}
 *   A GroupedModification object.
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.parseModifications = function () {
  // Parse the hunk of text from the textarea into individual modifications
  var modifications = this.parseCode(this.modifications.layout.newState.code),
  previous = this.parseCode(this.modifications.layout.priorState.code),
  group = new ThemeBuilder.GroupedModification(),
  count = 0;
  // Go through each selector in the modifications.
  for (var selector in modifications) {
    if (modifications.hasOwnProperty(selector)) {
      // Check if the previous state has this layout.
      var oldLayout = previous.hasOwnProperty(selector) ? previous[selector] : null,
      newLayout = modifications[selector],
      mod = new ThemeBuilder.layoutEditorModification(selector);
      // Remove the objects from the modifications and previous lists.
      delete modifications[selector];
      delete previous[selector];
      // If the new and old state have the same selector and the layout is the same, ignore it.
      // The selector is the page pattern, the state is the CSS columns class
      if (oldLayout === newLayout) {
        continue;
      }
      // If the new state has a selector that the previous state does not have, add it.
      // If the new and old state have the same selector, but the layout is different,
      // change it.
      mod.setPriorState(oldLayout);
      mod.setNewState(newLayout);
      group.addChild('layout-' + count, mod);
      count += 1;
    }
  }
  // If the new state is missing a selector that the previous state has, remove it.
  for (var selector in previous) {
    if (previous.hasOwnProperty(selector)) {
      var mod = new ThemeBuilder.layoutEditorModification(selector);
      mod.setPriorState(previous[selector]);
      // Passing in a blank class will cause the theme to delete the layout.
      mod.setNewState('');
      group.addChild('layout-' + count, mod);
      count += 1;
    }
  }
  return group;
}
/**
 * Parses the contents of the advanced layout editor textarea into an object whose
 * properties are the selectors, and values are the layout classes.
 *
 * @param code
 *   Raw contents of the textarea.
 * @return
 *   Object resulting from parsing the code text in the form:
 *   {
 *     "selector" : "body layout class name",
 *     ...
 *   }
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.parseCode = function(code) {
  var $ = jQuery,
    parsed = {},
    lines = this._standardizeLineEndings(code.split('\n'));

  for (var line in lines) {
    if (lines.hasOwnProperty(line)) {
      // Discard lines that only contain comments
      var comment = lines[line].split(';');
      if (comment.length > 1 && comment[0].match(/^\s*$/) !== null) {
        continue;
      }

      var rule = comment[0].split(':');
      // Discard lines that contain no colon
      if (rule.length == 0) {
        continue;
      }

      var selector = $.trim(rule[0]);
      var layout = $.trim(rule[1]);

      // Discard lines whose selector is empty
      if (selector.length > 0) {
        parsed[selector] = this.layoutNameToClass(layout);
      }
    }
  }

  return parsed;
}

/**
 * Provides the class name associated with the specified layout.  The class
 * can be attached to the body tag in the document to cause the specified
 * layout to be realized.
 *
 * @param {String} layoutName
 *   The name of the layout.
 *
 * @return {String}
 *   The class name corresponding to the specified layout.
 */
ThemeBuilder.AdvancedLayoutEditor.prototype.layoutNameToClass = function (layoutName) {
  var result = '';
  if (layoutName) {
    result = 'body-layout-' + layoutName.toLowerCase();
  }
  return result;
};


// @TODO - find the proper place add our subtab.
jQuery(function() {ThemeBuilder.AdvancedTab.getInstance().subtabs.push({id: 'themebuilder-advanced-layout',
    obj: ThemeBuilder.AdvancedLayoutEditor.getInstance()})});
