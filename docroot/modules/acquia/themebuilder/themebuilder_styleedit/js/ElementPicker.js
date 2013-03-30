
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true ThemeBuilder: true */

/**
 * @namespace
 */
ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * The ElementPicker class is responsible for highlighting elements
 * that are available for styling and allowing the user to select one
 * of those elements using the mouse.
 * @class
 */
ThemeBuilder.styles.ElementPicker = ThemeBuilder.initClass();

/**
 * Instantiates a new ElementPicker instance.
 */
ThemeBuilder.styles.ElementPicker.prototype.initialize = function () {
  this.clickItem = ThemeBuilder.bind(this, this._clickItem);
  this.mouseOverItem = ThemeBuilder.bind(this, this._mouseOverItem);
  this.refreshSelection = ThemeBuilder.bind(this, this._refreshSelection);
  this.hideHoverHighlight = ThemeBuilder.bind(this, this._hideHoverHighlight);
  var settings = ThemeBuilder.getApplicationInstance().getSettings();
  settings.addSettingsChangeListener(this);
};

/**
 * Registers the elements that correspond with the configured selectors so that they
 * will facilitate element selection with a pointing device.
 */
ThemeBuilder.styles.ElementPicker.prototype.registerElements = function () {
  var $ = jQuery;
  var selector;
  if (!this.path_selector) {
    // The selectorMap is a mapping that overrides the default Selector
    // behavior.  This map is associated with the currently selected theme,
    // and appears in the .info file.
    var selectorMap = ThemeBuilder.getApplicationInstance().getData().selectorMap;
    this.path_selector = new ThemeBuilder.styles.Selector(this._getPathFilter(selectorMap));
    this.path_selector.addSelectorListener(this);
    this.selectorEditor = new ThemeBuilder.styles.SelectorEditor(this.path_selector, '#path-selector');
  }
  this.getDomNavigator();
  this._addClickTargets();

  // Remove known event handlers that prevent theming.
  var preventClickEvent = ['.views-ajax-scroll-processed', 'body.themebuilder', '.flag-processed'];
  for (var i in preventClickEvent) {
    $(preventClickEvent[i]).unbind('click');
  }

  // Rather than creating a huge special cased blacklist, here we create a jQuery
  // object to use as the scope of our later search for items to add .style-clickable
  var scope = $('#page, #copyright, .region-page-bottom');

  $('*', scope)
    // Exclude generic divs and spans with no class.
    .not('div:not([class])')
    .not('span:not([class])')
    // Exclude things themebuilder has designated as unselectable.
    .not('.tb-no-select')
    // Add the #page, #copyright, and .region-page-bottom elements themselves
    // back into the selection.
    .add('#page, #copyright, .region-page-bottom')
    // Now that everything clickable is selected, bind click and hover handlers
    // to all of it.
    .bind('click', this.clickItem)
    .hover(this.mouseOverItem)
    // Mark everything that's gotten these handlers with a class, so we can
    // easily select it all again.
    .addClass('style-clickable');

  this.styling = true;

  // In the event that the element selector has already been used, once again
  // select the last used selection.
  var currentSelector = ThemeBuilder.util.getSelector();
  if (currentSelector && currentSelector !== '') {
    this.selectorSelected(currentSelector);
    this.getDomNavigator().highlightSelection(currentSelector);
  }

  //$('<div id="debug" style="position: fixed; top: 30px; background: #000; opacity: .9; z-index: 20000;"><div id="top"></div><div id="left"></div><div id="height"></div><div id="width"></div></div>').appendTo('body');
};

/**
 * Instantiates the appropriate path filter for the current theme.  This
 * determination is made based on a body class.  The path filter is
 * responsible for determining an appropriate selector for an element selected
 * by the user.
 *
 * @param selectorMap
 *   The selectorMap from the application init data which helps the
 *   SelectorEditor to fine-tune selectors.
 * @return
 *   The path filter appropriate for the current theme.
 */
ThemeBuilder.styles.ElementPicker.prototype._getPathFilter = function (selectorMap) {
  var filterNum = 1;
  var $ = jQuery;
  var bodyClasses = $('body').attr('class');
  var matches = bodyClasses.match(/theme-markup-([\d]+)/);
  if (matches && matches.length > 1) {
    filterNum = parseInt(matches[1], 10);
  }
  if (filterNum === 1) {
    return new ThemeBuilder.styles.ThemeMarkup1Filter(selectorMap);
  }
  else {
    return new ThemeBuilder.styles.ThemeMarkup2Filter(selectorMap);
  }
};

/**
 * Instantiates the appropriate DomNavigator for the current theme.  This
 * determination is made based on a body class.  The DomNavigator is
 * responsible for highlighting selected elements and allowing the user to
 * navigate throughout the DOM.
 *
 * @return
 *   A DomNavigator instance appropriate for the current theme.
 */
ThemeBuilder.styles.ElementPicker.prototype.getDomNavigator = function () {
  if (!this.domNavigator) {
    var filterNum = this._getThemeMarkupVersion();
    var settings = ThemeBuilder.getApplicationInstance().getSettings();
    var navigator = new ThemeBuilder.styles.PowerNavigator();
    // Original set of themes.  Disable the arrows
    // Version 2 of the theme markup.  Enable the arrows
    navigator.advanced = !(filterNum === 1 || !settings.powerThemeEnabled());
    this.domNavigator = navigator;
  }
  return this.domNavigator;
};

/**
 * Returns the version of the markup.  This version is used to instantiate
 * working parts of the themebuilder that are compatible with the theme.
 *
 * @return
 *   The version number, in integer form.
 */
ThemeBuilder.styles.ElementPicker.prototype._getThemeMarkupVersion = function () {
  var $ = jQuery;
  var version = 1;
  var bodyClasses = $('body').attr('class');
  var matches = bodyClasses.match(/theme-markup-([\d]+)/);
  if (matches && matches.length > 1) {
    version = parseInt(matches[1], 10);
  }
  return version;
};

/**
 * Adds click targets, if required by the theme.  Click targets are areas the
 * user can click to select an associated element that is completely occluded
 * by other elements.  This is often the case for elements that have a
 * negative z-index.
 *
 * The click targets should be created when the elements are registered with
 * the ElementPicker.
 *
 * @private
 */
ThemeBuilder.styles.ElementPicker.prototype._addClickTargets = function () {
  var $ = jQuery;
  var elements = $('.requires-click-target');
  for (var i = 0; i < elements.length; i++) {
    // The click target has an id that is derived from the id of the original
    // element.  It is a requirement that the original element has a unique
    // id.
    var div = '<div id="' + elements[i].id + '-target" class="tb-click-target"></div>';
    $('body').append($(div));
  }
};

/**
 * Removes the click targets.  This should be done when the ElementPicker
 * unregisteres elements so the click targets will disappear.
 *
 * @private
 */
ThemeBuilder.styles.ElementPicker.prototype._removeClickTargets = function () {
  var $ = jQuery;
  $('.tb-click-target').remove();
};

/**
 * Fetches the appropriate element.  This method handles the case in which the
 * specified element is actually a click target.  This method will return the
 * element associated with a click target.
 *
 * @private
 *
 * @param {DomElement} element
 *   The element to resolve.  Generally this would come from an event.
 * @return {DomElement}
 *   The element.  If the specified element is a click target, this will be
 *   the associated element rather than the click target.  Otherwise, the
 *   specified elemen is returned.
 */
ThemeBuilder.styles.ElementPicker.prototype._resolveElement = function (element) {
  var $ = jQuery;
  var result = element;
  if ($(element).hasClass('tb-click-target')) {
    // This is a click target; substitute the real target.
    var id = element.id.replace(new RegExp('-target$'), '');
    result = $('#' + id)[0];
  }
  return result;
};

/**
 * Unregisters elements.  See this.registerElements.
 */
ThemeBuilder.styles.ElementPicker.prototype.unregisterElements = function () {
  var $ = jQuery;
  $('.style-clickable')
    .removeClass('style-clickable')
    .unbind('mouseover', this.mouseOverItem)
    .unbind('click', this.clickItem);

  $('.style-clickable-ohover').removeClass('style-clickable-ohover');
  this.hideHoverHighlight();

  // Remove highlighter
  $('.selected').removeClass('selected');
  $('.tb-nav').hide();
  $('#the-hover').remove();
  $('.link-hover').remove();

  this.getDomNavigator().unhighlightSelection();
  this._removeClickTargets();
  this.styling = false;
};

/**
 * Called when the selector changed.
 *
 * @param {Selector} selector
 *   The Selector instance.
 */
ThemeBuilder.styles.ElementPicker.prototype.selectorChanged = function (selector) {
  var selectorString = selector.getCssSelector();
  this.selectorSelected(selectorString);
  this.getDomNavigator().highlightSelection(ThemeBuilder.util.removeStatePseudoClasses(selectorString));
};

/**
 * Respond to the user choosing a new selector.
 *
 * @param {string} selector
 *   The CSS selector that the user wants to style.
 */
ThemeBuilder.styles.ElementPicker.prototype.selectorSelected = function (selector) {
  var $ = jQuery;
  ThemeBuilder.util.setSelector(selector);

  ThemeBuilder.styleEditor.fontEditor.selectorChanged(selector);
  ThemeBuilder.styleEditor.boxEditor.selectorChanged(selector);
  ThemeBuilder.styleEditor.backgroundEditor.selectorChanged(selector);
};

/**
 * Called when the user enters a user-selectable element with the mouse.
 * Doing this in JavaScript is nicer than the CSS alternative because we
 * can highlight only 1 element even if multiple selectable elements are
 * nested; in css, all selectable items under the mouse will be highlighted
 * at once.
 *
 * @private
 *
 * @param {Event} the mouseover event.
 */
ThemeBuilder.styles.ElementPicker.prototype._mouseOverItem = function (event) {
  var $ = jQuery;
  if (!this.styling) {
    return;
  }
  var element = $(event.currentTarget);
  if (element.is('.tb-no-select') || this.hovershow) {
    return;
  }
  this.hideHoverHighlight();

  element.addClass('style-clickable-hover');

  if ($('#the-hover').length > 0) {
    element.append($('#the-hover'));
  }
  else {
    $('<div id="the-hover" class="the-hover tb-no-select"><div class="highlight-inner"></div></div>').appendTo('body');
  }
  // Do not stop this event or hover menus will not work on the styles
  // tab.
};

/**
 * Hides the highlight that appears when hovering over an element.
 */
ThemeBuilder.styles.ElementPicker.prototype._hideHoverHighlight = function () {
  var $ = jQuery;
  $('.style-clickable-hover').removeClass('style-clickable-hover');
};

/**
 * Called when the user exits a user-selectable element with the mouse.
 * This function simply turns off the border and turns on the border
 * for the parent selectable item, if any.
 *
 * @param {Event} event
 *   The click event.
 */
ThemeBuilder.styles.ElementPicker.prototype._clickItem = function (event) {
  var $ = jQuery,
      element,
      $element,
      link,
      settings = ThemeBuilder.getApplicationInstance().getSettings(),
      targetElement = event.currentTarget,
      $target = $(event.currentTarget);

  if (this.hovershow) {
    this.hovershow = false;
    $('#hovertext', parent.document).hide();
    return ThemeBuilder.util.stopEvent(event);
  }
  // If power theming is not enabled, we want to transfer clicks on the #page
  // and #copyright elements to the body.
  if (!settings.powerThemeEnabled()) {
    if ($target.is('#page, #copyright')) {
      $target = $('body');
      targetElement = $('body').get(0);
    }
  }
  if ($target.hasClass('tb-no-select')) {
    return;
  }
  element = this._resolveElement(targetElement);
  $element = $(element);
  // If we've somehow bound this click handler to something that shouldn't be
  // clickable, unbind it. Leave the body element alone, though, since clicks
  // can be transferred to it.
  if (!($element.hasClass('style-clickable') || $element.is('body'))) {
    $element.unbind('mouseover', this.mouseOverItem)
    .unbind('click', this.clickItem);
    return true;
  }

  this.getDomNavigator().highlightClicked($(element));

  this.hideHoverHighlight();

  $('.link-hover', parent.document).remove();

  // Obfuscate link clicks while theming
  link = ($element.is('a')) ? $element : ($element.parent('a').length) ? $element.parent('a') : $();
  this.createHoverLink(event, link);

  if (!Drupal.settings.themebuilderAdvanced) {
    this.path_selector.setElement(element);
    this.selectorSelected(this.path_selector.getCssSelector());
    this.refreshOnEdit();
    return ThemeBuilder.util.stopEvent(event);
  }
  return ThemeBuilder.util.stopEvent(event);
};

/**
 * Causes the editor's state to be reinitialzed when the user mouses into the
 * editor area.  See the _refreshSelection method for more information.
 */
ThemeBuilder.styles.ElementPicker.prototype.refreshOnEdit = function () {
  var $ = jQuery;
  $('#themebuilder-wrapper').unbind('mouseover', this.refreshSelection)
  .bind('mouseover', this.refreshSelection);

};

/**
 * Causes the style editors to refresh.  This is used to ensure the correct
 * initialization values are used for undo.  One case that warranted this
 * behavior is clicking on a link that has a different color when in the hover
 * state.  Because the user is hovering over the link when it is selected, the
 * link's hover state would be used to initialize the style editor rather than
 * the link's normal state.  Clicking on such a link, changing the color, and
 * then clicking Undo would cause the link's normal color to change such that
 * it is the same as the color in the hover state.
 *
 * This event handler is placed on #themebuilder-wrapper in the refreshOnEdit
 * method.  When the user mouses into the editor, the values in the editor are
 * refreshed.  Thus the user is not hovering over the initial selection, so we
 * initialize based on the element's non-hover state.
 *
 * @param {DomEvent} event
 *   The mouseover event.
 */
ThemeBuilder.styles.ElementPicker.prototype._refreshSelection = function (event) {
  var $ = jQuery;
  $('#themebuilder-wrapper').unbind('mouseover', this.refreshSelection);
  this.selectorEditor.pathSettingsModified();
};

/**
 * This method is called when a new element is selected.
 *
 * @param {Selector} selector
 *   The Selector instance that was assigned the new element.
 */
ThemeBuilder.styles.ElementPicker.prototype.selectorElementChanged = function (selector) {
  if (selector && selector.getCssSelector) {
    selector = selector.getCssSelector();
  }
  selector = ThemeBuilder.util.removePseudoClasses(selector);
  this.getDomNavigator().highlightSelection(selector);
};

/**
 * Called when the power theme setting has changed.  This function is
 * responsible for switching between simple navigation mode and power
 * theme mode.  Also the SelectorEditor is hidden if the user switches
 * out of power theme mode.
 *
 * @param {Settings} settings
 *   The settings object that represents the current themebuilder settings.
 */
ThemeBuilder.styles.ElementPicker.prototype.powerThemeSettingChanged = function (settings) {
  var domNavigator;
  if (this._getThemeMarkupVersion() > 1) {
    // Enable the arrows in the navigator in version 2 of the theme markup
    domNavigator = this.getDomNavigator();
    domNavigator.advanced = settings.powerThemeEnabled();
    domNavigator.updateDisplay();
  }
};

/**
 * Called when a link in the page is clicked. This prevents the default action
 * of the link and builds a small DOM component that presents the user with a URL
 * to follow with an extra click.
 *
 * @param {jQuery Obj} link
 *   The link to be obfuscated.
 */
ThemeBuilder.styles.ElementPicker.prototype.createHoverLink = function (event, link) {
  // Just return if no link was passed in.
  if (link.length === 0) {
    return;
  }
  var $ = jQuery;
  event.preventDefault();
  var hover = $('<div>', {
    html: $('<a>', {
      href: link.attr('href'),
      html: Drupal.t('Go to this link')
    }).click(this.followHoverLink)
  }).addClass('link-hover tb-no-select').appendTo($('body', parent.document));
  var off = link.offset();
  var top = 0;//parent.jQuery(parent).scrollTop();

  hover.css({
    top: parseInt(off.top + link[0].offsetHeight + top, 10) + 'px',
    left: parseInt(off.left, 10) + 'px'
  });

  var remove = function () {
    $(parent.document).unbind('click', remove);
    hover.remove();
  };

  $(window.parent.document).click(remove);
};

/**
 * The body has a click event associated with it, so this event's bubbling needs
 * to be stopped before it gets to the body.
 *
 * @param {event} event
 */
ThemeBuilder.styles.ElementPicker.prototype.followHoverLink = function (event) {
  event.stopImmediatePropagation();
};
