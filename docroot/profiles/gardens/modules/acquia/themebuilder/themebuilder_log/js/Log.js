/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true debug: true window: true*/

var ThemeBuilder = ThemeBuilder || {};

/**
 * @namespace
 */
ThemeBuilder.Log = ThemeBuilder.Log || {};

/**
 * The various legal values for log entry levels.
 */
ThemeBuilder.Log.ERROR = 1;
ThemeBuilder.Log.WARNING = 2;
ThemeBuilder.Log.INFO = 3;
ThemeBuilder.Log.TRACE = 7;
ThemeBuilder.Log.TIMING = 9;

/**
 * Logs the specified message as a gardens error.  All such log messages will
 * be alerted through nagios.
 *
 * @param {String} message
 *   The log message.
 * @param {String} info
 *   A string containing any additional info associated with the error.
 */
ThemeBuilder.Log.gardensError = function (message, info) {
  ThemeBuilder.Log.writeLogEntry(ThemeBuilder.Log.ERROR, message, true, info, 'GardensError');
};

/**
 * Logs the specified error message.
 *
 * @param {String} message
 *   The log message.
 * @param {String} info
 *   A string containing any additional info associated with the error.
 */
ThemeBuilder.Log.error = function (message, info) {
  ThemeBuilder.Log.writeLogEntry(ThemeBuilder.Log.ERROR, message, true, info, '');
};

/**
 * Logs a gardens warning message.  These messages will be alerted
 * through nagios, but will have the 'GardensWarning' text to work
 * nicely with the email filters described here: https://i.acquia.com/wiki/gardens-e-mail-alert-system-what-you-should-be-doing
 *
 * @param {String} message
 *   A string containing a static message that is exactly the same
 *   for every single instance of the problem being logged.
 * @param {String} info
 *   A string containing any additional info associated with the error.
 */
ThemeBuilder.Log.gardensWarning = function (message, info) {
  ThemeBuilder.Log.writeLogEntry(ThemeBuilder.Log.ERROR, message, true, info, 'GardensWarning');
};

/**
 * Logs the specified warning message.
 *
 * @param {String} message
 *   The log message.
 * @param {String} info
 *   A string containing any additional info associated with the error.
 */
ThemeBuilder.Log.warning = function (message, info) {
  ThemeBuilder.Log.writeLogEntry(ThemeBuilder.Log.WARNING, message, true, info, '');
};

/**
 * Logs the specified info message.
 *
 * @param {String} message
 *   The log message.
 * @param {String} info
 *   A string containing any additional info associated with the error.
 */
ThemeBuilder.Log.info = function (message, info) {
  ThemeBuilder.Log.writeLogEntry(ThemeBuilder.Log.INFO, message, true, info, '');
};

/**
 * Logs the specified trace message.
 *
 * @param {String} message
 *   The log message.
 */
ThemeBuilder.Log.trace = function (message) {
  ThemeBuilder.Log.writeLogEntry(ThemeBuilder.Log.TRACE, message, false, '', '');
};

/**
 * Logs the specified timing message.
 *
 * @param {String} message
 *   The log message.
 */
ThemeBuilder.Log.timing = function (message) {
  ThemeBuilder.Log.writeLogEntry(ThemeBuilder.Log.TIMING, message, false, '', '');
};

/**
 * Logs the specified message at the specified logging level.
 *
 * @param {int} level
 *   The logging level.
 * @param {String} message
 *   The log message.
 * @param {boolean} includeRequestDetails
 *   If true, request details will be added to the log message to make
 *   following up on the issue easier.
 * @param {String} info
 *   A string containing any additional info associated with the error.
 * @param {boolean} tag
 *   A tag that wraps the static part of the message.  This is used
 *   to aid in parsing the logs.  Example: 'GardensError', or
 *   'GardensWarning'.
 */
ThemeBuilder.Log.writeLogEntry = function (level, message, includeRequestDetails, info, tag) {
  // Only send the log message to the server if the log level is set
  // appropriately.
  if (level <= ThemeBuilder.Log.getLogLevel()) {
    if (false !== includeRequestDetails) {
      includeRequestDetails = true;
    }
    if (!info) {
      info = '';
    }
    if (!tag) {
      tag = '';
    }
    var entry = {level: level, message: message, includeRequestDetails: includeRequestDetails, info: info, tag: tag};
    ThemeBuilder.postBack(Drupal.settings.themebuilderLogPath, {logEntry: entry},
      ThemeBuilder.Log.callback);
    ThemeBuilder.logCallback(message);
  }
};

/**
 * This function is called when the log request returns.
 *
 * @param {Object} data
 *   Any data from the response.
 */
ThemeBuilder.Log.callback = function (data) {
};

/**
 * Returns the current log level, which governs whether log messages are sent
 * to the server.
 *
 * @return
 *   The current log level for this site.
 */
ThemeBuilder.Log.getLogLevel = function () {
  if (!ThemeBuilder.Log.logLevel) {
    var data = ThemeBuilder.getApplicationInstance().getData();
    if (data) {
      // Grab the log level from the application data.
      ThemeBuilder.Log.logLevel = data.logLevel;
    }
    else {
      // If logging messages before the app-data is received, we will assume
      // we should only log error messages.
      return ThemeBuilder.Log.ERROR;
    }
  }
  return ThemeBuilder.Log.logLevel;
};
