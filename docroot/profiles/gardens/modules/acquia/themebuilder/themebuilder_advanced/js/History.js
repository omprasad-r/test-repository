/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true window: true*/

var ThemeBuilder = ThemeBuilder || {};

/**
 * @class
 */
ThemeBuilder.History = ThemeBuilder.initClass();

/**
 * Returns the only instance of the History class.
 * 
 * The History class is implemented as a singleton because it operates
 * on a particular DOM element which is referenced by ID, and as such
 * there can only be one.
 */
ThemeBuilder.History.getInstance = function () {
  if (ThemeBuilder.History._instance) {
    return ThemeBuilder.History._instance;
  }
  return new ThemeBuilder.History();
};

/**
 * Constructor for the History class.
 */
ThemeBuilder.History.prototype.initialize = function () {
  if (ThemeBuilder.History._instance) {
    throw 'ThemeBuilder.History is a singleton that has already been instantiated.';
  }
  ThemeBuilder.History._instance = this;
  var $ = jQuery;
  this.customCount = 0;
  this.hiddenCount = 0;
  if ($('#themebuilder-wrapper').length > 0) {
    $('#themebuilder-wrapper').bind('css-history-contents-changed', ThemeBuilder.bind(this, this.contentsChanged));
  }
  this.domNavigator = new ThemeBuilder.styles.PowerNavigator();
  this.domNavigator.advanced = false;
  this.hiddenModifications = {};
};

/**
 * Initializes the History object.  This entails getting the custom
 * styles to populate the display.
 */
ThemeBuilder.History.prototype.init = function () {
  if (!this.customStyleManager) {
    this.customStyleManager = ThemeBuilder.CustomStyleManager.getInstance();
    this.customStyleManager.requestCustomStyles();
  }
};

/**
 * Called when the History panel is shown.
 */
ThemeBuilder.History.prototype.show = function () {
  var $ = jQuery;
  $('#themebuilder-wrapper #themebuilder-advanced .palette-cheatsheet').addClass('hidden');
  setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this, this.highlight, ThemeBuilder.util.getSelector()), 500);
};

/**
 * Called when the History panel is hidden.
 * 
 * If the user moves off of this tab with hidden properties, this
 * method prompts them to show or delete those properties.
 * 
 * @return {boolean}
 *   true, indicating it is legal to move off of this tab.
 */
ThemeBuilder.History.prototype.hide = function () {
  var $ = jQuery;
  this.domNavigator.unhighlightSelection();
  var count = this.hiddenCount;
  if (count > 0) {
    var message = Drupal.formatPlural(count,
      "You left 1 attribute hidden.  Select 'OK' to delete it so that your theme remains as shown.",
      "You left @count attributes hidden. Select 'OK' to delete them so that your theme remains as shown.",
      {'@count': count});
    if (confirm(message)) {
      this.deleteAllHidden();
    }
    else {
      this.showAll();
    }
  }
  return true;
};

/**
 * Called when the contents of the history tab are loaded.
 */
ThemeBuilder.History.prototype.loaded = function () {
  var $ = jQuery;
  $('#history-show-all').click(ThemeBuilder.bind(this, this.showAll));
  $('#history-hide-all').click(ThemeBuilder.bind(this, this.hideAll));
  $('#history-delete-all-hidden').click(ThemeBuilder.bind(this, this.deleteAllHidden));
};

/**
 * This callback is called any time the history contents are
 * refreshed.  This occurs when the page is loaded and any time new
 * properties are added or deleted.
 *
 * @param {Event} event
 *   The event associated with the trigger.
 * @param {CustomStyles} styles
 *   The custom styles.
 */
ThemeBuilder.History.prototype.contentsChanged = function (event, styles) {
  var $ = jQuery;
  this.styles = styles;
  var id, selector, properties, property, $propertyRow, $selectorRow;
  var $table = $('#css-history table.body');
  if ($table.length !== 1) {
    // The history contents have arrived before the markup for the
    // advanced tab has been added to the DOM.
    setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this, this.contentsChanged, event, styles), 50);
    return;
  }

  if (!this.contentsClicked) {
    // This will only be executed once.
    this.contentsClicked = ThemeBuilder.bind(this, this._contentsClicked);
    $table.bind('click', this.contentsClicked);
  }

  // When rewriting the table contents, we insert a marker row that
  // represents the current insertion point.  We don't simply clear
  // the table and start over because the status of each row will be
  // lost (ie. whether it is being hidden or not).  We instead reuse
  // each row, moving this marker row as we go, and at the end remove
  // the marker row and all subsequent rows.
  var $markerRow = $('<tr id="history-current-row">');
  if ($table.children().first().size() === 0) {
    // Need to append the marker
    $markerRow.appendTo($table);
  }
  else {
    // Need to insert the marker
    $markerRow.insertBefore($table.children().first());
  }

  // Clear the custom count; it will be incremented as each property is added.
  this.customCount = 0;

  // Add the new contents.
  var selectors = styles.getIterator();
  var $row;
  while (selectors.hasNext()) {
    var selectorData = selectors.next();
    id = ThemeBuilder.util.getSafeClassName(selectorData.selector);
    $row = $('#selector-' + id);
    if ($row.size() === 1) {
      $row.insertBefore($markerRow);
    }
    else {
      var row =
        '<tr class="history-table-row history-selector-row" id="selector-' + id + '">' +
	'  <th title="' + Drupal.t('Highlight the CSS selector') + '" class="history-selector history-element-text">' + selectorData.selector + '</th>' +
	'  <td class="operations">' +
	'    <div title="' + Drupal.t('Delete all attributes for this CSS selector') + '" class="history-operation history-delete">' + Drupal.t('delete') + '</div>' +
	'    <div title="' + Drupal.t('Hide all attributes for this CSS selector') + '" class="history-operation history-hide">' + Drupal.t('hide') + '</div>' +
	'    <div title="' + Drupal.t('Show all attributes for this CSS selector') + '" class="history-operation history-show">' + Drupal.t('show') + '</div>' +
	'  </td>' +
	'</tr>';

      $(row).insertBefore($markerRow);
    }
    this.renderProperties(selectorData.selector, selectorData.properties);
  }

  // Remove the marker row and all subsequent rows (these represent
  // deleted properties and selectors).
  $markerRow.nextAll().remove();
  $markerRow.remove();

  // If there is no current selection, change the title of the history panel.
  if ($('.history-selected').size() === 0) {
    $('#history-title-selector').text('');
  }

  this.fixSelectorOperations();
  this.fixGlobalButtons();
};

/**
 * Renders the specified properties.
 * 
 * @param {String} selector
 *   The CSS selector the properties are associated with.
 * @param {Iterator} properties
 *   The iterator used to visit an ordered list of the properties associated with a CSS selector.
 */
ThemeBuilder.History.prototype.renderProperties = function (selector, properties) {
  var $ = jQuery;
  var id, property, $propertyRow;
  var $markerRow = $('#history-current-row');
  var colorManager = ThemeBuilder.getColorManager();
  var palette = (colorManager.isInitialized() ? colorManager.getPalette() : undefined);
  while (properties.hasNext()) {
    property = properties.next();
    if (palette && property.name.indexOf('color') !== -1) {
      var colorIndex = colorManager.cleanIndex(property.value);
      if (colorManager.isValidIndex(colorIndex)) {
        var newColor = palette.paletteIndexToHex(colorIndex);
        if (newColor !== false) {
          property.value = '#' + newColor;
        }
        else if (colorManager.custom) {
          property.value = '#' + colorManager.custom.paletteIndexToHex(colorIndex);
        }
      }
    }
    id = ThemeBuilder.util.getSafeClassName(selector + '-' + property.name);
    var $e = $('#selector-' + id);
    if ($e.size() === 1) {
      $e.find('.history-property-value').text(property.value);
      $e.insertBefore($markerRow);
      this.customCount++;
    }
    else {
      var row = 
	'<tr class="history-table-row history-property" id="selector-' + id + '">' +
	'  <td><span class="history-property-name history-element-text">' + property.name + '</span><span class="history-separator history-element-text">: </span><span class="history-property-value history-element-text">' + property.value + '</span></td>' +
	'  <td class="operations">' +
	'    <div title="' + Drupal.t('Delete this attribute') + '" class="history-operation history-delete">' + Drupal.t('delete') + '</div>' +
	'    <div title="' + Drupal.t('Hide this attribute') + '" class="history-operation history-hide">' + Drupal.t('hide') + '</div>' +
	'  </td>' +
	'</tr>';
    
      $(row).insertBefore($markerRow);
      this.customCount++;
    }
  }
};

/**
 * The click event handler.
 * 
 * There is only one click event handler for the entire history
 * display.  This is much more efficient than attaching event handlers
 * to every interesting DOM element and cleaning up those handlers
 * when we clear the table.
 * 
 * This click event handler determines what the user's intentions are
 * based on the particular element that was clicked, and causes the
 * corresponding changes to occur.
 * 
 * @param {Event} event
 *   The event associated with the user's click.
 */
ThemeBuilder.History.prototype._contentsClicked = function (event) {
  var $ = jQuery;
  var action = this.interpretEvent(event);
  var id = this.getActionId(action);
  var modification, $target;
  switch (action.action) {
  case 'delete':
    modification = this.buildModification(action);
    if (modification) {
      ThemeBuilder.applyModification(modification);
    }
    this.removeHiddenModification(modification);
    // The custom count is recalculated after delete.
    break;

  case 'hide':
    modification = this.buildModification(action);
    if (modification) {
      ThemeBuilder.preview(modification);
      this.disableRows(this.rowHideGetAffectedRows(id));
      this.addHiddenModification(modification);
      this.fixSelectorOperations();
      this.fixGlobalButtons();
    }
    break;

  case 'show':
    modification = this.getHiddenModification(action.selector, action.property);
    if (modification) {
      ThemeBuilder.preview(modification, false);
      this.enableRows(this.rowHideGetAffectedRows(id));
      this.removeHiddenModification(modification);
      this.fixSelectorOperations();
      this.fixGlobalButtons();
    }
    break;

  case 'highlight':
    ThemeBuilder.util.setSelector(action.selector);
    this.highlight(action.selector);
    break;

  default:
  }
};

/**
 * Returns an object that indicates what action should be taken for the specified event within the CSS history display.
 * 
 * @param {Event} event
 *   The click event.
 * @return {Object}
 *   An object that indicates the action, the selector, property, and
 *   value.  Each of these is determined from the DOM element that the
 *   user clicked within the history view.
 */
ThemeBuilder.History.prototype.interpretEvent = function (event) {
  var $ = jQuery;
  var result = {
    action: 'none'
  };

  var $target = $(event.target);
  if ($target.hasClass('history-selector') || $target.hasClass('history-selector-row')) {
    // this is a selector.
    result.action = 'highlight';
    result.target = 'selector';
    if ($target.hasClass('history-selector-row')) {
      result.selector = $target.find('.history-selector').text();
    }
    else {
      result.selector = $target.text();
    }
  }
  else {
    var $row = $target.closest('.history-table-row');
    if ($row.hasClass('history-property')) {
      // This is a property
      result.property = $row.find('.history-property-name').text();
      result.value = $row.find('.history-property-value').text();
      result.selector = $row.closest('tr').prevAll('.history-selector-row').first().find('.history-selector').text();
    }
    else if ($row.hasClass('history-selector-row')) {
      // this is a selector
      result.selector = $row.find('.history-selector').text();
    }

    // Figure out what the requested operation was
    if ($target.hasClass('history-hide')) {
      // Hide
      result.action = 'hide';
    }
    else if ($target.hasClass('history-show')) {
      // Show
      result.action = 'show';
    }
    else if ($target.hasClass('history-delete')) {
      // delete;
      result.action = 'delete';
    }
    else {
      // Selected a property row;
      result.action = 'highlight';
      result.target = 'selector';
      delete result.property;
    }
  }
  return result;
};

/**
 * Returns the ID associated with the selector and optional property within the specified action object.
 * 
 * @param {Object} action
 *   The object that represents the action the user wishes to take,
 *   based on a click event within the history table.
 * @return {String}
 *   The element id.
 */
ThemeBuilder.History.prototype.getActionId = function (action) {
  var name = 'selector-' + action.selector;
  if (action.property) {
    name += '-' + action.property;
  }
  name = ThemeBuilder.util.getSafeClassName(name);
  return name;
};

/**
 * Creates a Modification instance based on the specified action.
 * 
 * The modification can be applied or reverted and represents the
 * deletion of one or more CSS properties from the custom.css file.
 * This is done using a Modification instance so these changes will
 * work with undo and redo.
 * 
 * @param {Object} action
 *   An object representing the action to be taken.  This is obtained
 *   by passing a history click event into this.interpretEvent.
 * @return {Modification}
 *   The modificaton.
 */
ThemeBuilder.History.prototype.buildModification = function (action) {
  if (!action.property) {
    // A selector was chosen.
    return this.buildGroupedModification(action.selector);
  }
  var modification = new ThemeBuilder.CssModification(action.selector);
  modification.setPriorState(action.property, action.value); // TODO: What about resources?
  modification.setNewState(action.property, '');
  return modification;
};

/**
 * Disables the specified rows in the history view.
 * 
 * A disabled row is on for which the user clicked the 'hide' button.
 * The custom css still exists in the theme files, but has been
 * removed from the browser's stylesheets so the user can see what the
 * theme looks like without the style(s).
 * 
 * @param {Array} $rows
 *   An array of jQuery objects, each of which represents a row in the
 *   history view that should be disabled.
 */
ThemeBuilder.History.prototype.disableRows = function ($rows) {
  for (var i = 0, len = $rows.length; i < len; i++) {
    this.disableRow($rows[i]);
  }
};

/**
 * Disables the specified row in the history view.
 * 
 * @param {jQuery} $row
 *   A jQuery object which represents a non-hidden row in the history view
 *   that should be hidden.
 */
ThemeBuilder.History.prototype.disableRow = function ($row) {
  var $ = jQuery;
  if (!$row.filter) {
    $row = $($row);
  }
  $row.addClass('hide');

  // If this is a property row, change hide to show.
  if ($row.hasClass('history-property')) {
    $row.find('.history-hide')
      .removeClass('history-hide')
      .addClass('history-show')
      .text(Drupal.t('show'))
      .attr('title', Drupal.t('Show this attribute'));
  }
};

/**
 * Adds the specified modification to the list of hidden modifications maintained by this History instance.
 * 
 * If the modification is a GroupedModification, all child
 * modifications will be added to the list.
 * 
 * @param {Modification} modification
 *   The modification to add to the list of hidden modifications.
 */
ThemeBuilder.History.prototype.addHiddenModification = function (modification) {
  if (modification instanceof ThemeBuilder.GroupedModification) {
    var children = modification.getChildren();
    for (var attribute in children) {
      if (children.hasOwnProperty(attribute)) {
        this.addHiddenModification(children[attribute]);
      }
    }
    return;
  }
  var priorState = modification.getPriorState();
  var selector = priorState.selector;
  var property = priorState.property;
  if (!this.hiddenModifications[selector]) {
    this.hiddenModifications[selector] = {};
  }
  if (!this.hiddenModifications[property]) {
    this.hiddenCount++;
  }
  this.hiddenModifications[selector][property] = modification;
};

/**
 * Removes the specified modification from the list of hidden modifications maintained by this History instance.
 * 
 * If the modification is a GroupedModification, all child
 * modifications will be removed from the list.
 * 
 * @param {Modification} modification
 *   The modification to remove from the list of hidden modifications.
 */
ThemeBuilder.History.prototype.removeHiddenModification = function (modification) {
  if (modification instanceof ThemeBuilder.GroupedModification) {
    var children = modification.getChildren();
    for (var attribute in children) {
      if (children.hasOwnProperty(attribute)) {
        this.removeHiddenModification(children[attribute]);
      }
    }
    return;
  }
  var priorState = modification.getPriorState();
  var selector = priorState.selector;
  var property = priorState.property;
  var $ = jQuery;
  if (this.hiddenModifications[selector] &&
      this.hiddenModifications[selector][property]) {
    delete this.hiddenModifications[selector][property];
    this.hiddenCount--;
    if ($.isEmptyObject(this.hiddenModifications[selector])) {
      delete this.hiddenModifications[selector];
    }
  }
};

/**
 * Determines which rows should be affected in the CSS History display if the property associated with the specified row id is hidden.
 * 
 * This method returns an array of jQuery objects, each of which
 * should be marked as hidden on the display when the row associated with the
 * specified id is deleted.
 * 
 * @param {String} id
 *   The element id associated with a row being hidden.
 * @return {Array}
 *   An array of jQuery objects, each of which represents a row in the
 *   CSS History table that should be marked as hidden.
 */
ThemeBuilder.History.prototype.rowHideGetAffectedRows = function (id) {
  var $ = jQuery;
  var $row, $property;
  var result = [];
  $row = $('#' + id);
  result.push($row);
  if ($row.hasClass('history-selector-row')) {
    // This is a selector.  Make sure to hide all of the properties.
    $property = $row.next();
    while ($property.length > 0 && $property.hasClass('history-property')) {
      result.push($property);
      $property = $property.next();
    }
  }
  return result;
};

/**
 * Returns a single modification that represents the hidden modification filtered by the specified selector and property.
 * 
 * If no property is provided, a GroupedModification will be created
 * that contains all currently hidden properties for the specified
 * selector.
 * 
 * If no selector is provided, a GroupedModification will be created
 * that contains all currently hidden properties for all selectors.
 * 
 * @param {String} selector
 *   An optional parameter that identifies the selector, which is used
 *   to filter the hidden modifications that are returned.
 * @param {String} property
 *   An optional parameter that identifies the property, which is used
 *   to return a specific modification using the specified selector
 *   and property.  In this case the result will not be a
 *   GroupedModification.
 * @return {Modification}
 *   A Modification instance that represents all currently hidden
 *   attributes filtered by the optional selector and property.
 */
ThemeBuilder.History.prototype.getHiddenModification = function (selector, property) {
  var $ = jQuery;
  var modification;
  if (!$.isEmptyObject(this.hiddenModifications)) {
    if (!property) {
      var selectors = [];
      var s;
      if (!selector) {
	// We want a modification that will apply to all hidden items.
        for (s in this.hiddenModifications) {
          if (this.hiddenModifications.hasOwnProperty(s)) {
            selectors.push(s);
          }
        }
      }
      else {
        selectors.push(selector);
      }
      // We will generate a grouped modification that includes all
      // currently hidden properties under the specified selector.
      modification = new ThemeBuilder.GroupedModification();
      for (var i = 0, len = selectors.length; i < len; i++) {
        if (this.hiddenModifications.hasOwnProperty(selectors[i])) {
          for (var attribute in this.hiddenModifications[selectors[i]]) {
            if (this.hiddenModifications[selectors[i]].hasOwnProperty(attribute)) {
              modification.addChild(selectors[i] + '-' + attribute, this.hiddenModifications[selectors[i]][attribute]);
            }
          }
        }
      }
    }
    else {
      modification = this.hiddenModifications[selector][property];
    }
  }
  return modification;
};

/**
 * Gets the selector rows in sync with the state of their associated property rows.
 * 
 * This method is responsible for managing the display of the
 * hide/show controls on the selector row and for showing the selector
 * is disabled or enabled.
 */
ThemeBuilder.History.prototype.fixSelectorOperations = function () {
  var $ = jQuery;
  var $selectorRow;
  var $rows;
  var hiddenRows;
  var $selectorRows = $('.history-selector-row');
  // When the table has several hundred rows, this loop can affect performance.
  // Use css() instead of hide() and show() to avoid performance problems in
  // Webkit.
  for (var i = 0, len = $selectorRows.size(); i < len; i++) {
    $selectorRow = $selectorRows.eq(i);
    $rows = $selectorRow.nextUntil('.history-selector-row');
    hiddenRows = $rows.filter('.hide').size();
    if (hiddenRows === 0) {
      // Show should not be available.
      $selectorRow.find('.history-show').css('display', 'none')
        .removeClass('hide');
    }
    else {
      $selectorRow.find('.history-show').css('display', 'block');
    }
    if (hiddenRows === $rows.size()) {
      // hide should be disabled.
      $selectorRow
        .addClass('hide')
	.find('.history-hide').css('display', 'none');
    }
    else {
      $selectorRow
	.removeClass('hide')
        .find('.history-hide').css('display', 'block');
    }
  }
};

ThemeBuilder.History.prototype.fixGlobalButtons = function () {
  var $ = jQuery;
  var $showButton = $('#history-show-all');
  var $hideButton = $('#history-hide-all');
  var $deleteButton = $('#history-delete-all-hidden');
  if (this.hiddenCount < this.customCount) {
    // There are some properties that are showing.  Ok to show the
    // hide button.
    $hideButton.removeClass('disabled');
  }
  else {
    // Disable the hide button.
    $hideButton.addClass('disabled');
  }
  if (this.hiddenCount > 0) {
    // There are some properties that are hidden.  Ok to show the show
    // and delete buttons.
    $showButton.removeClass('disabled');
    $deleteButton.removeClass('disabled');
  }
  else {
    // Disable the show and delete buttons.
    $showButton.addClass('disabled');
    $deleteButton.addClass('disabled');
  }
};

/**
 * Highlights the elements in the DOM associated with the specified selector, but only if it is present in custom.css.
 * 
 * @param {String} selector
 *   The selector
 */
ThemeBuilder.History.prototype.highlight = function (selector) {
  var $ = jQuery;
  var id = this.getActionId({selector: selector});
  var $rows = this.rowHideGetAffectedRows(id);
  $('#themebuilder-wrapper .history-table-row.history-selected').removeClass('history-selected');
  if (id && $('#' + id).size() > 0) {
    this.domNavigator.highlightSelection(selector);
    $('#' + id).addClass('history-selected');
    for (var i = 0, len = $rows.length; i < len; i++) {
      $rows[i].addClass('history-selected');
    }
    $('#history-title-selector').text(selector);
  }
  else {
    this.domNavigator.unhighlightSelection();
    $('#history-title-selector').text('');
  }
};

/**
 * Hides all custom CSS.
 * 
 * Hides every custom css property.  The difference between hide and
 * disable is that hide applies to the actual css property and causes
 * the associated style rule to be removed from the stylesheet, while
 * disable causes the associated row to be themed so it looks like the
 * associated style is disabled.
 */
ThemeBuilder.History.prototype.hideAll = function () {
  var $ = jQuery;

  // Create a modification for every custom property that is not currently hidden.
  var modification = new ThemeBuilder.GroupedModification();
  var children = [];
  var property;
  var selectors = this.styles.getIterator();
  while (selectors.hasNext()) {
    var selectorData = selectors.next();
    var selector = selectorData.selector;
    var properties = selectorData.properties;
    while (properties.hasNext()) {
      property = properties.next();
      if (!this.hiddenModifications[selector] || !this.hiddenModifications[selector][property.name]) {
	// The attribute is not already hidden, so add it.
        modification.addChild(selector + '-' + property.name, this.buildModification({selector: selector, property: property.name, value: property.value}));
      }
    }
  }

  // Cause the custom css to be hidden.
  ThemeBuilder.preview(modification);
  this.addHiddenModification(modification);

  // Display all rows as hidden
  this.disableRows($('.history-table-row'));
  this.fixSelectorOperations();
  this.fixGlobalButtons();
};

/**
 * Enables all currently hidden rows in the history panel.
 */
ThemeBuilder.History.prototype.enableAllRows = function () {
  var $ = jQuery;
  var $rows = $('#themebuilder-wrapper .history-table-row.hide');
  for (var i = 0, len = $rows.size(); i < len; i++) {
    this.enableRow($($rows[i]));
  }
};

/**
 * Enables the specified rows in the history view.
 * 
 * @param {Array} $rows
 *   An array of jQuery objects, each of which represents a row in the
 *   history view that should be enabled.
 */
ThemeBuilder.History.prototype.enableRows = function ($rows) {
  for (var i = 0, len = $rows.length; i < len; i++) {
    this.enableRow($rows[i]);
  }
};

/**
 * Enables the specified row in the history view.
 * 
 * @param {jQuery} $row
 *   A jQuery object which represents a hidden row in the history view
 *   that should be enabled.
 */
ThemeBuilder.History.prototype.enableRow = function ($row) {
  $row.removeClass('hide');

  // If this is a property row, change show to hide.
  if ($row.hasClass('history-property')) {
    $row.find('.history-show')
      .removeClass('history-show')
      .addClass('history-hide')
      .text(Drupal.t('hide'))
      .attr('title', Drupal.t('Show this attribute'));
  }
};

/**
 * Shows all currently hidden custom css properties and enables the associated rows.
 */
ThemeBuilder.History.prototype.showAll = function () {
  var modification = this.getHiddenModification();
  if (modification) {
    ThemeBuilder.preview(modification, false);
    this.removeHiddenModification(modification);

    // Show the rows again by removing the hide
    this.enableAllRows();
    this.fixSelectorOperations();
    this.fixGlobalButtons();
  }
};

/**
 * Deletes all currently hidden custom CSS properties.
 */
ThemeBuilder.History.prototype.deleteAllHidden = function (event) {
  event.preventDefault();
  var modification = this.getHiddenModification();
  if (modification) {
    ThemeBuilder.applyModification(modification);
    this.removeHiddenModification(modification);
    this.fixSelectorOperations();
    this.fixGlobalButtons();
    // Note that the row display will be fixed when the modfication is processed.
  }
};

/**
 * Removes the specified rows in the history view.
 * 
 * @param {Array} $rows
 *   An array of jQuery objects, each of which represents a row in the
 *   history view that should be removed.
 */
ThemeBuilder.History.prototype.removeRows = function ($rows) {
  for (var i = 0, len = $rows.length; i < len; i++) {
    this.removeRow($rows[i]);
  }
};

/**
 * Removes the specified row in the history view.
 * 
 * @param {Element} $row
 *   A DOM element or a jQuery object representing a row in the
 *   history view that should be removed.
 */
ThemeBuilder.History.prototype.removeRow = function (row) {
  var $ = jQuery;
  var $row = row;
  if (!row.filter) {
    $row = $(row);
  }
  $row.remove();
};

/**
 * Creates a grouped modification that represents the removal of all properties associated with the specified selector.
 *
 * @param {String} selector
 *   The selector.
 * @return {Modification}
 *   The modification that will remove all customized properties
 *   associated with the specified selector.
 */
ThemeBuilder.History.prototype.buildGroupedModification = function (selector) {
  var children = [];
  var property;
  var properties = this.styles.getPropertyIterator(selector);
  var modification = new ThemeBuilder.GroupedModification();
  while (properties.hasNext()) {
    property = properties.next();
    modification.addChild(selector + '-' + property.name, this.buildModification({selector: selector, property: property.name, value: property.value}));
  }
  return modification;
};
