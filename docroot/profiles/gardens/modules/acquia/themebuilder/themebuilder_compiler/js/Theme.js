
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

/**
 * The Theme class stores information about a particular theme.  In order to
 * guarantee the data is current, be sure to use the getTheme static function
 * to retrieve the theme instance.
 * @class
 */
ThemeBuilder.Theme = ThemeBuilder.initClass();

/**
 * Creates a new Theme instance.  The themeInfo should contain information
 * that will be used to initialize the theme.  This should include:
 * name - The human readable theme name
 * system_name - The system name for the theme
 * time - (optional) the time the theme was last saved.
 * isBase - (optional) indicates whether this is a base theme.
 *
 * @param {Object} themeInfo
 *   The data used to initialize the new Theme instance.
 */
ThemeBuilder.Theme.prototype.initialize = function (themeInfo) {
  this.update(themeInfo);
};

/**
 * Updates the theme instance with new data contained in the specified
 * themeInfo object.  Note that all of the information is the same as
 * for the initialize method, but all fields are optional.  This method
 * is generally called after saving a theme or publishing a theme to
 * update the theme data with fresh information from the server.
 *
 * @param {Object} themeInfo
 *   The data used to initialize the new Theme instance.
 */
ThemeBuilder.Theme.prototype.update = function (themeInfo) {
  var isPublished = false;
  var isSelected = false;
  if (themeInfo) {
    if (themeInfo.name) {
      this._name = themeInfo.name;
    }
    if (themeInfo.system_name) {
      var data = ThemeBuilder.getApplicationInstance().getData();
      this._systemName = themeInfo.system_name;
      isPublished = (this._systemName === data.published_theme);
      isSelected = (this._systemName === data.selectedTheme);
    }
    if (themeInfo.time) {
      this._time = themeInfo.time;
    }
    if (themeInfo.screenshot_url) {
      this._screenshotUrl = themeInfo.screenshot_url;
    }
    this._isBase = themeInfo.isBase === true;
  }
  this._isPublished = isPublished;
  this._isSelected = isSelected;
};

/**
 * Returns the human readable theme name.
 *
 * @return {String}
 *   The human readable name for this theme instance.
 */
ThemeBuilder.Theme.prototype.getName = function () {
  return this._name;
};

/**
 * Returns the theme system name.
 *
 * @return {String}
 *   The system name used to identify this theme instance.
 */
ThemeBuilder.Theme.prototype.getSystemName = function () {
  return this._systemName;
};

/**
 * Returns the published state of this theme.
 *
 * @return {boolean}
 *   Returns true if this is the published theme; false otherwise.
 */
ThemeBuilder.Theme.prototype.isPublished = function () {
  return this._isPublished;
};

/**
 * Returns the selected state of this theme.  The selected theme is the theme
 * that was cloned to create an edit session.
 *
 * @return {boolean}
 *   Returns true if this is the selected theme; false otherwise.
 */
ThemeBuilder.Theme.prototype.isSelected = function () {
  return this._isSelected;
};

/**
 * Indicates whether this is a base theme or a custom theme.  A base theme
 * cannot be modified (i.e. overwritten), but can be used as the base of a new
 * theme.  A custom theme can be modified.
 *
 * @return {boolean}
 *   Returns true if this is a base theme; false if this is a custom theme.
 */
ThemeBuilder.Theme.prototype.isBaseTheme = function () {
  return this._isBase;
};

/**
 * Deletes the theme associated with this theme instance.
 * 
 * @param {Object} callbacks
 *   Optional parameter that includes callbacks for success and fail
 *   that are called at the appropriate time.
 */
ThemeBuilder.Theme.prototype.deleteTheme = function (callbacks) {
  // TODO: There need to be some checks here - isBaseTheme, isPublished, etc.
  ThemeBuilder.postBack(Drupal.settings.themebuilderDeleteTheme,
    {theme_name: this.getSystemName()},
    ThemeBuilder.bind(this, this._themeDeleted, callbacks),
    ThemeBuilder.bind(this, this._themeDeleteFailed, callbacks));
};

/**
 * Called when the theme is actually deleted on the server side.
 * 
 * @private
 * @param {Object} event
 *   The event.
 * @param {String} result
 *   The type of error
 * @param {Object} callbacks
 *   The callbacks that were passed to the delete call.
 */
ThemeBuilder.Theme.prototype._themeDeleted = function (event, result, callbacks) {
  // Remove the theme inforation for a deleted theme from the application data.
  // updateData is called on the application, triggering a data updated event.
  var deletedThemeName = this.getSystemName(),
      themes = ThemeBuilder.getApplicationInstance().getData().themes,
      remainingThemes = [];
  for (var i = 0, len = themes.length; i < len; i++) {
    if (deletedThemeName !== themes[i].system_name) {
      remainingThemes.push(themes[i]);
    }
  }
  if (remainingThemes.length !== themes.length) {
    ThemeBuilder.getApplicationInstance().updateData({themes: remainingThemes});
    delete ThemeBuilder.Theme._themes[deletedThemeName];
  }
  
  if (callbacks && callbacks.success) {
    callbacks.success(this);
  }
};

/**
 * Called when the theme delete fails.
 *
 * @private
 * @param {Object} error
 *   The error.
 * @param {String} type
 *   The type of error
 * @param {String} level
 *   The error level (usually 'recoverableError')
 * @param {Object} callbacks
 *   The callbacks that were passed to the delete call.
 */
ThemeBuilder.Theme.prototype._themeDeleteFailed = function (event, type, level, callbacks) {
  if (callbacks && callbacks.fail) {
    callbacks.fail(this);
  }
};

/**
 * Publishes the theme associated with this theme instance.
 * 
 * @param {Object} callbacks
 *   Optional parameter that includes callbacks for success and fail
 *   that are called at the appropriate time.
 */
ThemeBuilder.Theme.prototype.publishTheme = function (callbacks) {
  ThemeBuilder.postBack(Drupal.settings.themebuilderPublishTheme,
    {theme_name: this.getSystemName()},
    ThemeBuilder.bind(this, this._themePublished, callbacks),
    ThemeBuilder.bind(this, this._themePublishFailed, callbacks));
};

/**
 * Called when the theme is actually published.
 * 
 * @private
 * @param {Object} result
 *   The result.
 * @param {String} type
 *   The type of error
 * @param {Object} callbacks
 *   The callbacks that were passed to the publish call.
 */
ThemeBuilder.Theme.prototype._themePublished = function (event, result, callbacks) {
  ThemeBuilder.getApplicationInstance().updateData({published_theme: this.getSystemName()});
  if (callbacks && callbacks.success) {
    callbacks.success(this);
  }
};

/**
 * Called when the theme publish fails.
 *
 * @private
 * @param {Object} error
 *   The error.
 * @param {String} type
 *   The type of error
 * @param {String} level
 *   The error level (usually 'recoverableError')
 * @param {Object} callbacks
 *   The callbacks that were passed to the publish call.
 */
ThemeBuilder.Theme.prototype._themePublishFailed = function (event, type, level, callbacks) {
  if (callbacks && callbacks.fail) {
    callbacks.fail(this);
  }
};

/**
 * Returns a ThemeBuilder application representation of a theme
 *
 * @param {Theme} theme
 *   A Theme instance.
 *
 * @return {Object}
 *   A ThemeBuilder application representation of a theme.
 */
ThemeBuilder.Theme.prototype.getThemeInfo = function () {
  return {
    dom_id: "themetile_" + this._systemName,
    isBase: this._isBase,
    is_base: this._isBase,
    name: this._name,
    system_name: this._systemName,
    screenshot_url: this._screenshotUrl
  };
};

/**
 * Copies the theme associated with this theme instance.
 * 
 * @param {String} newThemeName
 *   The name to use for the copy.
 * @param {Object} callbacks
 *   Optional parameter that includes callbacks for success and fail
 *   that are called at the appropriate time.
 */
ThemeBuilder.Theme.prototype.copyTheme = function (newThemeLabel, newThemeName, callbacks) {
  ThemeBuilder.postBack(Drupal.settings.themebuilderCopyTheme,
    {theme_name: this.getSystemName(),
     new_theme_label: newThemeLabel,
     new_theme_name: newThemeName},
    ThemeBuilder.bind(this, this._themeCopied, callbacks),
    ThemeBuilder.bind(this, this._themeCopyFailed, newThemeLabel, callbacks));
};

/**
 * Called when the theme is actually copied on the server side.
 * 
 * @private
 * @param {Object} result
 *   The result.
 * @param {String} type
 *   The type of error
 * @param {Object} callbacks
 *   The callbacks that were passed to the copy call.
 */
ThemeBuilder.Theme.prototype._themeCopied = function (event, result, callbacks) {
  var theme = new ThemeBuilder.Theme(event.theme_info);
  if (theme) {
    theme.addTheme();
  }
  if (callbacks && callbacks.success) {
    callbacks.success({originalTheme: this, newTheme: theme});
  }
};

/**
 * Called when the theme copy fails.
 *
 * @private
 * @param {Object} error
 *   The error.
 * @param {String} type
 *   The type of error
 * @param {String} level
 *   The error level (usually 'recoverableError')
 * @param {String} newThemeName
 *   The name of the new theme that the system failed to create.
 * @param {Object} callbacks
 *   The callbacks that were passed to the copy call.
 */
ThemeBuilder.Theme.prototype._themeCopyFailed = function (event, type, level, newThemeName, callbacks) {
  if (callbacks && callbacks.fail) {
    callbacks.fail({originalTheme: this, newName: newThemeName});
  }
};

/**
 * This static method is called during initialization to create a Theme
 * instance for each theme found on the system.  This function is registered
 * with the Application instance to perform initialization as soon as the
 * initialization data is available.
 *
 * @param {Object} data
 *   The application initialization data.
 */
ThemeBuilder.Theme.initializeThemes = function (data) {
  if (!data.themes || !(data.themes.length > 0)) {
    return;
  }
  var themeCount = data.themes.length;
  ThemeBuilder.Theme._themes = {};

  for (var i = 0; i < themeCount; i++) {
    var theme = new ThemeBuilder.Theme(data.themes[i]);
    if (theme.getSystemName() === data.published_theme) {
      theme._isPublished = true;
    }
    ThemeBuilder.Theme._themes[theme.getSystemName()] = theme;
  }
};

/**
 * Adds this theme instance to the set of themes known to the system.
 * By adding the theme here, other parts of the application will be able to
 * query and retrieve the theme.
 * 
 * Registered application listeners will be notified of theme changes as
 * a result of adding a theme using this method.
 */
ThemeBuilder.Theme.prototype.addTheme = function () {
  // Add this theme to the Theme object list.
  // This triggers the 'save' event
  var name = this.getSystemName();
  ThemeBuilder.Theme._themes[name] = this;
  
  // Push the theme info into the application if it is new i.e. added
  // via save as or theme import
  var app = ThemeBuilder.getApplicationInstance();
  var themes = app.getData().themes;
  var known = false;
  for (var i = 0, len = themes.length; i < len && !known; i++) {
    if (themes[i].system_name === name) {
      // This theme is already in the application.
      known = true;
    }
  }
  // If the application doesn't know about the theme, push the theme info
  if (!known) {
    themes.push(this.getThemeInfo());
    ThemeBuilder.getApplicationInstance().updateData({themes: themes});
  }
};

/**
 * Retrieves the theme identified by the specified systemName.
 *
 * @static
 * @param {String} systemName
 *   The system name associated with the desired theme instance.
 *
 * @return {Theme}
 *   The theme associated with the specified system name, or undefined if that
 *   theme does not exist.
 */
ThemeBuilder.Theme.getTheme = function (systemName) {
  var theme = ThemeBuilder.Theme._themes[systemName];
  if (theme) {
    var data = ThemeBuilder.getApplicationInstance().getData();
    var name = theme.getSystemName();
    theme._isPublished = (name === data.published_theme);
    theme._isSelected = (name === data.selectedTheme);
  }
  return ThemeBuilder.Theme._themes[systemName];
};

/**
 * Retrieves the currently selected theme instance.  The selected theme is the
 * theme that will be overwritten if the user saves.
 *
 * @return {Theme}
 *   The currently selected theme, or undefined if no theme is currently
 *   selected.
 */
ThemeBuilder.Theme.getSelectedTheme = function () {
  var theme;
  var data = ThemeBuilder.getApplicationInstance().getData();
  if (data) {
    theme = ThemeBuilder.Theme.getTheme(data.selectedTheme);
  }
  return theme;
};

/**
 * Retrieves the currently published theme.
 *
 * @return {Theme}
 *   The currently published theme, or undefined if no theme is currently
 *   published.
 */
ThemeBuilder.Theme.getPublishedTheme = function () {
  var theme;
  var data = ThemeBuilder.getApplicationInstance().getData();
  if (data) {
    theme = ThemeBuilder.Theme.getTheme(data.published_theme);
  }
  return theme;
};

/**
 * Causes the themes to be initialized as soon as the application
 * initialization data is available.
 */
ThemeBuilder.getApplicationInstance().addApplicationInitializer(ThemeBuilder.Theme.initializeThemes);