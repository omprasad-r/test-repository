
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true ThemeBuilder: true debug: true*/

var ThemeBuilder = ThemeBuilder || {};

/**
 * This class is responsible for displaying the available layouts and allowing
 * the user to choose among them.
 * @class
 */
ThemeBuilder.LayoutEditor = ThemeBuilder.initClass();

/**
 * The LayoutEditor is a singleton, and this static method is used to retrieve
 * the instance.
 */
ThemeBuilder.LayoutEditor.getInstance = function () {
  if (!ThemeBuilder.LayoutEditor._instance) {
    ThemeBuilder.LayoutEditor._instance = new ThemeBuilder.LayoutEditor();
  }
  return ThemeBuilder.LayoutEditor._instance;
};

/**
 * This is the constructor for the LayoutEditor class.
 */
ThemeBuilder.LayoutEditor.prototype.initialize = function () {
  var $ = jQuery;
  Drupal.settings.layoutWidth = 'all';
  Drupal.settings.layoutCols = 'all';
  Drupal.settings.originalLayoutIndex = Drupal.settings.layoutIndex || Drupal.settings.layoutGlobal;
  this.currentPage = window.location.pathname.substring(Drupal.settings.basePath.length);
  if (this.currentPage === "") {
    // This is the Front page.
    this.currentPage = '<front>';
  }
  this.localmod = new ThemeBuilder.layoutEditorModification(this.currentPage);
  this.localmod.setPriorState(this.layoutNameToClass(Drupal.settings.layoutIndex));

  this.globalmod = new ThemeBuilder.layoutEditorModification('<global>');
  this.globalmod.setPriorState(this.layoutNameToClass(Drupal.settings.layoutGlobal));
  ThemeBuilder.addModificationHandler(ThemeBuilder.layoutEditorModification.TYPE, this);
};

/**
 * Called when the contents for this tab have been loaded.  If the showOnLoad
 * method has been called, this will invoke the show method.
 */
ThemeBuilder.LayoutEditor.prototype.loaded = function () {
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
ThemeBuilder.LayoutEditor.prototype.isLoaded = function () {
  return this._isLoaded === true;
};

/**
 * Sets a flag that causes this tab to be displayed as soon as the contents
 * have been loaded.
 */
ThemeBuilder.LayoutEditor.prototype.showOnLoad = function () {
  this._showOnLoad = true;
};

/**
 * Puts the layouts into a carousel.  This can only happen after the contents
 * have been loaded.
 */
ThemeBuilder.LayoutEditor.prototype.initializeLayouts = function () {
  var $ = jQuery;
  $('#layouts_carousel').jcarousel({scroll: 2,
        initCallback: ThemeBuilder.bind(this, this.setCarousel)});
  $('#layouts_carousel .layout-shot .options div').click(ThemeBuilder.util.stopEvent);
	// The Cancel button needs to stop the event propogation as well, otherwise the .layout-shot will get a click event in IE and reapply the cancelled layout
  $('#layouts_carousel .layout-shot .cancel').click(ThemeBuilder.util.stopEvent);
};

/**
 * Sets the carousel for this LayoutEditor instance.
 *
 * @param carousel
 *   The new carousel instance.
 */
ThemeBuilder.LayoutEditor.prototype.setCarousel = function (carousel) {
  this.carousel = carousel;
};

/**
 * This function is part of the modfunc scheme introduced for the styles tab.
 * It is not used here and will soon be removed.
 */
ThemeBuilder.LayoutEditor.prototype.processModification = function (modification, state) {
};

/**
 * Called when the associated tab is selected by the user and the tab's
 * contents are to be displayed.
 */
ThemeBuilder.LayoutEditor.prototype.show = function () {
  if (!this.isLoaded()) {
    // Not ready to actually show anything yet.
    this.showOnLoad();
  }
  else if (!this.carousel) {
    this.initializeLayouts();
  }
};

/**
 * Called when a different tab is selected by the user and this tab's contents
 * are to be hidden.
 */
ThemeBuilder.LayoutEditor.prototype.hide = function () {
  this.cancelPreview();
  return true;
};

/**
 * Provides the class name associated with the specified layout.  The class
 * can be attached to the body tag in the document to cause the specified
 * layout to be realized.
 *
 * @param {String} layoutName
 *   The name of the layout.
 *
 * @return {String}
 *   The class name corresponding to the specified layout.
 */
ThemeBuilder.LayoutEditor.prototype.layoutNameToClass = function (layoutName) {
  var result = '';
  if (layoutName) {
    result = 'body-layout-' + layoutName;
  }
  return result;
};

/**
 * Provides a basic layout name given the body class that represents the layout.
 *
 * @param {String} classname
 *   The body class that represents the layout
 * @return
 *   A string containing the layout name.
 */
ThemeBuilder.LayoutEditor.prototype.classToLayoutName = function (classname) {
  var name = classname.split('body-layout-')[1]
    .split(' ')[0];
  return name;
};

/**
 * Returns the name of the layout based on the body class associated with the current page.
 *
 * Note that this isn't the full layout class, but rather the name of the layout only.
 *
 * @return
 *   The name of the layout.
 */
ThemeBuilder.LayoutEditor.prototype.getPageLayoutName = function () {
  var $ = jQuery;
  var name = this.classToLayoutName($('body', parent.document)
    .attr('class'));
  return name;
};

/**
 * This method is responsible for changing the layout for preview mode, a
 * global layout change, and a single page layout change.
 *
 * @param {String} layoutName
 *   The name of the layout being selected.
 * @param {String} scope
 *   Indicates the scope of the change being applied.  "all" indicates the
 *   change should be committed for all pages.  "single" indicates the change
 *   should be committed only for the current page.  Anything else indicates
 *   the change should be previewed only (not committed.
 */
ThemeBuilder.LayoutEditor.prototype.pickLayout = function (layoutName, scope) {
  var $ = jQuery;
  var layout = this.layoutNameToClass(layoutName);
  var elem = $('#themebuilder-main .layout-' + layoutName);
  if (scope === 'all' || scope === 'single') {
    $('div', elem).not('.applied').fadeOut('fast', function () {
        elem.removeClass('preview');
        $('div', elem).css('opacity', '').css('display', '');
        $('#themebuilder-main .layout-shot.preview').removeClass('preview');
      });
  } else {
    $('#themebuilder-main .layout-shot.preview').removeClass('preview');
  }
  // Make a dialog.
  switch (scope) {
  case 'all':
    this.saveLayoutSelection(layout, '<global>');
    break;
  case 'single':
    this.saveLayoutSelection(layout, this.currentPage);
    break;
  default:
    elem.addClass('preview');
    this.previewSelection(layoutName);
    break;
  }
};

/**
 * Causes the specified layout to be set in preview mode (i.e. not committed
 * to the server.
 *
 * @param {String} layoutName
 *   The name of the layout to preview.
 */
ThemeBuilder.LayoutEditor.prototype.previewSelection = function (layoutName) {
  var $ = jQuery;
  var name = this.getPageLayoutName();
  var layoutClass = this.layoutNameToClass(name);
  $('body', parent.document).removeClass(layoutClass);
  // change this
  $('body', parent.document).addClass(this.layoutNameToClass(layoutName));
  this.shuffleRegionMarkup(layoutName);
};

/**
 * Returns an object that reveals the display order given the specified layout
 * name.
 *
 * @param {String} layoutName
 *   The string that identifies the order in which the columns should be
 *   displayed
 * @return
 *   An object with fields 'left' and 'right' that indicate each of the
 *   columns that should be visible in the specified layout, and whether those
 *   should be floated to the left or to the right.
 */
ThemeBuilder.LayoutEditor.prototype.getSidebarOrdering = function (layoutName) {
  var layoutIndex = layoutName.lastIndexOf('-');
  if (layoutIndex === -1) {
    ThemeBuilder.logCallback('Error - ThemeBuilder.LayoutEditor.getSidebarOrdering failed to determine the layout order for the layout ' + layoutName);
    return null;
  }
  var layout = layoutName.slice(layoutIndex + 1);
  var layoutArray = layout.split('');
  var result = {left: [],  right: []};
  var side = result.left;
  for (var i = 0; i < layoutArray.length; i++) {
    if (layoutArray[i] === 'c') {
      // Switch sides
      side = result.right;
    }
    else {
      side.push(layoutArray[i]);
    }
  }
  return result;
};

/**
 * Moves the markup around to support previewing a layout.  The order is
 * revealed in the specified order object, in which the order of the columns
 * is identified in fields 'left' and 'right'.  Note that these orders
 * represent the desired display orders, but not necessarily the order in
 * which the column markup should be written.  (float right requires that the
 * right-most column be written before the column just to the left of it, if
 * both are floated right).
 *
 * @param {jQueryElement} $parentElement
 *   The parent within which the columns to be moved are placed.
 * @param {Object} order
 *   An object with fields left and right that reveal the desired display
 *   order of the columns.  Note that the content column ('c') should not
 *   appear in the arrays.
 */
ThemeBuilder.LayoutEditor.prototype.shuffleRegionMarkup = function (layoutName) {
  var $ = jQuery;
  var sidebarMap = {};
  var useTbPrefix = false;
  var $parentElement = $('.tb-preview-shuffle-regions');
  if ($parentElement.length <= 0) {
    // This theme adjusts the layout based solely on the body tag.  No further
    // manipulation required.
    return;
  }
  if ($parentElement.length > 1) {
    // The themebuilder can not currently handle more than one parent that
    // requires children to be shuffled.  This could be supported at a later
    // time assuming we include a scheme in which the user can select which
    // part of the layout is being modified.
    throw 'Found ' + $parentElement.length + ' tb-preview-shuffle-regions elements in the markup.  The themebuilder cannot currently handle this.';
  }
  // 1 shuffle region.
  var order = this.getSidebarOrdering(layoutName);
  if (!order) {
    return;
  }

  var $sidebars = $parentElement.find('.tb-sidebar');
  for (var i = 0; i < $sidebars.length; i++) {
    if (!useTbPrefix && $sidebars[i].id.indexOf('tb-') === 0) {
      useTbPrefix = true;
    }
    sidebarMap['#' + $sidebars[i].id] = $sidebars[i];
  }
  var $content = $parentElement.find('.tb-primary');
  // Write the left columns in order, applying tb-left to float it left.
  for (i = 0; i < order.left.length; i++) {
    var id = '#' + (useTbPrefix ? 'tb-' : '') + 'sidebar-' + order.left[i];
    $(id).appendTo($parentElement)
      .removeClass('tb-right right')
      .addClass('tb-left left')
      .removeClass('tb-hidden');
    delete sidebarMap[id];
  }

  // Write the right columns in *reverse* order, applying tb-right to float it right.
  for (i = order.right.length - 1; i >= 0; i--) {
    id = '#' + (useTbPrefix ? 'tb-' : '') + 'sidebar-' + order.right[i];
    $(id).appendTo($parentElement)
      .removeClass('tb-left left')
      .addClass('tb-right right')
      .removeClass('tb-hidden');
    delete sidebarMap[id];
  }

  // Write the remaining (hidden) columns.
  for (id in sidebarMap) {
    if (typeof(id) === 'string') {
      $(id).appendTo($parentElement)
        .removeClass('tb-left left')
        .addClass('tb-right right')
        .addClass('tb-hidden');
    }
  }
  // Append content.  Note that the content region must always go last.
  $content.appendTo($parentElement);
};

/**
 * Cancels the current preview, causing the layout currently committed for the
 * current page to be used.
 *
 * @param {DomEvent} event
 *   (Optional) The event used to trigger the cancelPreview.  If provided, the
 *   event will be stopped.
 */
ThemeBuilder.LayoutEditor.prototype.cancelPreview =  function (event) {
  var $ = jQuery;
  event = event || window.event;
  if (!Drupal.settings.layoutIndex) {
    this.previewSelection(Drupal.settings.layoutGlobal);
  }
  else {
    this.previewSelection(Drupal.settings.layoutIndex);
  }
  $('#themebuilder-main .layout-shot.preview').removeClass('preview');
  if (event) {
    return ThemeBuilder.util.stopEvent(event);
  }
};

/**
 * Saves the layout.
 *
 * @param {string} layout
 *   The name of the layout to apply.
 * @param {string} url_pattern
 *   The url pattern for which this theme should apply.  "<global>" indicates
 *   this layout should be used for the entire site; any other string will
 *   be used as a url pattern for which the specified layout will apply.
 */
ThemeBuilder.LayoutEditor.prototype.saveLayoutSelection = function (layout, url_pattern) {
  if (url_pattern !== '<global>') {
    this.localmod.setNewState(layout);
    ThemeBuilder.applyModification(this.localmod);
    this.localmod = this.localmod.getFreshModification();
  }
  else {
    this.globalmod.setNewState(layout);
    this.localmod.setNewState('');
    var modification = new ThemeBuilder.GroupedModification();
    modification.addChild(this.currentPage, this.localmod);
    modification.addChild('<global>', this.globalmod);
    ThemeBuilder.applyModification(modification);
    this.globalmod = this.globalmod.getFreshModification();
    this.localmod = this.localmod.getFreshModification();
  }
};

/**
 * Applies the specified modification description to the client side only.
 * This allows the user to preview the modification without committing it
 * to the theme.
 *
 * @param {Object} desc
 *   The modification description.  To get this value, you should pass in
 *   the result of Modification.getNewState() or Modification.getPriorState().
 * @param {Modification} modification
 *   The modification that represents the change in the current state that
 *   should be previewed.
 */
ThemeBuilder.LayoutEditor.prototype.preview = function (desc, modification) {
  var $ = jQuery;
  // Get the layout name...
  var name = this.getPageLayoutName();
  var layoutClass = this.layoutNameToClass(name);
  var newName = desc.layout.split('body-layout-')[1];

  // Highlight the appropriate image in the layout selector.
  var screenshot = $('#themebuilder-main .layout-' + newName);
  var scope = desc.selector === '<global>' ? 'all' : 'single';
  $('#themebuilder-main .layout-shot.' + scope).removeClass(scope);
  if (desc.selector === this.currentPage) {
    $('#themebuilder-main .layout-shot.single').removeClass('single');
    screenshot.addClass('single');
    Drupal.settings.layoutIndex = newName;
  }
  else if (desc.selector === '<global>') {
    $('#themebuilder-main .layout-shot.all').removeClass('all');
    screenshot.addClass('all');
    Drupal.settings.layoutGlobal = newName;
  }
  else {
    //not handling yet
  }

  // Fix the body class to set the new layout.
  $('body', parent.document).removeClass(layoutClass);
  if (desc.layout) {
    $('body', parent.document).addClass(desc.layout);
    this.shuffleRegionMarkup(this.classToLayoutName(desc.layout));
  }
  else {
    $('body', parent.document).addClass(this.layoutNameToClass(Drupal.settings.layoutGlobal));
    this.shuffleRegionMarkup(Drupal.settings.layoutGlobal);
  }
};
