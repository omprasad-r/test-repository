/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true debug: true Drupal: true window: true */

var ThemeBuilder = ThemeBuilder || {};
ThemeBuilder.styleEditor = ThemeBuilder.styleEditor || {};

/**
 * @class
 */
Drupal.behaviors.themebuilderBarLast = {
  attach: function (context, settings) {
    ThemeBuilder.initializeUI();
  }
};

/**
 * Initializes the initial state of the themebuilder.
 */
ThemeBuilder.initializeUI = function () {
  ThemeBuilder.addThemeEditButton();

  if (jQuery('body').hasClass('themebuilder')) {
    ThemeBuilder.open();
  }
  else {
    ThemeBuilder.close();
  }
};

/**
 * Overrides the "Appearance" toolbar button so that it launches themebuilder.
 */
ThemeBuilder.addThemeEditButton = function () {
  // Strip the 'admin/appearance' href from the "Appearance" toolbar link.
  // We can't use '#' as the href, because if we do, the link will show up
  // as highlighted when the overlay is closed and the hash changes to '#'.
  var $toolbarLink = jQuery('#toolbar-link-admin-appearance').removeAttr('href').css('cursor', 'pointer');

  // @see: function themebuilder_compiler_preprocess_html()
  // The themebuilder is disabled if we're using an admin theme, for example.
  if (!jQuery('body').hasClass('themebuilder-disabled')) {
    // Avoid attaching multiple event listeners.
    $toolbarLink.unbind('click', ThemeBuilder._appearanceButtonCallback);
    $toolbarLink.click(ThemeBuilder._appearanceButtonCallback);
  }
};

/**
 * Determines whether the browser being used is supported by the themebuilder.
 *
 * @return {boolean}
 *   true if the browser is supported; false otherwise.
 */
ThemeBuilder.browserSupported = function () {
  var browserOk = false;
  var browserDetect = new ThemeBuilder.BrowserDetect();
  switch (browserDetect.browser) {
  case 'Mozilla':
  case 'Firefox':
    browserOk = (parseFloat(browserDetect.version) >= 1.9);
    break;

  case 'Explorer':
    browserOk = (parseFloat(browserDetect.version) >= 8.0);
    break;

  case 'Safari':
    browserOk = true;
    break;

  case 'Chrome':
    browserOk = true;
    break;

  default:
  }
  return browserOk;
};

/**
 * Called when the appearance button is clicked.
 */
ThemeBuilder._appearanceButtonCallback = function () {
  // If the themebuilder is open and an overlay is open, 
  // clicking the appearance button will close the overlay
  // instead of acting on the themebuilder
  if (jQuery('body').hasClass('themebuilder') && Drupal.overlay.isOpen) {
    jQuery.bbq.removeState('overlay');
    return false;
  }
  if (ThemeBuilder.browserSupported()) {
    // This is idempotent, so no need to check if themebuilder is
    // open already.
    var bar = ThemeBuilder.Bar.getInstance();
    bar.openThemebuilder();
  }
  else {
    alert("Editing your site's appearance requires one of the following browsers: Firefox version 3.0 or higher, Internet Explorer 8, Safari 4, or Google Chrome 4.");
  }
  return false;
};

/**
 * Opens the themebuilder.  This function causes the shortcuts bar to disappear
 * and opens the themebuilder panel.
 */
ThemeBuilder.open = function () {
  if (jQuery('div.toolbar-shortcuts')) {
    Drupal.toolbar.collapse();
  }

  // Convert any embed tags into placeholder images to not break z-index
  jQuery('embed').each(ThemeBuilder.embedReplace);

  // Make sure the initialization data has been received.
  var app = ThemeBuilder.getApplicationInstance();
  var appData = app.getData();
  if (!appData) {
    app.addApplicationInitializer(ThemeBuilder.open);
    return;
  }
  app.addApplicationInitializer(ThemeBuilder.applicationDataInitialized);
  app.addUpdateListener(ThemeBuilder.applicationDataUpdated);
  var bar = ThemeBuilder.Bar.getInstance();
  bar.show();
  ThemeBuilder.populateUndoStack();
  ThemeBuilder.undoStack.addChangeListener(bar);
  ThemeBuilder.redoStack.addChangeListener(bar);
  bar.stackChanged();
};

/**
 * Closes the themebuilder panel.
 */
ThemeBuilder.close = function () {
  if (Drupal.toolbar) {
    Drupal.toolbar.expand();
  }
  var bar = ThemeBuilder.Bar.getInstance(false);
  if (bar && ThemeBuilder.undoStack) {
    ThemeBuilder.undoStack.removeChangeListener(bar);
    ThemeBuilder.redoStack.addChangeListener(bar);
  }
};

ThemeBuilder.embedReplace = function (index, element) {
  var $ = jQuery;
  var h = $(element).height();
  var w = $(element).width();

  var placeholder = $('<div class="flash-content tb-no-select" title="Flash content not available while Themebuilding"></div>').css({'height': h, 'width': w});

  $(element).replaceWith(placeholder);
};

/**
 * Called when the application data is initialized.
 *
 * @param {Array} data
 *   The initial application data returned from the server.
 */
ThemeBuilder.applicationDataInitialized = function (data) {
  // Trigger any behaviors that the server side code requested to be triggered.
  ThemeBuilder.triggerBehaviors(data);
};

/**
 * Called when the application data has changed.
 *
 * @param {Array} data
 *   The set of application data that changed.
 */
ThemeBuilder.applicationDataUpdated = function (data) {
  // Look for a change to the maintenance mode state and alert the user to save
  // their theme.
  if (data.maintenance_mode === true) {
    alert(Drupal.t('The ThemeBuilder will soon be undergoing a brief maintenance period.  Please save your work and close the ThemeBuilder.'));
  }
  else if (data.maintenance_mode === false) {
    // Probably don't need a message when we come out of maintenance mode.
  }

  // Trigger any behaviors that the server side code requested to be triggered.
  ThemeBuilder.triggerBehaviors(data);
};

/**
 * Triggers behaviors that the server side code requested to be triggered.
 *
 * @param {Array} data
 *   The data returned from the server, either on application initialization or
 *   on application update.
 */
ThemeBuilder.triggerBehaviors = function (data) {
  if (data.hasOwnProperty("behaviors_to_trigger")) {
    for (var behavior_to_trigger in data.behaviors_to_trigger) {
      if (data.behaviors_to_trigger.hasOwnProperty(behavior_to_trigger)) {
        jQuery('#themebuilder-main').trigger(behavior_to_trigger);
      }
    }
  }
};

/**
 * Work around an issue in Drupal's behavior code that causes the attach method to not be called if a previously called attach method encountered an error.
 *
 * Because of a lack of exception handling in behaviors code, it is
 * not guaranteed that all of the behaviors code will actually get
 * executed.  See http://drupal.org/node/990880.
 * Until that issue is resolved, wrap each behavior attach method with
 * a wrapper that catches and ignores any error.
 */
ThemeBuilder.protectAgainstBrokenInitializers = function () {
  for (var behavior in Drupal.behaviors) {
    if (jQuery.isFunction(Drupal.behaviors[behavior].attach)) {
      var attach = Drupal.behaviors[behavior].attach;
      Drupal.behaviors[behavior].attach = ThemeBuilder.errorCatchingWrapper(behavior, attach);
    }
  }
};

/**
 * This wrapper is used to wrap each behavior's attach method to prevent errors encountered during initialization from preventing subsequent initialization code from executing.
 *
 * @param {String} behavior
 *   The name of the behaivor the specified attach function is associated with.
 * @param {Function} attach
 *   The attach method that is to be wrapped with error handling functionality.
 */
ThemeBuilder.errorCatchingWrapper = function (behavior, attach) {
  return function () {
    try {
      return attach.apply(this, arguments);
    }
    catch (e) {
      var message = e.message ? e.message : e;
      ThemeBuilder.Log.gardensWarning('AN-25177 - Error encountered in the JavaScript initialization code', 'Drupal.behaviors.' + behavior + ': ' + message);
      if (ThemeBuilder.isDevelMode()) {
        alert(message);
      }
    }
  };
};

/**
 * Make sure that any errors encountered during initialization do not
 * make it impossible to open the themebuilder.
 */
ThemeBuilder.protectAgainstBrokenInitializers();
