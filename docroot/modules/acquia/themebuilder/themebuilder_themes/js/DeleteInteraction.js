/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

/**
 * @namespace
 */
ThemeBuilder.themes = ThemeBuilder.themes || {};

/**
 * The DeleteInteraction controler manages the entire interaction
 * pertaining to deleting themes.
 * @class
 * @extends ThemeBuilder.InteractionController
 */
ThemeBuilder.themes.DeleteInteraction = ThemeBuilder.initClass();
ThemeBuilder.themes.DeleteInteraction.prototype = new ThemeBuilder.InteractionController();

/**
 * Constructor for the DeleteInteraction.
 * 
 * @param {Theme} theme
 *   The theme to be deleted.
 */
ThemeBuilder.themes.DeleteInteraction.prototype.initialize = function (theme, callbacks) {
  this.setInteractionTable({
    // Show the name dialog
    begin: 'showDeleteDialog',
    deleteAccepted: 'ready',
    deleteCanceled: 'cancel',

    // Delete the theme
    ready: 'deleteTheme',
    deleteSuccess: 'showSuccess',
    deleteFailed: 'showFailure'
  });
  this.setCallbacks(callbacks);
  this.theme = theme;

  // We need an indirection method to get to the ready state.
  this.ready = this.makeEventCallback('ready');
};

/**
 * Shows the theme delete dialog
 * 
 * @param {Object} data
 *   An optional object that may have fields that customize the
 *   creation of the dialog.
 */
ThemeBuilder.themes.DeleteInteraction.prototype.showDeleteDialog = function (data) {
  
  var $ = jQuery;
  // This markup will be displayed in the dialog.
  var inputId = 'delete-theme-name';
  var name = data && data.name ? data.name : '';
  var $html = $('<div>').append(
    $('<img>', {
      src: Drupal.settings.themebuilderAlertImage
    }).addClass('alert-icon'),
    $('<span>', {
      html: Drupal.t('Deleting a theme cannot be undone. Are you sure you want to delete %theme?', {'%theme': this.theme.getName()})
    })
  );
  var dialog = new ThemeBuilder.ui.Dialog(jQuery('#themebuilder-wrapper'),
    {
      html: $html,
      buttons: [
        {
          label: Drupal.t('Delete theme'),
          action: this.makeEventCallback('deleteAccepted', data)
        },
        {
          label: Drupal.t('Cancel'),
          action: this.makeEventCallback('deleteCanceled', data)
        }
      ]
    }
  );
};

/**
 * Causes the theme deletion to occur.
 */
ThemeBuilder.themes.DeleteInteraction.prototype.deleteTheme = function () {
  var actionCallbacks = {
    success: this.makeEventCallback('deleteSuccess'),
    fail: this.makeEventCallback('deleteFailed')
  };
  ThemeBuilder.Bar.getInstance().showWaitIndicator();
  this.theme.deleteTheme(actionCallbacks);
};

/**
 * The callback for a successful theme duplication.
 *
 * @param {Object} data
 *    An object containing the original theme and the new theme.
 */
ThemeBuilder.themes.DeleteInteraction.prototype.showSuccess = function (data) {
  var bar = ThemeBuilder.Bar.getInstance();
  bar.hideWaitIndicator();
  bar.setStatus(Drupal.t('%theme has been deleted', {'%theme': this.theme.getName()}));
  this.event(data, 'interactionDone');
};

/**
 * The callback for a failed theme duplication.
 *
 * @param {Object} data
 *    An object containing the original theme and the new theme name.
 */
ThemeBuilder.themes.DeleteInteraction.prototype.showFailure = function (data) {
  var bar = ThemeBuilder.Bar.getInstance();
  bar.hideWaitIndicator();
  bar.setStatus(Drupal.t('Failed to delete theme %theme', {'%theme': this.theme.getName()}));
  this.event(data, 'interactionFailed');
};
