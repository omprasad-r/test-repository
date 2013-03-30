
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * The FontEditor class is responsible for editing font styles, sizes,
 * and colors.
 * @class
 * @extends ThemeBuilder.styles.Editor
 */
ThemeBuilder.styles.FontEditor = ThemeBuilder.initClass();
ThemeBuilder.styles.FontEditor.prototype = new ThemeBuilder.styles.Editor();

/**
 * The constructor of the FontEditor class.
 *
 * @param {ElementPicker} elementPicker
 *   The instance of ElementPicker that is used to select an element to theme.
 */
ThemeBuilder.styles.FontEditor.prototype.initialize = function (elementPicker) {
  this.elementPicker = elementPicker;
  this.tabName = "font";
  this.textAlignRadioButton = new ThemeBuilder.styles.RadioButton('.text-align-panel', 'text-align', 'left');
  this.textAlignRadioButton.addChangeListener(this);

  this.modifications = {};
  ThemeBuilder.getApplicationInstance().addApplicationInitializer(ThemeBuilder.bind(this, this.loadFontFaces));
};

/**
 * Add a stylesheet for @font-face fonts.
 *
 * The @font-face styles need to be lazy-loaded because they'll trigger lots of
 * font file downloading.
 */
ThemeBuilder.styles.FontEditor.prototype.loadFontFaces = function (appData) {
  this.fontFaces = appData.fontFaces;
  this.fontFaceStacks = {};
  var i, fontFace;
  var cssText = '';
  for (i = 0; i < this.fontFaces.length; i++) {
    fontFace = this.fontFaces[i];
    // For fonts with @font-face rules, add those rules to a stylesheet, and
    // create a lookup table for use in this.familyChanged().
    if (fontFace.fontFaceRule !== '') {
      cssText += fontFace.fontFaceRule + "\n";
      this.fontFaceStacks[fontFace.fontFamily] = fontFace.name;
    }
  }

  if (this.shouldRemoveServerFontStyles()) {
    // Some browsers can't handle @font-face fonts within a select element.
    // Remove the font-family from such elements.
    var $ = jQuery;
    $('#style-font-family .tb-server-font').css('font-family', 'inherit');
  }
  var stylesheet = ThemeBuilder.styles.Stylesheet.getInstance('fontface.css');
  stylesheet.setCssText(cssText);
};

/**
 * Determine whether the font styles should be removed for all server side
 * fonts within the themebuilder font-family select element.  On some browsers
 * setting @font-face fonts in the select list causes the select list to be
 * inoperative.
 *
 * @return
 *   True if the selector's font-face styles should be removed for server side
 *   fonts; false otherwise.
 */
ThemeBuilder.styles.FontEditor.prototype.shouldRemoveServerFontStyles = function () {
  var browserDetect = new ThemeBuilder.BrowserDetect();
  // If @font-face fonts are used in a select element in Safari, the select
  // element cannot be opened.  Note that this only occurs on Safari running
  // on Mac.
  return browserDetect.browser === 'Safari' && browserDetect.OS === 'Mac';
};

/**
 * Initializes the Font tab.  The purpose of this method is to attach event
 * listeners to the relevant DOM elements, effectively wiring the behavior to
 * the controls.
 */
ThemeBuilder.styles.FontEditor.prototype.setupTab = function () {
  var $ = jQuery;
  $('#themebuilder-style-font input, #themebuilder-style-font select').attr('disabled', true);
  $('#style-font-family').change(ThemeBuilder.bind(this, this.familyChanged));
  this.palettePicker = new ThemeBuilder.styles.PalettePicker($('#style-font-color'), 'color', $('#themebuilder-wrapper'));

  // font size
  $('#style-font-size').inputslider({
    min: 8,
    max: 30,
    step: 2,
    onSlide: ThemeBuilder.bind(this, this.sizePreview),
    onStop: ThemeBuilder.bind(this, this.sizeChanged),
    onShow: ThemeBuilder.bind(this, this.showSizeSlider)
  });
  $('#style-font-size').change(ThemeBuilder.bind(this, this.sizeFieldChanged));
  $('#style-font-size-u').change(ThemeBuilder.bind(this, this.sizeUnitsChanged));

  // leading
  $('#style-line-height').inputslider({
    min: 90,
    max: 200,
    step: 2,
    onSlide: ThemeBuilder.bind(this, this.heightPreview),
    onStop: ThemeBuilder.bind(this, this.heightChanged),
    onShow: ThemeBuilder.bind(this, this.showHeightSlider)
  });

  $('#style-line-height').change(ThemeBuilder.bind(this, this.heightFieldChanged));

  // kerning
  $('#style-letter-spacing').inputslider({
    min: -5,
    max: 5,
    step: 0.2,
    onSlide: ThemeBuilder.bind(this, this.spacingPreview),
    onStop: ThemeBuilder.bind(this, this.spacingChanged),
    onShow: ThemeBuilder.bind(this, this.showSpacingSlider)
  });
  $('#style-letter-spacing').change(ThemeBuilder.bind(this, this.spacingFieldChanged));

  $("#themebuilder-style-font .fg-button").mousedown(ThemeBuilder.bind(this, this.styleButtonClicked));

  $("#themebuilder-style-font #typekit-toggle").click(ThemeBuilder.bind(this, this.toggleTypekit));
};

/**
 * This method is called by ElementPicker:selectorSelected when the user
 * selects an element.  This causes the state of the FontEditor controls to
 * change, reflecting the current state of the specified selector.  This
 * method is called when the user selects an element or changes the selector
 * by interacting with the element selector widget.
 *
 * @param {String} selector
 *   The new CSS selector.
 */
ThemeBuilder.styles.FontEditor.prototype.selectorChanged = function (selector) {
  var $ = jQuery;
  this.enableInputs();
  $('#themebuilder-style-background input, #themebuilder-style-background select,#themebuilder-style-background button').removeAttr('disabled');
  $('#style-font-family option').eq(0).attr('selected', true);
  $('#style-font-size').val('');
  $('#style-font-size-u option').eq(0).attr('selected', true);
  $('#style-font-weight option').eq(0).attr('selected', true);
  $('#style-font-style option').eq(0).attr('selected', true);
  $('#themebuilder-style-font input').val('');
  $('#themebuilder-style-font button').removeClass('ui-state-active');

  this.currentSelector = selector;
  this.palettePicker.setSelector(this.currentSelector);
  this.refreshDisplay();
};

/**
 * This method is called when the state of radio buttons is changed.  This
 * handles the text-align property.
 *
 * @param {String} propertyName
 *   The name of the property being changed.
 * @param {String} oldValue
 *   The original value.
 * @param {String} newValue
 *   The new value.
 */
ThemeBuilder.styles.FontEditor.prototype.valueChanged = function (propertyName, oldValue, newValue) {
  var modification = new ThemeBuilder.CssModification(this.currentSelector);
  modification.setPriorState(propertyName, oldValue);
  modification.setNewState(propertyName, newValue);
  ThemeBuilder.applyModification(modification);
};

/**
 * Refreshes the display.  This should occur when the user selects a new
 * element or when the display changes for some other reason, such as clicking
 * undo or redo.  This method looks at the current set of properties for the
 * current selector and makes the property values on the FontEditor match.
 */
ThemeBuilder.styles.FontEditor.prototype.refreshDisplay = function () {
  var element = this.getSelectedElement();
  var getComputedStyle = ThemeBuilder.styleEditor.getComputedStyleFunction(element);
  this._refreshFamily(getComputedStyle);
  this._refreshColor(getComputedStyle);
  this._refreshSize(getComputedStyle);
  this._refreshBoldButtonProperty(getComputedStyle, '#themebuilder-style-font .fg-button.bold', 'font-weight');
  this._refreshButtonProperty(getComputedStyle, '#themebuilder-style-font .fg-button.italic', 'font-style', 'italic');
  this._refreshButtonProperty(getComputedStyle, '#themebuilder-style-font .fg-button.underline', 'text-decoration', 'underline');
  this._refreshTextAlign(getComputedStyle);
  this._refreshButtonProperty(getComputedStyle, '#themebuilder-style-font .fg-button.uppercase', 'text-transform', 'uppercase');
  this._refreshSliderValue(getComputedStyle, '#style-line-height', 'line-height', 'normal', this.processLineHeightValue);
  this._refreshSliderValue(getComputedStyle, '#style-letter-spacing', 'letter-spacing', 'normal', this.processLetterSpacingValue);
};

/**
 * Responsible for refreshing the font-family.
 *
 * @private
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 */
ThemeBuilder.styles.FontEditor.prototype._refreshFamily = function (getComputedStyle) {
  var $ = jQuery;
  var value = getComputedStyle('font-family');
  if (!value) {
    value = '';
  }
  value = this.normalizeFontValue(value);
  $('#style-font-family').val(value);
  this.modifications['font-family'] = new ThemeBuilder.CssModification(this.currentSelector);
  this.modifications['font-family'].setPriorState('font-family', value);
};

/**
 * If the font value is not normalized, small inconsistencies will make it
 * such that the selected font will not appear in the font-family drop down
 * menu.
 *
 * @param {String} font
 *   A string representing the font family.
 *
 * @return
 *   The normalized representation of the font value.
 */
ThemeBuilder.styles.FontEditor.prototype.normalizeFontValue = function (font) {
  if (font === "'auto'") {
    return 'inherit';
  }
  var parts = font.split(',');
  var result = [];
  for (var i = 0; i < parts.length; i++) {
    var font_string = parts[i];
    // Remove quotes from the value so it will match the option menu.  This is
    // required for Safari.
    font_string = font_string.replace(/\"/g, '');
    font_string = font_string.replace(/\'/g, '');
    font_string = jQuery.trim(font_string);
    font_string = "'" + font_string + "'";
    result.push(font_string);
  }
  return result.join(',');
};

/**
 * Responsible for refreshing the color.
 *
 * @private
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 */
ThemeBuilder.styles.FontEditor.prototype._refreshColor = function (getComputedStyle) {
  var $ = jQuery;
  var value = getComputedStyle('color');
  this.palettePicker.setIndex(value);
};

/**
 * Responsible for refreshing the font-size.
 *
 * @private
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 */
ThemeBuilder.styles.FontEditor.prototype._refreshSize = function (getComputedStyle) {
  var $ = jQuery;
  var value = getComputedStyle('font-size');
  var size = parseInt(value, 10);
  // There has to be a more correct way to handle this.
  var units = (value.slice(-2));
  $('#style-font-size').val(size);
  $('#style-font-size-u').val(units);
  this.modifications['font-size'] = new ThemeBuilder.CssModification(this.currentSelector);
  this.modifications['font-size'].setPriorState('font-size', '' + size + units);
};

/**
 * Responsible for refreshing the value associated with a slider.
 *
 * @private
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 * @param {String} controlSelector
 *   The css selector that uniquely identifies the control in the themebuilder
 *   that should be updated.
 * @param {String} propertyName
 *   The property name that is edited with the slider.
 * @param {String} defaultValue
 *   The value to use should a value for the specified propertyName not be
 *   set.
 * @param {function} processValue
 *   The function used to map the css value to appropriate values for the
 *   themebuilder control and the modification instance.
 */
ThemeBuilder.styles.FontEditor.prototype._refreshSliderValue = function (getComputedStyle, controlSelector, propertyName, defaultValue, processValue) {
  var $ = jQuery;
  var size = '';
  var valueString = defaultValue;
  var value = ThemeBuilder.util.parseCssValue(getComputedStyle(propertyName), defaultValue);

  var processedValue = processValue(getComputedStyle, value);
  $(controlSelector).val(processedValue.value);
  this.modifications[propertyName] = new ThemeBuilder.CssModification(this.currentSelector);
  this.modifications[propertyName].setPriorState(propertyName, processedValue.valueString);
};

/**
 * Calculates appropriate values for the value displayed in the leading
 * control and for the modification.  These are not always the same.  The
 * default value for line-height is 'normal', which we take to mean '150%'.
 *
 * @param {function} getComputedStyle
 *   The function that returns the computed style for the selected element.
 * @param {Object} value
 *   The value returned from ThemeBuilder.util.parseCssValue.
 * @result {Object}
 *   An object that provides the value to display in the control and a
 *   separate value to set into the modification instance.
 */
ThemeBuilder.styles.FontEditor.prototype.processLineHeightValue = function (getComputedStyle, value) {
  var result = {};
  if (value.number) {
    switch (value.units) {
    case '%':
      var size = value.number;
      break;
    case 'px':
      // The units aren't in percent, so calculate the percent based on the
      // font size.
      var fontSize = ThemeBuilder.util.parseCssValue(getComputedStyle('font-size'), '13px');
      if (fontSize.units === 'px') {
        value.number = Math.round((value.number / fontSize.number) * 100);
        value.units = '%';
      }
      break;
    default:
      ThemeBuilder.logCallback('FontEditor::processLineHeightValue encountered a line-height value that is unexpectedly expressed in units ' + value.units);
    }
    result.value = value.number;
    result.valueString = value.number + value.units;
  }
  else {
    result.value = '150';
    result.valueString = 'normal';
  }
  return result;
};

/**
 * Calculates appropriate values for the value displayed in the kerning
 * control and for the modification.  These are not always the same.  The
 * default value for letter-spacing is 'normal', which we take to mean '0px'.
 *
 * @param {function} getComputedStyle
 *   The function that returns the computed style for the selected element.
 * @param {Object} value
 *   The value returned from ThemeBuilder.util.parseCssValue.
 * @result {Object}
 *   An object that provides the value to display in the control and a
 *   separate value to set into the modification instance.
 */
ThemeBuilder.styles.FontEditor.prototype.processLetterSpacingValue = function (getComputedStyle, value) {
  var result = {};
  if (value.number) {
    if (value.units === 'px') {
      result.value = value.number;
      result.valueString = value.number + value.units;
    }
    else {
      ThemeBuilder.logCallback('FontEditor::processLetterSpacingValue encountered a line-height value that is unexpectedly expressed in units ' + value.units);
    }
  }
  else {
    result.value = '0';
    result.valueString = 'normal';
  }
  return result;
};

/**
 * Responsible for refreshing the bold toggle button on the font tab.
 *
 * This method is distinct from the _refreshButtonProerty method
 * because it deals with the possibility of either a string or an
 * integer value for the bold property.  Note that IE returns an
 * integer rather than retaining the string value used to set the
 * property.
 *
 * @private
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 * @param {String} controlSelector
 *   The css selector used to uniquely identify the button element.
 * @param {String} propertyName
 *   The name of the css property associated with the button.
 */
ThemeBuilder.styles.FontEditor.prototype._refreshBoldButtonProperty = function (getComputedStyle, controlSelector, propertyName) {
  var $ = jQuery;
  var value = getComputedStyle(propertyName);
  if (value && value.toLower) {
    // This is a string
  }
  if (value && !value.toLower && value > 500) {
    // Internet Explorer passes back the actual font weight as an
    // integer rather than retaining the value name that was used.
    value = 'bold';
  }

  if (value === 'bold') {
    $(controlSelector).addClass('ui-state-active');
  }
  else {
    $(controlSelector).removeClass('ui-state-active');
  }
  this.modifications[propertyName] = new ThemeBuilder.CssModification(this.currentSelector);
  this.modifications[propertyName].setPriorState(propertyName, value);
};

/**
 * Responsible for refreshing toggle buttons on the font tab.
 *
 * @private
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 * @param {String} controlSelector
 *   The css selector used to uniquely identify the button element.
 * @param {String} propertyName
 *   The name of the css property associated with the button.
 * @param {String} onValue
 *   The value associated with the 'on' state for the button.
 */
ThemeBuilder.styles.FontEditor.prototype._refreshButtonProperty = function (getComputedStyle, controlSelector, propertyName, onValue) {
  var $ = jQuery;
  var value = getComputedStyle(propertyName);
  if (value === onValue) {
    $(controlSelector).addClass('ui-state-active');
  }
  else {
    $(controlSelector).removeClass('ui-state-active');
  }
  this.modifications[propertyName] = new ThemeBuilder.CssModification(this.currentSelector);
  this.modifications[propertyName].setPriorState(propertyName, value);
};

/**
 * Refreshes the text-align control to match the current
 * selection.
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 */
ThemeBuilder.styles.FontEditor.prototype._refreshTextAlign = function (getComputedStyle) {
  var $ = jQuery;
  // Initialize the background-repeat value.
  var value = getComputedStyle('text-align');
  if (!value) {
    value = 'left';
  }
  switch (value) {
  case 'start':
  case 'auto':
    value = 'left';
    break;
  case 'end':
    value = 'right';
    break;
  }
  // Cause the display to be updated without simulating a user click.
  try {
    this.textAlignRadioButton.setEnabledButton(value);
  }
  catch (e) {
  }
  this.modifications['text-align'] = new ThemeBuilder.CssModification(this.currentSelector);
  this.modifications['text-align'].setPriorState('text-align', value);
};

/**
 * Called when the specified property has been changed to the specified value.
 * This method causes the change to be applied.
 *
 * @param {String} property
 *   The property that has changed.
 * @param {String} value
 *   The new value for the specified property.
 * @param {Array} resources
 *   Any resources that are required for the new property (font, image, etc.)
 */
ThemeBuilder.styles.FontEditor.prototype.propertyChanged = function (property, value, resources) {
  this.modifications[property].setNewState(property, value, resources);
  ThemeBuilder.applyModification(this.modifications[property]);
  this.modifications[property] = this.modifications[property].getFreshModification();
};

/**
 * Called when the user changes the font-family using the option menu.
 *
 * @param {DomEvent} event
 *   The event that represents the change.
 */
ThemeBuilder.styles.FontEditor.prototype.familyChanged = function (event) {
  var property = 'font-family';
  var resources = [];
  if (event && event.currentTarget) {
    var value = event.currentTarget.value;
    // Determine whether this is a @font-face font that requires a server-side
    // resource to be added to the theme.
    var fontName = this.fontFaceStacks[value];
    if (fontName) {
      resources.push({type: 'font', name: fontName});
    }
    this.propertyChanged(property, value, resources);
  }
};

/**
 * Called when the user changes the font-size units using the option menu.
 *
 * @param {DomEvent} event
 *   The event that represents the change.
 */
ThemeBuilder.styles.FontEditor.prototype.sizeUnitsChanged = function (event) {
  var $ = jQuery;
  var units = $('#style-font-size-u').val();
  var defaulthash = {'px': {min: 10, max: 30, step: 2},
                     'em': {min: 0.5, max: 4, step: 0.1}};
  var defaults = defaulthash[units];
  if ($('#style-font-size').val() > defaults.max) {
    $('#style-font-size').val(defaults.max);
  }
  else if ($('#style-font-size').val() < defaults.min) {
    $('#style-font-size').val(defaults.min);
  }
  this.propertyChanged('font-size', $('#style-font-size').val() + units);
};

/**
 * Called when the user clicks either the bold, italic, or uppercase button.
 * This causes the associated font property to be toggled.
 *
 * @param {DomEvent} event
 *   The event that represents the change.
 */
ThemeBuilder.styles.FontEditor.prototype.styleButtonClicked = function (event) {
  var node = jQuery(event.currentTarget);
  var propertyName = this.getPropertyFromClass(node);
  if (!propertyName) {
    return;
  }
  var value;
  var enabled;
  if (node.is('.ui-state-active.fg-button-toggleable, .fg-buttonset-multi .ui-state-active')) {
    node.removeClass("ui-state-active");
    enabled = false;
  }
  else {
    node.addClass('ui-state-active');
    enabled = true;
  }
  switch (propertyName) {
  case 'font-weight':
    value = enabled === true ? 'bold' : 'normal';
    break;
  case 'font-style':
    value = enabled === true ? 'italic' : 'normal';
    break;
  case 'text-decoration':
    value = enabled === true ? 'underline' : 'none';
    break;
  case 'text-transform':
    value = enabled === true ? 'uppercase' : 'none';
    break;
  default:
    return;
  }
  if (value) {
    this.propertyChanged(propertyName, value);
  }
};

/**
 * Event handler to display the site with or without Typekit fonts enabled.
 */
ThemeBuilder.styles.FontEditor.prototype.toggleTypekit = function (event) {
  var $ = jQuery;
  var typekitCss = $('link[href^=http://use.typekit.com]');
  var link = event.currentTarget;
  var text = $(link).html();
  var disableText = Drupal.t('show site without Typekit fonts');
  var enableText = Drupal.t('re-enable Typekit fonts');
  switch (text) {
  case disableText:
    // Turn off Typekit fonts.
    typekitCss.get(0).disabled = true;
    $(link).html(enableText);
    break;

  case enableText:
    // Turn on Typekit fonts.
    typekitCss.get(0).disabled = false;
    $(link).html(disableText);
    break;
  }
};

/**
 * Determines what the property name is based on the class associated with the
 * specified element.
 *
 * @param {jQuery element} node
 *   The element associated with the button.
 * @return {String}
 *   The property name that the specified element is meant to adjust.
 *   Undefined is returned if it cannot be determined from the element's
 *   classes.
 */
ThemeBuilder.styles.FontEditor.prototype.getPropertyFromClass = function (node) {
  var classMap = {'bold': 'font-weight',
                  'italic': 'font-style',
                  'underline': 'text-decoration',
                  'uppercase': 'text-transform'};
  for (var name in classMap) {
    if (typeof(classMap[name]) === 'string') {
      if (node.hasClass(name)) {
        return classMap[name];
      }
    }
  }
  return undefined;
};

/**
 * Causes the slider that adjusts the font size to be displayed.
 *
 * @param {jQuerySlider} slider
 *   The slider instance.
 * @param {DomElement} target
 *   The target element that will be
 */
ThemeBuilder.styles.FontEditor.prototype.showSizeSlider = function (slider, target) {
  var $ = jQuery;
  if ($('#element-to-edit .theelement').html() === 'no element selected') {
    return false;
  }
  var defaulthash = {'px': {min: 10, max: 72, step: 2},
                     'em': {min: 0.5, max: 4, step: 0.1}};
  var defaults = defaulthash[$('#style-font-size-u').val()];
  if (!defaults) {
    throw "Invalid Size Unit";
  }
  slider.slider.slider('option', 'max', defaults.max);
  slider.slider.slider('option', 'min', defaults.min);
  slider.slider.slider('option', 'step', defaults.step);
  var val = $(target).val();
  $(target).focus();
  if (val > defaults.max) {
    val = defaults.max;
  }
  else if (val < defaults.min) {
    val = defaults.min;
  }
  slider.slider.slider('value', val);
};

/**
 * Called when the size slider has moved and the change should be displayed.
 *
 * @param {jQuerySlider} slider
 *   The slider instance.
 * @param {DomEvent} event
 *   The event associated with sliding the thumb of the slider.
 * @param {String} value
 *   The new value
 * @param {DomElement} target
 *   The target element that will be
 */
ThemeBuilder.styles.FontEditor.prototype.sizePreview = function (sizer, event, value, target) {
  var $ = jQuery;
  this.modifications['font-size'].setNewState('font-size',
    value + $('#style-font-size-u').val());
  ThemeBuilder.preview(this.modifications['font-size']);
  $(target).val(value);
};

/**
 * Called when the size slider has been dismissed and the change should be
 * applied.
 *
 * @param {jQuerySlider} slider
 *   The slider instance.
 * @param {DomEvent} event
 *   The event associated with sliding the thumb of the slider.
 * @param {String} value
 *   The new value
 * @param {DomElement} target
 *   The target element that will be
 */
ThemeBuilder.styles.FontEditor.prototype.sizeChanged = function (sizer, event, value, target) {
  this.propertyChanged('font-size', value + jQuery('#style-font-size-u').val());
};

/**
 * Called when the user enters a value in the size field.
 *
 * @param {DomEvent} event
 *   The event that represents the change.
 */
ThemeBuilder.styles.FontEditor.prototype.sizeFieldChanged = function (event) {
  var $ = jQuery;
  var value = $('#style-font-size').val();
  if (value.currentTarget) {
    value = value.currentTarget.value;
  }
  this.propertyChanged('font-size', value + $('#style-font-size-u').val());
};

/**
 * Causes the slider that adjusts the font size to be displayed.
 *
 * @param {jQuerySlider} slider
 *   The slider instance.
 * @param {DomElement} target
 *   The target element that will be
 */
ThemeBuilder.styles.FontEditor.prototype.showHeightSlider = function (slider, target) {
  var $ = jQuery;
  if ($('#element-to-edit .theelement').html() === 'no element selected') {
    return false;
  }
  var val = $(target).val();
  if (!ThemeBuilder.util.isNumeric(val)) {
    val = '150';
  }
  $(target).focus();
  slider.slider.slider('value', val);
};

/**
 * Called when the height slider has moved and the change should be displayed.
 *
 * @param {jQuerySlider} slider
 *   The slider instance.
 * @param {DomEvent} event
 *   The event associated with sliding the thumb of the slider.
 * @param {String} value
 *   The new value
 * @param {DomElement} target
 *   The target element that will be
 */
ThemeBuilder.styles.FontEditor.prototype.heightPreview = function (sizer, event, value, target) {
  var $ = jQuery;
  var valueString = value;
  if (ThemeBuilder.util.isNumeric(value)) {
    valueString = value + '%';
  }
  else {
    // The default value is 150%.  Use this when the value entered is 'normal'.
    value = '150';
    valueString = value + '%';
  }
  this.modifications['line-height'].setNewState('line-height',
    valueString);
  ThemeBuilder.preview(this.modifications['line-height']);
  $(target).val(value);
};

/**
 * Called when the size slider has been dismissed and the change should be
 * applied.
 *
 * @param {jQuerySlider} slider
 *   The slider instance.
 * @param {DomEvent} event
 *   The event associated with sliding the thumb of the slider.
 * @param {String} value
 *   The new value
 * @param {DomElement} target
 *   The target element that will be
 */
ThemeBuilder.styles.FontEditor.prototype.heightChanged = function (sizer, event, value, target) {
  this.propertyChanged('line-height', value + '%');
};

/**
 * Called when the user enters a value in the size field.
 *
 * @param {DomEvent} event
 *   The event that represents the change.
 */
ThemeBuilder.styles.FontEditor.prototype.heightFieldChanged = function (event) {
  var $ = jQuery;
  var value = $('#style-line-height').val();
  if (ThemeBuilder.util.isNumeric(value)) {
    value += '%';
  }
  this.propertyChanged('line-height', value);
};

/**
 * Causes the slider that adjusts the font size to be displayed.
 *
 * @param {jQuerySlider} slider
 *   The slider instance.
 * @param {DomElement} target
 *   The target element that will be
 */
ThemeBuilder.styles.FontEditor.prototype.showSpacingSlider = function (slider, target) {
  var $ = jQuery;
  if ($('#element-to-edit .theelement').html() === 'no element selected') {
    return false;
  }
  var val = $(target).val();
  if (!ThemeBuilder.util.isNumeric(val)) {
    val = '0';
  }
  $(target).focus();
  slider.slider.slider('value', val);
};

/**
 * Called when the kerning slider has moved and the change should be displayed.
 *
 * @param {jQuerySlider} slider
 *   The slider instance.
 * @param {DomEvent} event
 *   The event associated with sliding the thumb of the slider.
 * @param {String} value
 *   The new value
 * @param {DomElement} target
 *   The target element that will be
 */
ThemeBuilder.styles.FontEditor.prototype.spacingPreview = function (sizer, event, value, target) {
  var $ = jQuery;
  var valueString = value;
  if (ThemeBuilder.util.isNumeric(value)) {
    valueString = value + 'px';
  }
  else {
    // The default value is 0px.  Use this when the value entered is 'normal'.
    value = '0';
    valueString = value + 'px';
  }
  this.modifications['letter-spacing'].setNewState('letter-spacing',
    valueString);
  ThemeBuilder.preview(this.modifications['letter-spacing']);
  $(target).val(value);
};

/**
 * Called when the kerning slider has been dismissed and the change should be
 * applied.
 *
 * @param {jQuerySlider} slider
 *   The slider instance.
 * @param {DomEvent} event
 *   The event associated with sliding the thumb of the slider.
 * @param {String} value
 *   The new value
 * @param {DomElement} target
 *   The target element that will be
 */
ThemeBuilder.styles.FontEditor.prototype.spacingChanged = function (sizer, event, value, target) {
  this.propertyChanged('letter-spacing', value + 'px');
};

/**
 * Called when the user enters a value in the kerning field.
 *
 * @param {DomEvent} event
 *   The event that represents the change.
 */
ThemeBuilder.styles.FontEditor.prototype.spacingFieldChanged = function (event) {
  var $ = jQuery;
  var value = $('#style-letter-spacing').val();
  if (ThemeBuilder.util.isNumeric(value)) {
    value += 'px';
  }
  this.propertyChanged('letter-spacing', value);
};

/**
 * Causes the inputs on this FontEditor instance to be disabled.
 */
ThemeBuilder.styles.FontEditor.prototype.disableInputs = function () {
  var $ = jQuery;
  $('#themebuilder-style-font input, #themebuilder-style-font select').attr('disabled', true);
};

/**
 * Causes the inputs on this FontEditor instance to be enabled.
 */
ThemeBuilder.styles.FontEditor.prototype.enableInputs = function () {
  var $ = jQuery;
  $('#themebuilder-style-font input, #themebuilder-style-font select').attr('disabled', false);
};
