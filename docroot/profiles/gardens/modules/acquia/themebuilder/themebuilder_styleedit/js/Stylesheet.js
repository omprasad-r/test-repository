
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * This class represents a stylesheet, such as custom.css or palette.css.
 */
ThemeBuilder.styles.Stylesheet = ThemeBuilder.initClass();

/**
 * Factory method to get an instance of a particular stylesheet.
 * 
 * @static
 * 
 * @param {string} sheetName
 *   Enough of the <link> element's href attribute to uniquely identify
 *   the stylesheet (e.g. "custom.css", "palette.css", "module/path/style.css").
 */
ThemeBuilder.styles.Stylesheet.getInstance = function (sheetName) {
  ThemeBuilder.styles.Stylesheet._sheets = ThemeBuilder.styles.Stylesheet._sheets || {};
  if (!ThemeBuilder.styles.Stylesheet._sheets[sheetName]) {
    ThemeBuilder.styles.Stylesheet._sheets[sheetName] = new ThemeBuilder.styles.Stylesheet(sheetName);
  }
  return (ThemeBuilder.styles.Stylesheet._sheets[sheetName]);
};

/**
 * Create a new <style> element.
 * 
 * We can't do this with standard jQuery methods since Internet Explorer
 * doesn't handle those well when it comes to <style> tags. See also:
 * http://www.phpied.com/dynamic-script-and-style-elements-in-ie/
 * http://www.mail-archive.com/jquery-en@googlegroups.com/msg16487.html
 * 
 * @static
 *
 * @return
 *   The <style> DOM element.
 */
ThemeBuilder.styles.Stylesheet.createStyleElement = function () {
  var stylesheet = document.createElement('style');
  stylesheet.setAttribute("type", "text/css");
  var head = document.getElementsByTagName('head')[0];
  head.appendChild(stylesheet);
  // We need to explicitly initialize the stylesheet object with empty CSS
  // or it won't work correctly.
  var css = '';
  if (stylesheet.styleSheet) {
    // Internet Explorer.
    try {
      stylesheet.styleSheet.cssText = ''; //css;
    }
    catch (e) {
      // IE8 no longer allows the cssText to be set.  Provide a line
      // of code as a breakpoint for debugging.
      var breakpoint = true;
    }
  }
  else {
    // Everyone else.
    var text = document.createTextNode(css);
    stylesheet.appendChild(text);
  }
  return stylesheet;
};

/**
 * Constructor for the Stylesheet class. This should not be called directly.
 *
 * @private
 *
 * @param {string} sheetName
 *   Enough of the <link> element's href attribute to uniquely identify
 *   the stylesheet (e.g. "custom.css", "palette.css", "module/path/style.css").
 */
ThemeBuilder.styles.Stylesheet.prototype.initialize = function (sheetName) {
  if (ThemeBuilder.styles.Stylesheet._sheets[sheetName]) {
    throw "ThemeBuilder.styles.Stylesheet objects should be created via ThemeBuilder.styles.Stylesheet.getInstance.";
  }
  var $ = jQuery;
  this.sheetSelector = 'link[href*=' + sheetName + ']';
  this.$sheet = $(this.sheetSelector, parent.document).eq(0);
  if (!this.$sheet.length) {
    var stylesheet = ThemeBuilder.styles.Stylesheet.createStyleElement();
    this.$sheet = jQuery(stylesheet);
  }
  this.sheet = this.$sheet.get(0);
};

/**
 * Get the rules from this stylesheet for a given selector.
 *
 * @param selector string
 *   The selector to grep for ("h1").
 * @return jQuery.rule
 */
ThemeBuilder.styles.Stylesheet.prototype.getRules = function (selector) {
  return jQuery.rule(selector, this.sheetSelector);
};

/**
 * Get all CSS rules from the stylesheet.
 */
ThemeBuilder.styles.Stylesheet.prototype.getAllCssRules = function () {
  return this.$sheet.sheet().cssRules();
};

/**
 * Add a CSS rule to this stylesheet.
 *
 * If a CSS rule already exists for the given selector, the new information will
 * be merged in as a new declaration for the existing rule.
 *
 * @param selector string
 *   The selector ("h1").
 * @param property string
 *   The property ("border-color").
 * @param value string
 *   The value ("#000000").
 */
ThemeBuilder.styles.Stylesheet.prototype.setRule = function (selector, property, value) {
  // Instead of setting an empty value in a rule, just remove the rule.
  if (value === '') {
    this.removeRule(selector, property);
    return;
  }

  var rules = this.getRules(selector);

  // If there's an existing rule for this selector, modify it.

  // KS: Tried rules.length, which Firebug thinks is a valid property, but in
  // practice it evaluates to undefined. I have no idea why.
  if (rules[0]) {
    // If there's more than one rule in the stylesheet for a given selector,
    // modify the last one.
    var rule = jQuery.rule(rules.slice(-1)[0]);

    var declarationFound = false;
    //var declarations = rule.text().replace(/;$/, '').split(";");
    var declarations = ThemeBuilder.styles.Declaration.getDeclarations(rule);
    var i;
    for (i = 0; i < declarations.length; i++) {
      if (declarations[i].property.toLowerCase() === property) {
        // We've found a previous declaration for this property. Replace it with
        // our new declaration.
        declarations[i].setProperty(property);
        declarations[i].setValue(value);
        declarationFound = true;
      }
    }
    if (!declarationFound) {
      // We'll need a new declaration for this property.
      declarations.push(new ThemeBuilder.styles.Declaration(property + ":" + value));
    }

    // Reassemble the rule's text() from the declarations we split it into.
    rule.text(ThemeBuilder.styles.Declaration.joinDeclarations(declarations));
  }
  // If there's no existing rule for this selector, create one.
  else {
    jQuery.rule(selector + "{" + property + ": " + value + ";}").appendTo(this.$sheet);
  }
};

/**
 * Remove a CSS rule.
 *
 * Note that the caller can ask to remove a property for a particular selector
 * without needing to know whether any other properties are defined for that
 * selector. In other words, given the following rule,
 *   h1 {color: #000; border-width: 2px;}
 * the caller can safely ask to remove the color property for h1. The rest of
 * the rule will stay intact.
 *
 * @param selector string
 *   The selector ("h1").
 * @param property string
 *   The property to remove ("color").
 */
ThemeBuilder.styles.Stylesheet.prototype.removeRule = function (selector, property) {
  var rules = this.getRules(selector);
  var i, j;
  // KS: Again, rules.length should work but doesn't, so no for loop for you!
  i = 0;
  while (rules[i]) {
    var declarations = ThemeBuilder.styles.Declaration.getDeclarations(rules[i]);
    // Check to see if this rule includes a declaration for our property.
    for (j = 0; j < declarations.length; j++) {
      // If there's a declaration for our property, get rid of it, but leave the
      // rest of the rule's declarations intact.
      if (declarations[j].property === property) {
        declarations[j].remove();
      }
    }
    // Reassemble the remaining declarations, to see if we still have a rule.
    var ruleText = ThemeBuilder.styles.Declaration.joinDeclarations(declarations);
    if (ruleText === '') {
      jQuery.rule(rules[i]).remove();
    } else {
      jQuery.rule(rules[i]).text(ruleText);
    }
    i++;
  }
};

/**
 * Disable the stylesheet.
 */
ThemeBuilder.styles.Stylesheet.prototype.disable = function () {
  this.$sheet.attr('disabled', 'disabled');
};

ThemeBuilder.styles.Stylesheet.prototype.enable = function () {
  this.$sheet.removeAttr('disabled');
};

/**
 * Remove the entire contents of a stylesheet.
 */
ThemeBuilder.styles.Stylesheet.prototype.clear = function () {
  var rules = this.getAllCssRules();
  jQuery.rule(rules).remove();
};

/**
 * Rewrite the stylesheet's color-related rules according to a given palette.
 *
 * @param {Palette} oldPalette
 *   A Palette object representing the existing colors.
 * @param {Palette} newPalette
 *   The new Palette object.
 */
ThemeBuilder.styles.Stylesheet.prototype.replacePalette = function (oldPalette, newPalette) {
  var i, j;
  var rules = this.getAllCssRules();
  for (i in rules) {
    if (ThemeBuilder.util.isNumeric(i)) {
      // Get a jQuery.rule object for the CSS rule we're editing.
      var rule = jQuery.rule(rules[i]);
      // Split the rule into declarations.
      var declarations = ThemeBuilder.styles.Declaration.getDeclarations(rule);
      for (j in declarations) {
        if (ThemeBuilder.util.isNumeric(j)) {
          declarations[j].replaceColor(oldPalette, newPalette);
        }
      }
      // Rewrite the rule with the new hex color.
      rule.text(ThemeBuilder.styles.Declaration.joinDeclarations(declarations));
    }
  }

};

/**
 * Set the contents of a stylesheet.
 *
 * TODO AN-14564: This only works with empty stylesheets created via
 * ThemeBuilder.styles.Stylesheet.prototype.createStyleElement().
 */
ThemeBuilder.styles.Stylesheet.prototype.setCssText = function (cssText) {
  cssText = cssText.toString();
  if (this.sheet.styleSheet) {
    // Internet Explorer.
    if (!cssText) {
      // For some reason, the browser crashes if we don't explicitly set
      // cssText to an empty string in this case.
      cssText = '';
    }
    try {
      this.sheet.styleSheet.cssText = cssText;
    }
    catch (e) {
      // IE8 is not allowing us to set the value of the cssText
      // property.  Provide a line of code as a breakpoint for
      // debugging.
      var breakpoint = true;
    }
  }
  else {
    // Everyone else.
    if (!this.sheet.firstChild) {
      this.sheet.appendChild(document.createTextNode(cssText));
    }
    else {
      this.sheet.replaceChild(document.createTextNode(cssText), this.sheet.firstChild);
    }
  }
};

/**
 * Returns the css rules as text.
 *
 * @return
 *   A string containing the text representation of the css rules associated
 *   with this stylesheet instance.
 */
ThemeBuilder.styles.Stylesheet.prototype.getCssText = function () {
  return this.cssRulesToText(this.getAllCssRules());
};

/**
 * Converts the specified rules into a text representation.
 *
 * @param cssRules
 *   The css rules to convert into text.
 * @return
 *   The text representation of the specified rules.
 */
ThemeBuilder.styles.Stylesheet.prototype.cssRulesToText = function (cssRules) {
  var result = '';
  if (cssRules && cssRules.length) {
    for (var i = 0; i < cssRules.length; i++) {
      result += cssRules[i].cssText + "\n";
    }
  }
  return result;
};

/**
 * Adds the specified rules to this stylesheet instance.
 *
 * @param {String array} rules
 *   The text form of the css rules.
 * @return
 *   An integer that identifies the number of rules added to this stylesheet
 *   instance.
 */
ThemeBuilder.styles.Stylesheet.prototype.addRules = function (rules) {
  var result = 0;
  var ruleCount = rules.length;
  for (var i = 0; i < ruleCount; i++) {
    var rule = rules[i];
    if (rule && rule.length > 0) {
      jQuery.rule(rule).appendTo(this.$sheet);
      result++;
    }
  }
  return result;
};
