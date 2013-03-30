// $Id$

/*jslint bitwise: true, eqeqeq: false, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true*/

/* Change above to white: true, indent: 2 after getting the other important
   stuff.  Also, change eqeqeq back to true.  Saving this for last as it
   has potential to change behavior in unexpected ways.  Saving the indentation
   to be committed after the rest of the team knows.  Otherwise there will be
   numerous svn conflicts if we work on the same file.*/


var ThemeBuilder = ThemeBuilder || {};

/**
 * @namespace
 */
ThemeBuilder.styleEditor = ThemeBuilder.styleEditor || {};

/**
 * @class
 */
Drupal.behaviors.styleEditor = {
  attach: function (context, settings) {
    jQuery('#themebuilder-style').bind('init', ThemeBuilder.styleEditor.init);
  }
};

ThemeBuilder.styleEditor.rules = [];
ThemeBuilder.styleEditor.selecting = false;
ThemeBuilder.styleEditor.hoverrule = null;
ThemeBuilder.styleEditor.hovershow = false;
ThemeBuilder.styleEditor.styling = false;
ThemeBuilder.styleEditor.modifications = {};
ThemeBuilder.styleEditor.currentTab = 'font';
ThemeBuilder.addModificationHandler(ThemeBuilder.CssModification.TYPE, ThemeBuilder.styleEditor);

/**
 * The themebuilder_bar module calls the init method after it's finished
 * loading the html for the "Fonts, colors and sizes" tab.
 */
ThemeBuilder.styleEditor.init = function () {
  var that = this;
  var $ = jQuery;

  // Create the sub-tabs under the main "Fonts, colors & sizes" tab.
  var editors = ["fontEditor", "boxEditor", "backgroundEditor"];
  $('#themebuilder-style').tabs({
    select: function (event, ui) {
      var tab_name = ui.panel.id.split('-').slice(-1)[0];
      var editor;
      for (var index in editors) {
        if (typeof(index) === 'string') {
          editor = that[editors[index]];
          if (editor) {
            if (false === editor.tabSelectionChanged(tab_name)) {
              return false;
            }
          }
        }
      }
      that.currentTab = tab_name;
      return true;
    },
    show: function (event, ui) {
      return true;
    }
  });
  this.elementPicker = new ThemeBuilder.styles.ElementPicker();
  this.fontEditor = new ThemeBuilder.styles.FontEditor(this.elementPicker);
  this.fontEditor.setupTab();
  this.boxEditor = new ThemeBuilder.styles.BoxEditor(this.elementPicker);
  this.boxEditor.setupTab();
  this.backgroundEditor = new ThemeBuilder.styles.BackgroundEditor(this.elementPicker);
  this.backgroundEditor.setupTab();

  // When the user is hovering over a selectable element, and then moves
  // to the themebuilder pane, quit highlighting it as clickable.
  $('#themebuilder-main').mouseover(this.elementPicker.hideHoverHighlight);
};

/**
 * Called when the contents for this tab have been loaded.  If the showOnLoad
 * method has been called, this will invoke the show method.
 */
ThemeBuilder.styleEditor.loaded = function () {
  this._isLoaded = true;
  if (this._showOnLoad === true) {
    this.show();
  }
};

/**
 * Returns a flag that indicates whether the contents for this tab have been
 * loaded.
 *
 * @return {boolean}
 *   true if the contents have been loaded; false otherwise.
 */
ThemeBuilder.styleEditor.isLoaded = function () {
  return this._isLoaded === true;
};

/**
 * Sets a flag that causes this tab to be displayed as soon as the contents
 * have been loaded.
 */
ThemeBuilder.styleEditor.showOnLoad = function () {
  this._showOnLoad = true;
};

/**
 * generic ThemeBuilder Tab function overrides: setTab, show, hide
 */
ThemeBuilder.styleEditor.setTab = function (i) {
  var $ = jQuery;
  if ($('#themebuilder-style').tabs('option', 'selected') === i) {
    return;
  }
  $('#themebuilder-style').tabs('select', i);
};

/**
 * Called when the associated tab is selected by the user and the tab's
 * contents are to be displayed.
 */
ThemeBuilder.styleEditor.show = function () {
  var $ = jQuery;
  var $currentSelection;
  if (!this.isLoaded()) {
    // Not ready to actually show anything yet.
    this.showOnLoad();
  }
  else {
    // Registering elements in the ElementPicker takes a fair bit of time.  Do
    // that after the tab has been switched to minimize the visual delay
    // during tab changes.
    setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this.elementPicker, this.elementPicker.registerElements), 150);

    $currentSelection = $(ThemeBuilder.util.getSelector());
    if ($currentSelection.size() > 0) {
      // The current selector refers to at least one element on the
      // current page.  Select that element by default.
      // TODO: We need to use the current selector if possible.
      setTimeout(function () {
        $($currentSelection.get(0)).triggerHandler('click');
      }, 200);
    }
  }
  return true;
};

ThemeBuilder.styleEditor.hide = function () {
  // Unregistering elements in the ElementPicker takes a fair bit of time.  Do
  // that after the tab has been switched to minimize the visual delay during
  // tab changes.
  if (this.elementPicker) {
    setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this.elementPicker, this.elementPicker.unregisterElements), 150);
  }
};

/**
 * This function is responsible for updating the themebuilder display
 * when a modification is applied.
 **/
ThemeBuilder.styleEditor.processModification = function (modification, state) {
  if (this.fontEditor) {
    this.fontEditor.refreshDisplay();
  }
  if (this.boxEditor) {
    this.boxEditor.refreshDisplay();
  }
  if (this.backgroundEditor) {
    this.backgroundEditor.refreshDisplay();
  }
};

/**
 * Create a hidden dummy node used to ascertain current CSS properties.
 *    used by ThemeBuilder.styles.FontEditor.prototype.selectorChanged in @FontEditor.js
 *    to supplement already known properties
 *
 * @param <string> selector
 */
ThemeBuilder.styleEditor.makeDummyNode = function (selector) {
  var $ = jQuery;
  var items = selector.split(' ');
  var node = 'body';
  var first = null;
  for (var it = 0; it < items.length; it++) {
    var parts = items[it].split('.');
    if (parts[0] === '' || parts[0][0] === '#') {
      node = $('<div></div>').appendTo(node);
    }
    else {
      node = $('<' + parts[0] + '></' + parts[0] + '>').appendTo(node);
    }
    if (!first) {
      first = node;
    }
    node.css('display', 'none');
    if (parts[0][0] === '#') {
      node.attr('id', parts[0].slice(1));
    }
    for (var i = 1; i < parts.length; i++) {
      node.addClass(parts[i]);
    }
  }
  var oldremove = first.remove;
  /**
   * @ignore
   */
  node.remove = function () {
    return oldremove.apply(first);
  };
  return node;
};

/**
 * Applies the specified modification description to the client side only.
 * This allows the user to preview the modification without committing it
 * to the theme.  Useful when sliding, selecting colors, etc.
 *
 * @param desc object
 *   The modification description.  To get this value, you should pass in
 *   the result of Modification.getNewState() or Modification.getPriorState().
 */
ThemeBuilder.styleEditor.preview = function (desc) {
  var $ = jQuery;
  var hexValue;
  if (!desc || !desc.selector || !desc.property) {
    return false;
  }
  desc.value = desc.value === undefined ? "" : desc.value;

  /**
   * JS: I don't like this too much... seems hardcoded and hacky.
   * I wonder if we shouldn't just pass the actual value?
   */
  if (this.backgroundEditor && (typeof desc.value).toLowerCase() == 'string' && desc.value.indexOf('url(') === 0) {
    desc.value = 'url(' + this.backgroundEditor.fixImage(desc.value) + ')';
  }
  var stylesheet;

  // Handle the case in which a color rule is being deleted.
  if (desc.value === '') {
    stylesheet = ThemeBuilder.styles.Stylesheet.getInstance('palette.css');
    stylesheet.removeRule(desc.selector, desc.property);
    stylesheet = ThemeBuilder.styles.Stylesheet.getInstance('custom.css');
    stylesheet.removeRule(desc.selector, desc.property);
    return;
  }
  
  // Handle palette indexes.
  if ((typeof desc.value).toLowerCase() == 'string' && desc.value.indexOf("{") !== -1) {
    // Convert the palette index to a hex code.
    var colorManager = ThemeBuilder.getColorManager();
    hexValue = colorManager.paletteIndexToHex(desc.value);
    // Only continue with the preview if we had a valid palette index.
    if (hexValue) {
      hexValue = colorManager.addHash(hexValue);
    }
    else {
      return false;
    }
    // Add the new rule to palette.css.
    stylesheet = ThemeBuilder.styles.Stylesheet.getInstance('palette.css');
    stylesheet.setRule(desc.selector, desc.property, hexValue);
    // Remove it from custom.css.
    stylesheet = ThemeBuilder.styles.Stylesheet.getInstance('custom.css');
    stylesheet.removeRule(desc.selector, desc.property);
  }
  // If the value wasn't a palette index, pass it into custom.css.
  else {
    stylesheet = ThemeBuilder.styles.Stylesheet.getInstance('custom.css');
    stylesheet.setRule(desc.selector, desc.property, desc.value);
  }

  // All border-related properties need to appear in the same stylesheet, for
  // IE's benefit. See AN-12796.
  if ($.browser.msie && desc.property.indexOf('border') > -1) {
    stylesheet = ThemeBuilder.styles.Stylesheet.getInstance('border.css');
    stylesheet.setRule(desc.selector, desc.property, hexValue || desc.value);
  }

  // If we are changing a background property, disable the highlighter.
  if (desc.property.indexOf('background') !== -1) {
    var highlighter = ThemeBuilder.styles.Stylesheet.getInstance('highlighter.css');
    highlighter.disable();
  }
  if (this.elementPicker) {
    this.elementPicker.domNavigator.updateDisplay();
  }
};
