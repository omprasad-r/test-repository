
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * The SelectorEditor class is responsible for rendering the user interface
 * devoted to allowing a user to interact with the settings of a Selector
 * instance.
 * @class
 */
ThemeBuilder.styles.SelectorEditor = ThemeBuilder.initClass();

/**
 * The constructor of the SelectorEditor class.
 *
 * @param {Selector} selector
 *   The Selector instance responsible for generating an appropriate
 *   css selector.
 * @param {string} elementSelector
 *   A string containing a css selector that identifies where this instance
 *   of SelectorEditor should render its user interface.  It is probably
 *   best if this is an element id, though that is not required.
 */
ThemeBuilder.styles.SelectorEditor.prototype.initialize = function (selector, elementSelector) {
  this.elementSelector = elementSelector;
  this.parentSelector = elementSelector + ' .path-selector-path';
  this.specificitySelector = elementSelector + ' .path-specificity';
  this.specificityMiddleSection = this.specificitySelector + ' .middle-section';
  this.specificityCenterSelector = this.specificitySelector + ' .panel-center';
  this.specificitySelectorLeft = this.specificitySelector + ' .left';
  this.specificitySelectorRight = this.specificitySelector + ' .right';
  this.veilSelector = elementSelector + ' .path-specificity-cancel';
  this.labelSelector = elementSelector + ' .path-selector-value';
  this.labelSelectorText = elementSelector + ' .path-selector-label-text';
  this.refinementSelector = elementSelector + ' .path-selector-refinement';
  this.disableControlsSelector = elementSelector + ' .disable-controls-veil';
  this.powerThemingToggle = elementSelector + ' .power-theming-label';
  this.powerThemingValue = elementSelector + ' .power-theming-value';
  this.naturalLanguageToggle = elementSelector + ' .natural-language-label';
  this.naturalLanguageValue = elementSelector + ' .natural-language-value';
  this.selector = selector;
  this.nodes = [];
  this.widgets = [];
  this.create();
  this._hasSelection = false;
  this.showSelectorWarning = ThemeBuilder.bind(this, this._showSelectorWarning);
  this.togglePowerTheming = ThemeBuilder.bind(this, this._togglePowerTheming);
  this.toggleNaturalLanguage = ThemeBuilder.bind(this, this._toggleNaturalLanguage);
  
  this._createUI();
  this._initializeUI();
};

/**
 * Creates the user inteface for this SelectorEditor instance.  The UI
 * elements will be added to the appropriate place in the DOM as a result of
 * calling this method.
 *
 * @private
 */
ThemeBuilder.styles.SelectorEditor.prototype._createUI = function () {
  var $ = jQuery;
  var markup = [];
  markup.push('<div class="path-selector-path"></div>');
  markup.push('<div class="path-selector-label"><span class="path-selector-label-text"></span><span class="path-selector-value"></span></div>');
  markup.push('<div class="path-selection-options">');
  markup.push('<div class="power-theming-label">' + Drupal.t('Power theming: ') + '<span class="power-theming-value"></span></div>');
  markup.push('<div class="natural-language-label">' + Drupal.t('Show CSS: ') + '<span class="natural-language-value"></span></div>');
  markup.push('</div>'); // .path-selection-options
  $(this.elementSelector).append(markup.join(''));
  
  $(this.elementSelector).append(this._createSpecificityPanel());
  markup = [];
  markup.push('<div class="path-specificity-cancel"></div>');
  markup.push('<div class="disable-controls-veil"></div');
  $(this.elementSelector).append(markup.join(''));
  
  // Set up a horizontal carousel.
  this.pathSelector = this._createWidget('.path-selector-path', 'PathSelector', 'HorizontalCarousel');
  this.pathSelector.hide(); // The widget is hidden until the user selected an element
};

/**
 * Creates the markup of the specificity panel.  This panel is a box that
 * appears to drop down as a result of clicking the arrow icon in any
 * editor node instance that has one.
 *
 * @return
 *   A jQuery object representing the specificity panel.
 */
ThemeBuilder.styles.SelectorEditor.prototype._createSpecificityPanel = function () {
  var $ = jQuery;
  var topSection = $('<div class="top-section">')
  .append('<div class="top-left">')
  .append('<div class="top">')
  .append('<div class="top-right">');

  var middleSection = $('<div class="middle-section">')
  .append('<div class="left">')
  .append('<div class="panel-center clearfix">')
  .append('<div class="right">');

  var bottomSection = $('<div class="bottom-section">')
  .append('<div class="bottom-left">')
  .append('<div class="bottom">')
  .append('<div class="bottom-right">');

  return $('<div class="path-specificity">')
  .append(topSection)
  .append(middleSection)
  .append(bottomSection);
};

/**
 * Initializes the user interface for this SelectorEditor instance.  The
 * appropriate listeners and click handlers will be attached as a result of
 * calling this method.
 *
 * @private
 */
ThemeBuilder.styles.SelectorEditor.prototype._initializeUI = function () {
  var $ = jQuery;
  this.selector.addSelectorListener(this);
  // Listen for tab switches.
  ThemeBuilder.Bar.getInstance().addBarListener(this);
  this._changeSelectorText(this.selector);
  $(this.disableControlsSelector).click(this.showSelectorWarning);
  this.refreshPowerThemingToggle();
  $(this.powerThemingToggle).click(this.togglePowerTheming);
  this.refreshNaturalLanguageToggle();
  $(this.naturalLanguageToggle).click(this.toggleNaturalLanguage);
  var settings = ThemeBuilder.getApplicationInstance().getSettings();
  settings.addSettingsChangeListener(this);
};

/**
 * This method is called when a new element is selected.  This refreshes
 * the set of editor nodes used to display and configure the css selector.
 *
 * @param {Selector} selector
 *   The Selector instance that was assigned the new element.
 */
ThemeBuilder.styles.SelectorEditor.prototype.selectorElementChanged = function (selector) {
  var settings = ThemeBuilder.getApplicationInstance().getSettings();
  var enabled = settings.powerThemeEnabled();
  this.destroy();
  this.create();
  this._changeSelectorText(selector);
  if (this.widgets.PathSelector) {
    this.widgets.PathSelector.updateUI();
  }
  if (enabled) {
    this.pathSelector.show();
  }
};

/**
 * Called when the selector changed.  This happens when the settings on any
 * part of the selector has changed, but not when a new element was set.
 *
 * @param {Selector} selector
 *   The Selector instance.
 */
ThemeBuilder.styles.SelectorEditor.prototype.selectorChanged = function (selector) {
  this.refresh();
  this._changeSelectorText(selector);
  if (this.widgets.PathSelector) {
    this.widgets.PathSelector.updateUI();
  }
};

/**
 * Reset returns the Style tab back to its original load state.  It is called
 * when the user switches tabs in the theme Builder.
 */
ThemeBuilder.styles.SelectorEditor.prototype.reset = function () {
  if (this._hasSelection) {
    var $ = jQuery;
    this.destroy();
    $('#themebuilder-wrapper').removeClass('tall');
    this.clearSelectorText();
    this._hasSelection = false;
  }
};

/**
 * Causes all of the node editors to be destroyed.  This is important
 * to do when the selector changes.
 */
ThemeBuilder.styles.SelectorEditor.prototype.destroy = function () {
  var len = this.nodes.length;
  while (len--) {
    this.nodes[len].destroy();
  }
};

/**
 * Changes the text that offers a human readable version of the currently
 * configured css selector.
 *
 * @param {Selector} selector
 *   The Selector instance; optional. Defaults to a blank Selector object.
 */
ThemeBuilder.styles.SelectorEditor.prototype._changeSelectorText = function (selector) {
  if (!selector) {
    selector = new ThemeBuilder.styles.Selector();
  }
  var $ = jQuery;
  var settings = ThemeBuilder.getApplicationInstance().getSettings();
  var naturalLanguageEnabled = settings.naturalLanguageEnabled();

  if (naturalLanguageEnabled === true) {
    var selectorText = selector.getHumanReadableSelector();
  }
  else {
    selectorText = selector.getCssSelector();
    // For raw selector text, insert some additional whitespace to make the
    // separations within the selector more clear.
    selectorText = selectorText.replace(/ /g, '<span class="path-selector-value-space"> </span>');
  }
  var labelText = Drupal.t('You are styling: ');
  if (!selectorText || selectorText.length === 0) {
    selectorText = Drupal.t('Please select an element to style.');
    labelText = '';
    this.disableStyles();
    $(this.refinementSelector).hide();
  }
  else {
    this.enableStyles();
    $(this.refinementSelector).show();
    $('#themebuilder-wrapper').addClass('tall');
  }
  $(this.labelSelector).html(selectorText);
  $(this.labelSelectorText).text(labelText);
  this._refreshRefinementText();
};

ThemeBuilder.styles.SelectorEditor.prototype.clearSelectorText = function () {
  this._changeSelectorText();
};

/**
 * Sets the default visibility for the element selector.  This method uses
 * the Application init data to determine whether the selector widget should
 * be displayed or not.
 *
 * @param {Object} data
 *   The application initialization data.
 */
ThemeBuilder.styles.SelectorEditor.prototype._setDefaultElementSelectorVisibility = function (data) {
  this.showElementSelector = (data.show_element_selector === true);
  if (this.showElementSelector !== true) {
    var $ = jQuery;
    this.pathSelector.hide();
  }
};

/**
 * Toggles power theming mode.  This causes the user interface to refresh so
 * the elements pertinent to the newly selected mode are available.
 */
ThemeBuilder.styles.SelectorEditor.prototype._togglePowerTheming = function () {
  var settings = ThemeBuilder.getApplicationInstance().getSettings();
  var enabled = settings.powerThemeEnabled();
  settings.setPowerThemeEnabled(!enabled);
};

/**
 * Refreshes the elements that comprise the power theming toggle.  This
 * includes the text that indicates the current state of the toggle as well as
 * the tooltip that describes what happens if the toggle state is changed.
 */
ThemeBuilder.styles.SelectorEditor.prototype.refreshPowerThemingToggle = function () {
  var $ = jQuery;
  var settings = ThemeBuilder.getApplicationInstance().getSettings();
  var enabled = settings.powerThemeEnabled();
  var text = Drupal.t('off');
  if (enabled === true) {
    text = Drupal.t('on');
  }
  $(this.powerThemingValue).text(text);
  $(this.powerThemingToggle).attr('title', enabled ? Drupal.t('Get me out of here!') : Drupal.t('Expose all theme elements and broaden or narrow how your styling is applied.'));
};

/**
 * Called when the power theme setting has been changed.  This method is
 * responsible for showing and hiding the refinement as appropriate for the
 * current setting.
 *
 * @param {Settings} settings
 *   The themebuilder settings.
 */
ThemeBuilder.styles.SelectorEditor.prototype.powerThemeSettingChanged = function (settings) {
  this.refreshPowerThemingToggle();
  var isPowerThemeEnabled = settings.powerThemeEnabled();
  this._showRefinement(isPowerThemeEnabled);
  if (this.widgets.PathSelector && isPowerThemeEnabled) {
    this.widgets.PathSelector.updateUI();
  }
};

/**
 * Toggles natural language mode.  This causes the user interface to refresh
 * so the elements pertinent to the newly selected mode are available.
 */
ThemeBuilder.styles.SelectorEditor.prototype._toggleNaturalLanguage = function () {
  var settings = ThemeBuilder.getApplicationInstance().getSettings();
  var enabled = settings.naturalLanguageEnabled();
  settings.setNaturalLanguageEnabled(!enabled);
};

/**
 * Refreshes the elements that comprise the natural language toggle.  This
 * includes the text that indicates the current state of the toggle as well as
 * the tooltip that describes what happens if the toggle state is changed.
 */
ThemeBuilder.styles.SelectorEditor.prototype.refreshNaturalLanguageToggle = function () {
  var $ = jQuery;
  var settings = ThemeBuilder.getApplicationInstance().getSettings();
  var enabled = settings.naturalLanguageEnabled();
  var text = Drupal.t('on');
  if (enabled === true) {
    text = Drupal.t('off');
  }
  $(this.naturalLanguageValue).text(text);
  $(this.naturalLanguageToggle).attr('title', enabled ? Drupal.t('Show css selectors.') : Drupal.t('Show css selectors in natural language.'));
};

/**
 * Called when the natural language setting has been changed.  This method is
 * responsible for refreshing the display of all such text within the
 * SelectorEditor as appropriate for the current setting.
 *
 * @param {Settings} settings
 *   The themebuilder settings.
 */
ThemeBuilder.styles.SelectorEditor.prototype.naturalLanguageSettingChanged = function (settings) {
  this.refreshNaturalLanguageToggle();
  this.selectorElementChanged(this.selector);
};

/**
 * Toggles the element selector visibility.
 */
ThemeBuilder.styles.SelectorEditor.prototype._toggleRefinement = function () {
  this.showElementSelector = !this.showElementSelector;
  ThemeBuilder.postBack(Drupal.settings.themebuilderSelectorVisibility, {visibility: this.showElementSelector});
  this._showRefinement(this.showElementSelector);
};

/**
 * Shows and hides the refinement depending on the state of the specified parameter.
 *
 * @param {boolean} show
 *   If true, the refinement controls will be displayed; otherwise the
 *   controls will be hidden.
 */
ThemeBuilder.styles.SelectorEditor.prototype._showRefinement = function (show) {
  if (show === undefined) {
    var settings = ThemeBuilder.getApplicationInstance().getSettings();
    show = settings.powerThemeEnabled();
  }
  var $ = jQuery;
  if (show === true) {
    this.pathSelector.show();
    $('#themebuilder-wrapper').addClass('tall');
  }
  else {
    this.pathSelector.hide();
    $('#themebuilder-wrapper').removeClass('tall');
  }
  this._refreshRefinementText();
};

/**
 * Called when the refinement panel is completely displayed (called after the
 * animation.
 */
ThemeBuilder.styles.SelectorEditor.prototype._showRefinementAnimationComplete = function () {
  // On some browsers, including Chrome and Safari the layout is incorrect
  // after this animation, particularly when the selector is long enough to
  // wrap.  Fix the layout by forcing the selector to change which causes the
  // browser to refresh the layout of the page.
  this.selectorChanged(this.selector);
};

/**
 * Refreshes the display of the refinement text.  The text changes based
 * on the visibility of the element selector.
 */
ThemeBuilder.styles.SelectorEditor.prototype._refreshRefinementText = function () {
  var $ = jQuery;
  var refinementString = this.showRefinementString;
  if (this.showElementSelector === true) {
    refinementString = this.hideRefinementString;
  }
  $(this.refinementSelector).text(refinementString);
};

/**
 * Called when the user causes any of the path settings to be modified.  This
 * function causes the rendering of the selector editor to be refreshed.
 */
ThemeBuilder.styles.SelectorEditor.prototype.pathSettingsModified = function () {
  if (!this.selector.path) {
    return;
  }
  var len = this.selector.path.length;
  // Update the UI elements
  this.selector.pathElementSettingsChanged();
  if (len !== this.selector.path.length) {
    // The modification caused the path to change.  Recreate the selector.
    this.destroy();
    this.create();
    if (this.editor) {
      this._changeSelectorText(this.editor.selector);
    }
  }
};

/**
 * Causes the set of SelectorNode instances associated with this
 * editor to be created.  As a result of calling this method the
 * relevant nodes are created and attached to the user interface.
 */
ThemeBuilder.styles.SelectorEditor.prototype.create = function () {
  var $ = jQuery;
  var parentElement = $(this.parentSelector)[0];
  this._hasSelection = true;

  var path = this.selector.path;
  if (path && path.length > 0) {
    for (var i = 0; i < path.length; i++) {
      this.nodes[i] = new ThemeBuilder.styles.SelectorNode(this, i, path[i], (i === path.length - 1));
      this.nodes[i].create(parentElement);
    }
  }
};

/**
 * Causes each node editor to refresh itself.  This is done when the element
 * changes and when the user changes the specificity or changes the set
 * of nodes that are enabled.
 */
ThemeBuilder.styles.SelectorEditor.prototype.refresh = function () {
  var len = this.nodes.length;
  while (len--) {
    this.nodes[len].refresh();
  }
};

/**
 * This should only be called when no element has been selected.
 * It causes a status message to appear, further prompting the user on
 * how to use the style editor.
 *
 * @param {Event} event
 *   The event associated with the user action that caused this method to
 *   be called.  The event parameter is not used.
 */
ThemeBuilder.styles.SelectorEditor.prototype._showSelectorWarning = function (event) {
  ThemeBuilder.Bar.getInstance().setStatus(Drupal.t('Select an element first by clicking above.'), 'info');
};

/**
 * Causes the style controls in the style editor to be disabled.  This is
 * used to block the user's ability to interact with style settings in the
 * themebuilder while there is no element selected.
 */
ThemeBuilder.styles.SelectorEditor.prototype.disableStyles = function () {
  var $ = jQuery;
  $(this.disableControlsSelector)
  .addClass('show');
  this.pathSelector.hide();
};

/**
 * Causes the style controls in the style editor to be enabled.  This
 * removes the veil used to block the user's access to the
 * themebuilder style settings.  After an element is selected, this
 * method should be called to allow the user to theme the selected
 * element.
 */
ThemeBuilder.styles.SelectorEditor.prototype.enableStyles = function () {
  var $ = jQuery;
  $(this.disableControlsSelector)
  .removeClass('show');
  this._showRefinement();
};


/**
 * React to the user changing tabs within Themebuilder.
 *
 * @param {Object} tab
 *   The object that manages the tab (such as ThemeBuilder.styleEditor).
 */
ThemeBuilder.styles.SelectorEditor.prototype.handleTabSwitch = function (tab) {
  if (tab) {
    if (!tab.currentTab || tab.currentTab !== 'font') {
      this.reset();
    }
  }
};

/**
 * Creates a standard widget
 *
 * @param {String} selector
 *   The CSS selector for the element that will be rendered as a widget.
 *   This takes a singular element at the moment, but could be extended to support
 *   multiple objects that match the selector
 * @param {String} [optional] identifier
 *   A string to identify the widget in the widgets array
 * @param {String} [optional] type
 *   The type of widget that should be rendered
 * @return
 *   A jQuery object referencing the widget in the DOM.
 */
ThemeBuilder.styles.SelectorEditor.prototype._createWidget = function (selector, identifier, type) {
  var $ = jQuery;
  var element = $(selector);
  
  if (type) {
    switch (type) {
    case 'HorizontalCarousel':
      var widget = new ThemeBuilder.ui.HorizontalCarousel(element);
      // If an identifier for this widget was provided, store it under that identifier
      // Otherwise just shove it into the widgets array
      if (identifier) {
        this.widgets[identifier] = widget;
      } else {
        this.widgets.push(widget); 
      }
      return widget.getPointer();
    case 'HorizontalSlider':
      // Placeholder for future widgets
      break;
    default:
      break;
    }
  }
  
  // If an identifier for this widget was provided, store it under that identifier
  // Otherwise just shove it into the widgets array
  if (identifier) {
    this.widgets[identifier] = widget;
  } else {
    this.widgets.push(widget); 
  }
  
  if (!element.updateUI) { // A temporary fix for the fact that non UI components do not have an updateUI() method
    /**
     * @ignore
     */
    element.updateUI = function () {
      return false; 
    };
  }
  
  return element;
};
