
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true AjaxUpload: true ThemeBuilder: true*/

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * The BackgroundEditor class is responsible for the background tab.
 * @class
 * @extends ThemeBuilder.styles.Editor
 */
ThemeBuilder.styles.BackgroundEditor = ThemeBuilder.initClass();
ThemeBuilder.styles.BackgroundEditor.prototype = new ThemeBuilder.styles.Editor();

ThemeBuilder.styles.BackgroundEditor.uploadDisabledTxt = Drupal.t('Uploading...');
ThemeBuilder.styles.BackgroundEditor.uploadEnabledTxt = null;

ThemeBuilder.styles.BackgroundEditor.prototype.initialize = function (elementPicker) {
  this.elementPicker = elementPicker;
  this.tabName = "background";
  this.repeatRadioButton = new ThemeBuilder.styles.RadioButton('.background-repeat-panel', 'background-repeat', 'repeat');
  this.repeatRadioButton.addChangeListener(this);
  this.scrollRadioButton = new ThemeBuilder.styles.RadioButton('.background-attachment-panel', 'background-attachment', 'scroll');
  this.scrollRadioButton.addChangeListener(this);
  this.highlighter = ThemeBuilder.styles.Stylesheet.getInstance('highlighter.css');
};

/**
 * This method is called when the state of radio buttons is changed.  This
 * handles the repeat and scroll properties.
 *
 * @param {String} propertyName
 *   The name of the property being changed.
 * @param {String} oldValue
 *   The original value.
 * @param {String} newValue
 *   The new value.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.valueChanged = function (propertyName, oldValue, newValue) {
  var modification = new ThemeBuilder.CssModification(this.selector);
  modification.setPriorState(propertyName, oldValue);
  modification.setNewState(propertyName, newValue);
  ThemeBuilder.applyModification(modification);
};

ThemeBuilder.styles.BackgroundEditor.prototype.setThumbnail = function (image) {
  var $ = jQuery;
  if (image === "none" || !image) {
    var $background = $('#themebuilder-style-background .background-image');
    $background.css('background-image', '');
    try {
      $background.css('background-repeat', 'none');
    }
    catch (err) {
      // IE has a problem with this.
    }
    $('#themebuilder-style-background .background-image img').attr('src', "").hide();
    return;
  }
  var image_url = this.fixImage(image);
  $('<img src="' + image_url + '">').load(ThemeBuilder.bindFull(this, this._imageLoaded, true, true, image_url));
};

/**
 * This function displays the thumbnail in the themebuilder interface.
 *
 * Called when a recently set background image is loaded into the browser.
 * @private
 *
 * @param {Image} image
 *   The image object.
 * @param {String} image_url
 *   The url to the image that was loaded.
 */
ThemeBuilder.styles.BackgroundEditor.prototype._imageLoaded = function (image, image_url) {
  var $ = jQuery;
  var $image = $(image);
  var $background = $('#themebuilder-style-background .background-image');
  var $backgroundImage = $('#themebuilder-style-background .background-image img');
  if ($image.attr('width') < 75 || $image.attr('height') < 75) {
    // Use a background image
    $background.css('background-image', 'url(' + image_url + ')')
      .css('background-repeat', 'repeat');
    $backgroundImage.attr('src', "").hide();
  }
  else {
    $background.css('background-image', 'none')
      .css('background-repeat', 'none');
    $backgroundImage.attr('src', image_url).show();
  }
};

/**
 * Extracts a urlencoded image path from a CSS image property value.
 *
 * @param {string} value
 *   A CSS image property value (e.g.
 *   "url(http://example.com/sites/all/themes/mytheme/Nice image.jpg").
 *
 * @return {string}
 *   The image path (e.g. "/sites/all/themes/mytheme/Nice%20image.jpg").
 */
ThemeBuilder.styles.BackgroundEditor.prototype.cleanImage = function (value) {
  return value
    .replace(new RegExp('^url\\(\\"?(.+)\\"?\\)$'), "$1")
    .replace(new RegExp("^http(s)*://" + document.domain), '')
    .replace(/"/g, '')
    .replace(/'/g, '')
    .replace(/ /g, "%20")
    .replace(/\(/g, '\\(')
    .replace(/\)/g, '\\)');
};

/**
 * Returns an absolute URL path to an image.
 *
 * @param value string
 *   A string in the form of url(imageurl_relative_to_theme_root).
 *
 * @return string
 */
ThemeBuilder.styles.BackgroundEditor.prototype.fixImage = function (value) {
  var imagePath = this.cleanImage(value);
  if (value && imagePath[0] === '/') { // Already an absolute URL.
    return imagePath;
  }
  return Drupal.settings.basePath + Drupal.settings.themebuilderCurrentThemePath + "/" + imagePath;
};

/**
 * Disables the controls on the background editor.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.disableInputs = function () {
  var $ = jQuery;
  $('#themebuilder-style-background select, #themebuilder-style-background select, #themebuilder-style-background button, #themebuilder-style-background input')
  .attr('disabled', true);
};

/**
 * Causes the uploader to be disabled.  This is desireable when the uploader is uploading
 * a file, thus preventing the user from interacting with it while it is busy.
 *
 * @param {String} newText
 *   Optional text that can be used to replace the button text as the uploader is disabled.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.disableUploader = function (newText) {
  var $ = jQuery;
  var button = $('#uploader');
  if (newText) {
    button.text(newText);
  }
  // @TOOD: Make this functional again.  JS disabled the disabler in demo panic.
  // AN-9040.
  this.uploader.disable();
};

/**
 * Causes the uploader to be enabled.  This reverses the actions taken when the
 * uploader was disabled.
 *
 * @param {String} newText
 *   Optional text that can be used to replace the button text.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.enableUploader = function (newText) {
  var $ = jQuery;
  if (newText) {
    $('#uploader').text(newText);
  }
  this.uploader.enable();
};

/**
 * Called when the background image has changed.  This method is responsible
 * for sending the change to the server.
 *
 * @param {string} value
 *   The new value of the background-image property.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.backgroundImageChanged = function (value, options) {
  var property = 'background-image';
  var modification = new ThemeBuilder.CssModification(this.selector);

  modification.setPriorState('background-image', this.currentImage);
  modification.setNewState(property, value);
  this.currentImage = value;
  if (options && options.setAutoHeight === true) {
    var $ = jQuery;
    // Retrieve a getComputedStyle function for the selected element that will
    // return styles excluding the blue box highlight.
    this.highlighter.disable();
    var getComputedStyle = ThemeBuilder.styleEditor.getComputedStyleFunction($(this.selector).get(0));
    this.highlighter.enable();

    var group = new ThemeBuilder.GroupedModification();
    group.addChild('image', modification);
    var height = new ThemeBuilder.CssModification(this.selector);
    height.setPriorState('height', getComputedStyle('height'));
    height.setNewState('height', options.height);
    group.addChild('height', height);
    var position = new ThemeBuilder.CssModification(this.selector);
    position.setPriorState('background-position', getComputedStyle('background-position'));
    var newPosition = '0% 0%';
    if (options.repeat === 'no-repeat') {
      newPosition = 'center';
    }
    position.setNewState('background-position', newPosition);
    group.addChild('background-position', position);
    modification = group;
  }
  ThemeBuilder.applyModification(modification);
};


/**
 * Called when the uploader is finished uploading a file.
 *
 * @param {String} file
 *   The name of the file that was uploaded.
 * @param {String} response
 *   The response code from the upload.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.uploadComplete = function (file, response) {
  var $ = jQuery;
  var elements = $(this.selector);
  if (elements.length === 1 && elements.hasClass('tb-auto-adjust-height')) {
    var image = new Image();
    image.onload = ThemeBuilder.bindIgnoreCallerArgs(this, this.imageReadyToApply, file, response, true, image);
    image.onerror = ThemeBuilder.bind(this, this.imageFailure, file, response);
    image.src = '/' + Drupal.settings.themebuilderCurrentThemePath + '/' + response;
  }
  else {
    this.imageReadyToApply(file, response, false);
  }
};

/**
 * Called if the recently uploaded image could not be loaded for any reason.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.imageFailure = function (event, file, response) {
  var $ = jQuery;
  $('#background-remove').removeClass('ui-state-disabled');
  this.enableUploader(ThemeBuilder.styles.BackgroundEditor.uploadEnabledTxt);
  if (event && event.currentTarget && event.currentTarget.src) {
    ThemeBuilder.errorHandler.logSilently('Failed to load file ' + event.currentTarget.src);
  }
};

/**
 * Called when the recently uploaded image is loaded and ready to apply
 * to the theme.
 *
 * @param {String} file
 *   A string that holds the filename of the uploaded image.
 * @param {String} response
 *   A string that holds the partial path (within the theme) of the
 *   uploaded image.
 * @param {Boolean} setAutoHeight
 *   If true, the height of the containing element will be adjusted to
 *   reflect the height of the image.
 * @param {Image} image
 *   Optional parameter that holds the Image instance if setAutoHeight
 *   is true.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.imageReadyToApply = function (file, response, setAutoHeight, image) {
  // Use a regex to make sure that this takes the form of a valid url.
  var validateRegex = "^([-a-zA-Z0-9_/. ]+)$";
  if (!response.match(validateRegex)) {
    alert("An error occurred while loading the image.  Please try again.");
    // Need to re-enable the uploader so users can actually try again
    this.enableUploader(ThemeBuilder.styles.BackgroundEditor.uploadEnabledTxt);
    return;
  }

  var $ = jQuery;
  this.enableUploader(ThemeBuilder.styles.BackgroundEditor.uploadEnabledTxt);
  // Set the background image to the upload path from the server.
  var imageValue = 'url("' + response + '")';
  //$('#themebuilder-style-background .background-image').empty();
  this.setThumbnail(response);
  var options = {};
  if (setAutoHeight === true) {
    var selectedElement = $(this.selector).get(0);
    // Retrieve a getComputedStyle function for this element that excludes
    // blue box highlighter styles.
    this.highlighter.disable();
    var getComputedStyle = ThemeBuilder.styleEditor.getComputedStyleFunction(selectedElement);
    this.highlighter.enable();

    var repeat = getComputedStyle('background-repeat');
    if (repeat === 'no-repeat' || repeat === 'repeat-x') {
      // Figure out what the size of the element should be.
      var element = $(this.selector)[0];
      var sizeOffset = ThemeBuilder.styleEditor.getTopOffset(element) +
        ThemeBuilder.styleEditor.getBottomOffset(element);
      options.height = '' + (image.height - sizeOffset) + 'px';
      options.repeat = repeat;
      options.setAutoHeight = true;
    }
  }
  this.backgroundImageChanged(imageValue, options);
  $('#background-remove').removeClass('ui-state-disabled');
};

ThemeBuilder.styles.BackgroundEditor.prototype.removeImage = function (event) {
  var $ = jQuery;
  if (!$('#background-remove').hasClass('ui-state-disabled')) {
    this.backgroundImageChanged('none');
    this.setThumbnail();
    $('#background-remove').addClass('ui-state-disabled');
    return ThemeBuilder.util.stopEvent(event);
  }
};

/**
 * Initializes the Background edit tab on the "Fonts, colors, & sizes" tab in the
 * themebuilder.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.setupTab = function () {
  var $ = jQuery;
  var that = this;
  /* @group Background Tab */
  this.disableInputs();

  this.picker = new ThemeBuilder.styles.PalettePicker($('#style-background-color'), 'background-color', $('#themebuilder-wrapper', parent.document));

  $('#background-remove').click(ThemeBuilder.bind(this, this.removeImage));
  var button = $('#uploader');
  ThemeBuilder.styles.BackgroundEditor.uploadEnabledTxt = Drupal.t(button.html());

  this.uploader = new AjaxUpload(button, {
    action: Drupal.settings.basePath + 'styleedit-file-upload',
    name: 'files[styleedit]',
    data: {
      'form_token': ThemeBuilder.getToken('styleedit-file-upload')
    },
    //responseType: 'json',
    onSubmit: ThemeBuilder.bindIgnoreCallerArgs(this, this.disableUploader, this.uploadDisabledText),
    onComplete: ThemeBuilder.bind(this, this.uploadComplete)
  });
};

/**
 * Refreshes the display.  This should occur when the user selects a
 * new element or when the display changes for some other reason, such
 * as clicking undo or redo.  This method looks at the current set of
 * properties for the selector and makes the property values match.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.refreshDisplay = function () {
  var selectedElement = this.getSelectedElement();

  this.highlighter.disable();
  var getComputedStyle = ThemeBuilder.styleEditor.getComputedStyleFunction(selectedElement);
  this.highlighter.enable();

  this.refreshBackgroundColor(getComputedStyle);
  this.refreshBackgroundImage(getComputedStyle);
  this.refreshBackgroundRepeat(getComputedStyle);
  this.refreshBackgroundAttachment(getComputedStyle);
};

/**
 * Refreshes the display of the color selection.
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.refreshBackgroundColor = function (getComputedStyle) {
  var color = getComputedStyle('background-color');
  if (!color) {
    color = 'transparent';
  }
  this.picker.setIndex(color);
};

/**
 * Refreshes the background image control to match the current
 * selection.
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.refreshBackgroundImage = function (getComputedStyle) {
  var $ = jQuery;
  $('#themebuilder-style-background select,#themebuilder-style-background button,#themebuilder-style-background input').attr('disabled', false);
  $('#themebuilder-style-background input').val('');
  var image = getComputedStyle('background-image');
  if (!image) {
    image = 'none';
  }
  $('#background-remove').addClass('ui-state-disabled');
  this.setThumbnail(image);
  if (image !== 'none') {
    // Enable the remove button.
    $('#background-remove').removeClass('ui-state-disabled');
  }
  this.currentImage = image;
};

/**
 * Refreshes the background-repeat control to match the current
 * selection.
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.refreshBackgroundRepeat = function (getComputedStyle) {
  var $ = jQuery;
  // Initialize the background-repeat value.
  var repeat = getComputedStyle('background-repeat');
  if (!repeat) {
    repeat = 'repeat';
  }
  // Cause the display to be updated without simulating a user click.
  this.repeatRadioButton.setEnabledButton(repeat);
};

/**
 * Refreshes the background-attachment control to match the current
 * selection.
 *
 * @param {function} getComputedStyle
 *   A getComputedStyle function specific to the currently selected element.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.refreshBackgroundAttachment = function (getComputedStyle) {
  var $ = jQuery;
  // Initialize the background-attachment value.
  var attach = getComputedStyle('background-attachment');
  if (!attach) {
    attach = 'scroll';
  }
  // Cause the display to be updated without simulating a user click.
  this.scrollRadioButton.setEnabledButton(attach);
};

/**
 * This method is called by loadSelection when the user selects an element
 * or an item in the option control.  Here we initialize the background tab.
 *
 * @param {String} selector
 *   The new selector.
 */
ThemeBuilder.styles.BackgroundEditor.prototype.selectorChanged = function (selector) {
  this.selector = selector;
  this.picker.setSelector(selector);
  this.refreshDisplay();
};
