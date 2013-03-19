/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

/**
 * @namespace
 */
ThemeBuilder.util = ThemeBuilder.util || {};

/**
 * Stops the specified event.  This is meant to be used as a callback for anything that generates events.
 *
 * usage:
 *  somecallBack = function(event){
 *    // do stuff
 *    return Themebuilder.util.stopEvent(event);
 *  };
 *
 * @param {event} event
 *   The event.
 */
ThemeBuilder.util.stopEvent = function (event) {
  if (event) {
    if (event.preventDefault) {
      event.preventDefault();
    }
    if (event.stopPropagation) {
      event.stopPropagation();
    }
  }
  return false;
};

/**
 * Capitalize the first letter in the specified string and force the rest to lowercase.
 *
 * @param {string} str
 *   The string to manipulate
 *
 * @return
 *   The modified string.
 */
ThemeBuilder.util.capitalize = function (str) {
  return str.charAt(0).toUpperCase() + str.substr(1).toLowerCase();
};

/**
 * Make a string into an acceptable CSS class name.
 *
 * @param {string} str
 *   The string to be munged.
 * @return {string}
 *   The class name.
 */
ThemeBuilder.util.getSafeClassName = function (str) {
  // Only alphanumeric, _, and - allowed.
  // TODO: Get rid of anything but an alpha character at the beginning.
  return str.toString().replace(new RegExp("[^a-zA-Z0-9_-]", 'g'), "-");
};

ThemeBuilder.util.themeLabelToName = function (label) {
  var separator = '_';
  var machine_name = label.toLowerCase().replace(/[^a-z0-9_]+/g, separator);

  if (machine_name.length === 0) {
    machine_name = 'untitled';
  }
  if (machine_name.length > 25) {
    machine_name = machine_name.substring(0, 25);
  }
  return 'acq_' + machine_name;
};

/**
 * Test whether an input is a valid number.
 *
 * Code adapted from http://stackoverflow.com/questions/18082/validate-numbers-in-javascript-isnumeric
 */
ThemeBuilder.util.isNumeric = function (input) {
  return !isNaN(Number(input)) && ('' + input).length > 0;
};

/**
 * Takes the specified value and returns an object that breaks it into its components.  For example, a value of "100px" will result in an object:
 *
 * result = {number: '100', units: 'px'};
 *
 * @param {String} value
 *   The css value.
 * @param {String} defaultValue
 *   The value to use if the specified value is undefined.
 * @return
 *   An object with fields for each value component.
 */
ThemeBuilder.util.parseCssValue = function (value, defaultValue) {
  var result = {};
  if (!value) {
    value = defaultValue;
  }
  var matches = value.match(/^\s*(-?\d*\.?\d*)?(\S*)?/);
  if (matches[1]) {
    result.number = matches[1];
    if (matches[2]) {
      result.units = matches[2];
    }
  }
  else {
    result.value = matches[2];
  }
  return result;
};

/**
 * Returns a selector that represents the specified css selector with all pseudoclasses that describe the element's state (as opposed to its identification) removed.
 *
 * For example, :hover, :link, :active, and :visited
 * would be removed.  This is helpful for displaying the selected elements
 * without requiring that those elements be in the configured state.
 *
 * @param {String} selector
 *   The css selector.
 * @return
 *   A string containing an equivalent css selector with all pseudoclasses
 *   that describe the element's state removed.
 */
ThemeBuilder.util.removeStatePseudoClasses = function (selector) {
  return selector.replace(/:+link|:+visited|:+hover|:+active/gi, '');
};

/**
 * Indicates whether the specified css selector contains one or more pseudoclasses that would describe an element's state.
 *
 * @param {String} selector
 *   The css selector.
 * @return
 *   true if the specified selector contains pseudoclasses that describe an
 *   element's state; false otherwise.
 */
ThemeBuilder.util.hasStatePseudoClasses = function (selector) {
  var result = selector.match(/(:+link|:+visited|:+hover|:+active)/i);
  return (result && result.length > 0);
};

/**
 * Returns a selector that represents the specified css selector with all pseudoclasses removed.
 *
 * @param {String} selector
 *   The css selector.
 * @return
 *   A string containing an equivalent css selector with all pseudoclasses *
 *   removed.
 */
ThemeBuilder.util.removePseudoClasses = function (selector) {
  return selector.replace(/:+\S*/g, '');
};

/**
 * Indicates whether the specified css selector contains one or more pseudoclasses.
 *
 * @param {String} selector
 *   The css selector.
 * @return
 *   true if the specified selector contains pseudoclasses; false otherwise.
 */
ThemeBuilder.util.hasPseudoClasses = function (selector) {
  var result = selector.match(/:+\S*/);
  return (result && result.length > 0);
};

/**
 * Returns the first pseudoclass from the specified css selector.  Only the name (not the colon) is returned.
 *
 * @param {String} selector
 *   The css selector.
 * @return
 *   The pseudoclass if present, otherwise ''.
 */
ThemeBuilder.util.getPseudoClass = function (selector) {
  var result = '';
  var matches = selector.match(/:+(\S*)/, '');
  if (matches && matches.length > 1) {
    result = matches[1];
  }
  return result;
};

/**
 * Returns the last child of element that is not of type TextNode.
 *
 * Note that many browsers convert '\n' characters in the markup to
 * TextNode elements, which makes the simple act of discovering the
 * last child a bit more * complex.
 *
 * @param {DomElement} element
 *   The element.
 * @return
 *   The last child of the specified element that is not a TextNode.  If the
 *   specified element does not have children, undefined is returned.
 */
ThemeBuilder.util.getLastChild = function (element) {
  var children = element.childNodes;
  var index = children.length - 1;
  var lastChild = children[index];
  while (lastChild.nodeType === 3 && index > 0) {
    lastChild = children[--index];
  }
  return lastChild;
};

/**
 * Determines whether the specified text is html markup or not.
 *
 * @param {String} text
 *   A string.
 * @return
 *   true if the string represents html markup; false otherwise.
 */
ThemeBuilder.util.isHtmlMarkup = function (text) {
  var result = false;
  // Simple test to determine whether this is html markup or perhaps
  // an exception that was thrown.
  text = ThemeBuilder.util.trim(text);
  if (text && text.length > 0 && text.indexOf('<') === 0 && text.charAt(text.length - 1) === '>') {
    result = true;
  }
  return result;
};

/**
 * Efficiently trims the whitespace from the beginning and end of the specified string.
 *
 * This function is more efficient than the typical trim function
 * because it avoids using a regular expression to do the work.
 * Regular expressions can be inefficient at detecting patterns at the
 * end of a string because typically the entire string is evaluated.
 *
 * This function is useful for triming large strings for which the
 * regular expression method would be significantly less efficient
 * than directly indexing into the string.
 *
 * @param {String} str
 *   The string to trim.
 * @return
 *   The trimmed string.
 */
ThemeBuilder.util.trim = function (str) {
  var whitespace = ' \n\r\t\f\x0b\xa0\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u200b\u2028\u2029\u3000';
  var begin = 0;
  var end = str.length;
  for (var i = 0; i < str.length; i++) {
    begin = i;
    if (whitespace.indexOf(str.charAt(i)) === -1) {
      break;
    }
  }
  for (i = str.length - 1; i >= begin - 1; i--) {
    end = i + 1;
    if (whitespace.indexOf(str.charAt(i)) === -1) {
      break;
    }
  }
  if (begin > 0 || end < str.length) {
    if (begin < end) {
      str = str.substring(begin, end);
    }
    else {
      str = '';
    }
  }
  return str;
};

/**
 * Takes two Unix timestamps (seconds since epoch)
 * and returns time difference in a nice form.
 *
 * @return {String}
 *   The time difference.
 */
ThemeBuilder.util.niceTime = function (timeCurrent, timePrevious) {
  // Set up some constants to make things look nice : these are all in terms of seconds. (duh.)
  var sec = 1;
  var min = sec * 60;
  var hour = min * 60;
  var day = hour * 24;
  var month = day * 30;

  // calculate the difference in time we'll be describing.
  var timeDelta = timeCurrent - timePrevious;
  var phrase = '';

  // Figure out what unit to describe the time difference in: seconds, minutes, hours, or days.
  if (timeDelta < min) {
    phrase = (timeDelta >= sec * 2) ? timeDelta + Drupal.t(' seconds ago') : Drupal.t('a second ago');
  } else if (timeDelta < hour) {
    phrase = (timeDelta >= min * 2) ? Math.floor(timeDelta / min) + Drupal.t(' minutes ago') : Drupal.t('a minute ago');
  } else if (timeDelta < day) {
    phrase = (timeDelta >= hour * 2) ? Math.floor(timeDelta / hour) + Drupal.t(' hours ago') : Drupal.t('an hour ago');
  } else if (timeDelta < month) {
    phrase = (timeDelta >= day * 2) ? Math.floor(timeDelta / day) + Drupal.t(' days ago') : Drupal.t('a day ago');
  } else if (timePrevious > 0) {
    // If the theme is older than 30 days, print the date.
    var d = new Date(timePrevious * 1000);
    phrase = d.getMonth() + 1;
    phrase += '/' + d.getDate();
    phrase += '/' + d.getFullYear();
  }
  else {
    // If there is no date data.
    phrase = Drupal.t('moments ago');
  }

  return phrase;
};

/**
 * Sets the application-wide notion of the current selector.
 *
 * This is set in the Styles tab and the Advanced->Styles CSS subtab,
 * and provides a mechanism through which this state can be easily
 * shared among objects that have no other interdependencies.
 *
 * @return {String}
 *   The selector.
 */
ThemeBuilder.util.getSelector = function () {
  if (Drupal && Drupal.settings && Drupal.settings.ThemeBuilder) {
    return Drupal.settings.ThemeBuilder.currentSelector;
  }
};

/**
 * Sets the current selector.
 *
 * @param {String}
 *   The selector.
 */
ThemeBuilder.util.setSelector = function (selector) {
  if (Drupal && Drupal.settings) {
    Drupal.settings.ThemeBuilder = Drupal.settings.ThemeBuilder || {};
    Drupal.settings.ThemeBuilder.currentSelector = selector;
  }
};
