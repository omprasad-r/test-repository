
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

/**
 * This class reveals the themebuilder settings and allows other objects to
 * register event listeners that are triggered when the settings are changed.
 * For that reason, this class should be used in preference to raw settings
 * from the themebuider-init-data request for values that represent the
 * settings for the themebuilder (not the theme or data related to the current
 * theming project).
 * @class
 * @constructor
 */
ThemeBuilder.Settings = ThemeBuilder.initClass();

/**
 * Constructor for the Settings class.  Note that this is a singleton to make
 * it difficult to have different parts of the system using different
 * settings.
 *
 * @param {array} settings
 *   The raw settings data from the themebuilder-init-data request.
 */
ThemeBuilder.Settings.prototype.initialize = function (settings) {
  if (ThemeBuilder.Settings._instance) {
    throw ('Settings is a singleton, please use Application.getSettings().');
  }
  this._settings = settings;
  this._settingsChangeListeners = [];
};

/**
 * Indicates whether power theme mode is enabled.
 *
 * @return
 *   True if power theming is enabled; false otherwise.
 */
ThemeBuilder.Settings.prototype.powerThemeEnabled = function () {
  return (this._settings.powerTheme === true || this._settings.powerTheme === 'true');
};

/**
 * Sets power theming mode.
 *
 * @param {boolean} enabled
 *   Whether power theme mode is enabled.
 */
ThemeBuilder.Settings.prototype.setPowerThemeEnabled = function (enabled) {
  if (this._settings.powerTheme !== enabled) {
    this._settings.powerTheme = enabled;
    this.notifyListeners('powerTheme');
    this.saveSettings();
  }
};

/**

 * Indicates whether natural language mode is enabled.  If enabled, the css
 * selector will be described using human readable text.
 *
 * @return
 *   True if natural language mode is enabled; false otherwise.
 */
ThemeBuilder.Settings.prototype.naturalLanguageEnabled = function () {
  return (this._settings.naturalLanguage === true || this._settings.naturalLanguage === 'true');
};

/**
 * Sets the natural language mode.
 *
 * @param {boolean} enabled
 *   Whether natural language mode is enabled.
 */
ThemeBuilder.Settings.prototype.setNaturalLanguageEnabled = function (enabled) {
  if (this._settings.naturalLanguage !== enabled) {
    this._settings.naturalLanguage = enabled;
    this.notifyListeners('naturalLanguage');
    this.saveSettings();
  }
};

/**
 * Adds the specified object as a change listener.  The object should include
 * methods of the form [property]SettingChanged for all properties for which
 * the object is interested.
 */
ThemeBuilder.Settings.prototype.addSettingsChangeListener = function (o) {
  this._settingsChangeListeners.push(o);
};

/**
 * Notifies change listeners that the specified setting has changed.
 *
 * @param {string} setting
 *   The name of the setting that has changed.
 */
ThemeBuilder.Settings.prototype.notifyListeners = function (setting) {
  var len = this._settingsChangeListeners.length;
  for (var i = 0; i < len; i++) {
    var o = this._settingsChangeListeners[i];
    var method = setting + 'SettingChanged';
    if (o[method]) {
      try {
        o[method](this);
      }
      catch (e) {
      }
    }
  }
};

/**
 * Saves settings to the server so they persist after a page refresh.
 */
ThemeBuilder.Settings.prototype.saveSettings = function () {
  ThemeBuilder.postBack(Drupal.settings.themebuilderSaveSettings,
    {settings: this._settings},
    ThemeBuilder.bind(this, this._settingsSaved));
};

/**
 * The callback which is called after the settings have been saved to the
 * server.
 *
 * @param {array} data
 *   The data resulting from the save request.
 */
ThemeBuilder.Settings.prototype._settingsSaved = function (data) {
};
