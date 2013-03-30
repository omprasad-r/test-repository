
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
 * @class
 */
ThemeBuilder.styles.ThemeMarkup1Filter = ThemeBuilder.initClass();

/**
 * The constructor of the ThemeMarkup1Filter class.
 *
 * @param {Array} selectorMap
 *   The current theme's selector map, which maps the default selector for a
 *   given element onto the selector preferred by the theme author.  On
 *   initial element selection, the theme author's choice will override this
 *   filter's default selection.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype.initialize = function (selectorMap) {
  this._themeSelectorMap = selectorMap || {};
  this.idBlackList = ['page-wrapper', 'gardens_ie', 'gardens_ie7', 'gardens_ie8'];
  this.classBlackList = ['rb-link', 'rb-textbox', 'clearfix', 'section', 'style-clickable', 'region-banner', 'region', 'html', 'logged-in', 'no-sidebars', 'page-node-', 'moz', 'moz2', 'mac', 'webkit', 'webkit5', 'themebuilder', 'theme-markup-1', 'toolbar', 'tb-auto-adjust-height', 'tb-breadcrumb', 'tb-hidden', 'tb-left', 'tb-preview-shuffle-regions', 'tb-primary', 'tb-right', 'tb-selector', 'tb-selector-preferred', 'tb-sidebar', 'tb-no-select'];
  this.tagBlackList = ['div', 'span'];
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
ThemeBuilder.styles.ThemeMarkup1Filter.prototype.activate = function (orig_path) {
  var path = this.blackListPath(orig_path);
  var newPath = [];
  var index = 0;
  var body = this.getBodyElement(path, index);
  if (body.element) {
    body.element.humanReadable = Drupal.t('Site background');
    newPath.push(body.element);
    index = body.index;
  }
  var region = this.getRegionElement(path);
  if (region.element) {
    newPath.push(region.element);
    index = region.index;
  }

  var block = this.getBlockElement(path);
  if (block.element) {
    newPath.push(block.element);
    index = block.index;
  }

  var pathEnd = [];
  var sufficientSpecificity = false;
  var selectedElementCount = 0;
  var selectedClassCount = 0;
  for (var elementIndex = path.length - 1; elementIndex > index; elementIndex--) {
    var selectedElement = path[elementIndex];
    selectedElement.disableAllButOneClass();
    selectedElement = this.normalizeSelector(selectedElement);
    if (selectedElement) {
      selectedElementCount++;
      var classes = selectedElement.getEnabledClasses() || [];
      if (!sufficientSpecificity || classes.indexOf('menu') !== -1) {
        var id = selectedElement.getId();
        if (id && id.length > 0) {
          sufficientSpecificity = true;
        }
        if (classes.length > 0) {
          selectedClassCount++;
          if (selectedClassCount > 1 || selectedElementCount > 2) {
            sufficientSpecificity = true;
          }
        }
      }
      else {
        selectedElement.setEnabled(false);
      }
      pathEnd.push(selectedElement);
    }
  }
  newPath = newPath.concat(pathEnd.reverse());
  if (newPath.length > 7) {
    // We shouldn't show too much.  Remove from the center (after the block).
    newPath = newPath.slice(0, 2).concat(newPath.slice(newPath.length - 4));
  }
  if (newPath.length === 1) {
    // The body tag was selected.  Make sure it is enabled.
    newPath[0].setEnabled(true);
  }
  newPath = this._substitutePathConfiguration(newPath);
  return newPath;
};

/**
 * Changes the path configuration for the specified path according to the path
 * map supplied by the theme.  The path map must be specified during
 * instantiation of the ThemeMarkup1Filter instance, and contains a mapping of
 * the selector that the ThemeMarkup1Filter would choose by default to a
 * selector that the theme author would prefer.  This makes the use of the
 * Selector much easier because trouble spots can be hand tuned by the theme
 * author.  The goal is to provide a default behavior and supporting selector
 * map that result in minimal need to modify the Selector.
 *
 * @private
 *
 * @param {PathElement array} path
 *   The array of PathElements representing the selected element path,
 *   configured as the default behavior of the ThemeMarkup1Filter would be.
 * @return {PathElement array}
 *   If the currently configured path is not in the theme selector map, the
 *   path is completely unchanged.  Otherwise, the path is reconfigured
 *   according to the theme author's wishes.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype._substitutePathConfiguration = function (path) {
  var selector = this._getSelectorFromPath(path);
  var newSelector = this._themeSelectorMap[selector] || selector;
  return this._applySelectorToPath(newSelector, path);
};

/**
 * Returns the selector represented by the specified path.
 *
 * @private
 *
 * @param {PathElement array} path
 *   The array of PathElements representing the currently selected element path.
 * @return {String}
 *   The css selector representing the specified path in its current configuration.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype._getSelectorFromPath = function (path) {
  var selector = [];
  for (var i = 0; i < path.length; i++) {
    var selectorPart = path[i].getEnabledCssSelector();
    if (selectorPart) {
      selector.push(selectorPart);
    }
  }
  return selector.join(' ');
};

/**
 * Applies the specified selector to the specified path.  This causes the path
 * to be reconfigured to reflect the specified selector.
 *
 * @private
 *
 * @param {String} selector
 *   The css selector representing the desired path configuration.
 * @param {PathElement array} path
 *   The array of PathElements representing the currently selected element path.
 * @return {PathElement array}
 *   The newly-configured element path.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype._applySelectorToPath = function (selector, path) {
  // Work from back to front, applying the selector to each node in the path.
  var parts = selector.split(' ').reverse();
  var partIndex = 0;
  for (var i = path.length - 1; i >= 0; i--) {
    if (partIndex >= parts.length) {
      path[i].setEnabled(false);
      continue;
    }
    // If the path element matches the current part, configure it, otherwise disable it.
    if (this._selectorMatchesPathElement(parts[partIndex], path[i])) {
      path[i].setEnabled(true);
      this._configurePathElement(path[i], parts[partIndex]);
      partIndex++;
    }
    else {
      path[i].setEnabled(false);
    }
  }
  return path;
};

/**
 * Applies the specified selector chunk to the specified path element.  This
 * causes the path element to be reconfigured to reflect the specified
 * selector chunk.
 *
 * @private
 *
 * @param {PathElement} pathElement
 *   The PathElement instance to configure.
 * @param {String} selector
 *   The css selector chunk representing the desired path element configuration.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype._configurePathElement = function (pathElement, selector) {
  switch (selector.charAt(0)) {
  case '#':
    pathElement.setTagEnabled(false);
    pathElement.setIdEnabled(true);
    pathElement.disableAllClasses();
    break;
  case '.':
    pathElement.setTagEnabled(false);
    pathElement.setIdEnabled(false);
    pathElement.disableAllClasses();
    pathElement.setClassEnabled(selector.slice(1), true);
    break;
  default:
    pathElement.setTagEnabled(true);
    pathElement.setIdEnabled(false);
    pathElement.disableAllClasses();
    break;
  }
};

/**
 * Determines whether the specified selector chunk corresponds with the
 * specified path element.
 *
 * @private
 *
 * @param {String} selector
 *   The part of the selector.
 * @param {PathElement} pathElement
 *   The pathElement instance.
 * @return {boolean}
 *   true if the specified selector chunk corresponds with the specified path
 *   element; false otherwise.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype._selectorMatchesPathElement = function (selector, pathElement) {
  var match = false;
  switch (selector.charAt(0)) {
  case '#':
    match = (selector === '#' + pathElement.getId());
    break;
  case '.':
    var classes = pathElement.getClasses();
    if (classes) {
      match = (classes.indexOf(selector.slice(1)) >= 0);
    }
    break;
  default:
    // The selector represents a tag.
    match = (selector === pathElement.getTag());
    break;
  }
  return match;
};

/**
 * Removes all blacklisted classes and tags from the specified path.
 *
 * @param {array} path
 *   The path from the Selector instance that called the filter.  Each
 *   element in the path will be checked and removed if necessary to
 *   honor the blacklist.
 *
 * @return {array}
 *   The new path containing elements that are not blacklisted.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype.blackListPath = function (path) {
  var newPath = [];
  for (var i = 0; i < path.length; i++) {
    var element = this.blackListPathElement(path[i]);
    if (element) {
      newPath.push(element);
    }
  }
  return newPath;
};

/**
 * Applies the blacklist to the specified path element.  The blacklist is a
 * set of ids, classes, and tags that should not appear in the specificity
 * options.  In some cases the entire element should be removed to achieve
 * the desired effect.
 *
 * @param {PathElement} pathElement
 *   The PathElement instance to filter with the blacklist.
 *
 * @return {PathElement}
 *   The PathElement instance with the blacklisted attributes removed, or
 *   undefined if the specified path element should be removed entirely.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype.blackListPathElement = function (pathElement) {
  if (pathElement.id) {
    if (this.idBlackList.contains(pathElement.id)) {
      pathElement.id = undefined;
    }
  }

  if (pathElement.classes) {
    var classes = [];
    for (var i = 0; i < pathElement.classes.length; i++) {
      if (!this.classBlackList.contains(pathElement.classes[i])) {
        classes.push(pathElement.classes[i]);
      }
    }
    pathElement.classes = classes;
  }

  if (pathElement.tag) {
    if (this.tagBlackList.contains(pathElement.tag)) {
      pathElement.tag = undefined;
      pathElement.setTagEnabled(false);
    }
  }
  return this.normalizeSelector(pathElement);
};

/**
 * Finds the index of an element in the path that has the specified
 * tag.  Only the index of the first element with a matching tag is
 * returned.
 *
 * @param {array} path
 *   An array of PathElement instances that represent the path.
 * @param {string} tag
 *   The tag that identifies the PathElement instance to find.
 *
 * @return
 *   The index within the path array corresponding with the matching
 *   PathElement instance, or -1 if no matches were found.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype.findElementIndexByTag = function (path, tag) {
  for (var i = 0; i < path.length; i++) {
    if (path[i].getTag() === tag) {
      return i;
    }
  }
  return -1;
};

/**
 * Finds the index of an element in the path that has the specified
 * class.  Only the index of the first element with a matching class
 * is returned.
 *
 * @param {array} path
 *   An array of PathElement instances that represent the path.
 * @param {string} class
 *   The class that identifies the PathElement instance to find.
 *
 * @return
 *   The index within the path array corresponding with the matching
 *   PathElement instance, or -1 if no matches were found.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype.getElementIndexByClass = function (path, classname) {
  for (var i = 0; i < path.length; i++) {
    var classes = path[i].getAllClasses();
    if (classes && classes.contains(classname)) {
      return i;
    }
  }
  return -1;
};

/**
 * Finds all indexes of elements in the specified path that have the specified
 * class.
 *
 * @param {array} path
 *   An array of PathElement instances that represent the path.
 * @param {string} class
 *   The class that identifies the PathElement instances to find.
 *
 * @return
 *   An array containing the indexes within the path array corresponding with
 *   the matching PathElement instances.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype.getElementIndexesByClass = function (path, classname) {
  var result = [];
  for (var i = 0; i < path.length; i++) {
    var classes = path[i].getAllClasses();
    if (classes && classes.contains(classname)) {
      result.push(i);
    }
  }
  return result;
};

/**
 * Returns an object that identifies the body element, given the specified
 * path.
 *
 * @param {array} path
 *   An array of PathElement instances that represent the path.
 *
 * @return
 *   An object that identifies the body element.  The has a field
 *   (element) that identifies the PathElement instance associated
 *   with the body element, and a field (index) that identifies the
 *   index within the specified path of the body element.  If no body
 *   element exists in the path, the element field will be undefined,
 *   and the index field will be -1.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype.getBodyElement = function (path) {
  var result = undefined;
  var index = this.findElementIndexByTag(path, 'body');
  if (index >= 0) {
    result = ThemeBuilder.clone(path[index]);
    result.disableAllClasses();
    result.classes = undefined;
    result.setIdEnabled(false);
    result.id = undefined;
    result.setEnabled(false);
    result = this.normalizeSelector(result);
  }
  return {'element': result, 'index': index};
};


/**
 * Returns an object that identifies the region element, given the specified
 * path.
 *
 * @param {array} path
 *   An array of PathElement instances that represent the path.
 *
 * @return
 *   An object that identifies the region element.  The has a field
 *   (element) that identifies the PathElement instance associated
 *   with the region element, and a field (index) that identifies the
 *   index within the specified path of the region element.  If no
 *   region element exists in the path, the element field will be
 *   undefined, and the index field will be -1.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype.getRegionElement = function (path) {
  var result = undefined;
  var indexes = this.getElementIndexesByClass(path, 'area');
  if (indexes.length > 0) {
    result = ThemeBuilder.clone(path[indexes[0]]);
    result.disableAllClasses();
    result.setElementGroup(Drupal.t('region'), Drupal.t('regions'));
    this.normalizeSelector(result);
  }
  return {'element': result, 'index': indexes[0]};
};

/**
 * Returns an object that identifies the block element, given the
 * specified path.
 *
 * @param {array} path
 *   An array of PathElement instances that represent the path.
 *
 * @return
 *   An object that identifies the block element.  The has a field
 *   (element) that identifies the PathElement instance associated
 *   with the block element, and a field (index) that identifies the
 *   index within the specified path of the block element.  If no
 *   block element exists in the path, the element field will be
 *   undefined, and the index field will be -1.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype.getBlockElement = function (path) {
  var result = undefined;
  var indexes = this.getElementIndexesByClass(path, 'block');
  if (indexes.length > 0) {
    result = ThemeBuilder.clone(path[indexes[0]]);
    result.disableAllButOneClass();
    result.setIdEnabled(false);
    result.setElementGroup(Drupal.t('block'), Drupal.t('blocks'));
    result.css_prefix = 'block';
    this.normalizeSelector(result);
  }
  return {'element': result, 'index': indexes[0]};
};

/**
 * Forces the specified PathElement instance into a valid state, or
 * causes the PathElement instance to be removed from the path.  A
 * valid state involves the selector including the element id or a
 * class or the element's tag, but not any combination.
 *
 * @param {PathElement} pathElement
 *   The PathElement instance to put into a valid state.
 *
 * @return {PathElement}
 *   The modified PathElement instance if its state was made valid, or
 *   undefined if the state could not be made valid.
 */
ThemeBuilder.styles.ThemeMarkup1Filter.prototype.normalizeSelector = function (pathElement) {
  if (pathElement.getId() && pathElement.isIdEnabled() === true) {
    // Using an id.  Do not include the tag or classes.
    pathElement.disableAllClasses();
    pathElement.setTagEnabled(false);
  }
  else if (pathElement.getEnabledClasses() !== undefined) {
    // Using a class.  Do not include the tag.
    pathElement.setTagEnabled(false);
  }
  else {
    // Not including a class or id.  Include the tag instead.
    if (pathElement.tag) {
      pathElement.setTagEnabled(true);
    }
    else {
      return undefined;
    }
  }

  // Make sure the path element is still valid.  It must have an id or
  // at least one class or a tag.
  var classes = [].concat(pathElement.classes, pathElement.greyClasses);
  if (pathElement.id || classes.length > 0 || pathElement.tag) {
    return pathElement;
  }
  return undefined;
};
