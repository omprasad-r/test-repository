/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true debug: true window: true*/

var ThemeBuilder = ThemeBuilder || {};

/**
 * Attach the Selector test button to the ui.
 */
Drupal.behaviors.themebuilderTest = {
  attach: function (context, settings) {
    if (jQuery('body').hasClass('themebuilder')) {
      ThemeBuilder.SelectorTest.addTestButton();
    }
  }
};

/**
 * The SelectorTest class is responsible for running through all of the
 * elements on a particular page, selecting them, and using the default CSS
 * selector from the SelectorEditor to try to theme the element.  This class
 * determines whether the theming worked, and if not reports the problem.
 *
 * This is useful for upgrade testing particularly so that when we upgrade a
 * custom theme we can determine if our old selectors are too specific, making
 * subsequent theming far more difficult for the end user.
 */
ThemeBuilder.SelectorTest = ThemeBuilder.initClass();

/**
 * Adds a test button to the themebuilder that allows the user to test the
 * element selector specificity.
 *
 * @static
 */
ThemeBuilder.SelectorTest.addTestButton = function () {
  var $ = jQuery;
  if (!$('#themebuilder-style-font .font-table').length) {
    setTimeout(ThemeBuilder.SelectorTest.addTestButton, 500);
    return;
  }
  $('<button id="themebuilder-selector-test" class="test ui-state-default ui-corner-top">' + Drupal.t('Start selector test') + '</button><span id="themebuilder-selector-test-status"></span>')
    .appendTo('#themebuilder-style-font .font-table')
    .click(ThemeBuilder.SelectorTest.startTest);
};

/**
 * Starts the selector test.  This kicks off a test in which every element that
 * is themable by the themebuilder is visited and modified in almost every way
 * the themebuilder can modify it to determine if it is possible to 'click and
 * theme', or if it is necessary to fiddle with the element selector to make
 * the CSS selector more specific for certain elements and properties.
 *
 * @static
 */
ThemeBuilder.SelectorTest.startTest = function () {
  var $ = jQuery;
  var selectorTest = new ThemeBuilder.SelectorTest();
  $('#themebuilder-selector-test').text('Stop selector test')
    .unbind()
    .click(ThemeBuilder.bind(selectorTest, selectorTest.stopTest));
  selectorTest.test();
};

/**
 * Constructor for the SelectorTest class.
 */
ThemeBuilder.SelectorTest.prototype.initialize = function () {
  this.run = ThemeBuilder.bind(this, this._run);
  this.configureElementSelector();
  this.issues = [];
  this.failedSelectors = [];
  // Grab a copy of the stylesheet contents before the test is run.  These
  // will be used to revert to the original styles after every modification so
  // interim stylings don't corrupt the results.
  this.customStylesheet = ThemeBuilder.styles.Stylesheet.getInstance('custom.css');
  this.customCssRules = this.customStylesheet.getCssText().split("\n");
  this.paletteStylesheet = ThemeBuilder.styles.Stylesheet.getInstance('palette.css');
  this.paletteCssRules = this.paletteStylesheet.getCssText().split("\n");
};

/**
 * Forcibly stops the selector test, displaying any results that have been
 * collected so far.
 */
ThemeBuilder.SelectorTest.prototype.stopTest = function () {
  var $ = jQuery;
  $('.tb-theme-test').removeClass('tb-theme-test');
  $('#themebuilder-selector-test-status').text('');
  $('#themebuilder-selector-test').text('Start selector test')
    .unbind()
    .click(ThemeBuilder.SelectorTest.startTest);
};

/**
 * This method gets a reference to the Selector instance used by the
 * themebuilder so the the rest of the code can interact with it.  Note that
 * this must be done before any other non-static method is called.
 */
ThemeBuilder.SelectorTest.prototype.configureElementSelector = function () {
  var bar = ThemeBuilder.Bar.getInstance();
  var styleObj = bar.getTabObject('themebuilder-style');
  this.elementPicker = styleObj.elementPicker;
  this.pathSelector = this.elementPicker.path_selector;
  this.selectorEditor = this.elementPicker.selectorEditor;
};

/**
 * Start the selector test.
 */
ThemeBuilder.SelectorTest.prototype.test = function () {
  this.removeResults();
  this.markAllElements();
  this.timer = setTimeout(this.run, 100);
};

/**
 * Removes the results panel, if it exists in the DOM.
 */
ThemeBuilder.SelectorTest.prototype.removeResults = function () {
  var $ = jQuery;
  var $results = $('#themebuilder-selector-test-results')
  .remove();
};

/**
 * Marks all elements that should be tested.  This is done by leveraging the
 * classes the themebuilder's style tab adds to each themable element.  Note
 * that as the test is executed, the mark class will be removed from the
 * processed elements.
 */
ThemeBuilder.SelectorTest.prototype.markAllElements = function () {
  var $ = jQuery;
  $('.style-clickable:visible').addClass('tb-theme-test');
};

/**
 * Runs the actual test.  This method is called many times in a timeout,
 * processing elements on each call.  When all of the elements have been
 * processed, it sends the report.
 *
 * @private
 */
ThemeBuilder.SelectorTest.prototype._run = function () {
  var $ = jQuery;
  var $elements = $('.tb-theme-test');
  if ($elements.length === 0) {
    this.showResults();
    this.sendReport();
    this.stopTest();
    ThemeBuilder.Bar.getInstance().setStatus(Drupal.t('Selector test completed.'), 'info');
    return;
  }
  var start = new Date().getTime();
  var index = 0;
  while (this.timeRemains(start) && index < $elements.length) {
    var element = $elements[index++];
    this.processOneElement(element);
    $(element).removeClass('tb-theme-test');
    this.displayProgress($elements.length - index);
  }
  this.timer = setTimeout(this.run, 100);
};

/**
 * Displays progress information in the UI to give a bit of feedback during
 * testing.
 *
 * @param {int} elementCount
 *   The number of elements remaining.
 */
ThemeBuilder.SelectorTest.prototype.displayProgress = function (elementCount) {
  var $ = jQuery;
  var failures = this.issues.length !== 0 ? Drupal.t('; !count failure(s)', {'!count' : this.issues.length}) : '';
  var message = Drupal.t('Selector test: !count elements remaining @failures', {'!count': elementCount, '@failures' : failures});
  $('#themebuilder-selector-test-status').text(message);
};

/**
 * Shows a report on the UI that indicates the result of the test.  This is
 * called after the test is completed.
 */
ThemeBuilder.SelectorTest.prototype.showResults = function () {
  var $ = jQuery;
  if (this.issues.length === 0) {
    var text = 'success';
  }
  else {
    var baseTheme = ThemeBuilder.getApplicationInstance().getData().base_theme;
    var browserDetect = new ThemeBuilder.BrowserDetect();
    var browserString = browserDetect.browser + ' ' + browserDetect.version + ' running on ' + browserDetect.OS;
    text = 'Failures encountered.  Theme: ' + baseTheme + ' on browser ' + browserString + '.  ';
    for (var i = 0; i < this.issues.length; i++) {
      var issue = this.issues[i];
      text += 'Selector: ' + issue.selector + '; property: ' + issue.property + '; expected: ' + issue.expected + '; actual: ' + issue.actual + '\n';
    }
  }
  $('<div id="themebuilder-selector-test-results">' + text + '</div>')
  .appendTo('#themebuilder-style-font .font-table');
};

/**
 * Sends the report to the server side, where it can be recorded.  Only
 * failures are reported.  Note that duplicates have been removed, so likely
 * only the first failure encountered will be reported for any given selector.
 */
ThemeBuilder.SelectorTest.prototype.sendReport = function () {
  ThemeBuilder.postBack(Drupal.settings.themebuilderTestReportPath,
    {issues: this.issues});
};

/**
 * Determines whether time time remains to process more elements.
 *
 * @param {int} startTime
 *   The unix timestamp that represents the time that the run function was
 *   called.
 *
 * @return
 *   A boolean that indicates whether more time to process elements remains or
 *   if control should be yielded so other javascript code can function.
 */
ThemeBuilder.SelectorTest.prototype.timeRemains = function (startTime) {
  var duration = 200;
  return (new Date().getTime() - startTime < 200);
};

/**
 * Processes a single element.  For each element, a set of CSS changes are
 * done and a determination is made of whether the change took effect or not.
 *
 * @param {DomElement} element
 *   The element to process.
 */
ThemeBuilder.SelectorTest.prototype.processOneElement = function (element) {
  var $ = jQuery;
  var selector = this.selectElement(element);
  if (!this.alreadyFailed(selector)) {
    var modifications = this.getModifications(element, selector);
    this.previewModifications(modifications);
    this.verifyModifications(modifications, element);
    this.revertModifications(modifications);
  }
};

/**
 * Causes the specified element to be selected.  Calling this method sets the
 * element into the Selector instance and causes the associated filter to run,
 * resulting in the default CSS selector being configured in the Selector
 * instance.
 *
 * @param {DomElement} element
 *   The element to select.
 */
ThemeBuilder.SelectorTest.prototype.selectElement = function (element) {
  this.pathSelector.setElement(element);
  this.selector = this.pathSelector.getCssSelector();
  return this.selector;
};

/**
 * Indicates whether theming on the specified selector has already failed.
 *
 * @param {String} selector
 *   The CSS selector.
 *
 * @return
 *   A boolean value that indicates whether theming on the specified selector
 *   has already encountered a failure.
 */
ThemeBuilder.SelectorTest.prototype.alreadyFailed = function (selector) {
  return this.failedSelectors.contains(selector);
};

/**
 * Returns a set of modifications that should be applied and verified during
 * the course of the test.
 *
 * @param {DomElement} element
 *   The selected element.
 * @param {String} selector
 *   The default CSS selector from the Selector instance.
 */
ThemeBuilder.SelectorTest.prototype.getModifications = function (element, selector) {
  var $ = jQuery;
  var $element = $(element);
  var modifications = [];
  if (!this.alreadyFailed(selector)) {
    var getComputedStyle = ThemeBuilder.styleEditor.getComputedStyleFunction(element);
    modifications.push(this.createCssModification(getComputedStyle, selector, 'color', ['rgb(255, 0, 0)', 'rgb(0, 255, 0)']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'font-family', ['Arial', 'Helvetica']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'font-size', ['20px', '8px']));
    // used to be 'bold' and '200', but since some browsers will set the weight to 700 when bold is selected, this seems to be better
    modifications.push(this.createCssModification(getComputedStyle, selector, 'font-weight', ['700', '200']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'font-style', ['italic', 'normal']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'text-transform', ['uppercase', 'capitalize']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'text-align', ['center', 'justify']));
    if (this.shouldDoLineHeight(element, getComputedStyle)) {
      modifications.push(this.createCssModification(getComputedStyle, selector, 'line-height', ['10px', '20px']));
    }
    modifications.push(this.createCssModification(getComputedStyle, selector, 'letter-spacing', ['1.2px', '0.8px']));

    // Box model.
    modifications.push(this.createCssModification(getComputedStyle, selector, 'border-top-width', ['67px', '42px']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'border-top-style', ['solid', 'dashed']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'border-top-color', ['rgb(255, 0, 0)', 'rgb(0, 255, 0)']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'margin-top', ['25px', '3px']));
    if (ThemeBuilder.util.trim(selector) !== 'body') {
      // This doesn't work on the body because the top padding is
      // set for the admin menu at the top of the page.
      modifications.push(this.createCssModification(getComputedStyle, selector, 'padding-top', ['7px', '17px']));
    }

    modifications.push(this.createCssModification(getComputedStyle, selector, 'border-right-width', ['67px', '42px']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'border-right-style', ['solid', 'dashed']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'border-right-color', ['rgb(255, 0, 0)', 'rgb(0, 255, 0)']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'margin-right', ['25px', '3px']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'padding-right', ['7px', '17px']));

    modifications.push(this.createCssModification(getComputedStyle, selector, 'border-bottom-width', ['67px', '42px']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'border-bottom-style', ['solid', 'dashed']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'border-bottom-color', ['rgb(255, 0, 0)', 'rgb(0, 255, 0)']));
    if (ThemeBuilder.util.trim(selector) !== 'body') {
      // This doesn't work on the body because the themebuilder is on
      // the bottom of the page.
      modifications.push(this.createCssModification(getComputedStyle, selector, 'margin-bottom', ['25px', '3px']));
    }
    modifications.push(this.createCssModification(getComputedStyle, selector, 'padding-bottom', ['7px', '17px']));

    modifications.push(this.createCssModification(getComputedStyle, selector, 'border-left-width', ['67px', '42px']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'border-left-style', ['solid', 'dashed']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'border-left-color', ['rgb(255, 0, 0)', 'rgb(0, 255, 0)']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'margin-left', ['25px', '3px']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'padding-left', ['7px', '17px']));

    modifications.push(this.createCssModification(getComputedStyle, selector, 'background-color', ['rgb(255, 0, 0)', 'rgb(0, 255, 0)']));
    if (this.shouldDoBackgroundImage(element, getComputedStyle)) {
      modifications.push(this.createCssModification(getComputedStyle, selector, 'background-image', ['none']));
    }
    modifications.push(this.createCssModification(getComputedStyle, selector, 'background-repeat', ['repeat', 'repeat-x']));
    modifications.push(this.createCssModification(getComputedStyle, selector, 'background-attachment', ['fixed', 'scroll']));
  }
  
  return modifications;
};

/**
 * Causes all of the specified modifications to be previewed.  This causes css
 * changes on the selected element.
 *
 * @param {Modification array} modifications
 *   An array of Modification instances that should be applied to the selected
 *   element.
 */
ThemeBuilder.SelectorTest.prototype.previewModifications = function (modifications) {
  for (var index = 0; index < modifications.length; index++) {
    ThemeBuilder.preview(modifications[index]);
  }
};

/**
 * Verifies all of the specified modifications took effect on the specified
 * element.  Any modification that has a newState that doesn't match the
 * current state of the specified element is considered a failure and will be
 * reported.
 *
 * @param {Modification array} modifications
 *   An array of Modification instances that should be verified on the
 *   selected element.
 */
ThemeBuilder.SelectorTest.prototype.verifyModifications = function (modifications, element) {
  var getComputedStyle = ThemeBuilder.styleEditor.getComputedStyleFunction(element);
  for (var index = 0; index < modifications.length; index++) {
    var modification = modifications[index];
    if (this.alreadyFailed(modification.getNewState().selector)) {
      // Already encountered a failure.  No need to continue with this
      // selector.
      break;
    }
    this.verifyModification(getComputedStyle, modification);
  }
};

/**
 * Causes all of the specified modifications to be reverted.  This causes the
 * selected element to go back to its state before the modifications were
 * previewed.
 *
 * @param {Modification array} modifications
 *   An array of Modification instances that should be reverted on the
 *   selected element.
 */
ThemeBuilder.SelectorTest.prototype.revertModifications = function (modifications) {
  // We want to revert to the original stylesheets.  We could simply revert
  // the modifications, but that results in a rule being added to the
  // stylesheet rather than the rule being removed.
  this.customStylesheet.clear();
  this.customStylesheet.addRules(this.customCssRules);
  this.paletteStylesheet.clear();
  this.paletteStylesheet.addRules(this.paletteCssRules);
};

/**
 * Creates a single CssModification instance that describes a change to the
 * selected element.
 *
 * @param {function} getComputedStyle
 *   The function that returns computed styles for the selected element.
 * @param {String} selector
 *   The default CSS selector from the Selector instance.
 * @param {String} property
 *   The CSS property.
 * @param {String array} values
 *   Appropriate values that could be chosen for the modification.  Before a
 *   value is selected, it is compared against the current value for the same
 *   property on the selected element.  A different value must be used or the
 *   test will be ineffective.  Note that to guarantee a different value is
 *   available, at least two different valid values must be provided in the
 *   values array.
 */
ThemeBuilder.SelectorTest.prototype.createCssModification = function (getComputedStyle, selector, property, values) {
  var currentValue = getComputedStyle(property);
  // Find a legal value that is different than the current value.
  for (var i = 0; i < values.length; i++) {
    if (currentValue !== values[i]) {
      var value = values[i];
      break;
    }
  }
  
  var result = new ThemeBuilder.CssModification(selector);
  result.setPriorState(property, currentValue);
  result.setNewState(property, value);
  return result;
};

/**
 * Verifies the specified modification took effect on the selected element.
 * Any modification that has a newState that doesn't match the current state
 * of the selected element is considered a failure and will be reported.
 *
 * @param {function} getComputedStyle
 *   The function that returns computed styles for the selected element.
 * @param {Modification} modification
 *   An array of Modification instances that should be verified on the
 *   selected element.
 */
ThemeBuilder.SelectorTest.prototype.verifyModification = function (getComputedStyle, modification) {
  var priorState = modification.getPriorState();
  var newState = modification.getNewState();
  var currentValue = getComputedStyle(newState.property);
  if (currentValue !== newState.value) {
    // This is an issue.
    var selector = this.pathSelector.getCssSelector();
    this.issues.push(this.createResult(selector, newState.property, newState.value, currentValue));
    this.failed(selector);
  }
};

/**
 * Creates an object that describes a failure.
 *
 * @param {String} selector
 *   The CSS selector.
 * @param {String} property
 *   The CSS property.
 * @param {String} expected
 *   The expected value of the specified property.
 * @param {String} actual
 *   The actual value of the specified property.
 * @return
 *   An object that describes the specific failure.
 */
ThemeBuilder.SelectorTest.prototype.createResult = function (selector, property, expected, actual) {
  return {selector: selector, property: property, expected: expected, actual: (actual ? actual : 'null')};
};

/**
 * Identifies the specified selector as one which has encountered a theming
 * failure.  This is done to prevent duplicate results.
 *
 * @param {String} selector
 *   The CSS selector.
 */
ThemeBuilder.SelectorTest.prototype.failed = function (selector) {
  this.failedSelectors.push(selector);
};

/**
 * Causes the tests to stop.  This can be used to forcibly halt the testing.
 */
ThemeBuilder.SelectorTest.prototype.stopTesting = function () {
  var $ = jQuery;
  $('.tb-theme-test').removeClass('tb-theme-test');
  throw 'Tests forcibly stopped';
};

/**
 * Determines whether the line height should be included in the test.
 *
 * @param {DomElement} element
 *   The element
 * @param {function} getComputedStyle
 *   The function that returns computed styles for the specified element.
 */
ThemeBuilder.SelectorTest.prototype.shouldDoLineHeight = function (element, getComputedStyle) {
  // Firefox can't change the line height on buttons.  See
  // http://www.cssnewbie.com/input-button-line-height-bug/ for more details.
  // Note that we have also encountered problems with inputs.
  if (element.tagName.toLowerCase() === 'button' ||
    element.tagName.toLowerCase() === 'input') {
    return false;
  }
  return true;
};

/**
 * Having difficulty setting a background image in which the set value is the
 * same as the actual value.  For now, disable it unless we can set it to
 * 'none'.
 *
 * @param {DomElement} element
 *   The element
 * @param {function} getComputedStyle
 *   The function that returns computed styles for the specified element.
 */
ThemeBuilder.SelectorTest.prototype.shouldDoBackgroundImage = function (element, getComputedStyle) {
  if (getComputedStyle('background-image') !== 'none') {
    return true;
  }
  return false;
};
