
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true */

ThemeBuilder.styles = ThemeBuilder.styles || {};
ThemeBuilder.styles.idRef = 0;

/**
 * @class
 */
ThemeBuilder.styles.PalettePicker = ThemeBuilder.initClass();

ThemeBuilder.styles.PalettePicker.prototype.initialize = function (swatchDiv, property, parentNode) {
  this.enabled = true;
  this.property = property;
  this.id = ThemeBuilder.styles.idRef++;
  this.template = 
  '<div class="PalettePickerMain">' +
     '<div class="header">' +
       '<div class="tabs">' +
         '<ul>' +
           '<li><a href="#picker-' + this.id + '"><div class="colorstab"></div></a></li>' +
           '<li><a href="#swatches-' + this.id + '"><div class="swatchestab"></div></a></li>' +
         '</ul>' + 
         '<div id="picker-' + this.id + '" class="picker"></div>' +
         '<div id="swatches-' + this.id + '" class="swatches">' +
           '<div class="palette-list-table"></div>' +
         '</div>' +
       '</div>' +
     '</div>' +
     '<div class="footer">' +
       '<div class="palette">' +
         '<table class="mycolors-table">' +
           '<tr><td class="cur-palette">Palette</td><td class="cust-palette">Custom</td></tr><tr><td><div class="current-palette palette-list"></div></td><td class="custom-palette-wrapper"><div class="custom-palette palette-list"></div></td></tr><tr><td></td><td></td><td rowspan="3"><div class="plus-button"></div></td></tr></table></div><table style="width:100%"><tr><td><a href class="cancelbutton">Cancel</a></td><td style="text-align:right"><button class="okbutton">OK</button></td></tr></table></div></div>';
  this.swatchDiv = jQuery(swatchDiv);
  this.paletteItems = {};

  if (parentNode) {
    this.dialog = jQuery(this.template).appendTo(parentNode);
  } else {
    this.dialog = jQuery(this.template).insertBefore(this.swatchDiv);
  }
  ThemeBuilder.getApplicationInstance().addApplicationInitializer(ThemeBuilder.bind(this, this.colorDataLoaded));
};

/**
 * Render the list of all of the different palettes.
 */
ThemeBuilder.styles.PalettePicker.prototype.renderPalettes = function (renderSection) {
  var colorManager = ThemeBuilder.getColorManager();
  if (!colorManager.isInitialized()) {
    // Cannot initialize yet.  We need the color manager to be fully initialized
    // first.
    
    setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this, this.colorDataLoaded), 50);
    return;
  }
  this.renderPalletesToTable(jQuery(renderSection, this.dialog));
  
  // When the palette changes, update all the color swatches in the dialog.
  jQuery("#themebuilder-style").bind("paletteChange", ThemeBuilder.bind(this, this.onPaletteChange));
 
  // When a swatch changes, update its corresponding PaletteItem.
  jQuery('#themebuilder-style').bind('swatchPreview', ThemeBuilder.bind(this, this.handleSwatchChange));
};

/**
 * Renders palletes with color previews to a jQuery object which is a <table>
 * @param jQuery targetTable
 *  A <table> jQuery object.
 * @param Integer maxPerColumn
 *   The number of palettes to show in a given column before starting a new one.
 * @return void
 */
ThemeBuilder.styles.PalettePicker.prototype.renderPalletesToTable  = function (targetTable, maxPerColumn) {
  var colorManager = ThemeBuilder.getColorManager();
  // Create a table row in the palettes tab for each available palette.
  var $ = jQuery;
  var palette_item, palette, i, index;
  var palettes = colorManager.getPalettes();
  var indexes = colorManager.getIndexes('palette main');
  var palettesInColumn = 0; // Number of palettes created in a given column.
  
  for (i in palettes) {
    if (typeof(palettes[i]) !== 'function') {
      palette = palettes[i];
      if (palettesInColumn === 0 || palettesInColumn === maxPerColumn) {
        palettesInColumn = 0;
        var palette_column = $('<div class="palette-column"></div>').appendTo(targetTable);
      }
      palettesInColumn += 1;
      
      var palette_wrapper = $('<div class="palette-group"></div>').appendTo(palette_column);
      var list_div = $('<div class="palette-list"></div>').appendTo(palette_wrapper);
      $('<div class="palette-name"></div>').appendTo(palette_wrapper).html(palette.name);
      this.renderPaletteItems(palette, indexes, list_div);
      palette_wrapper.data('palette_id', palette.id);
      palette_wrapper.click(ThemeBuilder.bind(this, this.setPalette));
    }
  }
};


/**
 * Creates a div for each color in indexes and appends to elem. 
 * 
 * @param ThemeBuilder.styles.Palette palette
 *  A Palette object like an element in the return of
 *  ThemeBuilder.styles.colorManager.getPalettes();
 *  
 * @param jQuery elem
 *  An element to append colored divs to.
 *
 * @param Array indexes
 *  An array of indexes as returned by ThemeBuilder.styles.colorManager.getIndexes().
 */
ThemeBuilder.styles.PalettePicker.prototype.renderPaletteItems = function (palette, indexes, elem) {
  for (var j = 0; j < indexes.length; j++) {
    var index = indexes[j];
    var hex = palette.paletteIndexToHex(index);
    var palette_item = jQuery('<div class="palette-item"></div>');
    palette_item.appendTo(elem);
    palette_item.css('background-color', "#" + hex);
    palette_item.addClass('item-' + index);
    palette_item.attr("title", hex);
  }
};

/**
 * Called when color data is available.  This data is received through a request
 * set by the Application class, very early in the loading process.
 */
ThemeBuilder.styles.PalettePicker.prototype.colorDataLoaded = function () {
  var colorManager = ThemeBuilder.getColorManager();
  if (!colorManager.isInitialized()) {
    // Cannot initialize yet.  We need the color manager to be fully initialized
    // first.
    setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this, this.colorDataLoaded), 50);
    return;
  }
  this.palette = colorManager.getPalette();
  this.custom = colorManager.getCustom();

  var indexes = colorManager.getIndexes('palette');
  this.createSwatches('current', this.palette, indexes);
  indexes = colorManager.getIndexes('custom');
  this.createSwatches('custom', this.custom, indexes);

  this.renderPalettes('#swatches-' + this.id + ' .palette-list-table');

  // initialize the custom color picker
  jQuery('#picker-' + this.id, this.dialog).ColorPicker({
    flat: true,
    color: '#00ff00',
    onSubmit: ThemeBuilder.bind(this, this.handleColorPickerSubmit)
  });

  // Append "Add" and "Replace" buttons to the colorpicker.
  var add_button = jQuery('<button class="add">Add</button>');
  add_button.data('action', 'add');
  var replace_button = jQuery('<button class="add">Replace</button>');
  replace_button.data('action', 'replace');
  add_button.appendTo(jQuery('#picker-' + this.id + ' .colorpicker', this.dialog));
  add_button.click(ThemeBuilder.bind(this, this.handleAddReplaceButtonSubmit));

  // Set up events:
  // Show/hide the dialog box when its swatch is clicked.
  this.swatchDiv.click(ThemeBuilder.bind(this, this.show));

  // Set up the "expand dialog" plus-sign button.
  jQuery('.plus-button', this.dialog).click(ThemeBuilder.bind(this, function () {
    this.dialog.toggleClass('expanded');
  }));

  // Handle the "OK" and "Cancel" buttons.
  jQuery('.okbutton', this.dialog).click(ThemeBuilder.bind(this, this.onOk));
  jQuery('.cancelbutton', this.dialog).click(ThemeBuilder.bind(this, this.onCancel));

  // When the palette changes, update all the color swatches in the dialog.
  jQuery("#themebuilder-style").bind("paletteChange", ThemeBuilder.bind(this, this.onPaletteChange));

  // When a swatch changes, update its corresponding PaletteItem.
  jQuery('#themebuilder-style').bind('swatchPreview', ThemeBuilder.bind(this, this.handleSwatchChange));


  // Create the two tabs in the expanded section of the dialog.
  jQuery('.header .tabs', this.dialog).tabs();
};

ThemeBuilder.styles.PalettePicker.prototype.createSwatches = function (type, palette, indexes) {
  var i, index, hex_code;
  var container = jQuery('.' + type + '-palette', this.dialog);
  for (i = 0; i < indexes.length; i++) {
    index = indexes[i];
    hex_code = palette.paletteIndexToHex(index);
    this.paletteItems[index] = new ThemeBuilder.styles.PaletteItem(this, hex_code, index, container);
  }
};

ThemeBuilder.styles.PalettePicker.prototype.handleColorPickerSubmit = function (hsb, hex, rgb, picker) {
  var action = jQuery(picker).data('action');
  var index;
  switch (action) {
  case 'add':
    // See if this color exists in the palette or the custom tray.
    var colorManager = ThemeBuilder.getColorManager();
    index = colorManager.hexToPaletteIndex(hex);
    // If it's a new color, add it to the custom tray.
    if (index === false) {
      index = colorManager.getNextCustomIndex();
      var swatchModification = this._addSwatchModification(index, hex);
      // Add a new listener, so that when the new palette item is created, we
      // can click it and change the color of the selected element.
      jQuery(this.dialog).bind('paletteItemCreated', ThemeBuilder.bind(this, this.handleNewPaletteItem));

      colorManager.swatchPreview(swatchModification.getNewState());
    }
    // This color already existed; just trigger its click event.
    else {
      this.paletteItems[index].click();
    }
    break;
  case 'replace':
    // Get the currently selected color index.
    //index = this.getIndex();
    // Figure out whether it's a palette or custom color.
    // Replace the color in the palette object and in the physical dialog.
    // If it's a palette color, trigger a palette change event.
    throw ('Replacing palette colors is not yet implemented.');
  default:
    throw "Colorpicker action buttons must have 'action' data associated.";
  }

};

/**
 * Add a SwatchModification to the existing grouped modification.
 */
ThemeBuilder.styles.PalettePicker.prototype._addSwatchModification = function (index, hex) {
  var modificationName = 'swatch' + index.toString();
  var swatchModification = this.modification.getChild(modificationName);
  if (!swatchModification) {
    // No previous SwatchModification for this custom color index. Set one up.
    swatchModification = new ThemeBuilder.SwatchModification('custom');
    swatchModification.setPriorState(index, null);
    this.modification.addChild(modificationName, swatchModification);
  }
  swatchModification.setNewState(index, hex);
  return swatchModification;
};

ThemeBuilder.styles.PalettePicker.prototype.handleAddReplaceButtonSubmit = function (e) {
  // Determine whether we're adding or replacing a color swatch.
  var action = jQuery.data(e.currentTarget, 'action');
  // Store that data on the picker element.
  var picker = jQuery('#picker-' + this.id, this.dialog);
  jQuery.data(picker.get(0), 'action', action);
  // Trigger the click event of the colorpicker. There is no other way to get
  // the color out of it, without rewriting the colorpicker plugin. :(
  jQuery('#picker-' + this.id + ' .colorpicker div.colorpicker_submit', this.dialog).click();
};

/**
 * Handle the swatchPreview event triggered by ColorManager when someone
 * adds a custom color.
 *
 * Note that this will be triggered on every preview and must be idempotent.
 */
ThemeBuilder.styles.PalettePicker.prototype.handleSwatchChange = function (e) {
  var index = e.index;
  var colorManager = ThemeBuilder.getColorManager();
  var hex = colorManager.paletteIndexToHex(index);
  var container;
  var indexType = colorManager.isValidIndex(index);
  switch (indexType) {
  case 'palette':
    container = jQuery('.current-palette', this.dialog);
    break;
  case 'custom':
    container = jQuery('.custom-palette', this.dialog);
    break;
  default:
    throw 'swatchChange event triggered with an invalid index';
  }
  // TODO: Handle replacing existing PaletteItems.
  if (this.paletteItems[index]) {
  }
  // If the PaletteItem doesn't already exist in the dialog, create it.
  else {
    this.paletteItems[index] = new ThemeBuilder.styles.PaletteItem(this, hex, index, container);
    jQuery(this.dialog).trigger({type: 'paletteItemCreated', index: index});
  }
};

/**
 * Handle the paletteItemCreated event.
 *
 * Only the PalettePicker where the user actually clicked on the colorpicker
 * should respond to this event, and it should respond only once. Nobody else
 * should care.
 */
ThemeBuilder.styles.PalettePicker.prototype.handleNewPaletteItem = function (e) {
  this.paletteItems[e.index].click();
  jQuery(this.dialog).unbind('paletteItemCreated');
};

/**
 * Change the palette.
 *
 * param {event} e
 *   The click event that triggered the palette set. Its target is a table
 *   row containing a palette, with a jQuery.data 'palette_id' attribute.
 */
ThemeBuilder.styles.PalettePicker.prototype.setPalette = function (e) {
  var colorManager = ThemeBuilder.getColorManager();
  var paletteModification = this.modification.getChild('palette');
  if (!paletteModification) {
    // Create a palette modification and initialize it.
    paletteModification = new ThemeBuilder.PaletteModification('global');
    this.palette = colorManager.getPalette();
    paletteModification.setPriorState(this.palette.id);
    this.modification.addChild('palette', paletteModification);
  }
  var palette_id = jQuery(e.currentTarget).data('palette_id');
  paletteModification.setNewState(palette_id);
  ThemeBuilder.preview(this.modification);
};

/**
 * On paletteChange, update the colors in the dialog box.
 */
ThemeBuilder.styles.PalettePicker.prototype.onPaletteChange = function (e) {
  var paletteId = e.paletteId;
  var newPalette = new ThemeBuilder.styles.Palette(paletteId);
  var colorManager = ThemeBuilder.getColorManager();

  // Change the color of each palette item (i.e. colored square) in the current
  // palette section of the dialog.
  var i, index, item;
  var indexes = colorManager.getIndexes('palette');
  for (i = 0; i < indexes.length; i++) {
    index = indexes[i];
    item = this.paletteItems[index];
    item.setHex(newPalette.colors[index].hex);
  }
  var currentColor = newPalette[this.paletteIndex];
  if (currentColor) {
    // Change the themebuilder color swatch for the selected item.
    this.setSwatchColor('#' + currentColor);
    // Select the new color in the colorpicker color wheel.
    jQuery('#picker-' + this.id, this.dialog).ColorPickerSetColor('#' + currentColor);
  }
};

/**
 * Handle the OK button.
 */
ThemeBuilder.styles.PalettePicker.prototype.onOk = function (event) {
  if (event) {
    ThemeBuilder.util.stopEvent(event);
  }
  // Special handling for the elementColor modification: Make sure it comes
  // after all the SwatchModifications, so the new custom color will actually
  // exist before we try applying it to an element.
  this.modification.bumpChild('elementColor');
  ThemeBuilder.applyModification(this.modification);
  this.hide();
};

/**
 * Handle the Cancel button.
 */
ThemeBuilder.styles.PalettePicker.prototype.onCancel = function (event) {
  if (event) {
    ThemeBuilder.util.stopEvent(event);
  }
  // Revert the preview of changes.
  ThemeBuilder.preview(this.modification, false);

  // If the user selected a color, be sure to reset the color rectangle accordingly.
  var selectionModification = this.modification.getChild('elementColor');
  if (selectionModification) {
    var priorState = selectionModification.getPriorState();
    this.setIndex(priorState.value);
  }
  this.hide();
};

/**
 * Change the color of the selected element.
 */
ThemeBuilder.styles.PalettePicker.prototype.modifySelectedElement = function (paletteIndex) {
  var cssModification = this.modification.getChild('elementColor');
  if (!cssModification) {
    // Initialize a css modification instance for the element color change.
    cssModification = new ThemeBuilder.CssModification(this.selector);
    if (this.paletteIndex) {
      cssModification.setPriorState(this.property, '{' + this.paletteIndex + '}');
    } else {
      cssModification.setPriorState(this.property, '');
    }
    this.modification.addChild('elementColor', cssModification);
  }

  cssModification.setNewState(this.property, '{' + paletteIndex + '}');
  ThemeBuilder.preview(cssModification);
};

/**
 * Set the physical appearance of the dialog box to a new palette index.
 *
 * @param <string> new_color
 *   Can be either a palette index, a custom color index, or a hex color. If a
 *   hex color is passed, will convert to a palette or custom index and use
 *   that instead.
 */
ThemeBuilder.styles.PalettePicker.prototype.setIndex = function (new_color) {
  var $ = jQuery;
  // Make sure the color we're moving to is valid.
  var colorManager = ThemeBuilder.getColorManager();
  new_color = ThemeBuilder.styleEditor.unrgb(new_color);
  var index = colorManager.cleanIndex(new_color);
  var is_hex = colorManager.isHex(index);
  var is_valid_index = colorManager.isValidIndex(index);
  if (!is_hex && !is_valid_index) {
    return;
  }
  // TODO: KS: This is exceedingly lame. The point is to translate a passed-in
  // hex to an index, if we can.
  if (is_hex) {
    var hex = index;
    index = colorManager.hexToPaletteIndex(index);
    if (!index) {
      // TODO: Actually add the passed-in hex to the custom colors tray, and
      // use the index of the new custom color here.
      index = hex;
    }
  }

  this.paletteIndex = index;
  var index_class = ThemeBuilder.util.getSafeClassName(index);

  // Move the red outline from the old swatch to the new swatch.
  if (!$('.palette .palette-item.item-' + index_class, this.dialog).hasClass('selected')) {
    $('.palette .palette-item.selected', this.dialog).removeClass('selected');
    $('.palette .palette-item.item-' + index_class, this.dialog).addClass('selected');
  }
  // TODO: Get the new color from someplace more sensible?
  var color = $('.palette .palette-item.item-' + index_class, this.dialog).data('color');
  if (!color) {
    if (is_hex) {
      color = hex;
    }
    else {
      return;
    }
  }
  // Change the themebuilder color swatch for the selected item.
  this.setSwatchColor(color);
  // Select the new color in the colorpicker color wheel.
  $('#picker-' + this.id, this.dialog).ColorPickerSetColor(color);
};

/**
 * Sets the color of the swatch associated with this PalettePicker instance.  It
 * is important to use this method rather than setting the color inline because
 * this method correctly handles transparency.
 *
 * @param {string} color
 *   The color to apply to the swatch.
 */
ThemeBuilder.styles.PalettePicker.prototype.setSwatchColor = function (color) {
  if ('#transparent' === color || 'transparent' === color) {
    this.swatchDiv.addClass('transparent');
  }
  else {
    this.swatchDiv.removeClass('transparent')
      .css('background-color', color);
  }
};

ThemeBuilder.styles.PalettePicker.prototype.getIndex = function () {
  return this.paletteIndex;
};

ThemeBuilder.styles.PalettePicker.prototype.setSelector = function (selector) {
  this.selector = selector;

};

/**
 * Show the palette dialog box.
 */
ThemeBuilder.styles.PalettePicker.prototype.show = function () {
  if (!this.enabled) {
    return;
  }
  jQuery('<div class="modal"></div>').appendTo('body');
  jQuery('<div class="modal"></div>').appendTo('#themebuilder-wrapper');
  //Set up a CSS modification object for changing an element to another color.
  this.modification = new ThemeBuilder.GroupedModification();

  // Do not allow the user to undo or redo while the dialog is being used.
  this.statusKey = ThemeBuilder.undoButtons.disable();

  // If show somehow got called when we're already showing, hide instead.
  if (this.dialog.css('display') === 'block') {
    this.dialog.fadeOut();
  }
  else {
    this.dialog.fadeIn('normal');
    // Move the dialog just right of its swatch.
    this.dialog.css('left', parseInt(this.swatchDiv.offset().left + 30, 10) + 'px');
    /*  None of this is necessary, the dialog box is always going to be hitting the bottom border, setting bottom: 0; in the css is better
    this.dialog.css('top', parseInt(this.swatchDiv.offset().top, 10) + 'px');
    // Make sure the entire dialog shows on the screen.
    //debugger;
    var themebuilderHeight = jQuery('#themebuilder-wrapper', parent.document).outerHeight();
    var dialogTop = parseInt(this.dialog.css('top'), 10);
    var dialogHeight = this.dialog.outerHeight();
    if (dialogTop + dialogHeight > themebuilderHeight) {
      // The dialog is too far down. Move it up.
      this.dialog.css('top', (themebuilderHeight - dialogHeight) + 'px');
    }
    */
    //$(document).bind('mousedown',hide);
    jQuery('.modal').click(ThemeBuilder.bind(this, this.onCancel));
  }
};

/**
 * Hide the palette dialog box. Should be called on OK and Cancel.
 */
ThemeBuilder.styles.PalettePicker.prototype.hide = function () {
  this.dialog.css('opacity', 1).fadeOut('medium').removeClass('expanded');
  jQuery('.modal').remove();

  // With the dialog dismissed, indicate that it is ok to show the undo and
  // redo buttons again.
  ThemeBuilder.undoButtons.clear(this.statusKey);
};

/**
 * Keep the dialog box from showing, even when the show() method is called.
 */
ThemeBuilder.styles.PalettePicker.prototype.disable = function () {
  this.enabled = false;
};

/**
 * Allow the dialog box to be shown.
 */
ThemeBuilder.styles.PalettePicker.prototype.enable = function () {
  this.enabled = true;
};






/**
 * A PaletteItem is a little colored square appearing within a PalettePicker.
 * @class
 */
ThemeBuilder.styles.PaletteItem = ThemeBuilder.initClass();

ThemeBuilder.styles.PaletteItem.prototype.initialize = function (palettePicker, hex, index, container) {
  this.palettePicker = palettePicker;
  this.index = index;
  this.container = container;
  this.div = jQuery('<div class="palette-item"></div>');
  this.div.appendTo(this.container);
  this.div.click(ThemeBuilder.bind(this, this.click));
  jQuery(this.div).data('index', this.index);
  this.div.addClass('item-' + ThemeBuilder.util.getSafeClassName(this.index));
  this.setHex(hex);
};

ThemeBuilder.styles.PaletteItem.prototype.click = function () {
  // TODO: Replace with an event listener.
  this.palettePicker.modifySelectedElement(this.index);
  this.palettePicker.setIndex(this.index);
};

/**
 * Set the hex color associated with this item and refresh its display.
 */
ThemeBuilder.styles.PaletteItem.prototype.setHex = function (hex) {
  this.hex = hex;
  var color = hex;
  if (hex !== 'transparent') {
    color = '#' + color;
  }
  this.div.css('background-color', color);
  jQuery(this.div).data('color', color);
};
