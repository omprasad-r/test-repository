
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true AjaxUpload: true ThemeBuilder: true*/

/**
 * @namespace
 */
ThemeBuilder.brand = ThemeBuilder.brand || {};

/**
 * The LogoPicker class is responsible for the Brand tab's Logo subtab.
 * @class
 */
ThemeBuilder.brand.LogoPicker = ThemeBuilder.initClass();

ThemeBuilder.brand.LogoPicker.uploadDisabledTxt = Drupal.t('Uploading...');
ThemeBuilder.brand.LogoPicker.uploadEnabledTxt = null;

ThemeBuilder.brand.LogoPicker.prototype.initialize = function () {
  this.tabName = "logo";
  var app = ThemeBuilder.getApplicationInstance();
  var data = app.getData();
  if (data) {
    this.createModifications(data);
  }
  else {
    app.addApplicationInitializer(ThemeBuilder.bind(this, this.createModifications));
  }
};

/**
 * Create modifications for the logo and favicon.
 *
 * @param {Object} data
 *   Application data from the ThemeBuilder.Application object.
 */
ThemeBuilder.brand.LogoPicker.prototype.createModifications = function (data) {
  this.logoModification = new ThemeBuilder.ThemeSettingModification('default_logo_path');
  this.logoModification.setPriorState(data.default_logo_path);
  this.faviconModification = new ThemeBuilder.ThemeSettingModification('default_favicon_path');
  this.faviconModification.setPriorState(data.default_favicon_path);
};

/**
 * Initializes the Logo subtab of the Brand tab.
 */
ThemeBuilder.brand.LogoPicker.prototype.setupTab = function () {
  var $ = jQuery;
  var that = this;

  // Set up "Browse" buttons.
  var logoButton = $('#logo-uploader');
  ThemeBuilder.brand.LogoPicker.uploadEnabledTxt = Drupal.t(logoButton.html());

  this.logoUploader = new AjaxUpload(logoButton, {
    action: Drupal.settings.basePath + 'styleedit-file-upload',
    name: 'files[styleedit]',
    data: {
      'form_token': ThemeBuilder.getToken('styleedit-file-upload')
    },
    //responseType: 'json',
    onSubmit: ThemeBuilder.bindIgnoreCallerArgs(this, this.disableUploader, this.uploadDisabledText),
    onComplete: ThemeBuilder.bind(this, this.logoUploadComplete)
  });

  var faviconButton = $('#favicon-uploader');
  this.faviconUploader = new AjaxUpload(faviconButton, {
    action: Drupal.settings.basePath + 'styleedit-file-upload/favicon',
    name: 'files[styleedit]',
    data: {
      'form_token': ThemeBuilder.getToken('styleedit-file-upload')
    },
    //responseType: 'json',
    onSubmit: ThemeBuilder.bindIgnoreCallerArgs(this, this.disableUploader, this.uploadDisabledText),
    onComplete: ThemeBuilder.bind(this, this.faviconUploadComplete)
  });

  // Set up "Remove" links.
  $('#themebuilder-main .themebuilder-brand-logo a').click(ThemeBuilder.bind(this, this.removeLogo));
  $('#themebuilder-main .themebuilder-brand-favicon a').click(ThemeBuilder.bind(this, this.removeFavicon));
};

/**
 * Disables the uploaders.
 *
 * @param {String} newText
 *   Optional text that can be used to replace the button text as the uploader is disabled.
 */
ThemeBuilder.brand.LogoPicker.prototype.disableUploader = function (newText) {
  var $ = jQuery;
  if (newText) {
    $('#logo-uploader').text(newText);
    $('#favicon-uploader').text(newText);
  }
  this.logoUploader.disable();
  this.faviconUploader.disable();
};

/**
 * Enables the uploaders.
 *
 * @param {String} newText
 *   Optional text that can be used to replace the button text.
 */
ThemeBuilder.brand.LogoPicker.prototype.enableUploader = function (newText) {
  var $ = jQuery;
  if (newText) {
    $('#logo-uploader').text(newText);
    $('#favicon-uploader').text(newText);
  }
  this.logoUploader.enable();
  this.faviconUploader.enable();
};

/**
 * Called when the uploader is finished uploading a file.
 *
 * @param {String} file
 *   The name of the file that was uploaded.
 * @param {String} response
 *   The response code from the upload.
 */
ThemeBuilder.brand.LogoPicker.prototype.logoUploadComplete = function (file, response) {
  this.enableUploader(ThemeBuilder.brand.LogoPicker.uploadEnabledTxt);
  if (this.isImageFile(response)) {
    this.logoModification.setNewState(response);
    ThemeBuilder.applyModification(this.logoModification);
    this.logoModification = this.logoModification.getFreshModification();
  }
};

/**
 * Called when the uploader is finished uploading a file.
 *
 * @param {String} file
 *   The name of the file that was uploaded.
 * @param {String} response
 *   The response code from the upload.
 */
ThemeBuilder.brand.LogoPicker.prototype.faviconUploadComplete = function (file, response) {
  this.enableUploader(ThemeBuilder.brand.LogoPicker.uploadEnabledTxt);
  if (this.isImageFile(response)) {
    this.faviconModification.setNewState(response);
    ThemeBuilder.applyModification(this.faviconModification);
    this.faviconModification = this.faviconModification.getFreshModification();
    // Favicons can't be updated live on IE or Webkit.
    if (jQuery.browser.msie || jQuery.browser.webkit) {
      var bar = ThemeBuilder.Bar.getInstance();
      bar.setStatus(Drupal.t('The favicon will appear on your next page refresh.'));
    }
  }
};


ThemeBuilder.brand.LogoPicker.prototype.removeLogo = function () {
  this.logoModification.setNewState('');
  ThemeBuilder.applyModification(this.logoModification);
  this.logoModification = this.logoModification.getFreshModification();
};

ThemeBuilder.brand.LogoPicker.prototype.removeFavicon = function () {
  this.faviconModification.setNewState('');
  ThemeBuilder.applyModification(this.faviconModification);
  this.faviconModification = this.faviconModification.getFreshModification();
};

/**
 * Acceptable image extensions.
 *
 * A case insensitive compare is done on the following extensions to
 * determine if the filename represents an acceptable image.
 */
ThemeBuilder.brand.LogoPicker.imageExtensions = ['jpg', 'jpeg', 'png', 'gif', 'ico'];

/**
 * Determines if the specified filename represents a valid image filename.
 *
 * We have had cases in which an HTML document gets added to the
 * theme's .info file, causing really bad things to happen.
 *
 * @param {String} file
 *   The filename
 * @return {boolean}
 *   true if the filename represents a valid image file; false otherwise.
 */
ThemeBuilder.brand.LogoPicker.prototype.isImageFile = function (file) {
  if (file && typeof(file) === 'string') {
    // The size of the 'value' property in the themebuilder_css table
    // is 512 characters.
    if (file.length < 500) {
      var extension = this.getFileExtension(file);
      if (extension !== null) {
        extension = extension.toLowerCase();
        var extensionSet = ThemeBuilder.brand.LogoPicker.imageExtensions;
        for (var i = 0; i < extensionSet.length; i++) {
          if (extension === extensionSet[i]) {
            return true;
          }
        }
      }
    }
  }
  return false;
};

/**
 * Returns the extension of the specified filename.
 *
 * If the parameter doesn't represent a file with an extension, null
 * is returned instead.
 *
 * @param {String} file
 *   The name of the image file.
 * @return {String}
 *   The file extension or null if the name doesn't have an extension.
 */
ThemeBuilder.brand.LogoPicker.prototype.getFileExtension = function (file) {
  var index = file.lastIndexOf('.');
  if (index > 0 && (index + 1) < file.length) {
    return file.substring(index + 1);
  }
  return null;
};
