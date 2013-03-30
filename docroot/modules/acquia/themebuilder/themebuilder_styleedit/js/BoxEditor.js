
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true */

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * The BoxEditor class is responsible for the CSS box model editor.
 * @class
 * @extends ThemeBuilder.styles.Editor
 */
ThemeBuilder.styles.BoxEditor = ThemeBuilder.initClass();
ThemeBuilder.styles.BoxEditor.prototype = new ThemeBuilder.styles.Editor();

ThemeBuilder.styles.BoxEditor.prototype.initialize = function (elementPicker) {
  this.elementPicker = elementPicker;
  this.tabName = "spacing";
};

/**
 * Refreshes the display.  This should occur when the user selects a
 * new element or when the display changes for some other reason, such
 * as clicking undo or redo.  This method looks at the current set of
 * properties for the selector and makes the property values match.
 */
ThemeBuilder.styles.BoxEditor.prototype.refreshDisplay = function () {
  var selectedElement = this.getSelectedElement();
  var getComputedStyle = ThemeBuilder.styleEditor.getComputedStyleFunction(selectedElement);
  this.modifications = {};
  this.refreshBoxEditor(getComputedStyle);
  this.refreshBorderStyle(getComputedStyle);
  this.refreshBorderColor(getComputedStyle);
  this.refreshElementSize(getComputedStyle);
};

/**
 * Refreshes the display of the box editor.
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 */
ThemeBuilder.styles.BoxEditor.prototype.refreshBoxEditor = function (getComputedStyle) {
  var $ = jQuery;
  var properties = {'margin-%': {id: '#tb-style-margin', fallback: 'margin', type: 'margin'},
    'border-%-width': {id: '#tb-style-border', fallback: 'border-width', type: 'border'},
    'padding-%': {id: '#tb-style-padding', fallback: 'padding', type: 'padding'}};
  var directions = ['top', 'right', 'bottom', 'left'];

  for (var property in properties) {
    if (properties[property].id) {
      for (var index = 0; index < directions.length; index++) {
        var direction = directions[index];
        var propertyName = property.replace(/%/, direction);
        var value = getComputedStyle(propertyName);
        value = parseInt(value, 10);
        if (isNaN(value)) {
          value = 0;
        }
        $(this._getElementForProperty(propertyName)).val(value);
      }
    }
  }
};

/**
 * Refreshes the display of the border style.
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 */
ThemeBuilder.styles.BoxEditor.prototype.refreshBorderStyle = function (getComputedStyle) {
  var $ = jQuery;
  var value = getComputedStyle('border-top-style');
  if (!value) {
    value = 'none';
  }
  $('#style-border-style').val(value);
  var modification = new ThemeBuilder.CssModification(this.currentSelector);
  modification.setPriorState('border-style', value);
  this.modifications['border-style'] = modification;
};

/**
 * Refreshes the display of the border color.
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 */
ThemeBuilder.styles.BoxEditor.prototype.refreshBorderColor = function (getComputedStyle) {
  var style = getComputedStyle('border-top-color');
  this.picker.setIndex(style);
};

/**
 * Refreshes the display of the element size.
 */
ThemeBuilder.styles.BoxEditor.prototype.refreshElementSize = function (getComputedStyle) {
  var $ = jQuery;
  var height = ThemeBuilder.util.parseCssValue(getComputedStyle('height'));
  var width = ThemeBuilder.util.parseCssValue(getComputedStyle('width'));
  if (ThemeBuilder.util.isNumeric(height.number)) {
    height.number = Math.round(height.number);
    height.string = height.number + height.units;
  } else {
    height.number = 'auto';
    height.units = 'px';
    height.string = 'auto';
  }
  if (ThemeBuilder.util.isNumeric(width.number)) {
    width.number = Math.round(width.number);
    width.string = width.number + width.units;
  } else {
    width.number = 'auto';
    width.units = 'px';
    width.string = 'auto';
  }

  $('#style-element-height').val(height.number);
  $('#style-element-height-u').val(height.units);
  $('#style-element-width').val(width.number);
  $('#style-element-width-u').val(width.units);

  var modification = new ThemeBuilder.CssModification(this.currentSelector);
  modification.setPriorState('height', height.string);
  this.modifications.height = modification;
  modification = new ThemeBuilder.CssModification(this.currentSelector);
  modification.setPriorState('width', width.string);
  this.modifications.width = modification;
};

ThemeBuilder.styles.BoxEditor.prototype.attributeTarget = function (target) {
  var $ = jQuery;
  var attribute;
  target = $(target);
  if (target.hasClass('corner')) {
    while (target[0].nodeName !== 'TABLE' && (target = target.parent())) {
    }
    attribute = target[0].id.split('-').slice(2).join('-');
    target = $('input[id*=tb-style-' + attribute + ']', target);
  }
  else {
    if (!target.is('input')) {
      target = target.children('input');
      target.focus();
    }
    attribute = target.attr('id').split('-').slice(2).join('-');
  }
  return {
    target: target,
    attribute: attribute
  };
};

ThemeBuilder.styles.BoxEditor.prototype._highlightBoxProperty = function (event, turnOn) {
  var $ = jQuery;
  var highlightClass = 'hovering';
  var element = $(event.currentTarget).closest('table');
  if (true === turnOn) {
    element.addClass(highlightClass);
  }
  else {
    element.removeClass(highlightClass);
  }
};

/**
 * Indicates whether the current modification is associated with the specified
 * element.  The element should be one of the text fields in the box editor.
 * If the current modification is changing the property associated with the
 * specified element, this method will return true.
 *
 * @param {DomElement} element
 *   The element that represents a text field in the box editor.
 *
 * @return
 *   true if the specified element matches the property currently being
 *   edited; false otherwise.
 */
ThemeBuilder.styles.BoxEditor.prototype._elementMatchesModification = function (element) {
  if (this.currentModification) {
    var info = this.attributeTarget(element);
    var modification = this.textEntryGetModification();
    var priorState = modification.getPriorState();
    if (info.attribute === priorState.property) {
      return true;
    }
  }
  return false;
};

/**
 * Called as the user presses keys while focused in the text fields in the box
 * editor.  This action will cause the new values to be previewed.
 *
 * @param {DomEvent} event
 *   The associated event.
 */
ThemeBuilder.styles.BoxEditor.prototype.textEntryChanged = function (event) {
  if (event.keyCode === 9) {
    // This is the tab key.  The user changed focus using tab.  Don't
    // initialize until the value is actually changed.
    return;
  }
  var element = event.currentTarget;
  if (!this._elementMatchesModification(element)) {
    this.createBoxModification(element);
  }
  var modification = this.textEntryGetModification();
  var property = modification.getPriorState().property;
  var value = element.value;
  if (ThemeBuilder.util.isNumeric(value)) {
    value = value + 'px';
  }
  modification.setNewState(property, value);
  ThemeBuilder.preview(this.currentModification);
};

/**
 * Returns a modification instance that is representative of a change being
 * done to the box editor.  Specifically this would be an instance of a
 * margin, border, or padding size modification (not a color or style
 * modification).
 *
 * @return
 *   A modification instance.
 */
ThemeBuilder.styles.BoxEditor.prototype.textEntryGetModification = function () {
  switch (this.currentModification.getType()) {
  case ThemeBuilder.CssModification.TYPE:
    return this.currentModification;

  case ThemeBuilder.GroupedModification.TYPE:
    for (var childname in this.currentModification.children) {
      if (this.currentModification.children.hasOwnProperty(childname)) {
        var child = this.currentModification.getChild(childname);
        var property = child.getPriorState().property;
        if (property !== 'border-style' || property !== 'border-color') {
          return child;
        }
      }
    }
    break;
  }
};

/**
 * Called when the user commits a value in one of the text fields in the box
 * editor.  This act will apply the modification.
 *
 * @param {DomEvent} event
 *   The associated event.
 */
ThemeBuilder.styles.BoxEditor.prototype.textEntryCommitted = function (event) {
  if (this._elementMatchesModification(event.currentTarget)) {
    if (this.currentModification.hasChanged()) {
      ThemeBuilder.applyModification(this.currentModification);
      delete this.currentModification;
    }
  }
};

/**
 * Called when the slider is displayed.
 *
 * @param {Input slider} slider
 *   The slider instance.
 * @param {DomElement} target
 *   The element the slider acts on.
 */
ThemeBuilder.styles.BoxEditor.prototype.slider_show = function (slider, target) {
  if (this.elementPicker.currentSelector === '#none#') {
    return false;
  }
  var info = this.attributeTarget(target);
  var value = info.target.val();
  slider.set('value', value);
  return true;
};

/**
 * Called when the slider is moved.
 *
 * @param {Input slider} slider
 *   The slider instance.
 * @param {Event} event
 *   The event associated with moving the slider.
 * @param {String} value
 *   The new value.
 * @param {DomElement} target
 *   The element the slider acts on.
 */
ThemeBuilder.styles.BoxEditor.prototype.slider_slide = function (slider, event, value, target) {
  var $ = jQuery;
  var info = this.attributeTarget(target);
  var modValue = value;
  if (ThemeBuilder.util.isNumeric(value)) {
    modValue = value + "px";
  }
  if (!this.currentModification) {
    if ($(target).hasClass('corner')) {
      this.currentModification = this._createCornerModification(info.attribute);
    }
    else {
      this.currentModification = new ThemeBuilder.CssModification(this.currentSelector);
      this.currentModification.setPriorState(info.attribute, modValue);
      this.currentModification.setNewState(info.attribute, modValue);
    }
  }

  this._updateModification(this.currentModification, modValue);
  ThemeBuilder.preview(this.currentModification);
  info.target.val(value);
};

/**
 * Returns the element responsible for displaying or editing the specified property.
 *
 * @param {String} property
 *   The name of the property.
 * @return {DomElement}
 *   The element responsible for editing the specified property.
 */
ThemeBuilder.styles.BoxEditor.prototype._getElementForProperty = function (property) {
  var id = 'tb-style-' + property;
  return document.getElementById(id);
};

/**
 * Returns the css property associated with the specified element.
 *
 * @param {DomElement} element
 *   The element.  Generally this will be an input field.
 * @return {String}
 *   The property associated with the element.
 */
ThemeBuilder.styles.BoxEditor.prototype._getPropertyForElement = function (element) {
  var property = '';
  var id = element.id;
  if (new RegExp('^tb-style-(.)*').test(id)) {
    property = id.slice('tb-style-'.length);
  }
  return property;
};

/**
 * Creates a Modification instance for a theming act initiated from one of the
 * BoxEditor corners.  This will cause all four sides to be modified, but not
 * through the use of the shortcut properties (padding, margin, border-width)
 * because that can cause subsequent theming of a single side to fail,
 * depending on the order of the contents of the custom.css file.
 *
 * Instead, this method creates a GroupedModification that represents four
 * independent changes (one for each side).  This also allows the use to undo
 * to get back to the previous state even if each side had a different border
 * width, for example.
 *
 * @param {String} property
 *   The property name being edited.  Can be one of "padding", "border", or "margin"
 * @return {Modification}
 *   A grouped modification with a child CssModification that represents each
 *   of the 4 sides of the box.  This modification should be updated using the
 *   _updateModification method.
 */
ThemeBuilder.styles.BoxEditor.prototype._createCornerModification = function (property) {
  var childProperties;
  switch (property) {
  case 'padding':
    childProperties = ['padding-top', 'padding-right', 'padding-bottom', 'padding-left'];
    break;
  case 'border':
    childProperties = ['border-top-width', 'border-right-width', 'border-bottom-width', 'border-left-width'];
    break;
  case 'margin':
    childProperties = ['margin-top', 'margin-right', 'margin-bottom', 'margin-left'];
    break;
  default:
    return undefined;
  }
  var modification = new ThemeBuilder.GroupedModification();
  for (var i = 0; i < childProperties.length; i++) {
    var child = new ThemeBuilder.CssModification(this.currentSelector);
    var propertyName = childProperties[i];
    var element = this._getElementForProperty(propertyName);
    var value = element.value;
    if (ThemeBuilder.util.isNumeric(value)) {
      value = value + 'px';
    }
    child.setPriorState(propertyName, value);
    modification.addChild(propertyName, child);
  }
  return modification;
};

/**
 * Updates the specified modification with the specified value.  If the
 * modification is ungrouped, this is the same as calling
 * modification.setNewState(modification.getPriorState().property, value);
 *
 * If the specified modification is a GroupedModification, the value change
 * will apply to all children that are not a border-style or border-color
 * change.  In effect, this changes all modifications that represent sides of
 * the box being edited.
 *
 * @param {Modification} modification
 *   The modification instance to update.
 * @param {String} value
 *   The new value
 */
ThemeBuilder.styles.BoxEditor.prototype._updateModification = function (modification, value) {
  var priorState;
  var child;
  switch (modification.getType()) {
  case ThemeBuilder.CssModification.TYPE:
    priorState = modification.getPriorState();
    modification.setNewState(priorState.property, value);
    break;
  case ThemeBuilder.GroupedModification.TYPE:
    for (var childName in modification.children) {
      if ((child = modification.getChild(childName))) {
        priorState = child.getPriorState();
        if (priorState.property !== 'border-style' && priorState.property !== 'border-color') {
          child.setNewState(priorState.property, value);
        }
      }
    }
    break;
  }
};

/**
 * Creates an appropriate Modification instance for the specified element.
 *
 * The new modification will be set into this.currentModification.
 *
 * @param {DomElement} target
 *   The element in the box UI that was clicked.
 */
ThemeBuilder.styles.BoxEditor.prototype.createBoxModification = function (target) {
  var $ = jQuery;
  var info = this.attributeTarget(target);
  if ($(target).hasClass('corner')) {
    this.currentModification = this._createCornerModification(info.attribute);
  }
  else {
    this.currentModification = new ThemeBuilder.CssModification(this.currentSelector);
    var propertyName = this._getPropertyForElement(info.target[0]);
    var getComputedStyle = ThemeBuilder.styleEditor.getComputedStyleFunction(this.getSelectedElement());
    var value = getComputedStyle(propertyName);
    this.currentModification.setPriorState(info.attribute, value);
    this.currentModification.setNewState(info.attribute, value);
  }

  // Handle border-width, as well as border-{top|right|bottom|left}-width.  If
  // If any part of the border is being modified and the style is set to
  // 'none', create a GroupedModification that includes a border style change
  // and a border width change.  This will cause both attributes to be modified
  // at the same time, and clicking undo will revert both changes.
  if (/^border/.test(info.attribute)) {
    if (jQuery('#style-border-style').val() === 'none') {
      jQuery('#style-border-style').val('solid');
      // Here we are changing both the style and the border width.  In
      // this case a GroupedModification must be used so undo will cause
      // both properties to be reverted at once.
      this.convertToGroupModification(info.attribute);
      var border = this.getNamedModification('border-style');
      if (!border) {
        // Add a modification that changes the border style.
        border = new ThemeBuilder.CssModification(this.currentSelector);
        border.setPriorState('border-style', 'none');
        border.setNewState('border-style', 'solid');
        this.currentModification.addChild('border-style', border);
      }
    }
  }
};

/**
 * Called when the slider is started.
 *
 * @param {Input slider} slider
 *   The slider instance.
 * @param {Event} event
 *   The event associated with moving the slider.
 * @param {String} value
 *   The starting value.
 * @param {DomElement} target
 *   The element the slider acts on.
 */
ThemeBuilder.styles.BoxEditor.prototype.slider_start = function (slider, event, value, target) {
  this.createBoxModification(event.currentTarget);
  return ThemeBuilder.util.stopEvent(event);
};

/**
 * Called when the slider is stopped.
 *
 * @param {Input slider} slider
 *   The slider instance.
 * @param {Event} event
 *   The event associated with moving the slider.
 * @param {String} value
 *   The final value.
 * @param {DomElement} target
 *   The element the slider acts on.
 */
ThemeBuilder.styles.BoxEditor.prototype.slider_stop = function (slider, event, value, target) {
  if (!this.currentModification) {
    return;
  }
  this._updateModification(this.currentModification, value + 'px');
  if (this.currentModification.getType() === ThemeBuilder.GroupedModification.TYPE ||
      this.currentModification.hasChanged()) {
    ThemeBuilder.applyModification(this.currentModification);
    this.currentModification = undefined;
  }
  this.attributeTarget(target).target.focus();
};

/**
 * This method sets up the tab.
 */
ThemeBuilder.styles.BoxEditor.prototype.setupTab = function () {
  var $ = jQuery;

  // Highlight the entire style if hovering over a corner element.  This applies
  // to margin, border, and padding.
  $('#themebuilder-style-spacing td.corner').
    hover(ThemeBuilder.bind(this, this._highlightBoxProperty, true),
      ThemeBuilder.bind(this, this._highlightBoxProperty, false)
    );

  $('#themebuilder-style-spacing td.corner,#themebuilder-style-spacing td.side,').inputslider({
    min: 0,
    max: 99,
    step: 1,
    autofocus: false,
    onShow: ThemeBuilder.bind(this, this.slider_show),
    onSlide: ThemeBuilder.bind(this, this.slider_slide),
    onStart: ThemeBuilder.bind(this, this.slider_start),
    onStop: ThemeBuilder.bind(this, this.slider_stop)
  });

  $('#themebuilder-style-spacing td input')
  .focusout(ThemeBuilder.bind(this, this.textEntryCommitted))
  .keyup(ThemeBuilder.bind(this, this.textEntryChanged))
  .change(ThemeBuilder.bind(this, this.textEntryCommitted));
  $('#style-border-style').change(ThemeBuilder.bind(this, this.borderStyleChanged));

  this.picker = new ThemeBuilder.styles.PalettePicker($('#style-border-color'), 'border-color', $('#themebuilder-wrapper', parent.document));

  var options = {
    min: 0,
    max: 1100,
    step: 1,
    onSlide: ThemeBuilder.bind(this, this.sizePreview),
    onStop: ThemeBuilder.bind(this, this.sizeSliderStop),
    onShow: ThemeBuilder.bind(this, this.showSizeSlider)
  };
  $('#style-element-width').inputslider(options);
  options.max = 400;
  $('#style-element-height').inputslider(options);
  $('#style-element-width, #style-element-width-u, #style-element-height, #style-element-height-u').change(ThemeBuilder.bind(this, this.sizeChanged));

};

/**
 * Initialize the element size slider.
 *
 * @param {Input slider} slider
 *   The slider instance.
 * @param {DomElement} target
 *   The element the slider is attached to.
 */
ThemeBuilder.styles.BoxEditor.prototype.showSizeSlider = function (slider, target) {
  var $ = jQuery;
  var val = $(target).val();
  if (!ThemeBuilder.util.isNumeric(val)) {
    val = '';
  }
  $(target).focus();
  slider.slider.slider('value', val);
};


/**
 * Called when a size slider has moved and the change should be displayed.
 *
 * @param {jQuerySlider} slider
 *   The slider instance.
 * @param {DomEvent} event
 *   The event associated with sliding the thumb of the slider.
 * @param {String} value
 *   The new value
 * @param {DomElement} target
 *   The target element.
 */
ThemeBuilder.styles.BoxEditor.prototype.sizePreview = function (sizer, event, value, target) {
  var $ = jQuery;
  var property = this._getSizeProperty(target.id);
  this.modifications[property].setNewState(property,
    value + $('#style-element-' + property + '-u').val());
  ThemeBuilder.preview(this.modifications[property]);
  $(target).val(value);
};

/**
 * Helper function to get the CSS property from one of the size sliders.
 */
ThemeBuilder.styles.BoxEditor.prototype._getSizeProperty = function (id) {
  var property;
  switch (id) {
  case 'style-element-width':
  case 'style-element-width-u':
    property = 'width';
    break;

  case 'style-element-height':
  case 'style-element-height-u':
    property = 'height';
    break;
  }
  return property;
};

/**
 * React to one of the size sliders stopping.
 */
ThemeBuilder.styles.BoxEditor.prototype.sizeSliderStop = function (sizer, event, value, target) {
  var $ = jQuery;
  // Trigger a change event on the input element.
  $(target).change();
};

/**
 * React to the element height or width UI elements being changed.
 */
ThemeBuilder.styles.BoxEditor.prototype.sizeChanged = function (event) {
  if (event && event.currentTarget) {
    var $ = jQuery;
    var property = this._getSizeProperty(event.currentTarget.id);
    var value = $('#style-element-' + property).val();
    if (value === 'auto') {
      value = 'auto';
    }
    else {
      var units = $('#style-element-' + property + '-u').val();
      value = value + units;
    }
    var modification = this.modifications[property];
    modification.setNewState(property, value);
    ThemeBuilder.applyModification(modification);
    this.modifications[property] = modification.getFreshModification();
    this.currentModification = undefined;
  }
};

/**
 * Called when the user selects a different border style.  This
 * function is responsible for causing the change to take effect.
 *
 * @param {DomEvent} event
 *   The event that contains the newly selected value.
 */
ThemeBuilder.styles.BoxEditor.prototype.borderStyleChanged = function (event) {
  var property = 'border-style';
  if (event && event.currentTarget) {
    var value = event.currentTarget.value;
    if (value === 'auto') {
      value = '';
    }
    var modification = this.modifications[property];
    modification.setNewState(property, value);
    ThemeBuilder.applyModification(modification);
    this.modifications[property] = modification.getFreshModification();
    this.currentModification = undefined;
  }
};

/**
 * This method is called by loadSelection when the user selects an element
 * or an item in the option control.  Here we initialize the spacing tab.
 *
 * @param {String} selector
 *   The new selector.
 */
ThemeBuilder.styles.BoxEditor.prototype.selectorChanged = function (selector) {
  var $ = jQuery;
  $('#themebuilder-style-spacing input').val('0');
  $('#themebuilder-style-spacing option').eq(0).attr('selected', true);

  this.currentSelector = selector;
  this.picker.setSelector(selector);
  this.refreshDisplay();
};

/**
 * Converts the current modification to a grouped modification.  This is
 * useful when modifying one attribute requires the modification of another
 * attribute simultaneously.  For example, changing the border width might
 * require that the style be set.
 *
 * If the current modification is already of type GroupModification, no
 * action is taken.
 *
 * @param {String} childName
 *   The name used to reference the current modification within the new group.
 */
ThemeBuilder.styles.BoxEditor.prototype.convertToGroupModification = function (childName) {
  if (!this.currentModification) {
    throw Drupal.t('The current modification has not yet been set.');
  }
  if (this.currentModification.getType() !== ThemeBuilder.GroupedModification.TYPE) {
    // Convert the current modification into a grouped modification.
    var group = new ThemeBuilder.GroupedModification();
    group.addChild(childName, this.currentModification);
    this.currentModification = group;
  }
};

/**
 * Returns the named modification if the current modification is a group.
 * Otherwise the current modification is returned.
 *
 * @param {String} childName
 *   The name of the child within the group.
 * @return
 *   The named child if the current modification is a GroupedModification,
 *   or the current modification otherwise.
 */
ThemeBuilder.styles.BoxEditor.prototype.getNamedModification = function (childName) {
  if (!this.currentModification) {
    throw Drupal.t('The current modification has not yet been set.');
  }
  if (this.currentModification.getType() === ThemeBuilder.GroupedModification.TYPE) {
    return this.currentModification.getChild(childName);
  }
  return this.currentModification;
};
