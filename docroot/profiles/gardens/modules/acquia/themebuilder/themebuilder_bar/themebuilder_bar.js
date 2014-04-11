/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true debug: true Drupal: true window: true */


var ThemeBuilder = ThemeBuilder || {};

/**
 * The Bar class is the interface through which the themebuilder is added to
 * the page.  It exposes application level functionality, such as save,
 * publish, export.  It also provides APIs for the rest of the application to
 * interact with the themebuilder user interface.
 *
 * @class
 *
 * This class implements the singleton design pattern.
 */
ThemeBuilder.Bar = ThemeBuilder.initClass();

/**
 * This static method is the only way an instance of the Bar class should be
 * retrieved.  It ensures only one instance of the Bar class exists.
 *
 * @static
 *
 * @param {boolean} createIfNeeded
 *   (Optional) Indicates whether the instance should be created if it doesn't
 *   already exist.  Defaults to true.
 *
 * @return {ThemeBuilder.Bar}
 *   The instance of the ThemeBuilder.Bar class.  If no instance currently
 *   exists, one will be created as a result of calling this method.
 */
ThemeBuilder.Bar.getInstance = function (createIfNeeded) {
  createIfNeeded = createIfNeeded === undefined ? true : createIfNeeded;
  if (!ThemeBuilder.Bar._instance && createIfNeeded) {
    ThemeBuilder.Bar._instance = new ThemeBuilder.Bar();
  }
  return (ThemeBuilder.Bar._instance);
};

/**
 * The constructor of the Bar class.
 */
ThemeBuilder.Bar.prototype.initialize = function () {
  if (ThemeBuilder.Bar._instance) {
    throw "ThemeBuilder.Bar is a singleton that has already been instantiated.";
  }
  this.changed = false;
  this._dialogs = {};
  this.saving = false;
  this.links = {};
  this.showing = false;
  this.loader = null;
  if (ThemeBuilder.undoButtons) {
    ThemeBuilder.undoButtons.addStatusChangedListener(this);
  }
  this._attachEventHandlers();
  this._tabLoadRequests = [];
  this.listeners = [];
};

/**
 * Attaches event handlers to the buttons.  This must be called or the buttons
 * won't do anything.
 *
 * @private
 */
ThemeBuilder.Bar.prototype._attachEventHandlers = function () {
  var $ = jQuery;
  // Before attaching event handlers, make sure the elements exist.
  var selectors = ['#themebuilder-wrapper .export',
    '#themebuilder-wrapper #save',
    '#themebuilder-wrapper #save-as',
    '#themebuilder-wrapper #publish',
    '#themebuilder-mini-button',
    '#themebuilder-exit-button'];
  for (var index = 0; index < selectors.length; index++) {
    var selector = selectors[index];
    if (selector.indexOf('#themebuilder') === 0) {
      if ($(selector).length === 0) {
        // The element does not exist on the page yet.  Wait for the element
        // to appear before adding event listeners.
        setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this, this._attachEventHandlers), 150);
        return;
      }
    }
  }
  // The elements exist.  Attach the event handlers.
  $('#themebuilder-wrapper').click(ThemeBuilder.bind(this, this.quarantineEvents));
  $('#themebuilder-wrapper .export').click(ThemeBuilder.bind(this, this.exportTheme));
  $('#themebuilder-wrapper #save').click(ThemeBuilder.bind(this, this.saveHandler));
  $('#themebuilder-wrapper #save-as').click(ThemeBuilder.bind(this, this.saveas));
  $('#themebuilder-wrapper #publish').click(ThemeBuilder.bind(this, this.publishHandler));
  $('#themebuilder-mini-button').click(ThemeBuilder.bind(this, this.toggleMinimize));
  $('#themebuilder-exit-button').click(ThemeBuilder.bindIgnoreCallerArgs(this, this.exit, true));
  $('#themebuilder-main').bind('save', ThemeBuilder.bind(this, this._saveCallback));
  $('#themebuilder-main').bind('modified', ThemeBuilder.bind(this, this.indicateThemeModified));
};

/**
 * Keypress event handler, to correctly handle the Enter key in save dialogs.
 *
 * @private
 *
 * @param {event} event
 *   The keypress event.
 * @param {string} dialogName
 *   The name of the save dialog.
 */
ThemeBuilder.Bar.prototype._handleKeyPress = function (event, dialogName) {
  if ((event.which && event.which === 13) || (event.keyCode && event.keyCode === 13)) {
    // This only works because the OK button appears first in the markup.
    this.getDialog(dialogName).siblings('.ui-dialog-buttonpane').find('button:first').click();
    return ThemeBuilder.util.stopEvent(event);
  }
};

/**
 * Displays an indicator over the current page to indicate the themebuilder is
 * busy.
 */
ThemeBuilder.Bar.prototype.showWaitIndicator = function () {
  var $ = jQuery;
  // Show the throbber to indicate it's loading.
  $('<div class="themebuilder-loader"></div>').
    appendTo($('body'));
};

/**
 * Removes the busy indicator.
 */
ThemeBuilder.Bar.prototype.hideWaitIndicator = function () {
  var $ = jQuery;
  $('.themebuilder-loader').remove();
};

/**
 * Called when the user clicks the button to enter the themebuilder.  This
 * method causes the themebuilder to open.
 */
ThemeBuilder.Bar.prototype.openThemebuilder = function () {
  if (this.showing && !this.forceReload) {
    return;
  }
  // Don't open Themebuilder if the overlay is open.
  if (Drupal.overlay && (Drupal.overlay.isOpen || Drupal.overlay.isOpening)) {
    Drupal.overlay.close();
  }
  var ic = new ThemeBuilder.StartInteraction();
  ic.start();
};
/**
 * Reloads the page.
 */
ThemeBuilder.Bar.prototype.reloadPage = function (data) {
  var $ = jQuery;

  // If there was an error, then stop.
  if (data && data.error) {
    alert(data.error);
    this.hideWaitIndicator();
    return;
  }

  // Reload, which is required even if there are no changes to the
  // theme to make sure the gears work after the themebuilder is
  // closed.
  parent.location.reload(true);
};

/**
 * Displays the themebuilder user interface and performs any necessary
 * initialization.
 */
ThemeBuilder.Bar.prototype.show = function () {
  if (this.showing) {
    return;
  }
  var $ = jQuery;
  this.showing = true;
  this.trackWindowSize();

  // Disable the undo and redo buttons until the themebuilder is fully started.
  var statusKey;
  if (ThemeBuilder.undoButtons) {
    statusKey = ThemeBuilder.undoButtons.disable();
  }

  $('#themebuilder-wrapper').show();
  $('body').addClass('themebuilder');
  $('#themebuilder-main').tabs(
    {select: ThemeBuilder.bind(this, this.tabSelected)}
  );
  this._initializeCurrentTab();

  if (!Drupal.settings.themebuilderSaved) {
    $('#themebuilder-save #save').addClass('disabled');
  }
  var state = this.getSavedState();
  $('#themebuilder-main').tabs('select', '#' + state.tabId);
  this.getTabObject(state.tabId).show();

  this._createVeil();

  // Allow the undo and redo buttons to be used now that the themebuilder is
  // ready.
  if (statusKey) {
    ThemeBuilder.undoButtons.clear(statusKey);
  }
  statusKey = undefined;

  $('#themebuilder-theme-name .theme-name').truncate({addtitle: true});
  this.showInitialAlert();
};

/**
 * Called when the application init data is available.
 *
 * @param {Object} data
 *   The Application init data.
 */
ThemeBuilder.Bar.initializeUserInterface = function (data) {
  var $ = jQuery;
  ThemeBuilder.Bar.showInitialMessage(data);
  var bar = ThemeBuilder.Bar.getInstance();
  bar.setChanged(Drupal.settings.themebuilderIsModified);
  bar.buildThemeUpdateLink();
  if (!bar.userMayPublish()) {
    bar.hidePublishButton();
  }
};

/**
 * Hides the Publish button as soon as it is available in the DOM.
 */
ThemeBuilder.Bar.prototype.hidePublishButton = function () {
  var $button = jQuery('#themebuilder-wrapper #publish');
  if ($button.length === 0) {
    setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this, this.hidePublishButton), 150);
    return;
  }

  // The publish button now exists in the DOM, so it can be hidden.
  $button.hide();
};

/**
 * Sets up the update theme indicator according to whether an update is
 * available or not.
 */
ThemeBuilder.Bar.prototype.buildThemeUpdateLink = function () {
  var data = ThemeBuilder.getApplicationInstance().getData(),
  $ = jQuery,
  alertImage = $();
  if (Drupal.settings.themebuilderAlertImage) {
    alertImage = $('<img>', {
      src: Drupal.settings.themebuilderAlertImage
    })
    .addClass('tb-alert-icon');
  }
  // Build the theme update link and append it to the #themebuilder-wrapper.
  // Someday we'll have templates for these things.
  $('<div>', {
    id: 'themebuilder-theme-update',
    // The empty object is necessary to make the div element render.
    html: $('<div>', {})
    .append(alertImage)
    .append($('<span>', {
        text: Drupal.t('New theme update')
      })
      .addClass('tb-label')
    )
    .append($('<span>', {
        text: Drupal.t('Preview')
      })
      .addClass('tb-link')
      .click(ThemeBuilder.bind(this, this.updateTheme))
    )
    .addClass('tb-update-available-wrapper')
  })
  .addClass((data.theme_update_available) ? '' : 'tb-disabled')
  .prependTo('#themebuilder-wrapper');
};

/**
 * Causes window resizes to be detected, and resizes the themebuilder panel
 * accordingly.
 */
ThemeBuilder.Bar.prototype.trackWindowSize = function () {
  var $ = jQuery;
  $(window).resize(ThemeBuilder.bind(this, this.windowSizeChanged));
  this.windowSizeChanged();
};

/**
 * When the window size changes, reset the max-width property of the
 * themebuilder.  Certain properties applied to the body tag will have an
 * effect on the layout of the themebuilder.  These include padding and
 * margin.  Because they change the size of the actual window, this type of
 * CSS "leakage" could not be fixed by the css insulator or iframe alone.
 *
 * @param {Event} event
 *   The window resize event.
 */
ThemeBuilder.Bar.prototype.windowSizeChanged = function (event) {
  var $ = jQuery;
  var win = window;
  if (event && event.currentTarget) {
    win = event.currentTarget;
  }
  $('#themebuilder-wrapper').css('max-width', $(win).width());
};

/**
 * Returns tab info for the specified tab.  The tab can be specified by id
 * (ex: themebuilder-layout) or name (ex: layout).  If the tabName is not
 * provided, the info for the currently selected theme will be used.
 *
 * @param tabName
 *   The name of the tab to provide info for.  If not provided, the current
 *   tab will be used.
 *
 * @return {Array}
 *   An array of information about the specified tab.  This includes the name,
 *   title, weight, link, and id.
 */
ThemeBuilder.Bar.prototype.getTabInfo = function (tabName) {
  var tabId, name, state, tabInfo;
  if (!tabName) {
    state = this.getSavedState();
    tabId = state.tabId;
    name = tabId.replace(/^themebuilder-/, '');
  }
  else {
    name = tabName.replace(/^[#]?themebuilder-/, '');
    tabId = 'themebuilder-' + name;
  }
  // Note (jbeach): This might not be the right place to do the following
  // fallback to a default tab. I'm not wild about calling saveState in a get.
  //
  // The saved state might differ from the available toolbar items if the user's
  // access privileges to this tab have been revoked since they last had the
  // ThemeBuilder open. If the tab is in the toolbarItems, we can proceed.
  if (name in Drupal.settings.toolbarItems) {
    tabInfo = ThemeBuilder.clone(Drupal.settings.toolbarItems[name]);
    tabInfo.id = tabId;
  }
  // Otherwise, get the first tab and return that as the current tab.
  else {
    tabInfo = ThemeBuilder.clone(this.getDefaultState());
    tabInfo.id = 'themebuilder-' + tabInfo.name;
    this.saveState(tabInfo.id);
  }
  return tabInfo;
};

/**
 * Returns the object that manages the tab with the specified id.
 *
 * The tab manager has the following methods:
 *   show - called when the tab is selected by the user.
 *   hide - called when the user traverses away from the tab.
 *
 * @param {String} id
 *   The id associated with the tab.
 *
 * @return {Object}
 *   The object that manages the tab associated with the id.  An exception is
 *   thrown if the id is unknown.
 */
ThemeBuilder.Bar.prototype.getTabObject = function (id) {
  switch (id) {
  case 'themebuilder-brand':
    return ThemeBuilder.brandEditor;
  case 'themebuilder-advanced':
    return ThemeBuilder.AdvancedTab.getInstance();
  case 'themebuilder-layout':
    return ThemeBuilder.LayoutEditor.getInstance();
  case 'themebuilder-style':
    return ThemeBuilder.styleEditor;
  case 'themebuilder-themes':
    return ThemeBuilder.themeSelector;
  default:
    throw Drupal.t('ThemeBuilder.Bar.getTabObject: unknown tab %id', {'%id': id});
  }
};

/**
 * Called when a major tab in the themebuilder is selected.  This method is
 * responsible for sending messages to the currently open tab and the tab
 * being opened so the tabs can do necessary cleanup before the UI is updated.
 *
 * @param {Event} event
 *   The tabselect event.
 * @param {Object} ui
 *   The jQuery object associated with changing tabs.  This object holds
 *   information about the tab being selected.
 *
 * @return {boolean}
 *   True if the tab selection should be honored; false if the tab selection
 *   should be aborted.
 */
ThemeBuilder.Bar.prototype.tabSelected = function (event, ui) {
  var currentTabInfo = this.getTabInfo();
  var newTabInfo = this.getTabInfo(ui.tab.hash);

  if (currentTabInfo.id !== newTabInfo.id) {
    // Only hide the current tab if we selected a different tab.
    var current = this.getTabObject(currentTabInfo.id);
    if (current.hide() === false) {
      return false;
    }
  }
  this._initializeTab(newTabInfo.name);
  var panel = this.getTabObject(newTabInfo.id);
  if (panel.show() === false) {
    return false;
  }
  for (var i = 0; i < this.listeners.length; i++) {
    if (this.listeners[i].selectorChanged) {
      this.listeners[i].handleTabSwitch(panel);
    }
  }

  return true;
};

/**
 * Saves the current state of the UI.
 *
 * The state include the current tab id, and can contain other
 * information as needed.  This method determines whether it is
 * necessary to save the state to avoid sending unnecessary requests
 * to the server.
 *
 * @param {String} tabId
 *   The element id of the currently selected tab.
 * @param {Object} otherInfo
 *   Any other information that should be saved with the state.
 * @param {Function} successCallback
 *   The callback that should be invoked upon success.
 * @param {Function} failureCallback
 *   The callback that should be invoken upon failure.
 */
ThemeBuilder.Bar.prototype.saveState = function (tabId, otherInfo, successCallback, failureCallback) {
  var originalState = this.getSavedState();
  // Avoid superfluous requests to the server.
  if (originalState && originalState.tabId === tabId &&
      !otherInfo &&
      !successCallback) {
    return;
  }

  // Send the request.
  var state = {
    tabId: tabId,
    info: otherInfo
  };
  ThemeBuilder.postBack('themebuilder-save-state', {state: JSON.stringify(state)}, successCallback, failureCallback);
  Drupal.settings.toolbarState = state;
};

/**
 * Returns the currently saved state.
 *
 * The saved state can be in one of two forms - the older form is a
 * simple string containing the tab id, the newer form is an object
 * that contains the tab id as a field.  This method reads both and
 * returns the newer object form.  This will ease the update
 * transition.
 *
 * @return {Object}
 *   An object containing the saved state, including a field name
 *   'tabId' that contains the id of the active tab.
 */
ThemeBuilder.Bar.prototype.getSavedState = function () {
  var state;
  state = Drupal.settings.toolbarState;
  if (!state) {
    throw new Error('Drupal.settings.toolbarState is not set.');
  }
  if (!state.tabId) {
    try {
      state = jQuery.parseJSON(Drupal.settings.toolbarState);
    } catch (e) {
      if (typeof(state) === 'string') {
	// The state is a string, not an object.  Originally the state
	// was a string that only contained the id of the tab element.
	// Create a new structure with that info.
        state = {
          tabId: state,
          info: {}
        };
      }
    }
    // Only parse this info once.
    Drupal.settings.toolbarState = state;
  }

  if (state.tabId[0] === '#') {
    state.tabId = state.tabId.slice(1);
    Drupal.settings.toolbarState = state;
  }
  return state;
};

/**
 *
 */
ThemeBuilder.Bar.prototype.getDefaultState = function () {
  var state, items, tab;
  items = Drupal.settings.toolbarItems;
  if (items && items.length === 0) {
    throw new Error('Drupal.settings.toolbarItems does not contain any items.');
  }
  else {
    // Get the first tab in the list. The tabs are keyed by tab name.
    tabInfo: for (tab in items) {
      if (items.hasOwnProperty(tab)) {
        state = items[tab];
        break tabInfo;
      }
    }
  }
  return state;
};

/**
 * Initializes the tab associated with the specified name.  This method loads
 * all of the javascript assocaited with the tab.
 *
 * @private
 *
 * @param {String} name
 *   The name of the tab to initialize.  This will be used to construct the
 *   element id associated with the tab.
 */
ThemeBuilder.Bar.prototype._initializeTab = function (name) {
  var obj = this.getTabInfo(name);
  if (this._tabLoadRequests[obj.id] === true) {
    // The tab contents have already been requested.
    this.saveState(obj.id);
    return;
  }
  var $ = jQuery;
  this.links[obj.name] = obj.link;
  var panel = $('#' + obj.id);
  if (obj.link) {
    ThemeBuilder.load(panel, obj.link, {}, ThemeBuilder.bind(this, this.tabResourcesLoaded, obj, name, panel), '', false);
  }
  // Allow modules to define tabs with no server-side markup.
  else {
    this.tabResourcesLoaded('', 'success', obj, name, panel);
  }

  // Remember that we requested the tab contents.
  this._tabLoadRequests[obj.id] = true;
};

/**
 * Called when the tab resources are loaded.  This method is responsible for
 * initializing the tabs as soon as the tab is loaded.
 *
 * @param {String} markup
 *   The markup resulting from loading the tab.
 * @param {String} status
 *   Indicates whether the load succeeded.  Should be "success".
 * @param {Object} obj
 *   Provides information about the tab.
 * @param {String} name
 *   The name of the tab.
 * @param {jQuery Object} panel
 *   The object representing the panel associated with the tab.
 */
ThemeBuilder.Bar.prototype.tabResourcesLoaded = function (markup, status, obj, name, panel) {
  if (status !== 'success') {
    // The load failed.  Allow the load to occur again.
    delete this._tabLoadRequests[obj.id];
  }
  else {
    // The tab load succeeded.  Remember which tab we are on.
    this.saveState(obj.id);
  }
  var $ = jQuery;
  var tabObject = this.getTabObject(obj.id);
  if (tabObject && tabObject.init) {
    tabObject.init();
  }
  if (tabObject && tabObject.loaded) {
    tabObject.loaded();
  }
  if (!panel.is('.ui-tabs-hide')) {
    if (this.loader) {
      this.loader.hide();
    }
    this.hideWaitIndicator();
    panel.show();
  }
};

/**
 * Causes the currently selected tab to be initialized.  If the tabs are being
 * lazy loaded, this is the only tab initialization that needs to be done.
 *
 * @private
 */
ThemeBuilder.Bar.prototype._initializeCurrentTab = function () {
  var currentTabInfo = this.getTabInfo();
  this._initializeTab(currentTabInfo.name);
};


ThemeBuilder.Bar.prototype.addBarListener = function (listener) {
  this.listeners.push(listener);
};

/**
 * Sets the change status of the theme being edited.  This causes the user
 * interface to reflect the current state of modification.
 *
 * @param {boolean} isChanged
 *   True if the theme has been modified since the last save; false otherwise.
 */
ThemeBuilder.Bar.prototype.setChanged = function (isChanged) {
  var $ = jQuery;
  if (isChanged === null) {
    isChanged = true;
  }
  this.changed = isChanged;

  if (this.changed) {
    // Enable the Save button, if the user has permission to use it.
    if (this.userMaySave()) {
      $('#themebuilder-save #save').removeClass('disabled');
    }
    $('#themebuilder-main').trigger('modified');
    this.indicateThemeModified();
  }
  else {
    $('#themebuilder-save #save').addClass('disabled');
    this.clearModifiedFlag();
  }
};

/**
 * Determines whether the user has permission to publish themes.
 */
ThemeBuilder.Bar.prototype.userMayPublish = function () {
  var data = ThemeBuilder.getApplicationInstance().getData();
  if (data.user_permissions && data.user_permissions.publish_theme) {
    return true;
  }
};


/**
 * Determines whether the user has permission to save a given theme.
 *
 * @param {String} themeName
 *   The name of the theme to be saved. Defaults to the current theme.
 */
ThemeBuilder.Bar.prototype.userMaySave = function (themeName) {
  var theme = (themeName ? ThemeBuilder.Theme.getTheme(themeName) : theme = ThemeBuilder.Theme.getSelectedTheme());

  // Determine whether the active theme is the published theme.
  if (theme.isPublished()) {
    // If so, saving would affect the published theme. Make sure the user has
    // publish theme permissions.
    return this.userMayPublish();
  }
  // If the theme being saved is not the published theme, any user may save it.
  return true;
};

/**
 * Called when a request is sent that changes status of the undo and redo
 * buttons.  This method is responsible for disabling the buttons accordingly
 * to prevent the user from causing the client and server to get out of sync.
 *
 * @param {boolean} status
 *   If true, the status is going from an empty undo/redo stack to a non-empty
 *   stack.  false indicates the opposite.
 *
 */
ThemeBuilder.Bar.prototype.undoStatusChanged = function (status) {
  if (status) {
    this.enableButtons();
  }
  else {
    this.disableButtons();
  }
  this.stackChanged();
};

/**
 * Called when the undo stack state has changed.  This function is responsible
 * for enabling and disabling the undo and redo buttons.
 *
 * @param stack object
 *   The stack that changed state.
 */
ThemeBuilder.Bar.prototype.stackChanged = function (stack) {
  if (!ThemeBuilder.undoButtons) {
    return;
  }
  var undoStatus = ThemeBuilder.undoButtons.getStatus();
  var $ = jQuery;
  if (!stack || stack === ThemeBuilder.undoStack) {
    var undoSize = ThemeBuilder.undoStack.size();
    if (undoSize <= 0 || undoStatus !== true) {
      $('.themebuilder-undo-button').addClass('undo-disabled')
        .unbind('click');
    }
    else {
      $('.themebuilder-undo-button.undo-disabled').removeClass('undo-disabled')
      .bind('click', ThemeBuilder.undo);
    }
  }
  if (!stack || stack === ThemeBuilder.redoStack) {
    var redoSize = ThemeBuilder.redoStack.size();
    if (redoSize <= 0 || !undoStatus) {
      $('.themebuilder-redo-button').addClass('undo-disabled')
      .unbind('click');
    }
    else {
      $('.themebuilder-redo-button.undo-disabled').removeClass('undo-disabled')
      .bind('click', ThemeBuilder.redo);
    }
  }
};

/**
 * Handler for the Save button.
 */
ThemeBuilder.Bar.prototype.saveHandler = function (event) {
  event.preventDefault();
  var $link = jQuery(event.currentTarget);
  if ($link.hasClass('disabled')) {
    // For users without save permissions, display a dialog letting them know
    // why they're not allowed to save.
    if (!this.userMaySave()) {
      var saveas = confirm(Drupal.t('Saving this theme would change the published (live) theme. Do you wish to save your work as a new theme?'));
      if (saveas) {
        this.saveas();
      }
    }
    return;
  }
  this.save();
};

/**
 * Causes the theme currently being edited to be saved.  This should only be
 * called if there is a theme system name to which the theme should be saved.
 * Otherwise, the user should provide a theme name.  That can be done with the
 * saveas method.
 */
ThemeBuilder.Bar.prototype.save = function () {
  if (!Drupal.settings.themebuilderSaved) {
    this.saveas();
    return;
  }

  var selectedTheme = ThemeBuilder.Theme.getSelectedTheme();
  if (selectedTheme.isPublished()) {
    var saveAnyway = confirm(Drupal.t('Clicking OK will change your published (live) theme. Do you want to continue?'));
    if (saveAnyway !== true) {
      this.saveas();
      return;
    }
  }

  this.disableThemebuilder();
  ThemeBuilder.postBack('themebuilder-save', {},
    ThemeBuilder.bind(this, this._themeSaved),
    ThemeBuilder.bind(this, this._themeSaveFailed));
  this.themeChangeNotification();
};

/**
 * Called when the theme has been saved.
 *
 * @private
 *
 * @param {Object} data
 *   The information returned from the server as a result of the theme being
 *   saved.
 */
ThemeBuilder.Bar.prototype._themeSaved = function (data) {
  var $ = jQuery;
  try {
    $('#themebuilder-main').trigger('save', data);

    // A theme created as part of the 'save as' process should take over the
    // applied state.
    var app = ThemeBuilder.getApplicationInstance();
    app.updateData({
      bar_saved_theme: ThemeBuilder.Theme.getTheme(data.system_name)
    });
  }
  catch (e) {
  }
  this.enableThemebuilder();
};

/**
 * Called when the the save fails.  This provides an opportunity to recover
 * and allow the user to continue without losing their work.
 *
 * @param {Object} data
 *   The data returned from the failed request.
 */
ThemeBuilder.Bar.prototype._themeSaveFailed = function (data) {
  ThemeBuilder.handleError(data, data.type, 'recoverableError');
  this.enableThemebuilder();
};

/**
 * Causes a dialog box to appear that asks the user for a theme name, and then
 * saves the theme.
 */
ThemeBuilder.Bar.prototype.saveas = function (event) {
  if (event) {
    event.preventDefault();
  }

  this.processSaveDialog('themebuilderSaveDialog', false, 'themebuilder-save', ThemeBuilder.bind(this, this._saveDialogCallback, false));
};

/**
 * Called when the user clicks the export link, and causes the theme to be
 * exported.
 */
ThemeBuilder.Bar.prototype.exportTheme = function () {
  this.processSaveDialog('themebuilderExportDialog', false, 'themebuilder-export', ThemeBuilder.bind(this, this._themeExported));
};

/**
 * Called when the user clicks the "Update available" link.
 */
ThemeBuilder.Bar.prototype.updateTheme = function () {
  if (confirm(Drupal.t("There is an update available for your site's theme which contains new features or bug fixes.\nClick OK to preview this update."))) {
    this.showWaitIndicator();
    this.disableThemebuilder();
    // Force the screen to refresh after the update.
    this.forceReload = true;
    ThemeBuilder.postBack('themebuilder-update-theme', {},
      ThemeBuilder.bind(this, this.reloadPage));
  }
};

/**
 * This method is called after the theme is exported.
 *
 * @private
 *
 * @param {Object} data
 *   The information returned from the server as a result of the theme being
 *   exported.
 */
ThemeBuilder.Bar.prototype._themeExported = function (data) {
  this.setStatus(Drupal.t('%theme_name was successfully exported.', {'%theme_name': data.name}));
  window.location = data.export_download_url;
  this.enableThemebuilder();
};

/**
 * Handler for the publish button.
 */
ThemeBuilder.Bar.prototype.publishHandler = function (event) {
  if (event) {
    event.preventDefault();
  }
  var $link = jQuery(event.currentTarget);
  if ($link.hasClass('disabled')) {
    return;
  }
  this.publish();
};

/**
 * Causes the theme to be published.  If the there is no associated system
 * name for the theme to save to, a dialog will appear asking the user for a
 * theme name.
 */
ThemeBuilder.Bar.prototype.publish = function () {
  if (!Drupal.settings.themebuilderSaved) {
    // Only need to save before publishing if the theme has never been saved
    // before and doesn't have a name.  Publishing the theme causes the draft
    // theme to be copied (same as the save functionality).
    this.processSaveDialog('themebuilderPublishDialog', true, 'themebuilder-save', ThemeBuilder.bind(this, this._saveDialogCallback));
  }
  else {
    // Publish the theme.
    this.disableThemebuilder();

    // Publish is really expensive because it rebuilds the theme
    // cache.  Avoid doing that if we are really just saving to the
    // published theme.  Save is way more efficient.
    var appData = ThemeBuilder.getApplicationInstance().getData();
    var publish = appData.selectedTheme !== appData.published_theme;
    ThemeBuilder.postBack('themebuilder-save', {publish: publish},
      ThemeBuilder.bind(this, this._publishCallback));
    this.themeChangeNotification();
  }
};

/**
 * This callback is invoked from the save dialog and used to cause the actual
 * save to occur.
 *
 * @private
 *
 * @param {Object} data
 *   The information entered into the save dialog by the user.
 * @param {String} status
 *   The status indicator.  "success" indicates the call was successful.
 * @param {boolean} publish
 *   If true, the saved theme will be published.
 */
ThemeBuilder.Bar.prototype._saveDialogCallback = function (data, status, publish) {
  // If the server informed us that this theme name already exists, prompt
  // for overwrite.
  if (data.themeExists) {
    var overwrite = confirm("A theme with that name already exists. Would you like to overwrite it?");
    if (overwrite) {
      var saveArguments = {
        'name': data.themeName,
        'overwrite': true
      };
      if (true === data.publish) {
        saveArguments.publish = data.publish;
      }
      this.disableThemebuilder();
      ThemeBuilder.postBack('themebuilder-save', saveArguments,
        ThemeBuilder.bind(this, this._saveDialogCallback, publish), ThemeBuilder.bind(this, this._themeSaveFailed));
      this.themeChangeNotification();
    }
    else {
      this.enableThemebuilder();
    }
  }
  // If the server disallowed the save (because an unprivileged user is trying
  // to overwrite the live theme), tell the user why their save failed.
  else if (data.overwriteDisallowed) {
    window.alert(Drupal.t('"@name" is the live theme and cannot be overwritten.', {"@name": data.themeName}));
    this.enableThemebuilder();
  }
  else {
    if (true === publish) {
      this._publishCallback(data);
    }
    else {
      this._themeSaved(data);
      this.themeChangeNotification();
    }
  }
};

/**
 * This callback is invoked after the theme has been published.  This method
 * causes the UI to reflect the current theme name and updates application
 * data to match the new published theme.
 *
 * @private
 *
 * @param {Object} data
 *   The data that is passed from the server upon publishing the theme.
 */
ThemeBuilder.Bar.prototype._publishCallback = function (data) {
  this.setChanged(false);
  this.setStatus(Drupal.t('%theme_name is now live.', {'%theme_name': data.name}));
  this.setInfo(data.name, data.time);

  var app = ThemeBuilder.getApplicationInstance();

  // This fetch of the app data and setting the published and selected themes
  // to the system_name is necessary for setVisibilityText to work. It's voodoo
  // in my [jbeach] opinion, but it works right now, so we'll go with it.
  var appData = app.getData();
  appData.published_theme = appData.selectedTheme = data.system_name;

  // Trigger an app data update
  app.updateData({
    bar_published_theme: ThemeBuilder.Theme.getTheme(data.system_name)
  });

  // Update the cached theme data to reflect the change.
  this.updateThemeData(data);

  // We updated the active theme, so reset the message
  ThemeBuilder.Bar.getInstance().setVisibilityText();
  this.enableThemebuilder();
};

/**
 * Updates the cached theme data.  This should be called any time the
 * theme has changed (save, publish).
 *
 * @param {Object} data
 *   The data associated with a save or publish response.
 */
ThemeBuilder.Bar.prototype.updateThemeData = function (data) {
  var theme = ThemeBuilder.Theme.getTheme(data.system_name);
  if (theme) {
    theme.update(data);
  }
  else {
    theme = new ThemeBuilder.Theme(data);
    theme.addTheme();
  }
  var appData = ThemeBuilder.getApplicationInstance().getData();
  appData.selectedTheme = theme.getSystemName();
};

/**
 * Called after the theme is saved.
 *
 * @private
 *
 * @param {DomElement} element
 *   The element from which the callback was triggered.
 * @param {Object} data
 *   The data associated with the save call.
 */
ThemeBuilder.Bar.prototype._saveCallback = function (element, data) {
  // Disable the "save" button until the theme is modified again.
  Drupal.settings.themebuilderSaved = true;
  var $ = jQuery;
  $('#themebuilder-save #save').addClass('disabled');

  // Update the theme name data
  this.setChanged(false);
  this.setInfo(data.name, data.time);

  if (data.name) {
    Drupal.settings.themeLabel = data.name;
  }
  if (true === data.publish) {
    this.setStatus(Drupal.t('%theme_name was successfully published.', {'%theme_name': data.name}));
  }
  else if (true === data.save_as) {
    // The user clicked "Save as"
    this.setStatus(Drupal.t('%theme_name was successfully copied and saved.', {'%theme_name': data.name}));
  } else {
    // The user clicked "Save"
    this.setStatus(Drupal.t('%theme_name was successfully saved.', {'%theme_name': data.name}));
  }

  // Fix the cached theme data.
  this.updateThemeData(data);
  this.enableThemebuilder();
};

/**
 * Displays and processes a standard ThemeBuilder save dialog.
 *
 * @param {string} dialogName
 *   The name for the dialog (e.g. themebuilderSaveDialog). It should have a
 *   corresponding item in the Drupal.settings object containing the HTML
 *   for the main part of the dialog form (e.g.
 *   Drupal.settings.themebuilderSaveDialog). The HTML needs to contain the
 *   'name' field (i.e. the id for the field must be "edit-name"). Buttons for
 *   "OK" and "Cancel" will be automatically added to the form.
 * @param {boolean} publish
 *   If true, the theme will be published after the save.
 * @param postbackPath
 *   The path to post results to when the "OK" button is clicked; this will be
 *   passed to ThemeBuilder.postBack as the path parameter.
 * @param callback
 *   The callback function to which the results of the POST request will be
 *   passed after the "OK" button is clicked. This will be passed to
 *   ThemeBuilder.postBack as the success_callback parameter.
 */
ThemeBuilder.Bar.prototype.processSaveDialog = function (dialogName, publish, postbackPath, callback) {
  var $ = jQuery;
  var $dialog = this.getDialog(dialogName);
  if ($dialog) {
    $dialog.dialog('open');
  }
  else {
    var dialogForm = Drupal.settings[dialogName];
    $dialog = $(dialogForm).appendTo('body').dialog({
      bgiframe: true,
      autoOpen: true,
      dialogClass: 'themebuilder-dialog',
      modal: true,
      overlay: {
        backgroundColor: '#000',
        opacity: 0.5
      },
      position: 'center',
      width: 335,
      buttons: {
        'OK': ThemeBuilder.bind(this, this._saveDialogOkButtonPressed, dialogName, postbackPath, publish, callback),
        'Cancel': ThemeBuilder.bind(this, this._saveDialogCancelButtonPressed, dialogName)
      },
      close: ThemeBuilder.bindIgnoreCallerArgs(this, this._saveDialogClose, dialogName),
      open: ThemeBuilder.bind(this, this._saveDialogOpen)
    });
    $dialog.find('form').keypress(ThemeBuilder.bind(this, this._handleKeyPress, dialogName));
    // Prevent users from naming a theme with a string longer than 25 characters
    // This addresses https://backlog.acquia.com/browse/AN-26333
    this._enableLiveInputLimit('#themebuilder-bar-save-form #edit-name');
    var input = '#themebuilder-bar-save-form #edit-name';
    this._limitInput(input);
    $(input).bind('paste', ThemeBuilder.bind(this, this._limitInput));

    this.setDialog(dialogName, $dialog);
  }
  // Put the cursor on the form
  $dialog.find('input:first').focus();
};

/**
 * Retrieve a reference to a jQuery UI Dialog.
 *
 * @param {string} dialogName
 *   The name of the dialog.
 *
 * @return {jQuery}
 *   The jQuery object containing the dialog.
 */
ThemeBuilder.Bar.prototype.getDialog = function (dialogName) {
  return this._dialogs[dialogName] || false;
};

/**
 * Save a reference to a jQuery UI Dialog.
 *
 * @param {string} dialogName
 *   The name of the dialog.
 * @param {jQuery} $dialog
 *   The jQuery object containing the dialog.
 *
 * @return {jQuery}
 *   The jQuery object containing the dialog.
 */
ThemeBuilder.Bar.prototype.setDialog = function (dialogName, $dialog) {
  this._dialogs[dialogName] = $dialog;
  return this._dialogs[dialogName];
};

/**
 * Called when the user presses the Ok button in the save dialog.  This method
 * causes the associated post to occur and closes the dialog.
 *
 * @private
 *
 * @param {DomEvent} event
 *   The event associated with the button press.
 * @param postbackPath
 *   The path to post results to when the "OK" button is clicked; this will be
 *   passed to ThemeBuilder.postBack as the path parameter.
 * @param {boolean} publish
 *   If true, the theme will be published after the save.
 * @param callback
 *   The callback function to which the results of the POST request will be
 *   passed after the "OK" button is clicked. This will be passed to
 *   ThemeBuilder.postBack as the success_callback parameter.
 */
ThemeBuilder.Bar.prototype._saveDialogOkButtonPressed = function (event, dialogName, postbackPath, publish, callback) {
  var $ = jQuery;
  var $dialog = this.getDialog(dialogName);
  var $nameField = $('.name:first', $dialog);
  // Validate the theme name field.
  if (!$nameField.val()) {
    if (!$nameField.hasClass("ui-state-error")) {
      $nameField.addClass("ui-state-error");
      $nameField.before("<div class='error-message'>" + Drupal.t("Please enter a theme name.") + "</div>");
    }
  }
  else {
    this.disableThemebuilder();
    var saveArguments = {'name': $nameField.val()};
    if (true === publish) {
      saveArguments.publish = publish;
    }
    ThemeBuilder.postBack(postbackPath, saveArguments, callback, ThemeBuilder.bind(this, this._themeSaveFailed));
    $dialog.dialog('close');
  }
};

/**
 * Called when the user presses the Cancel button in the save dialog.  This
 * method causes the dialog to be closed.
 *
 * @private
 */
ThemeBuilder.Bar.prototype._saveDialogCancelButtonPressed = function (event, dialogName) {
  var $ = jQuery;
  this.getDialog(dialogName).dialog('close');
};

/**
 * Called when the save dialog is opened.
 *
 * @private
 */
ThemeBuilder.Bar.prototype._saveDialogOpen = function () {
  this.maximize();
};

/**
 * Called when the save dialog is closed.
 *
 * @private
 */
ThemeBuilder.Bar.prototype._saveDialogClose = function (dialogName) {
  var $ = jQuery;
  var $dialog = this.getDialog(dialogName);
  // Clear the theme name field.
  $('input', $dialog).val("");
  // Clear any error messages.
  $('.name:first', $dialog).removeClass("ui-state-error");
  $('div.error-message', $dialog).remove();
};

/**
 * Exits themebuilder with an optional user confirmation.
 *
 * @param {boolean} confirm
 *   (Optional) If true, the user is prompted to confirm before exiting the
 *   themebuilder; otherwise the themebuilder exits with no prompt.
 * @param {String} destination
 *   (Optional) The URI to which the browser should be redirected after exit.
 */
ThemeBuilder.Bar.prototype.exit = function (confirm, destination) {
  if (confirm === true && !this.exitConfirm()) {
    return;
  }

  var $ = jQuery;

  // If the themebuilder is in the process of polling the server, stop it now,
  // so that we don't get any weird errors from contacting the server in one
  // thread while the themebuilder is in the process of being closed in
  // another.
  ThemeBuilder.getApplicationInstance().forcePollingToStop();

  this.showWaitIndicator();
  this.disableThemebuilder();
  ThemeBuilder.postBack('themebuilder-exit', {}, ThemeBuilder.bind(this, this._exited, destination));
};

/**
 * This method is called after the themebuilder has exited.  It is responsible
 * for reloading the page after exit to ensure that the correct theme is being
 * used.
 *
 * @private
 *
 * @param {String}
 *   (Optional) The URI to which the browser should be redirected.
 */
ThemeBuilder.Bar.prototype._exited = function (destination) {
  var $ = jQuery;
  $('body').removeClass('themebuilder');

  // Make sure to remove the themebuilder elements so automated tests
  // fail when trying to use the themebuilder after it is closed.
  $('#themebuilder-wrapper').remove();

  // Force reload so that any CSS changes get to the browser.
  if (destination && typeof destination !== "object") {
    parent.location.assign(destination);
  }
  this.reloadPage();
};

/**
 * Prompts the user with a message before the themebuilder exits.
 *
 * @param {String} message
 *   The message that should be displayed to the user.  If no message is
 *   provided, a default message will be used instead.
 */
ThemeBuilder.Bar.prototype.exitConfirm = function (message) {
  if (!message) {
    message = 'You have unsaved changes. Discard?';
  }
  if (this.changed === false || confirm(Drupal.t(message))) {
    return true;
  }
  this.enableThemebuilder();
  return false;
};

/**
 * Sets the data in the info bar.  The info bar indicates the name of the
 * theme and the last time the theme was saved.
 *
 * @param {String} name
 *   The name of the theme currently being edited.
 */
ThemeBuilder.Bar.prototype.setInfo = function (name) {
  var $ = jQuery;
  $('#themebuilder-wrapper .theme-name')
    .html(Drupal.checkPlain(name))
    .truncate({addtitle: true});
  this.setVisibilityText();
};

/**
 * Sets the text that indicates the theme visibility based on the currently published theme and whether the draft is dirty or not.
 */
ThemeBuilder.Bar.prototype.setVisibilityText = function () {
  var $ = jQuery;
  var message;
  var selectedTheme = ThemeBuilder.Theme.getSelectedTheme();
  if (selectedTheme) {
    if (selectedTheme.isPublished() && !this.changed) {
      message = Drupal.t('(Live - everyone can see this)');
    }
    else {
      message = Drupal.t('(Draft - only you can see this)');
    }
    $('#themebuilder-theme-name .theme-visibility').text(message);
  }
};

/**
 * Sets the message of the status bar in the themebuilder.  The status bar
 * appears when there is a new message to display and then hides itself after
 * some period of time.
 *
 * @param {String} message
 *   The message to display.
 * @param {String} type
 *   The type of message, either 'info' or 'warning'.  This parameter is
 *   optional, and if omitted the message will be displayed as an 'info' type.
 */
ThemeBuilder.Bar.prototype.setStatus = function (message, type) {
  var $ = jQuery;
  // If the status bar is still visible, don't allow it to be hidden by the
  // existing timeout
  if (this._statusTimeout) {
    clearTimeout(this._statusTimeout);
    delete this._statusTimeout;
  }

  if (!type) {
    type = 'info';
  }
  $('#themebuilder-status .themebuilder-status-icon').removeClass('info').removeClass('warning').addClass(type);

  // Estimate the required width...  Pull out the tags before counting
  // characters.
  var width = this._guesstimateStatusMessageWidth(message);
  $('#themebuilder-status .themebuilder-status-message').html(message);
  $('#themebuilder-status')
    .width(width + 'px')
    .fadeTo(1000, 0.8);
  // After 10 seconds close the status tab automatically.
  this._statusTimeout = setTimeout(ThemeBuilder.bind(this, this.hideStatus), 10000);
};

/**
 * Estimates the width that the status message should be set to.
 *
 * The status message is centered in a div that is not in the normal
 * document flow.  This actually presents something of a challenge
 * because we want the text to dictate the width of the element, and
 * the width of the element to be used to center the element in the
 * window.  Since it is not in flow, we have to set the width of the
 * element, and thus we have to do a bit of ridiculous estimation to
 * make this feature match the design.
 *
 * This method works by figuring out how many characters are in the
 * actual message (by stripping out any tags) and then using a
 * multiplier of the character count.  Since a variable-width font is
 * being used, special attention is paid to space characters to try to
 * achieve a reasonable estimate.
 *
 * @private
 * @param {String} message
 *   The message markup.
 * @return {int}
 *   The estimated width.
 */
ThemeBuilder.Bar.prototype._guesstimateStatusMessageWidth = function (message) {
  var elementPadding = 47;
  var avgCharWidth = 8;
  var narrowCharOffset = -2.5;
  var wideCharOffset = 3;
  var strippedMessage = message.replace(/<[^>]*>/g, '');
  var narrowCount = strippedMessage.length - strippedMessage.replace(/[ il1]/g, '').length;
  var wideCount = strippedMessage.length - strippedMessage.replace(/[mwMW]/g, '').length;
  var width = (strippedMessage.length * avgCharWidth) + (narrowCount * narrowCharOffset) + (wideCount * wideCharOffset) + elementPadding;
  return width;
};

/**
 * Causes the info bar to indicate the theme has been modified.
 */
ThemeBuilder.Bar.prototype.indicateThemeModified = function () {
  var $ = jQuery;
  var $modified = $('#themebuilder-wrapper .theme-modified');
  if ($modified.length === 0) {
    $('<span class="theme-modified"> *</span>').insertBefore('#themebuilder-wrapper .theme-name');
  }
  this.setVisibilityText();
};

/**
 * Clears the flag that indicates the theme is dirty.
 */
ThemeBuilder.Bar.prototype.clearModifiedFlag = function () {
  var $ = jQuery;
  $('#themebuilder-wrapper .theme-modified').remove();
};

/**
 * Causes the themebuilder status bar to disappear.  This is usually invoked
 * by a timeout that is set when the status bar is displayed.
 */
ThemeBuilder.Bar.prototype.hideStatus = function () {
  var $ = jQuery;
  if (this._statusTimeout) {
    clearTimeout(this._statusTimeout);
    delete this._statusTimeout;
  }
  $('#themebuilder-status').fadeOut(1000);
};

/**
 * Causes the themebuilder to be minimized.
 */
ThemeBuilder.Bar.prototype.minimize = function () {
  var $ = jQuery;
  $('#themebuilder-wrapper').addClass('minimized');
};

/**
 * Causes the themebuilder to be maximized.
 */
ThemeBuilder.Bar.prototype.maximize = function () {
  var $ = jQuery;
  $('#themebuilder-wrapper').removeClass('minimized');
};

/**
 * Causes the themebuilder to toggle from maximized to minimized or from
 * minimized to maximized depending on the current state.
 */
ThemeBuilder.Bar.prototype.toggleMinimize = function () {
  var $ = jQuery;
  $('#themebuilder-wrapper').toggleClass('minimized');
};

/**
 * Prevents clicks on ThemeBuilder elements from propagating outside the ThemeBuilder.
 * Because we assign a click handler to the <body> in ElementPicker.js, we need to prevent
 * certain events that modify the Selector from also triggering _clickItem in ElementPicker.js
 */
ThemeBuilder.Bar.prototype.quarantineEvents = function (event) {
  event.stopPropagation();
};

/**
 * If warranted, this function displays the status message when the
 * themebuilder is initially loaded.  This is useful when a status generating
 * event is performed just before or during a page load, not providing time
 * for the user to view the message before it would be refreshed.  This is
 * accomplished by setting an array containing 'message' and 'type' fields
 * into $_SESSION['init_message'].
 *
 * @static
 *
 * @param {Object} data
 *   The application initialization data.
 */
ThemeBuilder.Bar.showInitialMessage = function (data) {
  if (data && data.initMessage) {
    ThemeBuilder.Bar.getInstance().setStatus(data.initMessage.message, data.initMessage.type);
  }
};

/**
 * If an alert has been passed to the javascript client, display it now.
 *
 * @param {Object} data
 *   Optional parameter tha provides the application initialization data.  If
 *   not provided this method will retrieve the data from the Application
 *   instance.
 */
ThemeBuilder.Bar.prototype.showInitialAlert = function (data) {
  if (!data) {
    data = ThemeBuilder.getApplicationInstance().getData();
  }

  if (data && data.initAlert) {
    alert(data.initAlert);
  }
};

/**
 * Creates an element that serves as a veil that blocks all input into the
 * themebuilder.  This is useful for controling the rate of requests the users
 * can submit using the themebuilder.
 */
ThemeBuilder.Bar.prototype._createVeil = function () {
  var $ = jQuery;
  var veil = $('<div id="themebuilder-veil"></div>').appendTo('#themebuilder-wrapper');
};

/**
 * Applies a limit to the length of the input text
 *
 * @private
 * @param {Event} event
 *   The event that this function handles
 * @param {HTML Object} field
 *   A DOM field.
 *
 * Prevent users from naming a theme with a string longer than 25 characters
 * This addresses https://backlog.acquia.com/browse/AN-26333
 *
 * This function trims the string down to 25 characters if it is longer than 25 characters.
 */
ThemeBuilder.Bar.prototype._limitInput = function (field) {
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
 * Applies the NobleCount plugin to the supplied field
 *
 * @private
 * @param {HTML Object} field
 *   A DOM field.
 *
 * Prevent users from naming a theme with a string longer than 25 characters
 * This addresses https://backlog.acquia.com/browse/AN-26333
 */
ThemeBuilder.Bar.prototype._enableLiveInputLimit = function (field) {
  var $ = jQuery;
  var max = 25;
  var $field = $(field);
  if ($field.length > 0) {
    // Add the NobleCount input limiter
    // This must be given the ID #char-count-save because the save
    // dialog isn't destroyed after it's dismissed. So the id #char-count
    // would conflict with other char-counting fields on the page.
    $('<span>', {
      id: 'char-count-save'
    }).insertAfter($field);
    $field.NobleCount('#char-count-save', {
      max_chars: max,
      block_negative: true
    });
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
ThemeBuilder.Bar.prototype._trimField = function ($field, max) {
  var value = $field.val();
  if (value.length > max) {
    $field.val(value.substr(0, max));
  }
  // Keydown is called to kick the NobleCounter plugin to refresh
  $field.keydown();
};

/**
 * Disables the themebuilder by displaying the veil which absorbs all input.
 */
ThemeBuilder.Bar.prototype.disableThemebuilder = function () {
  var $ = jQuery;
  $('#themebuilder-veil').addClass('show');
  this.showWaitIndicator();
};

/**
 * Enables the themebuilder by removing the veil.
 */
ThemeBuilder.Bar.prototype.enableThemebuilder = function () {
  var $ = jQuery;
  $('#themebuilder-veil').removeClass('show');
  this.hideWaitIndicator();
};

/**
 * Causes the buttons at the top of the themebuilder to be enabled.
 */
ThemeBuilder.Bar.prototype.enableButtons = function () {
  var $ = jQuery;
  $('#themebuilder-control-veil').removeClass('on');

};

/**
 * Causes the buttons at the top of the themebuilder to be disabled.
 *
 * This is important for reducing the possibility of race conditions
 * in which a commit that takes a bit too long allows the user to save
 * the theme when the theme is incomplete, thus losing css
 * customizations.
 */
ThemeBuilder.Bar.prototype.disableButtons = function () {
  var $ = jQuery;
  $('#themebuilder-control-veil').addClass('on');
};

/**
 * Make an asynchronous request to the site about a theme change.
 *
 * When a theme is saved or published then the factory needs notification so it
 * can pick up the changes. To avoid doing this 3rd party request during the
 * user pageload, this asyncronous Javascript call will start it.
 */
ThemeBuilder.Bar.prototype.themeChangeNotification = function () {
  ThemeBuilder.getBack('themebuilder-change-notification');
};

/**
 * Adds the toolbar to the page.
 *
 * @static
 */
ThemeBuilder.Bar.attachToolbar = function () {
  // This keeps the themebuilder from dying whenever Drupal.attachBehaviors is called.
  if (ThemeBuilder.Bar.attachToolbar.attached !== undefined) {
    return;
  }
  ThemeBuilder.Bar.attachToolbar.attached = true;

  ThemeBuilder.getApplicationInstance();
  //Always add the toolbar
  jQuery('body').append(Drupal.settings.toolbarHtml);
  jQuery('#themebuilder-wrapper:not(.themebuilder-keep)').hide();
};

/**
 * This Drupal behavior causes the themebuilder toolbar to be attached to the
 * page.
 */
Drupal.behaviors.themebuilderBar = {
  attach: ThemeBuilder.Bar.attachToolbar
};
