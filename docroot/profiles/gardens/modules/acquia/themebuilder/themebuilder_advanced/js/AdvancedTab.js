/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true window: true*/

var ThemeBuilder = ThemeBuilder || {};

/**
 * Singleton class that manages the advanced CSS/JS tab.
 * @class
 */
ThemeBuilder.AdvancedTab = ThemeBuilder.initClass();

/**
 * Static method to retrieve the singleton instance of the AdvancedTab.
 *
 * @return
 *   The ThemeBuilder.AdvancedTab instance.
 */
ThemeBuilder.AdvancedTab.getInstance = function () {
  if (!ThemeBuilder.AdvancedTab._instance) {
    ThemeBuilder.AdvancedTab._instance = new ThemeBuilder.AdvancedTab();
  }
  return ThemeBuilder.AdvancedTab._instance;
};

/**
 * Constructor for the ThemeBuilder.AdvancedTab class.
 */
ThemeBuilder.AdvancedTab.prototype.initialize = function () {
  if (ThemeBuilder.AdvancedTab._instance) {
    throw "ThemeBuilder.AdvancedTab is a singleton that has already been instantiated.";
  }
  this.panes = {};
  this.subtabs = [];
  this.subtabs.push({id: 'themebuilder-advanced-history',
    obj: ThemeBuilder.History.getInstance()});
  this.subtabs.push({id: 'themebuilder-advanced-css',
    obj: ThemeBuilder.CodeEditor.getInstance()});
  if ('MetatagConfig' in ThemeBuilder) {
    this.subtabs.push({id: 'themebuilder-advanced-metatag',
      obj: new ThemeBuilder.MetatagConfig()});
  }
  this.currentSubtab = 0;
};

/**
 * Initializes the UI of the advanced tab.  Called automagically from
 * ThemeBuilder.Bar.prototype.tabResourcesLoaded.
 */
ThemeBuilder.AdvancedTab.prototype.init = function () {
  var $ = jQuery;
  var tabs = $('#themebuilder-advanced');
  tabs.tabs({
    show: ThemeBuilder.bind(this, this.showSubtab),
    select: ThemeBuilder.bind(this, this.selectSubtab)
  });

  // Initialize the subtabs
  for (var i = 0, len = this.subtabs.length; i < len; i++) {
    this.subtabs[i].obj.init();
  }
};

/**
 * Invoked when the Advanced tab is selected.
 */
ThemeBuilder.AdvancedTab.prototype.show = function () {
  this.subtabs[this.currentSubtab].obj.show();
};

/**
 * Invoked when the user traverses to a different tab.
 */
ThemeBuilder.AdvancedTab.prototype.hide = function () {
  return this.subtabs[this.currentSubtab].obj.hide();
};

/**
 * Invoked when the contents of the advanced tab are loaded.
 */
ThemeBuilder.AdvancedTab.prototype.loaded = function () {
  for (var i = 0; i < this.subtabs.length; i++) {
    if (this.subtabs[i].obj.loaded) {
      this.subtabs[i].obj.loaded();
    }
  }
};

/**
 * This is the callback that is invoked when a subtab is shown.  This
 * method figures out which tab was selected and informs the old tab
 * and the new tab of the change.  This allows cleanup to occur.
 *
 * @param {Event} event
 *   The event
 * @param {Tab} tab
 *   The jquery-ui tab object, which reveals which tab is being shown.
 */
ThemeBuilder.AdvancedTab.prototype.showSubtab = function (event, tab) {
  var newTab = this._getTab(tab);
  if (newTab !== -1) {
    this._switchTabs(this.currentSubtab, newTab);
  }
};

/**
 * This callback is invoked when a tab is selected.  Selection occurs
 * before the tab is shown, and is an opportunity for a tab controller
 * to prompt the user to commit changes if needed.
 *
 * @param {Event} event
 *   The event
 * @param {Tab} tab
 *   The jquery-ui tab object, which reveals which tab is being selected.
 * @return {boolean}
 *   true if it is ok to traverse away from the current tab; false otherwise.
 */
ThemeBuilder.AdvancedTab.prototype.selectSubtab = function (event, tab) {
  var obj = this.subtabs[this.currentSubtab].obj;
  if (obj.select) {
    return obj.select(event, tab);
  }
  return true;
};

/**
 * Returns the tab index associated with the specified tab.
 *
 * @private
 * @param {Tab} tab
 *   The jquery-ui tab object.
 * @return {int}
 *   The tab index associated with the specified tab.
 */
ThemeBuilder.AdvancedTab.prototype._getTab = function (tab) {
  var newTab = -1;
  if (tab && tab.panel && tab.panel.id) {
    for (var i = 0, len = this.subtabs.length; i < len; i++) {
      if (this.subtabs[i].id === tab.panel.id) {
	// This is the new tab.
        newTab = i;
        break;
      }
    }
  }
  return newTab;
};

/**
 * Switches tabs from the old file to the new file.
 *
 * @private
 * @param {int} oldTab
 *   The index of the tab that is being hidden.
 * @param {int} newTab
 *   The index of the tab that is being shown.
 */
ThemeBuilder.AdvancedTab.prototype._switchTabs = function (oldTab, newTab) {
  if (oldTab !== newTab) {
    this.subtabs[oldTab].obj.hide();
  }
  if (newTab >= 0 && newTab < this.subtabs.length) {
    this.currentSubtab = newTab;
    this.subtabs[newTab].obj.show();
  }
};
