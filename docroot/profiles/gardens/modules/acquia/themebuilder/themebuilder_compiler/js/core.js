/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true debug: true window: true*/

/**
 * @namespace
 */
var Drupal = Drupal || {};
/**
 * @namespace
 */
Drupal.behaviors = Drupal.behaviors || {};

/**
 * The ThemeBuilder is a namespace in which all themebuilder-specific
 * code will reside.
 *
 * @namespace
 */
var ThemeBuilder = ThemeBuilder || {};

/**
 * Binds an object method to the object so it the method can be used as a
 * callback that normally takes a function.  This scheme helps control the
 * creation of closures and the specific variables included in the closure's
 * environment to help avoid memory leaks.
 *
 * Caller arguments may be ignored if desired.  This is often helpful
 * when using an existing function to handle an event when the target
 * function doesn't take the event as an argument.
 *
 * The calling context (the 'this' reference) may be passed as an argument.
 *
 * All arguments past the 'method' argument will be stored within the resulting
 * closure and used as parameters on the method call.
 *
 * The order of parameters is [caller args], [this], [bound args].
 *
 * @function
 * @param {Object} object
 *   The object instance which holds the specified method.
 * @param {Function} method
 *   The method of the specified object to call.
 * @param {boolean} ignoreCallerArgs
 *   Optional, default is false.  If true, all arguments supplied by
 *   the caller of the resulting function will not be passed to the
 *   target method.
 * @param {boolean} passThisAsArg
 *   Optional, default is false.  If true, the context of the call to
 *   the resulting function will be passed as the first argument.
 */
ThemeBuilder.bindFull = function (object, method, ignoreCallerArgs, passThisAsArg) {
  if (!object) {
    if (ThemeBuilder.isDevelMode()) {
      // For production it is best not to throw an exception here.  If
      // there is a bug, likely the user will not notice it, but
      // throwing an exception will cause whatever behavior to
      // entirely break even before they try to use it.  Good
      // development stuff, but a poor production experience.
      throw ('Object cannot be null.');
    }
  }
  if (!method) {
    if (ThemeBuilder.isDevelMode()) {
      throw ('Method cannot be null.');
    }
  }
  if (ignoreCallerArgs !== true && ignoreCallerArgs !== false) {
    ignoreCallerArgs = false;
  }
  if (passThisAsArg !== true && passThisAsArg !== false) {
    passThisAsArg = false;
  }
  var _bind_args = [];  // Arguments passed into the bind call.
  for (var i = 4; i < arguments.length; i++) {
    _bind_args.push(arguments[i]);
  }
  i = undefined;
  return function () {
    var invocation_args = []; // Arguments called when the function is called.
    if (false === ignoreCallerArgs) {
      for (var i = 0; i < arguments.length; i++) {
        invocation_args.push(arguments[i]);
      }
    }
    if (true === passThisAsArg) {
      invocation_args.push(this);
    }
    var args = invocation_args.concat(_bind_args); // Final arguments.
    return method.apply(object, args);
  };
};

/**
 * Binds an object method to the object so it the method can be used as a
 * callback that normally takes a function.  This scheme helps control the
 * creation of closures and the specific variables included in the closure's
 * environment to help avoid memory leaks.
 *
 * All arguments past the 'method' argument will be stored within the resulting
 * closure and used as parameters on the method call.
 *
 * @param {Object} object
 *   The object instance which holds the specified method.
 * @param {Function} method
 *   The method of the specified object to call.
 */
ThemeBuilder.bind = function (object, method) {
  var args = [object, method, false, false];
  for (var i = 2; i < arguments.length; i++) {
    args.push(arguments[i]);
  }
  return ThemeBuilder.bindFull.apply(this, args);
};

/**
 * Binds an object method to the object so it the method can be used as a
 * callback that normally takes a function.  This scheme helps control the
 * creation of closures and the specific variables included in the closure's
 * environment to help avoid memory leaks.
 *
 * All arguments past the 'method' argument will be stored within the resulting
 * closure and used as parameters on the method call.  Arguments passed from
 * the caller will be ignored.
 *
 * @param object Object
 *   The object instance which holds the specified method.
 * @param method Function
 *   The method of the specified object to call.
 */
ThemeBuilder.bindIgnoreCallerArgs = function (object, method) {
  var args = [object, method, true, false];
  for (var i = 2; i < arguments.length; i++) {
    args.push(arguments[i]);
  }
  return ThemeBuilder.bindFull.apply(this, args);
};

/**
 * Clones the specified object.
 *
 * @param object mixed
 *   The object to clone.
 * @return
 *   The cloned object.
 */
ThemeBuilder.clone = function (object) {
  if (!object || typeof(object) !== 'object') {
    return object;
  }
  var temp;
  if (object.constructor === Array) {
    temp = [];
    for (var i = 0; i < object.length; i++) {
      temp.push(ThemeBuilder.clone(object[i]));
    }
    i = undefined;
  }
  else {
    temp = {};
    for (var key in object) {
      if (true) { // Make jslint happy
        try {
          temp[key] = ThemeBuilder.clone(object[key]);
        }
        catch (err) {
          // On IE we sometimes encounter an error when cloning the
          // computedStyles object on the outlineWidth property.
        }
      }
    }
    key = undefined;
  }
  return temp;
};

/**
 * Merges two objects
 */
ThemeBuilder.merge = function (destination, source) {
  var output = ThemeBuilder.clone(destination);
  for (var property in source) {
    // Filter out inherited properties
    if (source.hasOwnProperty(property)) {
      // Save to destination object. Overwrite if the property already exists.
      output[property] = source[property];
    }
  }
  return output;
};

/**
 * This function returns a new Class initializer.
 *
 * @return
 *   A new Class initializer which allows new instances to be created easily
 *   and supports a constructor method called 'initialize'.
 */
ThemeBuilder.initClass = function () {
  return function () {
    this.initialize.apply(this, arguments);
  };
};

/**
 * Simulates the behavior in typical OO languages by making a child class.
 *
 * @return void;
 */
ThemeBuilder.extend = function (subclass, superclass) {
  function Dummy() {}
  Dummy.prototype = superclass.prototype;
  subclass.prototype = new Dummy();
  subclass.prototype.constructor = subclass;
  subclass.superclass = superclass;
  subclass.superproto = superclass.prototype;
};

/**
 * Simple array-based stack implementation.  Implemented so the caller cannot
 * destroy the stack or inspect the data in ways other than what a stack would
 * permit.
 *
 * @class
 * @constructor
 */
ThemeBuilder.Stack = ThemeBuilder.initClass();

/**
 * Constructor for the stack.  Initialize the array that contains the data in
 * the stack.
 */
ThemeBuilder.Stack.prototype.initialize = function () {
  this._data = [];
  this._listeners = [];
};

/**
 * Add the specified item to the stack.
 *
 * @param item object
 *   The item to add to the top of the stack.
 */
ThemeBuilder.Stack.prototype.push = function (item) {
  this._data.push(item);
  this.notifyListeners();
};

/**
 * Removes the item from the top of the stack.  If there is nothing in the stack,
 * null is returned.
 *
 * @return
 *   The item at the top of the stack.
 */
ThemeBuilder.Stack.prototype.pop = function () {
  var modification = this._data.pop();
  this.notifyListeners();
  return modification;
};

/**
 * Returns a copy of the item at the top of the stack, without actually removing
 * the item from the stack.  The item is cloned so that the client code will
 * not inadvertently alter the copy of the object and by doing so destroy the
 * integrity of the stack.
 *
 * @return
 *   A copy of the item at the top of the stack.  This is useful for finding out
 *   information about the top item without actually removing it from the stack.
 */
ThemeBuilder.Stack.prototype.peek = function () {
  var size = this.size();
  if (size <= 0) {
    return null;
  }
  var obj = this._data[size - 1];
  return ThemeBuilder.clone(obj);
};

/**
 * Clears the contents of the stack.
 */
ThemeBuilder.Stack.prototype.clear = function () {
  this._data = [];
  this.notifyListeners();
};

/**
 * Returns the size of the stack.
 */
ThemeBuilder.Stack.prototype.size = function () {
  return this._data.length;
};

/**
 * Adds a stack listener which is called when the stack is changed in any way.
 *
 * @param listener object
 *   An object with a stackChanged method.
 */
ThemeBuilder.Stack.prototype.addChangeListener = function (listener) {
  this._listeners.push(listener);
};

/**
 * Removes the specified listener from the stack.
 *
 * @param listener object
 *   The listener to remove.
 */
ThemeBuilder.Stack.prototype.removeChangeListener = function (listener) {
  var listeners = [];
  for (var i = 0; i < this._listeners.length; i++) {
    if (this._listeners[i] !== listener) {
      listeners.push(this._listener[i]);
    }
  }
  this._listeners = listeners;
};

/**
 * Notifies the listeners that a change to the state of this stack has occurred.
 */
ThemeBuilder.Stack.prototype.notifyListeners = function () {
  for (var i = 0; i < this._listeners.length; i++) {
    this._listeners[i].stackChanged(this);
  }
};

/**
 * Returns true if the specified object is an array.
 *
 * @return
 *   true if the object is an array; false otherwise.
 */
ThemeBuilder.isArray = function (obj) {
  return Object.prototype.toString.apply(obj) === '[object Array]';
};

/**
 * Returns the last element of the array.
 *
 * @return {mixed}
 *   The last element.
 */
Array.prototype.last = function () {
  // jQuery sometimes (e.g., within $.param()) iterates an array and for every
  // value that's a function, calls the function without an object context, so
  // we can't assume that "this" is an array.
  // @todo Test if jQuery 1.5 has this bug, and if so, report it.
  if (this.slice) {
    return this.slice(-1)[0];
  }
};

/**
 * Returns true if this array contains the specified object.
 */
Array.prototype.contains = function (obj) {
  var i = this.length;
  while (i--) {
    if (this[i] === obj) {
      return true;
    }
  }
  return false;
};

/**
 * Wrapper around ThemeBuilder.sendRequest
 *
 * @see ThemeBuilder.sendRequest
 *
 */
ThemeBuilder.postBack = function (path, data, success_callback, error_callback, ajax_params) {
  ThemeBuilder.sendRequest("POST", path, data, success_callback, error_callback, ajax_params);
};


/**
 * Wrapper around ThemeBuilder.sendRequest
 *
 * @see ThemeBuilder.sendRequest
 *
 */
ThemeBuilder.getBack = function (path, data, success_callback, error_callback, ajax_params) { // handles tokens etc.
  ThemeBuilder.sendRequest("GET", path, data, success_callback, error_callback, ajax_params);
};

/**
 * Centalized handler for Ajax requests.  Handles errors and tokens.
 *
 * @param {String} method POST or GET
 * @param {String} path The Drupal path you are trying to request
 * @param {Object} data The data you wish to send
 * @param {Function} success_callback A function to be called on a correctly parsed
 * @param {Function} error_callback A function to be called if an error is thrown
 * @param {Object} ajax_params Additional Ajax Params to replace defaults
 * @param {Integer} retryCount The # of retries that have been attempted for this request.
 *
 *                          @see http://docs.jquery.com/Ajax/jQuery.ajax#options
 *
 */
ThemeBuilder.sendRequest = function (method, path, data, success_callback, error_callback, ajax_params, retryCount) {
  // @todo: Store this somewhere more sensibly.
  var maxRetries = 3; // There is the first attempt plus 3 retries if needed.
  var retryDelay = 5; // 5 seconds

  if (!data) {
    data = {};
  }

  if (!retryCount) {
    retryCount = 0;
  }

  /**
   * This function is called when the response arrives, no matter if
   * the request was successful or not.  This function allows
   * preprocessing of the data if needed.
   *
   * @param {object} data
   *   The data received from the server.
   */
  var pre_processing = function (data) {
    if (data && data.app_data) {
      ThemeBuilder.getApplicationInstance().updateData(data.app_data);
      delete data.app_data;
    }
  };

  /**
   * This function is called after the response arrives, no matter if the request
   * was successful or not.  This function allows any cleanup or setting of state.
   * I am using this function to enable the undo buttons, if they were disabled
   * as a result of the request.  Simply add a statusKey field, which holds the
   * key returned from invoking ThemeBuilder.undoButtons.disable().
   */
  var post_processing = function () {
    if (data.statusKey && ThemeBuilder.undoButtons) {
      ThemeBuilder.undoButtons.clear(data.statusKey);
    }
  };

  /**
   * This function wraps the success_callback passed in by the caller.  No matter
   * if the request succeeds or fails, there is post processing that needs to be
   * done.
   *
   * @param {object} data
   *   The data received from the server.
   * @param {string} type
   *   Indicates the type of response.
   */
  var success_wrapper = function (data, type) {
    pre_processing(data);
    if (data && data.exception === true) {
      // This was a non-fatal error.
      // It was triggered via a PHP exception, so we're trying to preserve
      // as much of the exception information as possible.
      // Contrast this with an actual error (below) which would be a parsererror
      // or a HTTP status code != 200.
      if (error_callback) {
        error_callback(data, data.type, 'recoverableError');
      }
      else {
        ThemeBuilder.handleError(data, data.type, 'recoverableError');
      }
    }
    else if (success_callback) {
      success_callback(data, type);
    }
    post_processing();
  };

  var error_wrapper = function (responseData, type, errorThrown) {
    pre_processing(responseData);
    if (responseData.status === 502 || responseData.status === 503 || responseData.status === 504) {
      if (retryCount >= maxRetries) {
        // We were not able to get a response from the server even after
        // several attempts.  Close the themebuilder.
        ThemeBuilder.Log.gardensError('AN-22452 - Failed to successfully submit a request after multiple attempts.', 'Tried ' + retryCount + ' times before failing.  Forcibly closing the themebuilder.');
        var error_data = {type: 'ThemebuilderException', exception: true, handlers: ['alertAndClose']};
        error_data.message = Drupal.t("Oops.  We were unable to process your request (service too busy).  The ThemeBuilder must close now.  After closing, please wait and try changing your site's appearance again.  If you see this message multiple times, please contact support for assistance.");
        ThemeBuilder.handleError(error_data, error_data.type, 'recoverableError');
        return;
      }
      // The delay is 5 * (2^n), where n goes from 0 to 2.  This
      // results in delays of 5s, 10s, 20s, for a total of 35 seconds
      // between the initial attempt and the final attempt (plus
      // failed request time).  Note that the configuration of the
      // balancer causes a webnode to come out of rotation for 30
      // seconds if it is failing to respond to requests.  By spanning
      // that time with the retry attempts, there is a good chance the
      // web server will come back up.  Complete web server failures
      // are rare.  When we don't delay at all, we are noticing about
      // 12 failures in which the themebuilder is forcibly closed per
      // day.
      var delay = retryDelay * Math.pow(2, retryCount);
      retryCount += 1;

      // Note: Do not use ThemeBuilder.Log for this message because it
      // communicates back to the server and this block of code is
      // only used when there are problems communicating with the
      // server.
      ThemeBuilder.logCallback('Request failed, waiting ' + delay + ' seconds for ' + path);

      // Retry the request after a delay.
      setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this, ThemeBuilder.sendRequest, method, path, data, success_callback, error_callback, ajax_params, retryCount), delay * 1000);
      return;
    }
    if (responseData.status === 200 && responseData.responseText === "") {
      // JS: How should we handle empty payloads?  Will throw a parsing error.
      // For now, going to just call the success callback, if it is a 200 code
      if (success_callback) {
        success_callback({});
      }
      post_processing();
      return;
    }
    ThemeBuilder.handleError(responseData, type, errorThrown);

    if (error_callback) {
      error_callback(responseData, type, errorThrown);
    } else {
      ThemeBuilder.handleError(responseData, type, errorThrown);
    }
    post_processing();
  };

  data.form_token = ThemeBuilder.getToken(path);
  // About to send an ajax request.  Make certain that the affinity
  // cookie is set.  Keep in mind that some users will delete their
  // cookies because they can.
  if (!jQuery.cookie('ah_app_server')) {
    if (Drupal && Drupal.settings && Drupal.settings.themebuilderServer) {
      jQuery.cookie('ah_app_server', Drupal.settings.themebuilderServer.webnode);
    }
  }

  jQuery.ajax(jQuery.extend({
    async: true,
    type: method,
    cache: false,
    url: Drupal.settings.callbacks[path].url,
    dataType: "json",
    data: data,
    success: success_wrapper,
    error: error_wrapper
  }, ajax_params));
};

/**
 * Handles errors which are not caught by the code making an ajax request.
 *
 * If the error is recoverable, it will iterate through specified fallback
 * handlers.
 *
 * @see themebuilder_compiler.php.
 *
 * @param mixed data
 *   The return from an ajax call.  If errorThrown is a 'recoverableError' it will
 *   be an object with properties like code, message and handlers.  Otherwise,
 *   it will be a string which is sent on serious fatal errors.
 *
 * @param string type
 *   If thrown as a recoverableError (see themebuilder_compile.php), this will
 *   be the name of the exception handling class.  Otherwise, will be provided
 *   by jQuery ajax and will be textStatus from the error function
 *   @see http://api.jquery.com/jQuery.ajax/.
 *
 * @param mixed errorThrown
 *   If a recoverable error will be 'recoverableError', if an ajax error will
 *   be an exception because the call was unable to be made.
 */
ThemeBuilder.handleError = function (data, type, errorThrown) {
  if (errorThrown === 'recoverableError') {
    if (data.handlers) {
      for (var i in data.handlers) {
        if (data.handlers.hasOwnProperty(i)) {
          var function_name = data.handlers[i];
          ThemeBuilder.errorHandler[function_name](data, type, errorThrown);
        }
      }
    }
  } else {
    ThemeBuilder.errorHandler.logSilently(data, type, errorThrown);
  }
};

/**
 * Mostly used in postBack and getBack methods
 *
 * @param path String
 *    The relative path (what you pass to url() in Drupal) a token is required for.
 *
 * @return string
 *   The token
 */
ThemeBuilder.getToken = function (path) {
  if (!path) {
    throw Drupal.t('Path argument is required when calling ThemeBuilder.getToken');
  }
  if (!Drupal.settings.callbacks[path] && !Drupal.settings.callbacks[path].token) {
    throw Drupal.t('Invalid callback specified or no token exists: ') + path;
  }
  return Drupal.settings.callbacks[path].token;
};


/**
 * Slightly modified version of jQuery.load to allow us to integrate tokens and
 * callbacks.
 *
 * @param {jQuery} element The Jquery element you wish to replace HTML into
 * @param {String} path The path you wish to request
 * @param {Object} data Key-Value pairs to send
 * @param {function} callback
 * @param {String} selector a jQuery expression to use on the requested HTML (if any).
 *
 * @return void
 *
 */
ThemeBuilder.load = function (element, path, data, callback, selector, sync) {
  var page = window.location.pathname.substring(Drupal.settings.basePath.length);
  if (page === '') {
    page = '<front>';
  }
  data = jQuery.extend(data, {"page": page});
  ThemeBuilder.sendRequest('GET', path, data,
    function (res, status) {
      if (!ThemeBuilder.util.isHtmlMarkup(res)) {
        // This is not html.  Interpret it as a JSON object.
        try {
	  /*jslint evil: true */
          var obj = eval('(' + res + ')');
	  /* jslint evil: false */
          if (obj.exception && obj.type) {
            ThemeBuilder.handleError(obj, obj.type, 'recoverableError');
            return;
          }
        }
        catch (e) {
          // Ok, so it is not a JSON object.  Go ahead and display the content.
        }
      }
    //ripped this from jquery.load
    // If successful, inject the HTML into all the matched elements
      if (status === "success" || status === "notmodified") {
        // See if a selector was specified
        element.html(selector ?
          // Create a dummy div to hold the results
          jQuery("<div/>")
            // inject the contents of the document in, removing the scripts
            // to avoid any 'Permission Denied' errors in IE
            // JS: jslint doesn't like the dot in the regex, but don't know why.
            // This is ripped from jQuery core directly.
          .append(res.replace(new RegExp('<script(.|\\s)*?\\/script>', 'g'), ""))

            // Locate the specified elements
            .find(selector) :

          // If not, just inject the full result
          res);
      }
      if (callback) {
        callback(res, status);
      }
    }, null, {'dataType': 'html', async: !sync});
};

/**
 * Make sure that spurious console.log messages don't cause javascript errors.
 * Also, warn developers away from using console.log.  This function replaces
 * the console.log method with one that indicates the debug framework should
 * be used instead.
 */
ThemeBuilder.replaceLogging = function () {
  if (window.console) {
    if (window.console.log) {
      ThemeBuilder.log = window.console.log;
    }
  }
  else {
    window.console = {};
  }
  window.console.log = ThemeBuilder._oldConsoleLog;
  if (debug && debug.setCallback) {
    debug.setCallback(ThemeBuilder.logCallback, true);
  }
};

/**
 * This is the function with which the console.log function will be replaced.
 * This function treats any log message as a warning and prefixes a developer
 * message to steer folks in the right direction.
 */
ThemeBuilder._oldConsoleLog = function () {
  var args = [];
  for (var i = 0; i < arguments.length; i++) {
    args.push(arguments[i]);
  }
};

/**
 * This is the callback that will be called to facilitate logging.  Currently
 * the log message will only appear if the console is enabled.
 */
ThemeBuilder.logCallback = function () {
  try {
    if (ThemeBuilder.log) {
      ThemeBuilder.log(arguments);
    }
  }
  catch (e) {
  }
};

// Replace the console.log facility with debug.
ThemeBuilder.replaceLogging();

// IE still has no Array.indexOf method.  Provide one for compatibility.
if (!Array.indexOf) {
  Array.prototype.indexOf = function (obj) {
    for (var i = 0; i < this.length; i++) {
      if (this[i] === obj) {
        return i;
      }
    }
    return -1;
  };
}

/**
 * Error handlers to be used when error_callback is not specified in ajax operations
 *
 * @namespace
 * @see ThemeBuilder.handlerError()
 */
ThemeBuilder.errorHandler = ThemeBuilder.errorHandler || {};

/**
 * @see ThemeBulder.handleError
 */
ThemeBuilder.errorHandler.logSilently = function (data, type, errorThrown) {
  ThemeBuilder.logCallback(data);
};

/**
 * @see ThemeBulder.handleError
 */
ThemeBuilder.errorHandler.alertAndClose = function (data, type, errorThrown) {
  var message = '';
  if (data.message) {
    message = data.message;
  }
  else {
    message = Drupal.t("Oops! something just happened and we're not sure what to do about it. The error has been logged, and we apologize for any inconvenience.");
  }

  alert(message);
  var bar = ThemeBuilder.Bar.getInstance();
  bar.exit();
};

/**
 * A ThemeBuilder exception handler that displays a message to the user but
 * allows them to continue.  Note that the postBack call tha sends the request
 * must be passed an error handler function that actually handles the error by
 * calling ThemeBuilder.handleError(data, data.type, 'recoverableError') in
 * order for this scheme to work.  This callback gives you the opportunity to
 * correct the user interface state for the failed request.
 *
 * @param data
 *   The data object resulting from the request.
 * @param type
 *   The data.type field
 * @param {String} errorThrown
 *   Should be 'recoverableError'
 */
ThemeBuilder.errorHandler.alert = function (data, type, errorThrown) {
  if (data.message) {
    alert(data.message);
  }
  var bar = ThemeBuilder.Bar.getInstance();
  bar.hideWaitIndicator();
};

/**
 * Returns true if we are currently running in development mode.
 *
 * In development mode we might run additional code to make error
 * conditions more prominent, but this would not be appropriate for
 * production mode.
 *
 * @return {boolean}
 *   TRUE if development mode is on; FALSE otherwise.
 */
ThemeBuilder.isDevelMode = function () {
  return true === Drupal.settings.gardensDevel;
};
