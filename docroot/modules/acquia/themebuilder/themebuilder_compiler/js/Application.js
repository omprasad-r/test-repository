
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

/**
 * The Application object maintains client-side application state for
 * the themebuilder and can receive updates to the data on each ajax
 * request.
 *
 * Note that Application is implemented as a singleton, so use
 * ThemeBuilder.getApplicationInstance() to get the only instance.
 *
 * @class
 * @constructor
 */
ThemeBuilder.Application = ThemeBuilder.initClass();

/**
 * The version of the JavaScript code comprising the ThemeBuilder.
 *
 * This version is used to determine if a cache clear is required to
 * make the client compatible with the server.
 *
 * Whenever a modification is done to the ThemeBuilder JavaScript
 * code, a cache clear is required.  The individual sites can perform
 * a cache clear as needed by incrementing this version string and the
 * version string in themebuilder_compiler.module (search for
 * 'THEME_BUILDER_JAVASCRIPT_VERSION').  The php version string is sent
 * to the client via Drupal.settings.themebuilderJavaScriptVersion.
 * That version is compared to the one here, and any difference will
 * cause a cache clear to be performed, followed by a page refresh.
 */
ThemeBuilder.Application.version = '1.00.9';

/**
 * Constructor for the Application class.  The Application class is meant to
 * be a singleton, and the constructor enforces that behavior.
 */
ThemeBuilder.Application.prototype.initialize = function () {
  if (ThemeBuilder.Application._instance) {
    throw ('Application is a singleton, please use ThemeBuilder.getApplicationInstance().');
  }

  this.initFunctions = [];
  this.updateFunctions = [];
  this.pollingRequests = {};
  this._requestInitData();
};

/**
 * Private method that requests the initial data from the server.
 */
ThemeBuilder.Application.prototype._requestInitData = function () {
  if (!Drupal.settings.themebuilderInitDataPath) {
    // It isn't possible to request the data until the Drupal settings are
    // established.  There is a security token in the settings which is
    // required to perform the request.
    setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this, this._requestInitData), 50);
    return;
  }

  if (Drupal.settings.themebuilderInEditMode === true && Drupal.settings.themebuilderJavaScriptVersion !== ThemeBuilder.Application.version) {
    // The user is editing a theme and client and server are out of
    // sync.  Clear the JavaScript and CSS caches and reload the page.
    var bar = ThemeBuilder.Bar.getInstance();
    ThemeBuilder.postBack(Drupal.settings.themebuilderClearCachePath, {version: ThemeBuilder.Application.version}, ThemeBuilder.bind(ThemeBuilder.bind(this, this._reloadPage), ThemeBuilder.bind(this, this._reloadPage)));
    return;
  }

  // If the themebuilder cannot save drafts, do not bother to start.
  if (!Drupal.settings.themebuilderWritable) {
    throw ('The theme directory does not appear to be writable.');
  }

  // @todo AN-11140 Currently this loads on every request for admins.
  // We should be smarter about this.
  ThemeBuilder.postBack(Drupal.settings.themebuilderInitDataPath, {},
    ThemeBuilder.bind(this, this._initDataReceived));
};

/**
 * Private method used as a callback for the data request.  Any functions that
 * have been registered as interested in the initialization data will be
 * called when the data is received.
 *
 * @param data
 *   The application initialization data.
 */
ThemeBuilder.Application.prototype._initDataReceived = function (data) {
  this.applicationData = data;
  for (var i = 0; i < this.initFunctions.length; i++) {
    this.initFunctions[i](this.applicationData);
  }
  this.initFunctions = [];
};

/**
 * Called when application data updates have arrived.  This method is
 * responsible for updating the application data with the new values
 * and notifying listeners.
 *
 * @param {Object} data
 *   The key/value pairs representing the changed application data.
 */
ThemeBuilder.Application.prototype.updateData = function (data) {
  for (var name in data) {
    if (data.hasOwnProperty(name)) {
      this.applicationData[name] = data[name];
    }
  }
  this.notifyUpdateListeners(data);
};

/**
 * Adds a function that will be called when the application data is received.
 *
 * @param f
 *   The function to call when data is received.
 */
ThemeBuilder.Application.prototype.addApplicationInitializer = function (f) {
  if (this.applicationData) {
    // The application data has already been received.  Invoke the callback,
    // but do so asyncronously.
    setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this, f, this.applicationData), 0);
  }
  else {
    this.initFunctions.push(f);
  }
};

/**
 * Adds a function that will be called when the application data is updated.
 *
 * @param f
 *   The function to call when data is updated.
 *
 */
ThemeBuilder.Application.prototype.addUpdateListener = function (f) {
  this.updateFunctions.push(f);
};

/**
 * Notifies interested listeners that application data has been updated.
 *
 * @param {Object} data
 *   The key/value pairs representing the changed application data.
 */
ThemeBuilder.Application.prototype.notifyUpdateListeners = function (data) {
  for (var i = 0; i < this.updateFunctions.length; i++) {
    this.updateFunctions[i](data);
  }
};

/**
 * Starts polling the server in a separate thread, to trigger server-side tasks.
 *
 * When this function is called, the current themebuilder instance will begin
 * polling the server at a ten second interval, hitting a special URL designed
 * for this purpose. It does so in a separate thread, so that if the tasks
 * being run on the server take a long time to complete, the performance on the
 * client side is not affected.
 *
 * The server-side code can communicate the results of this polling back to the
 * client side using application data updates, so you can listen for results of
 * this polling by adding an update listener to the themebuilder application;
 * see ThemeBuilder.Application.prototype.addUpdateListener(). Overall, this
 * mechanism can be used for server-side code that needs to queue up several
 * long-running tasks that affect the client side, and only alert the client
 * side when some portion of those task are complete.
 *
 * Multiple parts of the themebuilder may request that polling occur, based on
 * local conditions in that part of the themebuilder; these are differentiated
 * using the requestName parameter to this function. Regardless of how many
 * different requests are made, only one polling thread will be maintained;
 * however, the polling thread will not be stopped during this themebuilder
 * instance until *all* such types of polling requests have been specifically
 * requested to stop (or until ThemeBuilder.Application.prototype.forcePollingToStop()
 * is called).
 *
 * @param {String} requestName
 *   A unique name for this type of polling request. This will typically be
 *   based on the part of the themebuilder code that is making the request.
 *   Every time this function is called with a new requestName, it guarantees
 *   that the current themebuilder instance will not stop polling until
 *   ThemeBuilder.Application.prototype.stopPolling() is called with the same
 *   request name.
 */
ThemeBuilder.Application.prototype.startPolling = function (requestName) {
  // Store the name of the request that initiated the polling.
  this.pollingRequests[requestName] = true;

  // Since a specific request was made, we'll hit the server once immediately
  // (so the caller doesn't have to wait up to 10 seconds for their polling to
  // start).
  setTimeout(ThemeBuilder.bind(this, this._pollServer), 0);

  // If we aren't currently polling, start a new thread that polls once per ten
  // seconds.
  if (!this.pollingId) {
    this.pollingId = setInterval(ThemeBuilder.bind(this, this._pollServer), 10000);
  }
};

/**
 * Make a request to stop polling the server.
 *
 * Polling will only actually stop during this themebuilder instance once *all*
 * types of polling requests that were made are specifically asked to stop.
 *
 * If you need to force polling to stop unconditionally, use
 * ThemeBuilder.Application.prototype.forcePollingToStop() rather than this
 * function.
 *
 * @param {String} requestName
 *   The name of the type of polling request that you would like to stop. This
 *   should match what your code passed in when it made the request to start
 *   polling via ThemeBuilder.Application.prototype.startPolling().
 */
ThemeBuilder.Application.prototype.stopPolling = function (requestName) {
  // Remove the name of the request for the list of active polling requests.
  delete this.pollingRequests[requestName];

  // If we are currently polling and there are no more active polling requests,
  // stop polling now.
  if (this.pollingId && !this.pollingRequests.length) {
    clearInterval(this.pollingId);
    delete this.pollingId;
  }
};

/**
 * Force the themebuilder to stop polling the server.
 *
 * This will cause polling to stop regardless of whether there are still active
 * polling requests. In most cases, rather than calling this function you
 * should call ThemeBuilder.Application.prototype.stopPolling() instead, so as
 * not to interfere with other code that may want the polling to continue.
 */
ThemeBuilder.Application.prototype.forcePollingToStop = function () {
  this.pollingRequests = {};
  if (this.pollingId) {
    clearInterval(this.pollingId);
    delete this.pollingId;
  }
};

/**
 * Make an asynchronous polling request to the server.
 */
ThemeBuilder.Application.prototype._pollServer = function () {
  ThemeBuilder.getBack('themebuilder-phone-home');
};

/**
 * Returns the application data if it has already been loaded.
 */
ThemeBuilder.Application.prototype.getData = function () {
  return (this.applicationData);
};

/**
 * Returns an instance of the ThemeBuilder.Settings class that indicates the
 * current themebuilder settings.  This class should be used rather than the
 * raw data from the themebuilder-init-data request because it allows objects
 * to register callbacks when various settings are changed.
 *
 * @return
 *   The themebuilder settings.
 */
ThemeBuilder.Application.prototype.getSettings = function () {
  if (!ThemeBuilder.Settings._instance) {
    var data = this.getData();
    if (!data) {
      throw ('Requested settings before the application initialization data has been received.');
    }
    ThemeBuilder.Settings._instance = new ThemeBuilder.Settings(data.app_settings);
  }
  return ThemeBuilder.Settings._instance;
};

/**
 * Indicates whether we are running in a preproduction configuration or not.
 *
 * If in preproduction mode, we may have features enabled for testing
 * that we would not have enabled in production.  This value is set in
 * the gardens_preproduction module, which should be enabled for
 * preproduction mode or disabled otherwise.
 *
 * @return {Boolean}
 *   True if running in preproduction mode; false otherwise.
 */
ThemeBuilder.Application.prototype.preproductionMode = function () {
  return undefined !== Drupal.settings.gardens_preproduction;
};

/**
 * Reloads the page.
 *
 * This is used to refresh the page if the JavaScript version and the
 * ThemeBuilder version do not match.
 *
 * @private
 */
ThemeBuilder.Application.prototype._reloadPage = function () {
  parent.location.reload(true);
};

/**
 * Returns the Application instance.  Application is a singleton, and more than
 * one instantiation of the Application class will result in an exception.
 *
 * @return
 *   The Application instance.
 */
ThemeBuilder.getApplicationInstance = function () {
  if (!ThemeBuilder.Application._instance) {
    ThemeBuilder.Application._instance = new ThemeBuilder.Application();
  }
  return ThemeBuilder.Application._instance;
};
