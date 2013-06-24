/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true window: true*/

(function($) {

  /**
   * Generates the "Viewport settings" subtab for theme settings configuration.
   *
   * @class
   */
  ThemeBuilder.MetatagConfig = ThemeBuilder.initClass();

  /**
   * Constructor for the ThemeBuilder.MetatagConfig class.
   *
   * This constructor should not be called directly, but instead the getInstance
   * static method should be used.
   */
  ThemeBuilder.MetatagConfig.prototype.initialize = function() {
    this.modifications = {};
    this.context = '#themebuilder-advanced-metatag';
  };

  /**
   * Initializes the UI of the tab.
   */
  ThemeBuilder.MetatagConfig.prototype.init = function() {
    var app;

    ThemeBuilder.addModificationHandler(ThemeBuilder.NestedThemeSettingModification.TYPE, this);

    // Update button press.
    $(this.context).delegate('.update-button', 'click', ThemeBuilder.bind(this, this.update));
    $(this.context).delegate('[type="text"]', 'blur keyup', ThemeBuilder.bind(this, this.fieldUpdate));
    $(this.context).delegate('[type="checkbox"]', 'change', ThemeBuilder.bind(this, this.fieldUpdate));
    $(window).bind("ModificationCompleted", ThemeBuilder.bind(this, this.modificationCompleted));

    // Load the application data and execute the method for loading the
    // theme settings configuration data.
    app = ThemeBuilder.getApplicationInstance();
    app.addApplicationInitializer(ThemeBuilder.bind(this, this.loadMetatagConfig));
  };

   /**
    * Called when a modification has been applied on the server side.
    *
    * Applying a viewport modification takes significant time, so a
    * veil is placed over the themebuilder to prevent state changes
    * during the period the modification is being applied.  This
    * method is called when the modification has been successfully
    * applied and is used to remove the veil so the user can continue
    * working with the ThemeBuilder.
    *
    * @param {Object} event
    *   The event.
    * @param {Modification} modification
    *   The modification that was committed.
    * @param {String} operation
    *   "apply" if the modification was applied for the first time,
    *   "undo" if the modification was undone, "redo" if the modification
    *   was redone.
    */
   ThemeBuilder.MetatagConfig.prototype.modificationCompleted = function(event, modification, operation) {
     switch (modification.getType()) {
       case ThemeBuilder.GroupedModification.TYPE:
         ThemeBuilder.Bar.getInstance().enableThemebuilder();
         break;

       case ThemeBuilder.NestedThemeSettingModification.TYPE:
         ThemeBuilder.Bar.getInstance().enableThemebuilder();
         break;
     }
   };

  /**
   * Loads the theme settings and executes a callback to generate the form.
   *
   * This method does a GET request for the theme setings and causes the
   * contents to be placed in the UI.
   *
   * @param object data
   *   The ThemeBuilder application data.
   */
  ThemeBuilder.MetatagConfig.prototype.loadMetatagConfig = function(data) {
    var url = Drupal.settings.basePath + 'themebuilder-metatag-config-load';

    $.get(url, ThemeBuilder.bind(this, this.MetatagConfigLoaded, data));
  };

  /**
   * Calculates the structure of the viewport configuration object.
   *
   * We need to handle various cases and impose a default value for whether or
   * not the viewport metatag is enabled. This method calculates the structure
   * of the viewport configuation object based on various scenarios.
   *
   * @param string configuration
   *   The whitelisted theme settings array loaded from the info file.
   * @param object data
   *   The ThemeBuilder application data.
   */
  ThemeBuilder.MetatagConfig.prototype.calculateViewportConfig = function(configuration, data) {
    var viewport = configuration.viewport || '';

    // If viewport is a non-empty string rather than an object, the viewport
    // settings mustn't been updated yet. Until the viewport settings are
    // updated we want to impose a default state.
    if (typeof viewport === 'string' && viewport !== '') {
      viewport = {
        content: viewport,
        enabled: Number(data.viewportDefaultEnabled)
      };
    }
    // If viewport is an empty string it means either the old viewport value is
    // empty or there is no viewport setting in this theme. Therefore, set the
    // viewport content property to the default value and disable it.
    else if (viewport === '' || viewport.content === '') {
      viewport = {
        content: data.viewportDefaultContent,
        enabled: 0
      };
    }
    // By this time we should have a configuration object with content and
    // enabled properties. We need to ensure the viewport value is a number,
    // rather than a numeric string or boolean value so explicitely type-cast
    // here.
    else {
      viewport.enabled = Number(viewport.enabled);
    }

    return viewport;
  };

  /**
   * Generates the UI for the theme settings form.
   *
   * This callback function generates the configuration UI and initializes the
   * Modification instance to control the state of the form.
   *
   * @todo This is somewhat hard-coded for viewport. Adding more theme settings
   * may require further abstraction of this code.
   *
   * @param string configuration
   *   The whitelisted theme settings array loaded from the info file.
   * @param string textStatus
   *   The returned status message of jQuery's get method.
   * @param XMLHttpRequest xhr
   *   The XMLHttpRequest data.
   * @param object data
   *   The ThemeBuilder application data.
   */
  ThemeBuilder.MetatagConfig.prototype.MetatagConfigLoaded = function(configuration, textStatus, xhr, data) {
    var viewport = this.calculateViewportConfig(configuration, data);

    $('<div>', {
      'class': 'form-item'
    })
    .append(
      $('<input>', {
        type: 'checkbox',
        name: 'themebuilder-metatag-viewport-enabled-value',
        id: 'themebuilder-metatag-viewport-enabled'
      })
    )
    .append(
      $('<label>', {
        text: Drupal.t('Enable viewport metatag'),
        'for': 'themebuilder-metatag-viewport-enabled',
        'class': 'option'
      })
    )
    .appendTo($('.content', this.context));

    $('<div>', {
      'class': 'form-item'
    })
    .append(
      $('<label>', {
        text: Drupal.t('Viewport metatag'),
        'for': 'themebuilder-metatag-viewport-content'
      })
    )
    .append(
      $('<input>', {
        type: 'text',
        value: viewport.content,
        name: 'themebuilder-metatag-viewport-content-value',
        id: 'themebuilder-metatag-viewport-content'
      })
    )
    .append(
      $('<p>', {
        'class': 'description'
      })
      .append(
        $('<span>', {
          text: Drupal.t('For example@seperator', {'@seperator': ': '})
        })
      )
      .append(
        $('<a>', {
          text: data.viewportDefaultContent,
          href: '#',
          'class': 'action-link'
        })
        // Insert the default as the value of the textfield.
        // This is janky.
        .click(function (event) {
          event.preventDefault();
          var $this = $(this);
          var text = $this.text();
          var $context = $this.closest('.form-item');
          $('[type="text"]', $context).val(text).keyup();
        })
      )
    )
    .appendTo($('.content', this.context));

    // Set the initial states of the checkbox and input element.
    this.setFormValue('#themebuilder-metatag-viewport-enabled', viewport.enabled);

    // Toggle the viewport content element's "disabled" attribute on the change
    // event of the viewport enabled checkbox.
    $(this.context).delegate('#themebuilder-metatag-viewport-enabled', 'change', this.toggleDisabled);

    for (var setting in viewport) {
      if (setting && viewport.hasOwnProperty(setting)) {
        // Instantiate a new NestedThemeSettingModification and set the prior state.
        var modification = new ThemeBuilder.NestedThemeSettingModification('viewport');
        modification.setPriorState([setting], viewport[setting]);
        this.modifications['viewport-' + setting] = modification;
      }
    }
  };

  /**
   * Required callback - called when this subtab is deselected.
   */
  ThemeBuilder.MetatagConfig.prototype.hide = function(event, tab) {
    // If tab is undefined, avoid calling the select handler. This avoids double
    // confirmation dialogs appearing when the user selects a different subtab
    // with a dirty form.
    if (typeof tab !== 'undefined') {
      return this.select();
    }

    return true;
  };

  /**
   * Required callback - called when this subtab is selected.
   */
  ThemeBuilder.MetatagConfig.prototype.show = function() {
    $('#themebuilder-wrapper #themebuilder-advanced .palette-cheatsheet').addClass('hidden');
    $('#themebuilder-wrapper #themebuilder-advanced .layout-cheatsheet').addClass('hidden');
  };

  /**
   * Required callback - called when ThemeBuilder.preview is invoked.
   */
  ThemeBuilder.MetatagConfig.prototype.preview = function() {};

  /**
   * Toggles the "disabled" attribute of the viewport content element.
   */
  ThemeBuilder.MetatagConfig.prototype.toggleDisabled = function() {
    var checked = $(this).is(':checked');

    // When the checkbox is not checked, disable the text field and vice versa.
    $('#themebuilder-metatag-viewport-content').attr('disabled', !checked);
  };

  /**
   * Prompts the user to save or discard any changes they may have made.
   *
   * This method is invoked when a different subtab is clicked (before the panel
   * is shown). It checks to see if the subtab is dirty (has changes), and if
   * so, prompts the user to save or discard their changes.
   *
   * @return boolean
   *   Always returns true, indicating it is ok to move off of the tab.
   */
  ThemeBuilder.MetatagConfig.prototype.select = function(event, tab) {
    var bar = ThemeBuilder.Bar.getInstance(),
        updateChanges = false;

    // If the form is dirty, prompt the user to save or discard their changes.
    if (this.isDirty()) {
      updateChanges = confirm(Drupal.t('Would you like to commit your changes?'));
    }

    // If the user chose to save their changes, run the update handler to save
    // them.
    if (updateChanges) {
      this.update();
    }

    // The user chose not to save their changes so we must revert them.
    else if (this.modifications) {
      var modifications = this.modifications;

      for (var modification in modifications) {
        if (modification && modifications.hasOwnProperty(modification)) {
          // Get the prior state which we'll use for resetting the form.
          var prior = modifications[modification].getPriorState();

          // Reset the form elements to their prior values.
          this.processModification(modification, prior);
        }
      }
    }

    // Update the state of the update button and remove the control veil.
    this.setUpdateButtonState();
    bar.enableButtons();

    return true;
  };

  /**
   * Sets the state of form elements using a given value.
   *
   * @param string selector
   *   A CSS selector (passed directly to jQuery) for the form element.
   * @param mixed value
   *   The element value to use for adjusting the state of the form.
   */
  ThemeBuilder.MetatagConfig.prototype.setFormValue = function(selector, value) {
    var $element = $(selector, this.context),
        type = $element.get(0).type;

    switch (type) {
      case 'text':
        $element.val(value);
        break;

      case 'checkbox':
        if (value) {
          $element.attr('checked', 'checked');
        }
        else {
          $element.attr('checked', false);
        }
        // Ensures the viewport content element's "disabled" attribute is
        // toggled when the form state is updated programmatically.
        this.toggleDisabled.call($element);
        break;
    }
  };

  /**
   * Populates the state object with the corresponding form element values.
   *
   * This method is invoked when we suspect a form element for a setting has
   * been changed. It gets the current values of form elements and sets them in
   * the state object.
   */
  ThemeBuilder.MetatagConfig.prototype.fieldUpdate = function(event) {
    var modifications = this.modifications;

    for (var modification in modifications) {
      if (modification && modifications.hasOwnProperty(modification)) {
        var element = this._getElementForProperty(modification);
        var value = this.getElementValue.call($(element, this.context));
        var parents = modifications[modification].getPriorState().parents;
        modifications[modification].setNewState(parents, value);
      }
    }

    this.setUpdateButtonState();
  };

  /**
   * Returns the current value of a form element.
   *
   * Form elements require their values to be extracted in various ways.
   *
   * @return mixed
   *   The current value of the form element.
   */
  ThemeBuilder.MetatagConfig.prototype.getElementValue = function() {
    var type = this.get(0).type,
        value;

    switch (type) {
      case 'text':
        value = this.val();
        break;

      case 'checkbox':
        value = Number(this.is(':checked'));
        break;
    }

    return value;
  };

  /**
   * Commits any changes to the server.
   *
   * This method is invoked when the update button is clicked or when the user
   * chooses to save their changes after navigating away with a dirty form.
   */
  ThemeBuilder.MetatagConfig.prototype.update = function(event) {
    if (this.isDirty()) {
      var next;
      var group = new ThemeBuilder.GroupedModification();

      // Committing the viewport modification can take significant
      // time (a few seconds).  Disable the themebuilder to prevent its
      // state from changing during this update.
      ThemeBuilder.Bar.getInstance().disableThemebuilder();

      for (var modificationName in this.modifications) {
        if (modificationName && this.modifications.hasOwnProperty(modificationName)) {
          next = this.modifications[modificationName].getNewState();
          group.addChild(modificationName, ThemeBuilder.clone(this.modifications[modificationName]));
          this.modifications[modificationName] = this.modifications[modificationName].getFreshModification();
          this.modifications[modificationName].setNewState(next.parents, next.value);
        }
      }

      ThemeBuilder.applyModification(group);

      this.setUpdateButtonState();
    }
  };

  /**
   * Handles undo/redo actions.
   *
   * @param {Object} modification
   *   The modification object that handles this element.
   * @param {Object} state
   *   Represents the state of the viewport configuration after an undo or redo
   *   operation.
   */
  ThemeBuilder.MetatagConfig.prototype.processModification = function(modification, state) {
    if (state && state.selector) {
      var property = state.selector + '-' + state.parents.join('-');
      var value = state.value;
      var element = this._getElementForProperty(property);
      this.setFormValue(element, value);
    }
  };

  /**
   * Returns the element responsible for displaying or editing the specified setting.
   *
   * @param {String} setting
   *   The name of the property.
   * @return {HTMLElement}
   *   The element responsible for editing the specified setting.
   */
  ThemeBuilder.MetatagConfig.prototype._getElementForProperty = function (setting) {
    var id = 'themebuilder-metatag-' + setting;
    return document.getElementById(id);
  };

  /**
   * Returns the setting associated with the specified element.
   *
   * @param {HTMLElement} element
   *   The element. Generally this will be an input field.
   * @return {String}
   *   The setting associated with the element.
   */
  ThemeBuilder.MetatagConfig.prototype._getPropertyForElement = function (element) {
    var property = '';
    var id = element.id;
    if (new RegExp('^themebuilder-metatag-(.)*').test(id)) {
      property = id.slice('themebuilder-metatag-'.length);
    }
    return property;
  };

  /**
   * Controls the state of the update button and control veil.
   *
   * This method disables the update button and the main tabs when there are
   * changes to the form and vice versa, stopping the user from saving the form
   * or navigating away from the Advanced tab when there are changes to
   * consider.
   */
  ThemeBuilder.MetatagConfig.prototype.setUpdateButtonState = function() {
    var bar = ThemeBuilder.Bar.getInstance();

    if (this.isDirty()) {
      $('.update-button', this.context).removeClass('disabled');
      bar.disableButtons();
    }
    else {
      $('.update-button', this.context).addClass('disabled');
      bar.enableButtons();
    }
  };

  /**
   * Checks to see if there are changes to the form.
   *
   * This method compares the prior and new states to see if they differ. The
   * prior state contains the last-saved values and the new state is the current
   * state of the form in the browser.
   *
   * This is useful for knowing whether the current form contents differ from
   * the last-saved values. It is used for the undo/redo buttons and when the
   * user navigates away from the current subtab with changes so we can prompt
   * them to save or discard their changes.
   *
   * @return boolean
   *   True if the editor contents have not been committed to the server; false
   *   otherwise.
   */
  ThemeBuilder.MetatagConfig.prototype.isDirty = function() {
    var modifications = this.modifications;
    for (var modification in modifications) {
      if (modification && modifications.hasOwnProperty(modification)) {
        if (modifications[modification].hasChanged()) {
          return true;
        }
      }
    }
    return false;
  };

}(jQuery, Drupal, ThemeBuilder));

