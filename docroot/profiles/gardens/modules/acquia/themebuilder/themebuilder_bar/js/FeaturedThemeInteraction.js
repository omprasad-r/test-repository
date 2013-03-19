/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

ThemeBuilder.themes = ThemeBuilder.themes || {};

/**
 * @class
 * @extends ThemeBuilder.InteractionController
 */
ThemeBuilder.themes.FeaturedThemeInteraction = ThemeBuilder.initClass();
ThemeBuilder.themes.FeaturedThemeInteraction.prototype = new ThemeBuilder.InteractionController();

/**
 * Constructor for the FeaturedThemeInteraction.
 */
ThemeBuilder.themes.FeaturedThemeInteraction.prototype.initialize = function (callbacks, data) {
  var $ = jQuery;

  this.setInteractionTable({
    // Show the name dialog
    begin: 'setState',
    ready: 'showModalDialog',
    themeAccepted: 'saveTheme',
    dialogCanceled: 'cancelDialog',
    done: 'hideModalDialog'
  });
  // Set the callbacks
  this.setCallbacks(callbacks);
  // Make UI elements
  this.speed = 250;
  this.ui = {};
  // Store the panels
  this.ui.panels = {
    mythemes: $('#themebuilder-themes-mythemes'),
    featured: $('#themebuilder-themes-featured')
  };
  // Store the actions
  this.ui.actions = {
    pointer: $('#themebuilder-themes-actions')
  };
  var appData = ThemeBuilder.getApplicationInstance().getData();
  this.selectedTheme = appData.selectedTheme;
  this.currentCustomTheme = appData.selectedTheme;
};

ThemeBuilder.themes.FeaturedThemeInteraction.prototype.setCustomTheme = function (theme) {
  this.currentCustomTheme = theme;
};

/**
 * Hides the modal dialog and brings the mythemes carousel back
 */
ThemeBuilder.themes.FeaturedThemeInteraction.prototype.hideModalDialog = function (event) {
  if (this.ui.modal) {
    this.ui.modal.pointer.remove();
    delete this.ui.modal;
  }
  this.ui.panels.featured.smartToggle('hide', {speed: this.speed});
  this.ui.actions.pointer.smartToggle('show', {speed: this.speed});
  this.ui.panels.mythemes.smartToggle('show', {speed: this.speed});
};

/**
 * Adds the featuredTheme state information to the application
 */
ThemeBuilder.themes.FeaturedThemeInteraction.prototype.setState = function (event) {
  var bar = ThemeBuilder.Bar.getInstance();
  var obj = bar.getTabInfo();
  var customState = {
    interactions: {
      featuredTheme: 'showModalDialog'
    },
    currentCustomTheme: this.currentCustomTheme
  };
  bar.saveState(obj.id, customState);
  this.event({event: event}, 'ready');
};

/**
 * Removes the featuredTheme state information from the application.
 *
 * @param {function} callback
 *   A function to call when the state has been successfully saved.
 */
ThemeBuilder.themes.FeaturedThemeInteraction.prototype.removeState = function (callback) {
  var bar = ThemeBuilder.Bar.getInstance();
  var obj = bar.getTabInfo();
  bar.saveState(obj.id, {}, callback);
};

/**
 * Handle the Cancel button on the modal.
 */
ThemeBuilder.themes.FeaturedThemeInteraction.prototype.cancelDialog = function (event) {
  if (this.selectedTheme !== this.currentCustomTheme) {
    this.removeState(ThemeBuilder.bind(this, this.switchToCustomTheme));
  }
  else {
    this.removeState();
    this.event({event: event}, 'done');
  }
};

/**
 * Switch to the custom theme that was open before the modal.
 */
ThemeBuilder.themes.FeaturedThemeInteraction.prototype.switchToCustomTheme = function () {
  ThemeBuilder.themeSelector.switchTheme(this.currentCustomTheme);
};

/**
 * Shows the featured theme selction modal dialog and the featured theme carousel
 * 
 * @param {Object} data
 *   An optional object that may have fields that customize the
 *   creation of the dialog.
 */
ThemeBuilder.themes.FeaturedThemeInteraction.prototype.showModalDialog = function (event) {
  if (!this.ui.modal) {
    this._buildModal();
  }
  this.ui.actions.pointer.smartToggle('hide', {speed: this.speed});
  this.ui.panels.mythemes.smartToggle('hide', {speed: this.speed});
  this.ui.modal.pointer.smartToggle('show', {speed: this.speed});
  this.ui.panels.featured.smartToggle('show', {speed: this.speed});
  
};

/**
 * Builds the HTML for the Cancel/Choose featured themes modal interaction flow.
 */
ThemeBuilder.themes.FeaturedThemeInteraction.prototype._buildModal = function () {
  var $ = jQuery;
  // Modal buttons
  // Check if the current active theme is the published theme.
  // Disabled the save button if it is
  var saveLinkClasses = ['themebuilder-button', 'primary'];
  var app = ThemeBuilder.getApplicationInstance();
  var settings = app.applicationData;
  if (settings.published_theme === settings.selectedTheme) {
    saveLinkClasses.push('disabled');
  }
  var buttons = new ThemeBuilder.ui.ActionList(
    {
      wrapper: {
        classes: ['horizontal']
      },
      actions: [
        {
          label: Drupal.t('Cancel'),
          action: this.makeEventCallback('dialogCanceled'),
          linkClasses: ['themebuilder-button']
        },
        {
          label: Drupal.t('Choose'),
          action: this.makeEventCallback('themeAccepted'),
          linkClasses: saveLinkClasses
        }
      ]
    }
  );
  // Add the buttons to the wrapper
  var $modal = $('<div>', {
    id: 'themebuilder-themes-featured-modal'
  }).prependTo($('#themebuilder-main'));
  
  $modal.append(
    $('<p>', {
      html: Drupal.t('Choose a theme. They can be customized later.')
    }),
    buttons.getPointer()
  );
    
  // Store the ui elements in this instance
  this.ui.modal = {
    obj: buttons,
    pointer: $('#themebuilder-themes-featured-modal')
  };
};

ThemeBuilder.themes.FeaturedThemeInteraction.prototype.saveTheme = function (event) {
  ThemeBuilder.Bar.getInstance().save();
  this.removeState();
  this.event({event: event}, 'done');
};

/**
 * A static method that puts the FeaturedThemeInteraction in the ready state and skips the begin state.
 *
 * This is invoked after a page refresh when the modal interaction is still valid, but we don't need to
 * initialize it again.
 */
ThemeBuilder.themes.FeaturedThemeInteraction.invoke = function () {
  var invoke = false;
  var customTheme;
  var state = ThemeBuilder.Bar.getInstance().getSavedState();
  if (state.info && state.info.interactions) {
    var interactions = state.info.interactions;
    for (var i in interactions) {
      if (interactions.hasOwnProperty(i)) {
        // We might eventually want a map of available interactions. For the moment,
        // we'll just call up the ones we know about directly.
        if (i === 'featuredTheme') {
          invoke = (interactions[i] === 'showModalDialog');
        }
      }
    }
    customTheme = state.info.currentCustomTheme;
  }
  if (invoke) {
    var modal = new ThemeBuilder.themes.FeaturedThemeInteraction();
    if (customTheme) {
      modal.setCustomTheme(customTheme);
    }
    modal.start();
  }
};
