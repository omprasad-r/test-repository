
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * A filter implementation used in the ThemeBuilder.  This filter is
 * responsible for implementing all of our policies about what should be
 * selected by default, and which elements should be included in the
 * selector.
 *
 * This selector is designed to work with themes that contain the
 * 'theme-markup-2' body class.  The ElementPicker instantiates the
 * appropriate filter class when creating a Selector instance.
 * @class
 * @extends ThemeBuilder.styles.ThemeMarkup1Filter
 */
ThemeBuilder.styles.ThemeMarkup2Filter = ThemeBuilder.initClass();
ThemeBuilder.styles.ThemeMarkup2Filter.prototype = new ThemeBuilder.styles.ThemeMarkup1Filter();

/**
 * The constructor of the ThemeMarkup2Filter class.
 *
 * @param {Array} selectorMap
 *   The current theme's selector map, which maps the default selector for a
 *   given element onto the selector preferred by the theme author.  On
 *   initial element selection, the theme author's choice will override this
 *   filter's default selection.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.initialize = function (selectorMap) {
  this._themeSelectorMap = selectorMap || {};
  this.classBlackList = [
    'clearfix',
    'rb-link',
    'rb-textbox',
    'region-banner',
    'region',
    'section',
    'html',
    'logged-in',
    'no-sidebars',
    'page-node-',
    'moz',
    'moz2',
    'mac',
    'webkit',
    'webkit5',
    'themebuilder',
    'theme-markup-2',
    'toolbar',
    'style-clickable',
    'tb-auto-adjust-height',
    'tb-breadcrumb',
    'tb-content-wrapper-1',
    'tb-content-wrapper-2',
    'tb-content-wrapper-3',
    'tb-content-wrapper-4',
    'tb-header-inner-1',
    'tb-header-inner-2',
    'tb-header-inner-3',
    'tb-header-wrapper-1',
    'tb-header-wrapper-2',
    'tb-height-balance',
    'tb-hidden',
    'tb-left',
    'tb-no-select',
    'tb-precontent-1',
    'tb-precontent-2',
    'tb-precontent-3',
    'tb-prefooter-1',
    'tb-prefooter-2',
    'tb-prefooter-3',
    'tb-preview-shuffle-regions',
    'tb-primary',
    'tb-region',
    'tb-right',
    'tb-selector',
    'tb-selector-preferred',
    'tb-sidebar',
    'tb-scope',
    'tb-scope-prefer',
    'tb-terminal'
  ];
};

/**
 * Causes the filter to be executed on the specified path.  The entire
 * path is passed to the filter so the filter can make choices about which
 * elements should be enabled or disabled or even removed entirely.
 *
 * @param {array} path
 *   The array of PathElement instances that together represent the entire
 *   css path to the element that the user selected.
 *
 * @return {array}
 *   A similar array to what was passed in that identifies the new path
 *   that should be used by the Selector instance that called the filter.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.activate = function (path) {
  path = this.removeEverythingAboveBodyTag(path);
  path = this.removeNonselectableEndElements(path);
  var body = this.configureBodyElement(path);

  // Terminal elements are tagged with the class 'tb-terminal', which
  // indicates the associated element should only appear in the element
  // selector if it is the last element in the selector.
  path = this.removeTerminalElements(path);

  // Scope elements are tagged with the class 'tb-scope', which indicates they
  // are significant structural elements that should always appear in the
  // selector.  Note that only one of these will be enabled by default.
  var scopeElements = this.getScopeElements(path);

  // Create a selector for the actual item selected that provides enough
  // specificity and options for theming.
  var items = this.getItemElements(path);

  // Construct the path
  path = [body].concat(scopeElements, items);

  // Configure significant elements in the path.
  this.configureRegionElement(path);
  this.configureBlockElement(path);
  this.configureViewElement(path);
  this.configureMenuElement(path);

  // Make sure blacklisted items never appear as options in the specificity
  // selector.
  path = this.blackListPath(path);

  this.enableLastElement(path);
  return path;
};

/**
 * Removes everything above the body tag in the specified path.  This is
 * useful for dropping the html tag from the path.
 *
 * @param {array} path
 *   The path, which is an array of PathElement objects.
 * @return
 *   A new path array with every element above the body tag removed.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.removeEverythingAboveBodyTag = function (path) {
  var result = [];
  for (var index = 0; index < path.length; index++) {
    var element = path[index];
    if (element.getTag() === 'body') {
      result = path.slice(index);
      break;
    }
  }
  return result;
};

/**
 * Removes non-selectable elements from the right side of the element
 * selector.
 *
 * @param {array} path
 *   The path, which is an array of PathElement objects.
 * @return
 *   A new path array with every element that cannot be selected removed from
 *   the right side.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.removeNonselectableEndElements = function (path) {
  var result = undefined;
  // Find the first relevant element from the right side.
  var len = path ? path.length : 0;
  for (var i = len - 1; i >= 0; i--) {
    var element = this.blackListPathElement(ThemeBuilder.clone(path[i]));
    if (element) {
      // Found the first element.
      if (i === path.length - 1) {
        result = path;
      }
      else {
        result = path.slice(0, i);
      }
      break;
    }
  }
  return result;
};

/**
 * Remvoves terminal elements from the specified path.  Terminal elements are
 * DOM elements that have the 'tb-terminal' class applied.  This class
 * indicates the associated element should only appear in the selector if it
 * is the element selected by the user.
 *
 * @param {array} path
 *   The path, which is an array of PathElement objects.
 * @return
 *   A new path array with all of the terminal elements removed.  If the user
 *   selected a terminal element (it is the last one in the path), then it
 *   will appear in the resulting path.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.removeTerminalElements = function (path) {
  var result = [];
  for (var i = 0; i < path.length; i++) {
    var element = path[i];
    if (!element.hasClass('tb-terminal') || i === path.length - 1) {
      result.push(element);
    }
  }
  return result;
};

/**
 * Finds and configures the body element in the specified path.  The body
 * element will be stripped of all ids and classes and will be disabled by
 * default.
 *
 * @param {array} path
 *   The path, which is an array of PathElement objects.
 * @return
 *   The configured body element.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.configureBodyElement = function (path) {
  var body = undefined;
  for (var i = 0; !body && i < path.length; i++) {
    if (path[i].getTag() === 'body') {
      body = path[i];
      break;
    }
  }
  body.humanReadable = Drupal.t('Site background');
  body.disableAllClasses();
  body.setIdEnabled(false);
  body.setEnabled(false);
  return body;
};

/**
 * Returns an array of PathElement objects representing the scope elements in
 * the path.  Scope elements are DOM elements that have the 'tb-scope' class
 * applied.  This class indicates the associated element(s) are significant
 * structural elements that should always appear in the selector.
 *
 * This method also configures the scope elements.  All but one of the scope
 * elements will be disabled.  The general rule is that the most specific
 * (farthest to the right) scope element will be enabled.  This can be
 * overridden in the markup by applying the 'tb-scope-prefer' class, which
 * provides a hint as to which of the scope elements should be enabled.  This
 * hint will be honored unless the user directly selected one of the other
 * scope elements available.
 *
 * @param {array} path
 *   The path, which is an array of PathElement objects.
 * @return
 *   A new path array which only contains elements that have the 'tb-scope'
 *   class applied.  The elements will be configured, with only one of the set
 *   of elements enabled.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.getScopeElements = function (path) {
  var result = [];
  var indexes = this.getElementIndexesByClass(path, 'tb-scope');
  for (var i = 0; i < indexes.length; i++) {
    var element = ThemeBuilder.clone(path[indexes[i]]);
    if (this.canUseId(element)) {
      // Prefer the id.  As a general rule, all tb-scope elements should have
      // an id.
      element.disableAllClasses();
    }
    else {
      var classes = this.getNonBlacklistedClasses(element);
      element.disableAllClasses();
      element.setClassEnabled(classes[0], true);
    }
    element.setEnabled(false);
    this.normalizeSelector(element);
    result.push(element);
  }

  // Enable the appropriate tb-scope element.
  if (result.length > 0) {
    if (path.length > 0 && path[path.length - 1].hasClass('tb-scope')) {
      // The most specific tb-scope element was selected by the user.  Enable
      // that rather than honoring the tb-scope-prefer tag.
      result[result.length - 1].setEnabled(true);
    }
    else {
      var index = this.getElementIndexByClass(result, 'tb-scope-prefer');
      if (index >= 0) {
        // A tb-scope-prefer tag is overriding the most specific tb-scope
        // element.
        result[index].setEnabled(true);
      }
      else {
        // The default behavior is to enable the most specific tb-scope
        // element.
        result[result.length - 1].setEnabled(true);
      }
    }
  }
  return result;
};

/**
 * Returns an object that configures the region element, given the specified
 * path.
 *
 * @param {array} path
 *   The path, which is an array of PathElement objects.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.configureRegionElement = function (path) {
  var index = this.getElementIndexByClass(path, 'tb-region');
  if (index >= 0) {
    var element = path[index];
    if (this.canUseId(element)) {
      // Prefer the id.
      element.disableAllClasses();
      element.setTagEnabled(false);
      element.setIdEnabled(true);
    }
    else {
      var classes = this.getNonBlacklistedClasses(element);
      element.disableAllClasses();
      if (classes && classes.length > 0) {
        // Use a class.
        element.setClassEnabled(classes[0], true);
        element.setTagEnabled(false);
      }
      else {
        // Use the tag.
        element.setTagEnabled(true);
      }
    }
    element.setElementGroup(Drupal.t('region'), Drupal.t('regions'));
  }
};

/**
 * Returns an object that configures the block element, given the specified
 * path.
 *
 * @param {array} path
 *   The path, which is an array of PathElement objects.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.configureBlockElement = function (path) {
  var index = this.getElementIndexByClass(path, 'block');
  if (index >= 0) {
    var element = path[index];
    // Prefer the block class.
    element.disableAllButOneClass('block');
    element.setTagEnabled(false);
    element.setIdEnabled(false);
    element.setElementGroup(Drupal.t('block'), Drupal.t('blocks'));
    if (path.length > 0 && path[path.length - 1].getId() === element.getId()) {
      // Normally we don't select the block, but in this case the user
      // selected the block specifically.
      element.setEnabled(true);
    }
    else {
      element.setEnabled(false);
    }
  }
};

/**
 * Returns an object that configures the View element, given the specified
 * path.
 *
 * @param {array} path
 *   The path, which is an array of PathElement objects.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.configureViewElement = function (path) {
  var index = this.getElementIndexByClass(path, 'view');
  if (index >= 0) {
    var element = path[index];
    // Prefer the second class on the view element if more than one exists
    var classes = this.getNonBlacklistedClasses(element);
    if (classes && classes.length > 1) {
      element.disableAllButOneClass(classes[1]);
    }
    else {
      element.disableAllButOneClass(classes[0]);
    }
    element.setTagEnabled(false);
    element.setIdEnabled(false);
    element.setElementGroup(Drupal.t('view'), Drupal.t('views'));
    element.setEnabled(true);
  }
};

/**
 * Returns an object that configures the View element, given the specified
 * path.
 *
 * @param {array} path
 *   The path, which is an array of PathElement objects.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.configureMenuElement = function (path) {
  // This code assumes a maximum 5 levels of nested menus. Any more is an edge
  // case.
  for (var i = 1; i < 6; i++) {
    var index = this.getElementIndexByClass(path, 'level-' + i);
    if (index >= 0) {
      var element = path[index];
      // Prefer the second class on the menu element if more than one exists
      var classes = this.getNonBlacklistedClasses(element);
      if (classes && classes.length > 1 && !(element.getAllClasses().contains('leaf'))) {
        element.disableAllButOneClass(classes[1]);
      }
      else {
        element.disableAllButOneClass(classes[0]);
      }
      element.setElementGroup(Drupal.t('menu item'), Drupal.t('menu items'));
      // Enable the second-level menu items to create a good scope.
      if (i === 2) {
        element.setTagEnabled(false);
        element.setIdEnabled(false);
        element.setEnabled(true);
      }
    }
  }
};

/**
 * Returns an array of configured PathElement objects that will be sufficient
 * for describing the selected element.  This method focuses only on the
 * selected element, not the full selector.
 *
 * @param {array} path
 *   The path, which is an array of PathElement objects.
 * @return
 *   A new path array which only contains elements that sufficiently represent
 *   the element that the user selected.  The elements will be configured with
 *   only the number of elements enabled to reasonably identify the selection.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.getItemElements = function (path) {
  var pathEnd = [];
  var farEnough = false;
  var enable = true;
  var caps = ['block'];
  var extenders = ['view', 'pulldown'];
  var isLong = false;
  var tagOrClassCount = 0;
  for (var elementIndex = path.length - 1; !farEnough && elementIndex > 0 && elementIndex >= 0; elementIndex--) {
    var element = path[elementIndex];
    // If the element is a .tb-scope element, then we've gone far enough.
    if (element.hasClass('tb-scope')) {
      farEnough = true;
    }
    else {
      // Process the element.
      element.setEnabled(enable);
      // Use the element's ID.
      if (this.canUseId(element)) {
        element.disableAllClasses();
        element.setTagEnabled(false);
        element.setIdEnabled(true);
        enable = false;
      }
      // Use the element's tag.
      else if (!this.tagBlackList.contains(element.getTag())) {
        element.disableAllClasses();
        element.setIdEnabled(false);
        element.setTagEnabled(true);
        tagOrClassCount++;
      }
      // Use the element's first class.
      else {
        var classes = this.getNonBlacklistedClasses(element);
        if (classes && classes.length > 0) {
          element.disableAllButOneClass(classes[0]);
          element.setIdEnabled(false);
          element.setTagEnabled(false);
          tagOrClassCount++;
        }
      }
      // Stop enabling elements if the first two are already enabled.
      if (tagOrClassCount > 1) {
        enable = false;
      }
      pathEnd.push(element);
      // If we find an element in the path that extends the path end, then mark
      // it as isLong. This is done for components like Views or menus so that
      // all the elements of the component are available for selection.
      for (var i = 0; i < extenders.length; i++) {
        if (element.hasClass(extenders[i])) {
          isLong = true;
        }
      }
      // If we find an element in the path that is identified as a cap, then the
      // path has been parsed enough.
      for (var j = 0; j < caps.length; j++) {
        if (element.hasClass(caps[j])) {
          farEnough = true;
        }
      }
    }
  }

  // Trim the number of elements if isLong is false and the path is long.
  if (pathEnd.length > 3 && !isLong) {
    pathEnd = pathEnd.slice(0, 3);
  }
  // The code works backwards (right to left), so reverse the resulting path
  // to achieve the correct order of PathElement instances.
  return pathEnd.reverse();
};

/**
 * Returns classes associated with the specified element which do not appear
 * in the class blacklist.
 *
 * @param {PathElement}
 *   The path element object.
 * @return
 *   An array of classes associated with the specified object that are not in
 *   the class blacklist.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.getNonBlacklistedClasses = function (element) {
  var result = [];
  var classes = element.getClasses();
  for (var i = 0; classes && i < classes.length; i++) {
    if (!this.classBlackList.contains(classes[i])) {
      result.push(classes[i]);
    }
  }
  return result;
};

/**
 * Determines whether the id can be used to identify the specified element.
 *
 * @param {PathElement}
 *   The path element object.
 * @return
 *   true if the id associated with the specified element can be used to
 *   identify the element; false otherwise.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.canUseId = function (element) {
  var id = element.getId();
  return id && !this.idBlackList.contains(id) && !this.isNodeId(id);
};

/**
 * Returns true if the specified element id represents a node id.
 *
 * @param {String} id
 *   The element id.
 * @return
 *   True if the id represents a node; false otherwise.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.isNodeId = function (id) {
  return id.match(/^node-(\d)+$/);
};

/**
 * Causes the last element in the path to be selected.  This is important
 * because it represents the element closest to the element selected by the
 * user.
 *
 * @param {array} path
 *   The path, which is an array of PathElement objects.
 */
ThemeBuilder.styles.ThemeMarkup2Filter.prototype.enableLastElement = function (path) {
  if (path.length > 0) {
    path[path.length - 1].setEnabled(true);
  }
};
