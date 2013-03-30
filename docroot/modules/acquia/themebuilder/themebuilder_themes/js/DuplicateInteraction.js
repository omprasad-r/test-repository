/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

ThemeBuilder.themes = ThemeBuilder.themes || {};

/**
 * The DuplicateInteraction class manages the entire interaction with the user and asyncronous calls to accomplish the duplication of a theme.
 * 
 * @class
 * @extends ThemeBuilder.InteractionController
 */
ThemeBuilder.themes.DuplicateInteraction = ThemeBuilder.initClass();
ThemeBuilder.themes.DuplicateInteraction.prototype = new ThemeBuilder.InteractionController();

/**
 * Constructor for the DuplicateInteraction.
 * 
 * @param {Theme} theme
 *   The theme to be duplicated.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype.initialize = function (theme, callbacks) {
  this.setInteractionTable({
    // Show the name dialog
    begin: 'showNameDialog',
    nameAccepted: 'verifyName',
    nameCanceled: 'cancel',

    // Verify the theme name
    noNameProvided: 'throwNoEntryError',
    nameAlreadyUsed: 'showOverwriteDialog',
    nameOk: 'ready',

    // Theme already exists
    overwriteTheme: 'ready',
    doNotOverwrite: 'showNameDialog',

    // Duplicate the theme
    ready: 'duplicateTheme',
    duplicateSuccess: 'showSuccess',
    duplicateFailed: 'showFailure'
  });
  this.setCallbacks(callbacks);
  this.theme = theme;

  // We need an indirection method to get to the ready state.
  this.ready = this.makeEventCallback('ready');
};

/**
 * Preserves the specified data, which originates from the theme name dialog.
 * 
 * @private
 * This is necessary because there are potentially two dialogs in this
 * interaction.  After confirming a theme overwrite no data is passed
 * back.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype._preserveData = function (data) {
  this.data = data;
};

/**
 * Retrieves the preserved data.
 * 
 * @private
 * @return {Object} data
 *   The save data.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype._getPreservedData = function () {
  return this.data;
};

/**
 * Applies a limit to the length of the input text
 * 
 * @private
 * @param {Event} event
 *   The event that this function handles
 * @param {HTML Object} field
 *   A DOM field.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype._limitInput = function (field) {
  var $ = jQuery;
  var max = 25;
  // If this method is called by an event, field will be an event
  field = (field.target) ? field.target : field;
  var $field = $(field);
  if ($field.length > 0) {
    // Trim the text down to max if it exceeds
    // The delay is necessary to allow time for the paste action to complete
    setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this, this._trimField, $field, max), 200);
  }
};

/**
 * Trims a field's value down to the max
 * 
 * @private
 * @param {jQuery Object} $field
 *   The HTML field to be trimmed.
 * @param {int} max
 *   The maximum number of characters allowed in this field.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype._trimField = function ($field, max) {
  var value = $field.val();
  if (value.length > max) {
    $field.val(value.substr(0, max));
  }
  // Keydown is called to kick the NobleCounter plugin to refresh
  $field.keydown();
};

/**
 * Applies the NobleCount plugin to the supplied field
 * 
 * @private
 * @param {HTML Object} field
 *   A DOM field.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype._enableLiveInputLimit = function (field) {
  var $ = jQuery;
  var max = 25;
  var $field = $(field);
  if ($field.length > 0) {
    // Add the NobleCount input limiter
    $('<span>', {
      id: 'char-count'
    }).insertAfter($field);
    $field.NobleCount('#char-count', {
      max_chars: max,
      block_negative: true
    });
  }
};

/**
 * A helper function that creates the dialog that collects the new theme name
 * 
 * @private
 * @param {jQuery Object} html
 *   The html to be rendered in the dialog.
 * @param {Object} data
 *   An optional object that may have fields that customize the
 *   creation of the dialog.
 * @return {Object} dialog
 *   A reference to the dialog instance.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype._buildNameDialog = function (html, data) {
  var dialog = new ThemeBuilder.ui.Dialog(jQuery('#themebuilder-wrapper'),
    {
      html: html,
      buttons: [
        {
          label: Drupal.t('Copy theme'),
          action: this.makeEventCallback('nameAccepted')
        },
        {
          label: Drupal.t('Cancel'),
          action: this.makeEventCallback('nameCanceled', data)
        }
      ]
    }
  );
  // Limit the name field input
  this._enableLiveInputLimit('#duplicate-theme-name');
  this._limitInput('#duplicate-theme-name');
  
  return dialog;
};

/**
 * Shows the theme name dialog
 * 
 * @param {Object} data
 *   An optional object that may have fields that customize the
 *   creation of the dialog.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype.showNameDialog = function (data) {
  var $ = jQuery;

  // This markup will be displayed in the dialog.
  var inputId = 'duplicate-theme-name';
  var name = data && data.name ? data.name : '';
  var $html = $('<form>').append(
    $('<label>', {
      html: Drupal.t('Theme name:')
    }).attr('for', inputId),
    $('<input>', {
      name: "name",
      id: inputId,
      value: name
    }).bind('paste', ThemeBuilder.bind(this, this._limitInput, inputId))
  );
  this._buildNameDialog($html, data);
};

/**
 * Verifies the theme name.
 * 
 * @param {Object} data
 *   An object containing the original theme and the new theme.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype.verifyName = function (data) {
  this._preserveData(data);
  if (data.name.length === 0) {
    this.event(data, 'noNameProvided');
    return;
  }
  var sysName = ThemeBuilder.util.themeLabelToName(data.name);
  // Check for a blank entry
  // Check to see if the name is already in use.
  var theme = ThemeBuilder.Theme.getTheme(sysName);
  if (theme) {
    // The theme exists.
    this.event(data, 'nameAlreadyUsed');
    return;
  }
  this.event(data, 'nameOk');
};

/**
 * Creates and displays the Overwrite theme confirmation dialog
 * 
 * @param {Object} data
 *   An object containing the original theme and the new theme.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype.showOverwriteDialog = function (data) {
  var $ = jQuery;
  var dialog = new ThemeBuilder.ui.Dialog($('#themebuilder-wrapper'),
    {
      html: Drupal.t('The theme %theme already exists.  Would you like to overwrite the existing theme?', {'%theme': data.name}),
      buttons: [
        {
          label: Drupal.t('Yes'),
          action: this.makeEventCallback('overwriteTheme', data)
        },
        {
          label: Drupal.t('No'),
          action: this.makeEventCallback('doNotOverwrite', data)
        }
      ]
    }
  );
};

/**
 * Causes the theme duplication to occur.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype.duplicateTheme = function () {
  var data = this._getPreservedData();
  var sysName = ThemeBuilder.util.themeLabelToName(data.name);
  var actionCallbacks = {
    success: this.makeEventCallback('duplicateSuccess'),
    fail: this.makeEventCallback('duplicateFailed')
  };
  ThemeBuilder.Bar.getInstance().showWaitIndicator();
  this.theme.copyTheme(data.name, sysName, actionCallbacks);
};

/**
 * Called when the noNameProvided event is fired. This catches the case where a 
 * user tries to save a theme without providing a theme name.
 *
 * @param {Object} data
 *    An object containing the original theme and the new theme.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype.throwNoEntryError = function (data) {
  var $ = jQuery;
  // This markup will be displayed in the dialog.
  var inputId = 'duplicate-theme-name';
  var name = data && data.name ? data.name : '';
  var $html = $('<form>').append(
    $('<div>', {
      html: Drupal.t('Please enter a name.')
    }).addClass('ui-state-error-text block'),
    $('<label>', {
      html: Drupal.t('Theme name:')
    }).attr('for', inputId),
    $('<input>', {
      name: "name",
      id: inputId,
      value: name,
      change: ThemeBuilder.bind(this, this._limitInput, '#duplicate-theme-name')
    }).addClass('ui-state-error')
  );
  this._buildNameDialog($html, data);
};

/**
 * The callback for a successful theme duplication.
 *
 * @param {Object} data
 *    An object containing the original theme and the new theme.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype.showSuccess = function (data) {
  var bar = ThemeBuilder.Bar.getInstance();
  bar.hideWaitIndicator();
  bar.setStatus(Drupal.t('%theme has been duplicated.', {'%theme': data.originalTheme.getName()}));
  this.event(data, 'interactionDone');
};

/**
 * The callback for a failed theme duplication.
 *
 * @param {Object} data
 *    An object containing the original theme and the new theme name.
 */
ThemeBuilder.themes.DuplicateInteraction.prototype.showFailure = function (data) {
  var bar = ThemeBuilder.Bar.getInstance();
  bar.hideWaitIndicator();
  bar.setStatus(Drupal.t('Failed to duplicate theme %theme as %newTheme.', {'%theme': data.originalTheme.getName(), '%newTheme': data.newName}));
  this.event(data, 'interactionFailed');
};
