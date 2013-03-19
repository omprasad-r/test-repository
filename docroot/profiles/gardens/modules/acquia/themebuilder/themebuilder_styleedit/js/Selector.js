
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

ThemeBuilder.styles = ThemeBuilder.styles || {};


/**
 * The selector class represents a css selector broken down into its
 * constituent parts.
 * @class
 */
ThemeBuilder.styles.Selector = ThemeBuilder.initClass();

/**
 * The constructor of the Selector class.
 *
 * @param {object} filter
 *   The filter to use with this Selector instance.  The filter is responsible
 *   for filtering elements from the path.
 */
ThemeBuilder.styles.Selector.prototype.initialize = function (filter) {
  if (filter && typeof filter.activate !== 'function') {
    throw "filter parameter must have an activate method.";
  }
  this.filter = filter;
  this.listeners = [];
};

/**
 * Called when the settings have changed.  This is intended to be
 * called by the UI code when the user interacts with the path.
 * Modify any of the path element settings and then call this method
 * so the path can be run through the filter again.
 */
ThemeBuilder.styles.Selector.prototype.pathElementSettingsChanged = function () {
  this.selectorChanged();
};

/**
 * Adds the specified listener, which will be called when changes are done
 * to the selector.  This is a simple scheme that can be used to keep the
 * UI in sync with changes to the underlying selector.
 *
 * @param {object} listener
 *  The listener object, which can have a selectorChanged method that is called
 *  when the selector is modified, and a selectorElementChanged method that
 *  is called when the selected element is changed.
 */
ThemeBuilder.styles.Selector.prototype.addSelectorListener = function (listener) {
  this.listeners.push(listener);
};

/**
 * Called when changes occur to the selector.  This causes the listeners
 * to be notified.
 */
ThemeBuilder.styles.Selector.prototype.selectorChanged = function () {
  for (var i = 0; i < this.listeners.length; i++) {
    if (this.listeners[i].selectorChanged) {
      this.listeners[i].selectorChanged(this);
    }
  }
};

/**
 * Called when the selected element is changed.
 */
ThemeBuilder.styles.Selector.prototype.selectorElementChanged = function () {
  for (var i = 0; i < this.listeners.length; i++) {
    if (this.listeners[i].selectorElementChanged) {
      this.listeners[i].selectorElementChanged(this);
    }
  }
};

/**
 * Reset this instance with the specified element.  The path of the
 * specified element will replace the existing path.
 *
 * @param {DOMElement} element
 *   The DOM element representing the target of this selector.  The path
 *   will be generated by looking up from the specified element.
 */
ThemeBuilder.styles.Selector.prototype.setElement = function (element) {
  var path = [];
  this._selectedElement = element;
  while (element && element.nodeType === 1) {
    var pathelement = new ThemeBuilder.styles.PathElement(element);
    path.push(pathelement);
    element = element.parentNode;
  }
  this.path = path.reverse();
  if (this.filter) {
    this.originalPath = this.path;
    this.path = this.filter.activate(this.path.slice());
  }
  this.selectorElementChanged();
};

/**
 * Returns the element that was selected by the user.
 *
 * @return {DOMElement}
 *   The DOM element representing the user's selection.
 */
ThemeBuilder.styles.Selector.prototype.getElement = function () {
  return this._selectedElement;
};

/**
 * Returns the element that should be highlighted or used for figuring out
 * current property values.
 */
ThemeBuilder.styles.Selector.prototype.getSelectedElement = function () {
  var $ = jQuery;
  var selector = this.getCssSelector();
  selector = ThemeBuilder.util.removeStatePseudoClasses(selector);
  var selectedElement = this.getElement();
  if (selectedElement) {
    for (var element = selectedElement; element.parentNode; element = element.parentNode) {
      if ($(element).is(selector)) {
        return element;
      }
    }
  }
  return $('body').get(0);
};

/**
 * Returns the css selector that corresponds to the current state of this
 * Selector instance.
 *
 * @return {string}
 *   A string containing the css selector.
 */
ThemeBuilder.styles.Selector.prototype.getCssSelector = function () {
  var path_strings = [];
  if (this.path) {
    for (var i = 0; i < this.path.length; i++) {
      var selector = this.path[i].getEnabledCssSelector();
      if (selector !== '') {
        path_strings.push(selector);
      }
    }
  }
  return path_strings.join(' ');
};

/**
 * Returns a human readable phrase that describes the current selection.
 *
 * @return {string}
 *   A string containing a human readable phrase.
 */
ThemeBuilder.styles.Selector.prototype.getHumanReadableSelector = function () {
  var path_strings = [];
  if (this.path) {
    // Construct the phrase from the inside out.
    for (var i = this.path.length - 1; i >= 0; i--) {
      var selector = this.path[i].getEnabledCssSelector();
      if (selector !== '') {
        var description = this.path[i].getHumanReadableLabelFromSelector(selector);
        if (path_strings.length === 0) {
          description = ThemeBuilder.util.capitalize(description);
        }
        path_strings.push(description);
      }
    }
  }
  return path_strings.join(Drupal.t(' in '));
};


//------------- PathElement --------------------
/**
 * An instance of the PathElement class represents a single element and
 * its associated attributes (id, tag, classes) as appropriate for the
 * specific element it represents.
 * @class
 */
ThemeBuilder.styles.PathElement = ThemeBuilder.initClass();

/**
 * Constructor for the PathElement class.  This constructor takes a
 * DOM element and initializes the PathElement instance accordingly.
 */
ThemeBuilder.styles.PathElement.prototype.initialize = function (element) {
  this._initializeFromElement(element);
  this._filterClasses();
};

/**
 * Initializes this PathElement instance from the specified DOM element.
 * By default, the element will be enabled, all of its classes will be
 * enabled, and its id will be enabled if it exists.
 *
 * This class is not the right place to implement selection policy; it
 * makes everything available, and each of these pieces can be filtered
 * by a separate object.
 *
 * @param {DOMElement} element
 *   The DOM element from which this instance should be initialized.
 */
ThemeBuilder.styles.PathElement.prototype._initializeFromElement = function (element) {
  this.element_group_singular = '';
  this.element_group_plural = 'items';
  this.enabled = true;
  this.tag = element.tagName.toLowerCase();
  this.tag_enabled = true;
  this.id = element.id;
  // Make sure the id is valid.
  if (this.id && !this.id.match(/^[A-Za-z]+[A-Za-z0-9-_:.]*$/)) {
    this.id = undefined;
  }
  this.id_enabled = (this.id !== undefined);
  this._addPseudoClasses(element);

  this.classes_enabled = {};
  if (element.className) {
    this.classes = element.className.split(' ');
    for (var i = 0; i < this.classes.length; i++) {
      if (typeof this.classes[i] === 'string') {
        this.classes_enabled[this.classes[i]] = true;
      }
    }
  }
  this.pseudoclasses_enabled = {};
  if (this.pseudoclasses) {
    for (i = 0; i < this.pseudoclasses.length; i++) {
      if (typeof this.pseudoclasses[i] === 'string') {
        this.pseudoclasses_enabled[this.pseudoclasses[i]] = false;
      }
    }
  }
};

/**
 * Adds pseudoclasses to appropriate elements.
 *
 * @private
 *
 * @param {DomElement} element
 *   The element that was selected.
 */
ThemeBuilder.styles.PathElement.prototype._addPseudoClasses = function (element) {
  if (this.tag === 'a') {
    this.pseudoclasses = [Drupal.t('none'), 'link', 'visited', 'hover', 'active'];
  }
  else {
    this.pseudoclasses = [Drupal.t('none'), 'hover'];
  }
  
  if (element.parentNode.firstChild === element) {
    this.pseudoclasses.push('first-child');
  }
  if (ThemeBuilder.util.getLastChild(element.parentNode) === element) {
    this.pseudoclasses.push('last-child');
  }
};

/**
 * Sets the group that this element falls into.  Examples of groups
 * are "region" and "block".  These group names are used in the
 * construction of the human readable string that represents this
 * element.
 *
 * @param {string} groupSingular
 *   The name of the group, singular form.
 * @param {string} groupPlural
 *   The name of the group, plural form.
 */
ThemeBuilder.styles.PathElement.prototype.setElementGroup = function (groupSingular, groupPlural) {
  this.element_group_singular = groupSingular;
  this.element_group_plural = groupPlural;
};

/**
 * Returns the name of the group that this element falls into.
 *
 * @param {boolean} plural
 *   Optional parameter that indicates whether the singular or plural
 *   form is desired.  The default is singular if this parameter is
 *   not specified.
 *
 * @return {string}
 *   The element group.
 */
ThemeBuilder.styles.PathElement.prototype.getElementGroup = function (plural) {
  if (plural === true) {
    return this.element_group_plural;
  }
  return this.element_group_singular;
};

/**
 * Returns the human readable label that corresponds to this path element
 * instance.
 *
 * @return {string}
 *   A string containing the human readable label.
 */
ThemeBuilder.styles.PathElement.prototype.getHumanReadableLabel = function () {
  if (this.humanReadable) {
    return (this.humanReadable);
  }

  var pseudoclass = null;
  var scopePseudoclass = '';
  var statePseudoclass = '';
  if (this.pseudoclasses_enabled) {
    pseudoclass = this.pseudoclasses_enabled[0];
    switch (this.getPseudoClassType(pseudoclass)) {
    case 'scope':
      scopePseudoclass = pseudoclass;
      break;
    case 'state':
      statePseudoclass = pseudoclass;
      break;
    }
  }
  if (this.id_enabled && this.id) {
    return Drupal.t('the @scope@id @group',
      {"@scope": scopePseudoclass,
       "@id": this.convertToHuman(this.getId()),
       "@group": this.getElementGroup(false)});
  }
  var classes = this.getEnabledClasses();
  if (classes) {
    return Drupal.t('all @scope@class @group',
      {"@scope": scopePseudoclass,
       '@class': this.convertToHuman(classes[0]),
      '@group': this.getElementGroup(true)});
  }
  if (this.tag_enabled === true) {
    return Drupal.t('all @scope@tag @group',
      {"@scope": scopePseudoclass,
       '@tag': this.getTagName(),
      '@group': this.getElementGroup(true)});
  }
  return (this.getEnabledCssSelector());
};

/**
 * Returns the human readable label that corresponds to this path element
 * instance given the specified css selector.  This form is needed because
 * it allows the human readable label to be generated for this element
 * for states that the element is currently not in.  This is a requirement
 * for rendering an option menu that allows the user to select the desired
 * element state from a list.
 *
 * @param {string} css
 *   The css selector representing the state of this element that should be
 *   rendered in human readable text.
 *
 * @return {string}
 *   A string containing the human readable label.
 */
ThemeBuilder.styles.PathElement.prototype.getHumanReadableLabelFromSelector = function (css) {
  var result = '';
  var pseudoclass = ThemeBuilder.util.getPseudoClass(css);
  css = ThemeBuilder.util.removePseudoClasses(css);
  var which = '';
  var state = '';
  if (pseudoclass) {
    switch (pseudoclass) {
    case 'first-child':
      which = Drupal.t('first ');
      break;
    case 'last-child':
      which = Drupal.t('last ');
      break;
    default:
      state = Drupal.t(' in the @state state', {'@state': pseudoclass});
    }
  }
  switch (css.charAt(0)) {
  case '#':
    result =  Drupal.t('the @which@id @group@state', {
        '@which': which,
        '@id': this.convertToHuman(css.substr(1)),
        '@group': this.getElementGroup(false),
        '@state': state
      });
    break;

  case '.':
    var group = this.getElementGroup(true);
    if (!group) {
      group = Drupal.t('objects');
    }
    result = Drupal.t('all @which@class @group@state', {
        '@which': which,
        '@class': this.convertToHuman(css.substr(1)),
        '@group': this.getElementGroup(true),
        '@state': state
      });
    break;
  // the body element
  case 'b':
    if (css === 'body' && this.humanReadable) {
      result = Drupal.t('@result@state', {
        '@result': this.humanReadable,
        '@state': state
      });
    }
    break;
  default:
    // This would be a tag name
    result = Drupal.t('all @which@tag@state', {
        '@which': which,
        '@tag': this.getTagName(css),
        '@state': state
      });
  }
  return result;
};

/**
 * Converts the specified css selector name string to human readable
 * text.  Note that this is only designed to work with a single part
 * of the css.  Specify either the id, a class, or a tag.  Not any
 * combination.  This method converts the name part of the selector, not
 * the type specifier character ('#', '.').  Essentially this method
 * removes confusing duplication that occurs when the region and the
 * selector have the same text.
 *
 * Example: block-shortcut-shortcuts => shortcut-shortcuts, and used
 * in the human readable label "The shortcut-shortcuts block".
 *
 * @param {string} css
 *   A string containing a css selector that represents the state of this
 */
ThemeBuilder.styles.PathElement.prototype.convertToHuman = function (css) {
  if (this.css_prefix) {
    if (css === this.css_prefix) {
      css = '';
    }
    else {
      // Make sure it includes the separator.
      var prefix = css.substring(0, this.css_prefix.length + 1);
      if (prefix === this.css_prefix + '-') {
        css = css.substring(prefix.length);
      }
    }
  }
  // Remove redundant "-region" suffixes.
  var regionRegExp = new RegExp("-region$");
  css = css.replace(regionRegExp, '');
  return css;
};

/**
 * Filters out the classes in a blacklist so they will not be selectable
 * by the user.
 */
ThemeBuilder.styles.PathElement.prototype._filterClasses = function () {
  if (!ThemeBuilder.styles.PathElement.classBlackList) {
    ThemeBuilder.styles.PathElement.classBlackList = [
      'style-clickable', 'overlay-processed', 'selected', 'selection'
    ];
  }
  if (!ThemeBuilder.styles.PathElement.classGreyList) {
    // The grey list represents classes that should never be selected
    // by default, though should be made available to the user.
    ThemeBuilder.styles.PathElement.classGreyList = [
      'first', 'last', 'leaf', 'area', 'tb-region'
    ];
  }
  if (!this.classes) {
    return;
  }
  var classes = [];
  var greyClasses = [];
  var len = this.classes.length;
  for (var i = 0; i < len; i++) {
    if (!ThemeBuilder.styles.PathElement.classBlackList.contains(this.classes[i])) {
      if (!ThemeBuilder.styles.PathElement.classGreyList.contains(this.classes[i])) {
        classes.push(this.classes[i]);
      }
      else {
        greyClasses.push(this.classes[i]);
      }
    }
  }
  this.classes = classes;
  this.greyClasses = greyClasses;
};

/**
 * Returns the xhtml tag name associated with this PathElement instance.
 * The tag name will always be lower case.
 *
 * @return
 *   A string containing the tag name associated with this path element.
 */
ThemeBuilder.styles.PathElement.prototype.getTag = function () {
  return this.tag;
};

/**
 * Returns the id associated with this element.
 *
 * @return
 *   A string containing the element id, or undefined if the id is not
 *   present.
 */
ThemeBuilder.styles.PathElement.prototype.getId = function () {
  return this.id;
};

/**
 * Returns all of the primary classes associated with this path element.
 * The primary classes do not include the grey classes.
 *
 * @return
 *   An array containing the associated classes, or undefined if no
 *   non-grey css classes are associated with this PathElement
 *   instance.
 */
ThemeBuilder.styles.PathElement.prototype.getClasses = function () {
  var classes = undefined;
  if (this.classes) {
    classes = this.classes.slice();
  }
  return classes;
};

/**
 * Returns all classes associated with this element that are selectable
 * by the user.  This list includes the primary classes and the grey
 * classes.  The grey classes are those that are selectable by the
 * user but would never be automatically selected when the user chooses
 * an element.
 *
 * @return
 *   An array containing the associated classes, or undefined if no
 *   css classes are associated with this PathElement instance.
 */
ThemeBuilder.styles.PathElement.prototype.getAllClasses = function () {
  var allClasses = undefined;
  if ((this.classes && this.classes.length > 0) ||
    (this.greyClasses && this.greyClasses.length > 0)) {
    allClasses = [].concat(this.classes, this.greyClasses);
  }
  return allClasses;
};

/**
 * Indicates whether this element has the specified CSS class associated with
 * it.
 *
 * @param {String} classname
 *   The CSS class to query for.
 * @return
 *   true if this element has the specified classname; false otherwise.
 */
ThemeBuilder.styles.PathElement.prototype.hasClass = function (classname) {
  var allClasses = this.getAllClasses() || [];
  return allClasses.contains(classname);
};

/**
 * Returns only the enabled classes associated with this path element
 * instance.
 *
 * @return
 *   An array containing the associated enabled classes, or undefined if
 *   no css classes are associated with this PathElement instance.
 */
ThemeBuilder.styles.PathElement.prototype.getEnabledClasses = function () {
  var classes = this.getAllClasses();
  var enabledClasses = [];
  if (classes) {
    for (var i = 0; i < classes.length; i++) {
      if (this.classes_enabled[classes[i]] === true) {
        enabledClasses.push(classes[i]);
      }
    }
  }
  if (enabledClasses.length === 0) {
    enabledClasses = undefined;
  }
  return enabledClasses;
};

/**
 * Returns all pseudoclasses associated with this element that are selectable
 * by the user.  
 *
 * @return
 *   An array containing the associated pseudoclasses, or undefined if no css
 *   pseudoclasses are associated with this PathElement instance.
 */
ThemeBuilder.styles.PathElement.prototype.getAllPseudoClasses = function () {
  var allClasses = undefined;
  if (this.pseudoclasses && this.pseudoclasses.length > 0) {
    allClasses = [].concat(this.pseudoclasses);
  }
  return allClasses;
};

/**
 * Returns only the enabled pseudoclasses associated with this path element
 * instance.
 *
 * @return
 *   An array containing the associated enabled pseudoclasses, or undefined if
 *   no css pseudoclasses are associated with this PathElement instance.
 */
ThemeBuilder.styles.PathElement.prototype.getEnabledPseudoClasses = function () {
  var pseudoclasses = this.getAllPseudoClasses();
  var enabledClasses = [];
  if (pseudoclasses) {
    for (var i = 0; i < pseudoclasses.length; i++) {
      if (this.pseudoclasses_enabled[pseudoclasses[i]] === true) {
        enabledClasses.push(pseudoclasses[i]);
      }
    }
  }
  if (enabledClasses.length === 0) {
    enabledClasses = undefined;
  }
  return enabledClasses;
};

/**
 * Flags this element to be included or omitted from the css selector.
 *
 * @param {boolean} enabled
 *   If true, this element will be included in the css selector; otherwise
 *   it will be omitted.
 */
ThemeBuilder.styles.PathElement.prototype.setEnabled = function (enabled) {
  this.enabled = (enabled === true);
};

/**
 * Gets the enabled flag associated with this element.
 *
 * @return {boolean}
 *   True if this element is enabled; false otherwise.
 */
ThemeBuilder.styles.PathElement.prototype.getEnabled = function () {
  return (this.enabled);
};

/**
 * Flags the element tag to be included or omitted from the css selector.
 *
 * @param {boolean} enabled
 *   If true, the element tag associated with this PathElement instance
 *   will be included in the css selector; otherwise it will be omitted.
 */
ThemeBuilder.styles.PathElement.prototype.setTagEnabled = function (enabled) {
  this.tag_enabled = (enabled === true);
};

/**
 * Flags the element id to be included or omitted from the css
 * selector.
 *
 * @param {boolean} enabled
 *   If true, the id will be included in the css selector; otherwise it
 *   will be omitted.
 */
ThemeBuilder.styles.PathElement.prototype.setIdEnabled = function (enabled) {
  this.id_enabled = (enabled === true);
};

/**
 * Indicates whether the id will be included or omitted from the css selector.
 *
 * @return
 *   true if the id for this element will be included in the selector; false
 *   otherwise.
 */
ThemeBuilder.styles.PathElement.prototype.isIdEnabled = function () {
  return this.id_enabled;
};

/**
 * Flags the element class(es) to be included or omitted from the css
 * selector.
 *
 * @param {mixed} classes
 *   A string or array of strings that identify the class(es) to include or
 *   omit from the final css selector.
 * @param {boolean} enabled
 *   A boolean value that indicates whether the specified class(es) should
 *   be enabled or not.
 */
ThemeBuilder.styles.PathElement.prototype.setClassEnabled = function (classes, enabled) {
  var i;
  var allClasses = [].concat(this.classes, this.greyClasses);
  if (!classes || allClasses.length <= 0) {
    return;
  }
  if (typeof classes === 'string') {
    // Dealing with a single string representing a classname.
    if (this.classes_enabled[classes] === true ||
  this.classes_enabled[classes] === false) {
      this.classes_enabled[classes] = (enabled === true);
    }
  }
  else if (typeof classes === 'object' && classes instanceof Array) {
    // Dealing with an array of strings representing classnames.
    for (i = 0; i < classes.length; i++) {
      if (typeof classes[i] === 'string') {
        this.classes_enabled[classes[i]] = (enabled === true);
      }
    }
  }
};

/**
 * Flags the element pseudoclass(es) to be included or omitted from the css
 * selector.
 *
 * @param {mixed} pseudoclasses
 *   A string or array of strings that identify the pseudoclass(es) to include
 *   or omit from the final css selector.
 * @param {boolean} enabled
 *   A boolean value that indicates whether the specified pseudoclass(es)
 *   should be enabled or not.
 */
ThemeBuilder.styles.PathElement.prototype.setPseudoClassEnabled = function (pseudoclasses, enabled) {
  var i;
  if (!pseudoclasses || pseudoclasses.length <= 0) {
    return;
  }
  var allClasses = [].concat(this.pseudoclasses);
  if (typeof pseudoclasses === 'string') {
    // Dealing with a single string representing a classname.
    if (this.pseudoclasses_enabled[pseudoclasses] === true ||
	this.pseudoclasses_enabled[pseudoclasses] === false) {
      this.pseudoclasses_enabled[pseudoclasses] = (enabled === true);
    }
  }
  else if (typeof pseudoclasses === 'object' && pseudoclasses instanceof Array) {
    // Dealing with an array of strings representing classnames.
    for (i = 0; i < pseudoclasses.length; i++) {
      if (typeof pseudoclasses[i] === 'string') {
        this.pseudoclasses_enabled[pseudoclasses[i]] = (enabled === true);
      }
    }
  }
};

/**
 * Causes all of the classes associated with this element to be disabled.
 */
ThemeBuilder.styles.PathElement.prototype.disableAllClasses = function () {
  this.setClassEnabled(this.getAllClasses(), false);
};

/**
 * Causes all of the classes associated with this element with the exception
 * of the specified classname to be disabled.
 *
 * @param {string} classname
 *   The name of the class to enable; the remaining classes will be disabled.
 */
ThemeBuilder.styles.PathElement.prototype.disableAllButOneClass = function (classname) {
  this.disableAllClasses();
  if (classname) {
    this.setClassEnabled(classname, true);
  }
  else {
    var classes = this.getClasses();
    if (classes && classes.length > 0) {
      this.setClassEnabled(classes[0], true);
    }
  }
};

/**
 * Returns the full css selector that can be used to reference the
 * element associated with this PathElement instance.  All of the identifying
 * attributes are included regardless of whether they are enabled.
 *
 * @return
 *   A string representing a part of a CSS selector string that identifies
 *   the element associated with this PathElement instance.
 */
ThemeBuilder.styles.PathElement.prototype.getFullCssSelector = function () {
  var selector = this.getTag();
  var id = this.getId();
  if (id) {
    selector += '#' + id;
  }
  var classes = this.getAllClasses();
  if (classes) {
    selector += '.' + classes.join('.');
  }
  return selector;
};

/**
 * Returns a css selector that reflects the state of this element as it is
 * currently configured.  Only the parts of the element that are enabled will
 * be reflected in the resulting css selector.
 *
 * @return {string}
 *   A string representing a part of a CSS selector string that identifies
 *   the element associated with this PathElement instance as it is currently
 *   configured.
 */
ThemeBuilder.styles.PathElement.prototype.getCssSelector = function () {
  var selector = '';
  if (this.tag_enabled && this.tag) {
    selector += this.getTag();
  }
  if (this.id_enabled && this.id) {
    selector += '#' + this.id;
  }
  var classes = this.getEnabledClasses();
  if (classes) {
    selector += '.' + classes.join('.');
  }
  var pseudoclasses = this.getEnabledPseudoClasses();
  if (pseudoclasses) {
    selector += ':' + pseudoclasses.join(':');
  }
  return selector;
};

/**
 * Returns the css selector that can be used to reference the
 * element associated with this PathElement instance.  Only the enabled
 * attributes of the associated element will be included in the resulting
 * css selector.
 *
 * @return
 *   A string representing a part of a CSS selector string that identifies
 *   the element associated with this PathElement instance.
 */
ThemeBuilder.styles.PathElement.prototype.getEnabledCssSelector = function () {
  var selector = '';
  if (this.enabled === true) {
    selector += this.getCssSelector();
  }
  return selector;
};

/**
 * Returns the tag name associated with this css element.  The tag name is
 * returned as a human readable string, suitable for display in a ui.
 *
 * @return {string}
 *   The tag name.
 */
ThemeBuilder.styles.PathElement.prototype.getTagName = function () {
  if (!this.tagNameMap) {
    this.tagNameMap = {
      'a': Drupal.t('links'),
      'abbr': Drupal.t('abbreviations'),
      'acronym': Drupal.t('acronyms'),
      'address': Drupal.t('addresses'),
      'applet': Drupal.t('applets'),
      'area': Drupal.t('image map areas'),
      'b': Drupal.t('bold text'),
      'big': Drupal.t('big font elements'),
      'blockquote': Drupal.t('block quotes'),
      'body': Drupal.t('site background'),
      'br': Drupal.t('line breaks'),
      'button': Drupal.t('buttons'),
      'caption': Drupal.t('captions'),
      'center': Drupal.t('centered text'),
      'cite': Drupal.t('citations'),
      'code': Drupal.t('code listings'),
      'col': Drupal.t('table columns'),
      'colgroup': Drupal.t('table column groups'),
      'dd': Drupal.t('definitions'),
      'del': Drupal.t('deleted text'),
      'dfn': Drupal.t('definitions'),
      'dir': Drupal.t('directory titles'),
      'div': Drupal.t('divisions'),
      'dl': Drupal.t('definition lists'),
      'dt': Drupal.t('definition terms'),
      'em': Drupal.t('emphasized text'),
      'fieldset': Drupal.t('fieldsets'),
      'font': Drupal.t('font tags'),
      'form': Drupal.t('forms'),
      'frame': Drupal.t('frames'),
      'frameset': Drupal.t('framesets'),
      'h1': Drupal.t('primary headers'),
      'h2': Drupal.t('secondary headers'),
      'h3': Drupal.t('tertiary headers'),
      'h4': Drupal.t('header 4s'),
      'h5': Drupal.t('header 5s'),
      'h6': Drupal.t('header 6s'),
      'hr': Drupal.t('horizontal lines'),
      'i': Drupal.t('italicized text'),
      'iframe': Drupal.t('inline frames'),
      'img': Drupal.t('images'),
      'input': Drupal.t('input fields'),
      'ins': Drupal.t('inserted text'),
      'kbd': Drupal.t('user typed text'),
      'label': Drupal.t('input labels'),
      'legend': Drupal.t('legends'),
      'li': Drupal.t('list items'),
      'object': Drupal.t('objects'),
      'ol': Drupal.t('ordered lists'),
      'option': Drupal.t('options'),
      'p': Drupal.t('paragraphs'),
      'pre': Drupal.t('preformated sections'),
      'q': Drupal.t('quoted text'),
      's': Drupal.t('strikethrough text'),
      'samp': Drupal.t('sample code'),
      'select': Drupal.t('select lists'),
      'small': Drupal.t('small text'),
      'span': Drupal.t('spans'),
      'strike': Drupal.t('strikethrough text'),
      'strong': Drupal.t('emphasized text'),
      'sub': Drupal.t('subscript text'),
      'sup': Drupal.t('superscript text'),
      'table': Drupal.t('tables'),
      'tbody': Drupal.t('table bodies'),
      'td': Drupal.t('table data'),
      'textarea': Drupal.t('text areas'),
      'tfoot': Drupal.t('table footers'),
      'th': Drupal.t('table column headings'),
      'thead': Drupal.t('table headers'),
      'title': Drupal.t('titles'),
      'tr': Drupal.t('table rows'),
      'tt': Drupal.t('teletype text'),
      'u': Drupal.t('underlined text'),
      'ul': Drupal.t('unordered lists'),
      'var': Drupal.t('variable text')
    };
  }
  var tag = this.getTag();
  var tagName = this.tagNameMap[tag];
  if (!tagName) {
    tagName = tag;
  }
  return tagName;
};

/**
 * Builds a specificity map for this elemnent.  The specificity map is
 * effectively an ordered list of selector options that describe this
 * element using its id, classes, and tag.  The map is used to provide
 * options to the user for setting the specificity of the css selector
 * associated with this element.
 *
 * @return {array}
 *   An array of objects that comprise the map.  Each object contains
 *   a name field which contains the css string and a use field which indicates
 *   what part of the element would be used for that specificity
 *   selection (id, class, tag).
 */
ThemeBuilder.styles.PathElement.prototype.buildSpecificityMap = function () {
  if (!this.specificityMap) {
    this.specificityMap = {'identification': [], 'pseudoclass': []};
    var id = this.getId();
    if (id) {
      this.specificityMap.identification.push({'name': '#' + id, 'use': 'id'});
    }
    var classes = this.getClasses();
    if (classes) {
      for (var i = 0; i < classes.length; i++) {
        this.specificityMap.identification.push({
          'name': '.' + classes[i],
          'use': 'class',
          'classname': classes[i]
        });
      }
    }
    if (this.tag) {
      this.specificityMap.identification.push({'name': this.getTag(), 'use': 'tag'});
    }
    var greyClasses = this.greyClasses;
    if (greyClasses) {
      for (i = 0; i < greyClasses.length; i++) {
        this.specificityMap.identification.push({
          'name': '.' + greyClasses[i],
          'use': 'class',
          'classname': greyClasses[i]
        });
      }
    }
    var pseudoclasses = this.pseudoclasses;
    if (pseudoclasses && pseudoclasses.length > 1) {
      var pseudoclassMap = this.specificityMap.pseudoclass;
      for (i = 0; i < pseudoclasses.length; i++) {
        this.specificityMap.pseudoclass.push({
          'name': (i === 0 ? pseudoclasses[i] : ':' + pseudoclasses[i]),
          'use': 'pseudoclass',
          'classname': (i === 0 ? '' : pseudoclasses[i])
        });
      }
    }
  }
};

/**
 * Returns an array of specificity options available for this element.
 * In addition to the fields provided by the buildSpecificityMap method,
 * this method adds a field that indicates whether the option is selected
 * or not.
 *
 * @return {array}
 *   An array of specificity options available for this element.
 */
ThemeBuilder.styles.PathElement.prototype.getSpecificityOptions = function () {
  this.buildSpecificityMap();
  var result = {'identification': [], 'pseudoclass': []};
  var types = ['identification', 'pseudoclass'];
  var currentSelector = this.getCssSelector();
  var enabledPseudoclasses = this.getEnabledPseudoClasses();
  if (enabledPseudoclasses && enabledPseudoclasses.length > 0) {
    var pseudoclass = enabledPseudoclasses[0];
  }
  for (var typeIndex = 0; typeIndex < types.length; typeIndex++) {
    var type = types[typeIndex];
    if (type === 'identification') {
      // Need to remove the pseudoclass for this comparison.
      currentSelector = ThemeBuilder.util.removePseudoClasses(currentSelector);
    }
    else {
      if (!pseudoclass) {
        currentSelector = Drupal.t('none');
      }
      else {
        currentSelector = ':' + pseudoclass;
      }
    }
    for (var i = 0; i < this.specificityMap[type].length; i++) {
      var option = ThemeBuilder.clone(this.specificityMap[type][i]);
      if (option.classname === 'tb-region' || option.classname === 'area') {
        // These classes should be blacklisted but we need them for
        // identifying the region.  Simply don't add them to the specificity
        // option panel.
        continue;
      }
      if (currentSelector === option.name) {
        option.selected = true;
      }
      result[type].push(option);
    }
  }
  return result;
};

/**
 * Selects the specificity option corresponding to the specified option
 * index.
 *
 * @param {int} sindex
 *   The specificity index corresponding to the specificity option that
 *   should be selected.
 */
ThemeBuilder.styles.PathElement.prototype.setSpecificity = function (type, sindex) {
  this.buildSpecificityMap();
  if (sindex >= 0 && sindex < this.specificityMap[type].length) {
    var entry = this.specificityMap[type][sindex];
    switch (entry.use) {
    case 'id':
      this.setIdEnabled(true);
      this.disableAllClasses();
      this.setTagEnabled(false);
      break;
    case 'class':
      this.setIdEnabled(false);
      this.disableAllButOneClass(entry.classname);
      this.setTagEnabled(false);
      break;      
    case 'tag':
      this.setIdEnabled(false);
      this.disableAllClasses();
      this.setTagEnabled(true);
      break;
    case 'pseudoclass':
      this.setPseudoClassEnabled(this.getAllPseudoClasses(), false);
      this.setPseudoClassEnabled(entry.classname, true);
    }
  }
};

/**
 * The Filter class is responsible for filtering path elements within a
 * selector.  This is an example filter which is used for testing purposes.
 * @class
 */
ThemeBuilder.styles.Filter = ThemeBuilder.initClass();

/**
 * The constructor of the Filter class.
 */
ThemeBuilder.styles.Filter.prototype.initialize = function () {
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
ThemeBuilder.styles.Filter.prototype.activate = function (path) {
  var newPath = [];
  for (var i = 0; i < path.length; i++) {
    var pathelement = ThemeBuilder.clone(path[i]);
    this.filter(pathelement);
    newPath.push(pathelement);
  }
  return newPath;
};

/**
 * Filter out naked divs - elements of type 'div' that do not have
 * identifying attributes.
 *
 * @param {PathElement} pathElement
 *   The PathElement instance to filter.  If the path element is a div
 *   and has no id or classes, it will be disabled as a result of calling
 *   the filter method.
 */
ThemeBuilder.styles.Filter.prototype.filter = function (pathElement) {
  if (pathElement.getTag() === 'div') {
    if (!pathElement.getId() && !pathElement.getClasses()) {
      pathElement.setEnabled(false);
    }
  }
};