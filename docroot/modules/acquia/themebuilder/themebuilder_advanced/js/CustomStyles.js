/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true window: true*/

var ThemeBuilder = ThemeBuilder || {};

/**
 * The CustomStyleManager class is responsible for requesting and managing the update of custom styles.
 * @class
 */
ThemeBuilder.CustomStyleManager = ThemeBuilder.initClass();

/**
 * Returns the CustomStyleManager instance.  If the instance has not
 * been created yet, it will be created as a result of this call.
 *
 * @return {CustomStyleManager}
 *   The CustomStyleManager instance.
 */
ThemeBuilder.CustomStyleManager.getInstance = function () {
  if (!ThemeBuilder.CustomStyleManager._instance) {
    ThemeBuilder.CustomStyleManager._instance = new ThemeBuilder.CustomStyleManager();
  }
  return ThemeBuilder.CustomStyleManager._instance;
};

/**
 * Constructor for the CustomStyleManager class.
 *
 * @private
 */
ThemeBuilder.CustomStyleManager.prototype.initialize = function () {
  var $ = jQuery;
  $(window).bind('ModificationCommitted', ThemeBuilder.bind(this, this._commit));
};

/**
 * Requests all custom styles from the server.
 *
 * These styles cannot be taken from the stylesheet alone because the
 * styles for properties dealing with color all appear in the palette
 * stylesheet only, which is a mixture of custom styles and styles
 * from the base theme.
 *
 * When the styles are retrieved, interested parties will be notified
 * by sending a 'css-history-contents-changed' event, which is
 * attached to the themebuilder wrapper element.
 */
ThemeBuilder.CustomStyleManager.prototype.requestCustomStyles = function () {
  if (Drupal.settings.themebuilderGetCustomCss) {
    var history = ThemeBuilder.History.getInstance();
    ThemeBuilder.postBack(Drupal.settings.themebuilderGetCustomCss, {}, ThemeBuilder.bind(this, this._cssDataReceived));
  }
};

/**
 * Called when the custom style data is received from the server.
 *
 * @private
 * @param {Object} data
 *   An object containing selectors mapped to objects representing all
 *   of the properties and values associated with those selectors.
 */
ThemeBuilder.CustomStyleManager.prototype._cssDataReceived = function (data) {
  this.styles = new ThemeBuilder.CustomStyles(data);
  var $ = jQuery;
  $('#themebuilder-wrapper').trigger('css-history-contents-changed', this.styles);
};


/**
 * This method is called when a modification is committed.
 *
 * When this method is called, the specified modification is
 * integrated into the Css history data, and the changes are pushed to
 * interested listeners (such as the History UI).
 *
 * @private
 * @param {Event} event
 *   The event associated with the commit.  This event generally is not used.
 * @param {Modification} modification
 *   The modification that was committed.
 * @param {String} operation
 *   One of 'apply', 'redo', or 'undo', which indicates whether the
 *   modification is being applied or reverted.
 */
ThemeBuilder.CustomStyleManager.prototype._commit = function (event, modification, operation) {
  var data = this.styles.getData();
  switch (operation) {
  case 'apply':
  case 'redo':
    if (modification.getType() === ThemeBuilder.CssModification.TYPE ||
       modification.getType() === ThemeBuilder.GroupedModification.TYPE) {
      this._applyModificationState(modification.getNewState(), data);
    }
    break;
  case 'undo':
    if (modification.getType() === ThemeBuilder.CssModification.TYPE ||
       modification.getType() === ThemeBuilder.GroupedModification.TYPE) {
      this._applyModificationState(modification.getPriorState(), data);
    }
    break;
  }
  setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this, this._cssDataReceived, data), 100);
};

/**
 * Applies the specified modification state (either next or prev) to the specified data set.
 *
 * This method is responsible for updating the data to reflect the
 * modification that was just applied.
 *
 * @private
 * @param {Object} state
 *   The modification state to apply to the specified data set.
 * @param {Object} data
 *   The data set to apply the modification state to.
 * @return {Object}
 *   The updated data.
 */
ThemeBuilder.CustomStyleManager.prototype._applyModificationState = function (state, data) {
  if (state instanceof Array) {
    // The state could be an array of the original modification was of
    // type GroupedModification.
    for (var i = 0, len = state.length; i < len; i++) {
      data = this._applyModificationState(state[i], data);
    }
  }
  else {
    // This is a single modification.
    if (state.value) {
      // Ensure there is an object associated with the selector.
      if (!data[state.selector]) {
        data[state.selector] = {};
      }
      data[state.selector][state.property] = state.value;
    }
    else {
      // The state indicates the property should be entirely removed
      // from custom styling.
      if (data[state.selector]) {
        delete data[state.selector][state.property];
      }

      // If the selector has no properties associated with it, remove
      // the selector from the data also.
      var count = 0;
      for (var property in data[state.selector]) {
        if (data[state.selector].hasOwnProperty(property)) {
          count++;
	  // No point in processing all of them; one is enough to keep
	  // the selector.
          break;
        }
      }
      if (count === 0) {
        // All of the properties have been deleted.
        delete data[state.selector];
      }
    }
  }
  return data;
};

/**
 * The CustomStyles class is responsible for containing raw style data and exposing it in a useful manner.
 *
 * One of the most interesting responsibilities of this class is to
 * provide the data in a useful order.
 * @class
 */
ThemeBuilder.CustomStyles = ThemeBuilder.initClass();

/**
 * The constructor.
 *
 * @private
 * @param {Object} data
 *   The object containing the raw custom style data.
 */
ThemeBuilder.CustomStyles.prototype.initialize = function (data) {
  this.setData(data);
};

/**
 * Sets the data into this CustomStyles instance.
 *
 * Sorts the data and adjusts the internal state of this instance to
 * correspond to the new data set.
 *
 * @param {Object} data
 *   the object containing the raw custom style data.
 */
ThemeBuilder.CustomStyles.prototype.setData = function (data) {
  var scoreMap = {};
  var selectors = [];
  for (var selector in data) {
    if (data.hasOwnProperty(selector)) {
      scoreMap[selector] = ThemeBuilder.Specificity.getScore(selector);
      selectors.push(selector);
    }
  }
  this.data = data;
  this.selectors = selectors;
  this.scoreMap = scoreMap;

  this.selectors.sort(ThemeBuilder.bind(this, this.selectorSort));
};

/**
 * Returns the internal data used by this CustomStyles instance.
 *
 * @return {Object}
 *   The raw data.
 */
ThemeBuilder.CustomStyles.prototype.getData = function () {
  return this.data;
};

/**
 * Comparison function used for sorting selectors.
 *
 * The selectors must have been populated into this instance before
 * using this function.  The specified selectors are not actually
 * compared, but rather their specificity scores are compared to
 * determine the appropriate order.
 *
 * @param {String} a
 *   The first selector.
 * @param {String} b
 *   The second selector.
 */
ThemeBuilder.CustomStyles.prototype.selectorSort = function (a, b) {
  var result = String(this.scoreMap[a]).localeCompare(this.scoreMap[b]);
  if (result === 0) {
    if (this.scoreMap[a].toString() === '000,000,000,001') {
      // They have the same specificity and the selector represents an
      // element or a pseudoelement.
      result = this.getElementRank(a) - this.getElementRank(b);
    }
    if (result === 0) {
      // They have the same specificity.  Sort by length instead (shortest first).
      result = a.length - b.length;
      if (result === 0) {
        // They have the same length.  Sort in alphabetical order instead.
        result = String(a).localeCompare(b);
      }
    }
  }
  return result;
};

ThemeBuilder.CustomStyles._elementRanks = [
  // Document
  'html',
  'body',

  // Block
  'p',
  'h1',
  'h2',
  'h3',
  'h4',
  'h5',
  'h6',
  'ul',
  'ol',
  'li',
  'dl',
  'dt',
  'dd',
  'address',
  'blockquote',
  'del',
  'hr',
  'pre',
  'ins',
  'noscript',
  'center',
  'table',
  'legend',
  'thead',
  'tfoot',
  'col',
  'colgroup',
  'th',
  'tbody',
  'tr',
  'td',
  'caption',
  'form',
  'fieldset',
  'textarea',

  // Inline
  'menu',
  'input',
  'select',
  'option',
  'optgroup',
  'button',
  'label',
  'img',
  'a',
  'span',
  'abbr',
  'acronym',
  'cite',
  'em',
  'i',
  'strong',
  'b',
  'sub',
  'sup',
  'small',
  'big',
  'strike',
  'q',
  'var',
  'samp',
  'code',
  'br',
  'font'
];

/**
 * Returns the rank of the specified element.
 *
 * This is used to sort selectors that have a specificity of
 * 000,000,000,001 such that the most general elements appear near the
 * top and more specific ones appear later.
 *
 * @param {String} element
 *   The selector describing an element.
 * @return {int}
 *   The rank.
 */
ThemeBuilder.CustomStyles.prototype.getElementRank = function (element) {
  var result;
  if (!ThemeBuilder.CustomStyles._elementRankMap) {
    // Build a map that makes it efficient to rank the elements.
    var rank = 0;
    ThemeBuilder.CustomStyles._elementRankMap = {};
    for (var i = 0, len = ThemeBuilder.CustomStyles._elementRanks.length; i < len; i++) {
      ThemeBuilder.CustomStyles._elementRankMap[ThemeBuilder.CustomStyles._elementRanks[i]] = rank++;
    }
  }
  result = ThemeBuilder.CustomStyles._elementRankMap[element];
  if (undefined === result) {
    result = ThemeBuilder.CustomStyles._elementRankMap.length + 1;
  }
  return result;
};

/**
 * Returns an iterator that can be used to visit each of the selectors in the custom CSS data in either ascending or descending order.
 *
 * @param {Boolean} asc
 *   Optional argument that indicates whether the sort should be
 *   ascending (true) or descending (false).  Ascending sort is
 *   done by default.
 * @return {Iterator}
 *   An iterator that traverses over the set of selectors.
 */
ThemeBuilder.CustomStyles.prototype.getIterator = function (asc) {
  return new ThemeBuilder.CustomStyleSelectorIterator(this, asc);
};

/**
 * Returns an iterator that can be used to visit each of the properties of the specified selector.
 *
 * @param {Boolean} selector
 *   The selector
 * @return {Iterator}
 *   An iterator that traverses over the set of properties.
 */
ThemeBuilder.CustomStyles.prototype.getPropertyIterator = function (selector) {
  return new ThemeBuilder.PropertyIterator(this.data[selector]);
};

/**
 * The CustomStyleSelectorIterator iterates over a set of selectors.
 * @class
 */
ThemeBuilder.CustomStyleSelectorIterator = ThemeBuilder.initClass();

/**
 * Constructor for the CustomStyleSelectorIterator.
 *
 * The iterator traverses over the set of selectors in ascending or
 * descending order based on the selector's specificity.
 *
 * @param {CustomStyles} styles
 *   The CustomStyles instance this iterator uses as data.
 * @param {Boolean} asc
 *   If true, the selectors will be traversed in ascending order.
 *   Otherwise the traversal will be in descending order.
 */
ThemeBuilder.CustomStyleSelectorIterator.prototype.initialize = function (styles, asc) {
  if (true !== asc && false !== asc) {
    asc = true;
  }
  this.asc = asc;
  this.styles = styles;
  this.index = (this.asc ? 0 : styles.selectors.length - 1);
};

/**
 * Indicates whether there is another selector in the set.
 *
 * @return {Boolean}
 *   true if there is another element; false otherwise.
 */
ThemeBuilder.CustomStyleSelectorIterator.prototype.hasNext = function () {
  return this.index >= 0 && this.index < this.styles.selectors.length;
};

/**
 * Returns the next selector in the list.
 *
 * @return {Object}
 *   The next selector and its associated properties.  This is in the
 *   form of an object that contains fields for the selector {String}
 *   and the properties {iterator}.
 */
ThemeBuilder.CustomStyleSelectorIterator.prototype.next = function () {
  if (this.hasNext()) {
    var selector = this.styles.selectors[this.index];
    var properties = this.styles.data[selector];
    this.index = this.asc ? this.index + 1 : this.index - 1;

    return {selector: selector, properties: new ThemeBuilder.PropertyIterator(properties)};
  }
  return null;
};

/**
 * The PropertyIterator class is responsible for iterating over the set of properties associated with a single CSS selector.
 * @class
 */
ThemeBuilder.PropertyIterator = ThemeBuilder.initClass();

/**
 * Constructor for the PropertyIterator class.
 *
 * @param {Object} properties
 *   Raw property data associated with a particular CSS selector.
 */
ThemeBuilder.PropertyIterator.prototype.initialize = function (propertyData) {
  this.propertyData = propertyData;
  this.properties = [];
  for (var property in this.propertyData) {
    if (this.propertyData.hasOwnProperty(property)) {
      this.properties.push(property);
    }
  }
  this.properties.sort();
  this.index = 0;
};

/**
 * Indicates whether there is another property in the set.
 *
 * @return {Boolean}
 *   true if there is another element; false otherwise.
 */
ThemeBuilder.PropertyIterator.prototype.hasNext = function () {
  var hasNext = this.index >= 0 && this.index < this.properties.length;
  return hasNext;
};

/**
 * Returns the next property in the list.
 *
 * @return {Object}
 *   The next property name and value.  This is in the
 *   form of an object that contains fields for the name {String}
 *   and the value {String}.
 */
ThemeBuilder.PropertyIterator.prototype.next = function () {
  if (this.hasNext()) {
    var name = this.properties[this.index];
    this.index++;
    return {name: name, value: this.propertyData[name]};
  }
  return null;
};

/**
 * The Specificity object has static methods that calculate the specificity value for a given selector.
 */
ThemeBuilder.Specificity = {};

/**
 * Calculates the specificity score for the specified selector.
 *
 * This score is returned in object form, but has a toString method
 * that will provide a suitable form with which specificities can be
 * compared with a simple string sort.  The object also has a compare
 * method that allows you to easily compare the value with another
 * specificity score.
 *
 * @param {String} selector
 *   The CSS selector to calculate the specificity score from.
 * @return {SpecificityScore}
 *   An object containing specificity components a (style from the
 *   element), b (number of ids in the selector), c (the number of
 *   classes in the selector, including pseudoclasses), and d (the
 *   number of elements in the selector, including pseudoelements).
 *   This object also contains a toString method that provides the
 *   data in an easily comparable string format.
 */
ThemeBuilder.Specificity.getScore = function (selector) {
  var score = new ThemeBuilder.SpecificityScore(0, 0, 0, 0);
  /* @see AN-16195 - selector occassionally arrives set to an empty string. */
  if (selector) {
    var chunks = ThemeBuilder.Specificity._getSelectorChunks(selector.toLowerCase());
    for (var i = 0, len = chunks.length; i < len; i++) {
      var chunk = chunks[i];
      ThemeBuilder.Specificity._addChunk(chunk, score);
    }
  }
  return score;
};

/**
 * Breaks the specified selector into selector chunks.
 *
 * For example, the selector 'h2 .active li' would return ['h2',
 * '.active', 'li'].  Another example shows a more complex selector:
 * 'h2 a#id.active>li' returns ['h2', 'a#id.active', 'li'].  This
 * breaks down the problem into discreet chunks that can be evaluated.
 * The total specificity score is simply the sum of its constituent
 * chunks.
 *
 * @private
 * @param {String} selector
 *   The CSS selector.
 * @return {Array}
 *   An array with each element containing the discreet parts of the
 *   selector.
 */
ThemeBuilder.Specificity._getSelectorChunks = function (selector) {
  return selector.match(new RegExp("([^\\s+>~]+)", 'g'));
};

/**
 * Calculates the specificity value for the specified chunk and then increments the specified specificity score accordingly.
 *
 * This is where all the magic happens.  The guts of this selector
 * chunk are parsed and the value determined.  This value is added to
 * the specified value, creating the running total.
 *
 * @private
 * @param {String} chunk
 *   The css chunk.
 * @param {SpecificityScore} score
 *   This object contains the cumulative specificity value for the
 *   entire selector.  Each selector chunk should be passed through
 *   this function and the total value of the selector is the sum of
 *   the values of all of the chunks.
 */
ThemeBuilder.Specificity._addChunk = function (chunk, score) {
  var name;

  for (var i = 0, len = chunk.length; i < len; i++) {
    switch (chunk.charAt(i)) {

    case '*':
      break;

    case '#':
      // Element id
      score.b++;
      break;

    case '.':
      // Element class
      score.c++;
      break;

    case '[':
      // Element attribute (counted as a class)
      var bIndex = chunk.indexOf(']', i);
      if (bIndex > i) {
        i = bIndex;
        score.c++;
      }
      break;

    case ':':
      // Pseudoelement or pseudoclass.
      if (chunk.substr(i, 2) === '::') {
        // This is a pseudoelement, not a pseudoclass.  Skipping past the
        // extra ':' because browsers must continue to support the
        // single colon form.
        i++;
      }
      name = ':';
      var n = i + 1;
      for (; n < len && chunk.charAt(n) !== '#' &&
             chunk.charAt(n) !== '.' && chunk.charAt(n) !== ':'; i++, n++) {
        name += chunk.charAt(n);
      }
      if (ThemeBuilder.Specificity._isPseudoClass(name)) {
        score.c++;
      }
      else if (ThemeBuilder.Specificity._isPseudoElement(name)) {
        score.d++;
      }
      break;

    default:
      // Element by tag name
      if (i === 0) {
        score.d++;
      }
    }
  }
};

/**
 * These are the pseudoclasses.
 *
 * There are actually a few more but we probably don't need them and
 * this simple parser wasn't written to handle the ones that contain
 * parenthesis, so they were omitted from this list.
 */
ThemeBuilder.Specificity._pseudoclasses = [
  ':link',
  ':visited',
  ':hover',
  ':active',
  ':focus',
  ':target',
  ':enabled',
  ':disabled',
  ':checked',
  ':indeterminate',
  ':root',
  ':first-child',
  ':last-child',
  ':first-of-type',
  ':last-of-type',
  ':only-child',
  ':empty'
];

/**
 * These are the pseudoelements.
 *
 * Pseudoelements are supposed to be prefixed with 2 colons, but
 * browsers must accept the single colon form also.  In the parser we
 * are detecting the second colon and throwing it away to make the
 * comparison simpler and to make it handle both cases.
 */
ThemeBuilder.Specificity._pseudoelements = [
  ':first-line',
  ':first-letter',
  ':first-child',
  ':before',
  ':after'
];

/**
 * Returns true if the specified class name represents a pseudoclass
 *
 * @private
 * @param {String} name
 *   A name, possibly representing a pseudoclass.
 * @return {boolean}
 *   true if the name represents a pseudoclass; false otherwise.
 */
ThemeBuilder.Specificity._isPseudoClass = function (name) {
  return ThemeBuilder.Specificity._pseudoclasses.contains(name);
};

/**
 * Returns true if the specified class name represents a pseudoelement
 *
 * @private
 * @param {String} name
 *   A name, possibly representing a pseudoelement.
 * @return {boolean}
 *   true if the name represents a pseudoelement; false otherwise.
 */
ThemeBuilder.Specificity._isPseudoElement = function (name) {
  return ThemeBuilder.Specificity._pseudoelements.contains(name);
};

/**
 * The SpecificityScore is an object used to hold and compare specificity values for a given CSS selector.
 * @class
 */
ThemeBuilder.SpecificityScore = ThemeBuilder.initClass();

/**
 * Initializes a new instance of SpecificityScore with the specified values.
 *
 * @param {int} a
 *   1 if the element has an inline style; 0 otherwise.
 * @param {int} b
 *   The count of ids in a particular selector.
 * @param {int} c
 *   The count of classes and pseudoclasses in a particular selector.
 * @param {int} d
 *   The count of elements and pseudoelements in a particular selector.
 */
ThemeBuilder.SpecificityScore.prototype.initialize = function (a, b, c, d) {
  a = (a === undefined ? 0 : a);
  b = (b === undefined ? 0 : b);
  c = (c === undefined ? 0 : c);
  d = (d === undefined ? 0 : d);
  this.a = a;
  this.b = b;
  this.c = c;
  this.d = d;
};

/**
 * Compares this score with the specified specificity score.
 *
 * @param {SpecificityScore} a
 *   The first specificity score.
 * @param {SpecificityScore} b
 *   The second specificity score.
 *
 * @return {int}
 *   An integer that reveals equality or the difference in weight of
 *   the two specificity values.  A result of 0 indicates equality.
 *   '1' indicates a is greater than b, and '-1' indicates b is
 *   greater than a.
 */
ThemeBuilder.SpecificityScore.prototype.compare = function (s) {
  var components = ['a', 'b', 'c', 'd'];
  var delta;
  for (var i = 0; i < components.length; i++) {
    delta = this[components[i]] - s[components[i]];
    if (delta !== 0) {
      return delta > 0 ? 1 : -1;
    }
  }
  return 0;
};

/**
 * Converts this specificity score to a string with a format that makes comparisons easy.
 *
 * This function is not to be called directly, but rather as part of
 */
ThemeBuilder.SpecificityScore.prototype.toString = function () {
  return this._padNumber(this.a, 3, '0') + ',' +
    this._padNumber(this.b, 3, '0') + ',' +
    this._padNumber(this.c, 3, '0') + ',' +
    this._padNumber(this.d, 3, '0');
};

/**
 * Pads the specified value to the specified length.
 *
 * @private
 * @param {mixed} value
 *   The value to pad.
 * @param {int} len
 *   The desired length of the value.
 * @param {String} pad
 *   The character used to pad the result.
 * @return {String}
 *   A string representation of the specified value padded to the
 *   specified length.
 */
ThemeBuilder.SpecificityScore.prototype._padNumber = function (value, len, pad) {
  var out = '' + value;
  while (out.length < len) {
    out = pad + out;
  }
  return out;
};
