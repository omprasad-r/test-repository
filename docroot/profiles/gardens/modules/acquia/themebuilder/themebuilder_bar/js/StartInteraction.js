/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/
"use strict";

/**
 * The StartInteraction is an interaction that is responsible for opening the themebuilder.
 * 
 * This act is complicated by several factors including error
 * conditions that cause the open to fail, existing themebuilder
 * sessions in other webnodes, and theme corruption that might require
 * the intervention of the theme elves.
 * @class
 * @extends ThemeBuilder.InteractionController
 */
ThemeBuilder.StartInteraction = ThemeBuilder.initClass();
ThemeBuilder.StartInteraction.prototype = new ThemeBuilder.InteractionController();

ThemeBuilder.StartInteraction._lock = false;

/**
 * Constructor for the StartInteraction.
 * 
 * @param {Object} callbacks
 *   The set of callbacks that will be called upon the completion of this interaction.
 */
ThemeBuilder.StartInteraction.prototype.initialize = function (callbacks) {
  this.setInteractionTable({
    begin: 'prepareUIForOpen',
    userNotified: 'verifyWebnode',

    webnodeConfirmed: 'setCookie',
    cookieSet: 'startThemeBuilder',
    cookieNotSet: 'exitWithMessage',

    failedToGetWebnode: 'setGenericExitMessage',

    themeBuilderStarted: 'reloadPage', // This constitutes the end of this interaction.

    // The themebuilder session is open in another browser.
    editSessionInProgress: 'showTakeOverSessionConfirmation',
    takeoverAccepted: 'takeoverSession',
    takeoverCanceled: 'openCanceled',

    // Exceptions:
    openFailed: 'handleOpenFailure',
    errorOnOpen: 'invokeThemeElves',
    themeElfSuccess: 'startThemeBuilder',
    themeElfFailure: 'setGenericExitMessage',

    // Cancel the open request.
    openCanceled: 'cancelOpen',
    exitMessageSet: 'exitWithMessage',
    exitMessageDismissed: 'cancelOpen'
  });
  this.setCallbacks(callbacks);

  // Create the redirects.
  this.openCanceled = this.makeEventCallback('openCanceled');
  this.errorOnOpen = this.makeEventCallback('errorOnOpen');

  this.errorMap = ['error', 'errorName', 'errorType'];
};

/**
 * Prepares the user interface for opening the themebuilder.
 * 
 * This includes displaying a spinner to indicate the application has
 * accepted the user's request and is working.
 * 
 * If this interaction has been locked, the interaction will stop
 * immediately with no further action taken.  The lock indicates the
 * themebuilder open is already in progress, and prevents the user
 * from being able to cause multiple start processes simultaneously.
 * 
 * @param {Object} data
 *   The data object that is passed in to every state in the
 *   interaction.  This data object collects information from the
 *   system state and the user's choices and facilitates moving the
 *   application into the opened state.
 */
ThemeBuilder.StartInteraction.prototype.prepareUIForOpen = function (data) {
  var $ = jQuery;
  if (this.isLocked()) {
    // The user is in the middle of opening the themebuilder.  Ignore the request.
    return;
  }
  this.setLock(true);
  var bar = ThemeBuilder.Bar.getInstance();
  bar.showWaitIndicator();
  Drupal.toolbar.collapse();

  // The themebuilder start options provide a mechanism for taking
  // over a themebuilder session.  By default this option is not set
  // so the user will be prompted before a session is taken.
  if (!data) {
    data = {startOptions: {}};
  }
  data.elfInvocations = 0;
  this.event(data, 'userNotified');
};

/**
 * Verifies the webnode that will be used to connect to the themebuilder.
 * 
 * If the webnode provided when the page was loaded is too old, this
 * method will request the webnode from the server.
 * 
 * @param {Object} data
 *   The data object that is passed in to every state in the
 *   interaction.  This data object collects information from the
 *   system state and the user's choices and facilitates moving the
 *   application into the opened state.
 */
ThemeBuilder.StartInteraction.prototype.verifyWebnode = function (data) {
  // Check that we have a current server name for a cookie.  If not, request
  // one via ajax.  Set the cookie before opening the ThemeBuilder.
  var info = Drupal.settings.themebuilderServer;
  var d = new Date();
  var time = Math.floor(d.getTime() / 1000);
  if (info && info.webnode && time <= info.time) {
    // Set the webnode into the data.
    data.server = info;
    this.event(data, 'webnodeConfirmed');
  }
  else {
    // Request the webnode from the server.
    var map = [
      'server', 'type'
    ];
    ThemeBuilder.postBack('themebuilder-get-server', {}, ThemeBuilder.bind(this, this.event, map, data, 'webnodeConfirmed'), ThemeBuilder.bind(this, this.event, this.errorMap, data, 'failedToGetWebnode'));
  }
};

/**
 * Establishes a cookie that causes all requests to go to the webnode specified
 * in the parameter and causes a themebuilder edit session to open.
 * 
 * The open is accomplished by sending a request to the server, and
 * upon success the page must be reloaded.
 *
 * @param {Object} data
 *   The data object that is passed in to every state in the
 *   interaction.  This data object collects information from the
 *   system state and the user's choices and facilitates moving the
 *   application into the opened state.
 */
ThemeBuilder.StartInteraction.prototype.setCookie = function (data) {
  var $ = jQuery;
  var webnode = data.server.webnode;
  var bar = ThemeBuilder.Bar.getInstance();

  // Note: Do not set the expires to 0.  That is the default, but this
  // results in the cookie not being set in IE.
  $.cookie('ah_app_server', data.server.webnode, {path: '/'});

  // Verify that the cookie exists.  Note that if the cookie was set
  // incorrectly, the problem is not always detectable via JavaScript,
  // but we can at least try knowing that we can't totally trust the
  // results.
  if ($.cookie('ah_app_server') === data.server.webnode) {
    this.event(data, 'cookieSet');
  }
  else {
    data.userMessage = Drupal.t("The ThemeBuilder cannot be started, possibly because your browser's privacy settings are too strict.");
    this.event(data, 'cookieNotSet');
  }
};

/**
 * Opens the themebuilder.
 * 
 * The open is accomplished by sending a request to the server, and
 * upon success the page must be reloaded.
 *
 * @param {Object} data
 *   The data object that is passed in to every state in the
 *   interaction.  This data object collects information from the
 *   system state and the user's choices and facilitates moving the
 *   application into the opened state.
 */
ThemeBuilder.StartInteraction.prototype.startThemeBuilder = function (data) {
  var map = ['start', 'type'];
  ThemeBuilder.postBack('themebuilder-start', data.startOptions,
    ThemeBuilder.bind(this, this.event, map, data, 'themeBuilderStarted'), ThemeBuilder.bind(this, this.event, this.errorMap, data, 'openFailed'));
};

/**
 * Causes the page to refresh.
 * 
 * @param {Object} data
 *   The data object that is passed in to every state in the
 *   interaction.  This data object collects information from the
 *   system state and the user's choices and facilitates moving the
 *   application into the opened state.
 */
ThemeBuilder.StartInteraction.prototype.reloadPage = function (data) {
  // Reload, which is required even if there are no changes to the
  // theme to make sure the gears work after the themebuilder is
  // closed.
  this.setLock(false);
  parent.location.reload(true);
};

/**
 * Handles any error condition returned from the server.
 * 
 * This method is responsible for inspecting the error and putting
 * this controller into the appropriate state to deal with the
 * problem.
 * 
 * @param {Object} data
 *   The data object that is passed in to every state in the
 *   interaction.  This data object collects information from the
 *   system state and the user's choices and facilitates moving the
 *   application into the opened state.
 */
ThemeBuilder.StartInteraction.prototype.handleOpenFailure = function (data) {
  if (data.errorName === 'ThemeBuilderEditInProgressException') {
    this.event(data, 'editSessionInProgress');
    this.setLock(false);
  }
  if (data.errorName === 'error') {
    this.event(data, 'errorOnOpen');
  }
  this._clearError(data);
};

/**
 * Shows a confirmation dialog to take over the session.
 * 
 * If an existing TB editing session is underway in another browser and a user
 * tries to enter edit mode, this will fire.  If the user accepts, we will re-try
 * to start the session, this time forcing it to take precedence.
 * 
 * @param {Object} data
 *   The data object that is passed in to every state in the
 *   interaction.  This data object collects information from the
 *   system state and the user's choices and facilitates moving the
 *   application into the opened state.
 */
ThemeBuilder.StartInteraction.prototype.showTakeOverSessionConfirmation = function (data) {
  var $ = jQuery;
  var message = Drupal.t('An active draft exists from a previous session.  Click Cancel if you do not want to edit the appearance in this browser, otherwise click OK to take over the draft in this window.');

  // This markup will be displayed in the dialog.
  var inputId = 'active-draft-exists';
  var $html = $('<div>').append(
    $('<img>', {
      src: Drupal.settings.themebuilderAlertImage
    }).addClass('alert-icon'),
    $('<span>', {
      html: message
    })
  );
  var dialog = new ThemeBuilder.ui.Dialog($('body'),
    {
      html: $html,
      buttons: [
        {
          label: Drupal.t('OK'),
          action: this.makeEventCallback('takeoverAccepted', data)
        },
        {
          label: Drupal.t('Cancel'),
          action: this.makeEventCallback('takeoverCanceled', data)
        }
      ],
      // The default action, which is invoked if the user hits Esc or
      // the 'x' in the dialog.
      defaultAction: this.makeEventCallback('takeoverCanceled', data)
    }
  );
};

/**
 * Causes the existing themebuilder session to be taken over.
 * 
 * This allows the user to get into the themebuilder even if they
 * already have a session open in a different browser.  The other
 * session in the other browser will be closed as a result.
 * 
 * @param {Object} data
 *   The data object that is passed in to every state in the
 *   interaction.  This data object collects information from the
 *   system state and the user's choices and facilitates moving the
 *   application into the opened state.
 */
ThemeBuilder.StartInteraction.prototype.takeoverSession = function (data) {
  data.startOptions = {'take_over_session': true};
  this.startThemeBuilder(data);
};

/**
 * Invoked when the themebuilder open is canceled.
 * 
 * This could be due to error conditons or because the user chose 'Cancel' when the message about an existing themebuilder session appears.
 * 
 * This method is responsible for cleaning up the UI such that the
 * user can continue interacting with the site and click the
 * Appearance button again if desired.
 * 
 * @param {Object} data
 *   The data object that is passed in to every state in the
 *   interaction.  This data object collects information from the
 *   system state and the user's choices and facilitates moving the
 *   application into the opened state.
 */
ThemeBuilder.StartInteraction.prototype.cancelOpen = function (data) {
  var bar = ThemeBuilder.Bar.getInstance();
  bar.hideWaitIndicator();
  Drupal.toolbar.expand();
  this.setLock(false);
};

/**
 * Causes the theme elves to be invoked.
 * 
 * The theme elves are server-side code that detect and correct
 * problems in themes that may prevent the themebuilder from opening.
 * If there is an error that prevents the user from starting the
 * themebuilder, it is a good idea to give the theme elves a chance to
 * fix it and try again.
 * 
 * @param {Object} data
 *   The data object that is passed in to every state in the
 *   interaction.  This data object collects information from the
 *   system state and the user's choices and facilitates moving the
 *   application into the opened state.
 */
ThemeBuilder.StartInteraction.prototype.invokeThemeElves = function (data) {
  data.elfInvocations++;
  if (data.elfInvocations > 2) {
    // Be careful not to invoke the elves an infinite number of times.
    // It takes a substantial amount of time to run them and it is
    // very unlikely to be useful running them more than twice.
    this.event(data, 'themeElfFailure');
    return;
  }
  var map = ['recovery', 'type'];
  ThemeBuilder.postBack('themebuilder-fix-themes', {}, ThemeBuilder.bind(this, this.event, map, data, 'themeElfSuccess'), ThemeBuilder.bind(this, this.event, this.errorMap, 'themeElfFailure'));
};

/**
 * This method displays a dialog that indicates to the user that the themebuilder was unable to start.
 * 
 * @param {Object} data
 *   The data object that is passed in to every state in the
 *   interaction.  This data object collects information from the
 *   system state and the user's choices and facilitates moving the
 *   application into the opened state.
 */
ThemeBuilder.StartInteraction.prototype.setGenericExitMessage = function (data) {
  if (Drupal.settings.gardensMisc.isSMB) {
    data.userMessage = Drupal.t('An error has occurred. Please let us know what you tried to do in the <a target="_blank" href="http://www.drupalgardens.com/forums">Drupal Gardens forums</a>, and we will look into it.');
  }
  else {
    data.userMessage = Drupal.t('An error has occurred. Please contact support to let us know what you tried to do and we will look into it.');
  }
  this.event(data, 'exitMessageSet');
};

/**
 * Retrieves the state of the lock.
 *
 * @return {boolean}
 *   true if the lock is set; false otherwise.
 */
ThemeBuilder.StartInteraction.prototype.isLocked = function () {
  return ThemeBuilder.StartInteraction._lock;
};

/**
 * Sets the state of the start lock.
 * 
 * @param {boolean} isLocked
 *   If true, the lock will be set; otherwise the lock will be cleared.
 */
ThemeBuilder.StartInteraction.prototype.setLock = function (isLocked) {
  ThemeBuilder.StartInteraction._lock = isLocked === true;
};

/**
 * Displays a dialog that presents a message stored in data.userMessage.
 * 
 * @param {Object} data
 *   The data object that is passed in to every state in the
 *   interaction.  This data object collects information from the
 *   system state and the user's choices and facilitates moving the
 *   application into the opened state.
 */
ThemeBuilder.StartInteraction.prototype.exitWithMessage = function (data) {
  var $ = jQuery;
  var dialog = new ThemeBuilder.ui.Dialog($('body'), {
    html: $('<span>').html(data.userMessage),
    buttons: [
      {
        label: Drupal.t('OK'),
        action: this.makeEventCallback('exitMessageDismissed', data)
      }
    ]
  });
};

/**
 * Clears the error from the specified data object.
 * 
 * When an error occurs, there are fields that are added to the data
 * object that must be cleared so the error won't be detected
 * erroneously on subsequent requests.  This method clears those
 * fields so the interaction can continue.
 * 
 * @private
 * @param {Object} data
 *   The data object that is passed in to every state in the
 *   interaction.  This data object collects information from the
 *   system state and the user's choices and facilitates moving the
 *   application into the opened state.
 */
ThemeBuilder.StartInteraction.prototype._clearError = function (data) {
  delete data.error;
  delete data.errorName;
  delete data.errorType;
};
