/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global window: true jQuery: true Drupal: true ThemeBuilder: true*/

var ThemeBuilder = ThemeBuilder || {};

/**
 * @namespace
 */
ThemeBuilder.brandEditor = ThemeBuilder.brandEditor || {};

ThemeBuilder.brandEditor.PalettePicker = function (swatchDiv) {
  this.enabled = true;
  this.swatchDiv = jQuery(swatchDiv);
  this.paletteItems = {};

  // Set up a CSS modification object for changing an element to another color.
  this.modification = new ThemeBuilder.GroupedModification();
  ThemeBuilder.getApplicationInstance().addApplicationInitializer(
    ThemeBuilder.bind(this, this.colorDataLoaded));
};

ThemeBuilder.extend(ThemeBuilder.brandEditor.PalettePicker,
  ThemeBuilder.styles.PalettePicker);

ThemeBuilder.brandEditor.PalettePicker.prototype.show = function () {
  this.swatchDiv.show();
};

ThemeBuilder.brandEditor.PalettePicker.prototype.colorDataLoaded = function () {
  var colorManager = ThemeBuilder.getColorManager();
  if (!colorManager.isInitialized()) {
    // Cannot initialize yet. We need the color manager to be fully initialized
    // first.
    setTimeout(ThemeBuilder.bindIgnoreCallerArgs(this, this.colorDataLoaded),
      50);
    return;
  }
  this.palette = colorManager.getPalette();
  this.custom = colorManager.getCustom();

  var indexes = colorManager.getIndexes('palette');
  this.renderPalletesToTable(jQuery('.palette-list-table', this.swatchDiv), 7);

  // Set up events:
  // Show/hide the dialog box when its swatch is clicked.
  this.swatchDiv.click(ThemeBuilder.bind(this, this.show));
};

ThemeBuilder.brandEditor.PalettePicker.prototype.setPalette = function (e) {
  // call the super class
  ThemeBuilder.brandEditor.PalettePicker.superproto.setPalette.call(this, e);
  // Also commit the change.
  ThemeBuilder.applyModification(this.modification);
  // Create a new modification instance to keep the modifications distinct.
  this.modification = new ThemeBuilder.GroupedModification();
};

ThemeBuilder.brand = ThemeBuilder.brand || {};

var Drupal = Drupal || parent.Drupal;

(function ($) {

  /**
   * Invoked after the brand tab is started. Primarily it is used to setup event
   * handlers
   * 
   */
  ThemeBuilder.brandEditor.init = function () {
    // tab-ize
    $('#themebuilder-brand').tabs({
      show : function (event, ui) {
        return true;
      }
    });
    // Set up the logo subtab.
    this.logoPicker = new ThemeBuilder.brand.LogoPicker();
    this.logoPicker.setupTab();

    var palettePicker = new ThemeBuilder.brandEditor.PalettePicker(
      '#themebuilder-brand-palette-picker');
    palettePicker.show();
    ThemeBuilder.addModificationHandler(ThemeBuilder.ThemeSettingModification.TYPE, this);
    $(window).bind('ModificationCommitted', ThemeBuilder.brandEditor._modificationCommitted);
  };

  /**
   * Invoked when a different tab is selected.
   */
  ThemeBuilder.brandEditor.hide = function () {
  };

  /**
   * Invoked when the brand tab is selected.
   */
  ThemeBuilder.brandEditor.show = function () {
    return true;
  };

  /**
   * Applies the specified modification description to the client side only.
   * This allows the user to preview the modification without committing it
   * to the theme.
   *
   * @param {Object} state
   *   The modification description.  To get this value, you should pass in
   *   the result of Modification.getNewState() or Modification.getPriorState().
   * @param {Modification} modification
   *   The modification that represents the change in the current state that
   *   should be previewed.
   */
  ThemeBuilder.brandEditor.preview = function (state, modification) {
    var imagePath;
    switch (state.selector) {
    case 'default_logo_path':
      ThemeBuilder.brandEditor._blockSet = false;
      ThemeBuilder.brandEditor._logoCommitted = false;
      var headerInner = '.stack-header-inner .col-c';
      // Specify a logo image.
      if (state.value) {
        // TODO: The cleanImage() function could clearly use a better home.
        imagePath = Drupal.settings.basePath +
          Drupal.settings.themebuilderCurrentThemePath + "/" +
          ThemeBuilder.styles.BackgroundEditor.prototype.cleanImage(state.value);

        if (jQuery('.logo img', headerInner).length > 0) {
          jQuery('.logo:hidden', headerInner).show();
          jQuery('.logo img', headerInner).attr('src', imagePath);
        }
        else {
          // The markup for the logo is not on the page.  Try to
          // enable the block.
          if (confirm(Drupal.t("The logo will only appear if the 'Site logo' block is enabled.  Would you like to enable it now?"))) {
            // Note that we use the same handler for success and
            // failure.  With older themes that don't use the block, the
            // success of setting the block makes no difference because a
            // page refresh will cause the markup for the logo to be
            // rendered.
            ThemeBuilder.postBack('themebuilder-brand-configure-logo', {}, ThemeBuilder.brandEditor._blockConfigured, ThemeBuilder.brandEditor._blockConfigured);
            ThemeBuilder.Bar.getInstance().disableThemebuilder();
          }
        }
        jQuery("#themebuilder-main #themebuilder-brand-logo .themebuilder-brand-logo img").attr('src', imagePath);
      }
      // Remove logo image
      else {
        // Remove the logo and put back the original 1px logo.png.
        imagePath = Drupal.settings.basePath + Drupal.settings.themebuilderCurrentThemePath + "/logo.png";
        jQuery('.logo', headerInner).hide();
        jQuery("#themebuilder-main #themebuilder-brand-logo .themebuilder-brand-logo img").attr('src', imagePath);
      }
      break;
    case 'default_favicon_path':
      if (state.value) {
        // Change the favicon.
        imagePath = Drupal.settings.basePath +
          Drupal.settings.themebuilderCurrentThemePath + "/" +
          ThemeBuilder.styles.BackgroundEditor.prototype.cleanImage(state.value);
      }
      // Put back the default favicon for SMB only.
      else if (Drupal.settings.gardensMisc.isSMB) {
        imagePath = Drupal.settings.basePath + 'misc/favicon.ico';
      }
      // Otherwise use a transparent favicon. A transparent favicon is required
      // so that when the user removes the favicon in the brand tab, the icon is
      // visibly removed. Otherwise, the browser displays the favicon from its
      // cache.
      else {
        imagePath = Drupal.settings.basePath + 'favicon.ico';
      }
      // Update the favicon itself.
      var link = document.createElement('link');
      link.type = 'image/x-icon';
      link.rel = 'shortcut icon';
      link.href = imagePath;
      jQuery('link[rel="shortcut icon"]').remove();
      jQuery('head').append(link);
      // Update the Themebuilder UI.
      jQuery(
        "#themebuilder-main #themebuilder-brand-logo .themebuilder-brand-favicon img")
        .attr('src', imagePath);
      break;
    }
  };

  /**
   * Called when the site logo block was successfully configured.
   *
   * @param {Object} data
   *   The data that resulted from the server request.
   */
  ThemeBuilder.brandEditor._blockConfigured = function (data) {
    ThemeBuilder.brandEditor._blockSet = true;
    ThemeBuilder.brandEditor._reloadPageAfterEnablingBlock();
  };

  /**
   * Called when a modification is successfully committed.
   *
   * This method is used to determine if a page reload is required.
   * If the default logo was successfully changed and the user
   * requested to have the logo block configured automatically, a
   * refresh is required to reload the page, getting the new markup
   * for the site logo.
   *
   * @param {Event} event
   *   The event associated with the commit operation.
   * @param {Modification} modification
   *   The modification that was committed.
   * @param {String} operation
   *   Indicates whether the modification was applied ("apply"),
   *   undone ("undo"), or redone ("redo").
   */
  ThemeBuilder.brandEditor._modificationCommitted = function (event, modification, operation) {
    if (operation === 'apply' && modification.getSelector() === 'default_logo_path') {
      ThemeBuilder.brandEditor._logoCommitted = true;
      ThemeBuilder.brandEditor._reloadPageAfterEnablingBlock();
    }
  };

  /**
   * Performs the actual page reload.
   *
   * This occurs only if the user requested to have the logo block
   * configured and committed a new logo image.
   */
  ThemeBuilder.brandEditor._reloadPageAfterEnablingBlock = function () {
    if (ThemeBuilder.brandEditor._blockSet === true && ThemeBuilder.brandEditor._logoCommitted === true) {
      parent.location.reload(true);
    }
  };

   /**
    * @class
    */
  Drupal.behaviors.editBrand = {
    attach : function (context, settings) {
      // Add brand tab actions to page.
      jQuery('#themebuilder-brand').bind('init', function (e) {
        ThemeBuilder.brandEditor.init();
      });
    }
  };
}(jQuery));

