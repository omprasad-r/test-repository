/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true debug: true Drupal: true window: true */

var ThemeBuilder = ThemeBuilder || {};

/**
 * @class
 */
Drupal.behaviors.editThemes = {
  attach: function (context, settings) {
    var $ = jQuery;
    var $themebuilderMain = $('#themebuilder-main');

    $themebuilderMain.bind('save', function (e, data) {
      // If the server tells us that a screenshot has been scheduled as a
      // result of the new theme being saved (which is normally expected if
      // themebuilder_screenshot.module is enabled), start polling the server
      // so we can update our theme listing once the screenshot is ready.
      if (data.theme_screenshot_scheduled) {
        ThemeBuilder.getApplicationInstance().startPolling('themebuilder_screenshot');
      }
    });

    // This event is triggered on application initialization, if the server has
    // pending screenshot requests that still need to be processed (for
    // example, from a previous themebuilder session). If that's the case,
    // start polling the server so we can get the new screenshot (and update
    // our theme listing with it) as soon as it's ready.
    $themebuilderMain.bind('theme_screenshots_scheduled', function (e) {
      ThemeBuilder.getApplicationInstance().startPolling('themebuilder_screenshot');
    });

    // This event is triggered when we've been trying to poll the server for
    // new screenshots to add to our theme listing, but the server tells us
    // that no more are available. In that case, we can stop polling the
    // server.
    $themebuilderMain.bind('theme_screenshots_complete', function (e) {
      ThemeBuilder.getApplicationInstance().stopPolling('themebuilder_screenshot');
    });
  }
};

/**
 * @class
 */
ThemeBuilder.themeSelector = {
  carousels: {
    base: null,
    custom: null
  },
  deadnode: null,
  currentTheme: null,
  showing: false,
  themeChanged: false,
  themeSwitching: false,
  /**
   * Magically named function that's called the first time the Themes tab is loaded.
   */
  init: function () {
    var app = ThemeBuilder.getApplicationInstance();
    // Set the current theme.
    this.currentTheme = Drupal.settings.currentTheme;
    // Draw the UI as soon as we have application data.
    app.addApplicationInitializer(ThemeBuilder.bind(this, this.drawUI));
    // Refresh the UI whenever the application data changes.
    app.addUpdateListener(ThemeBuilder.bind(this, this.refreshUI));
  },
  /**
   * Draw the initial UI.
   *
   * @param data
   *   Application data from ThemeBuilder.getApplicationInstance().
   */
  drawUI: function (data) {
    var $ = jQuery;
    var i, theme, $actions, $tab, $ul, $li;
    var carousels = {};
    var tabs = {};
    var panel = $('#themebuilder-themes');
    // Set the published theme and selected theme
    this.publishedTheme = data.published_theme;
    this.selectedTheme = data.selectedTheme;
    // Add the Choose new theme link
    var chooseNewTheme = ThemeBuilder.bind(this, this.chooseNewTheme);
    var actionList = new ThemeBuilder.ui.ActionList(
      {
        actions: [
          {
            label: Drupal.t('+ Choose a new theme'),
            action: chooseNewTheme
          }
        ]
      }
    );
    // Add the theme tab's actions. Create a top-level-tab actions container
    $('<div>', {
      id: "themebuilder-themes-actions"
    }).append(
      $('<span>', {
        html: Drupal.t('Saved themes')
      }).addClass('header'),
      actionList.getPointer()
    )
    .addClass('secondary-actions horizontal')
    .prependTo(panel);
    // Create the markup for the two carousels.
    var types = ['mythemes', 'featured'];
    for (i = 0; i < types.length; i++) {
      $tab = $('<div>', {
        id: 'themebuilder-themes-' + types[i]
      })
      .addClass('mode');
      if (i > 0) {
        $tab.smartToggle();
      }
      $tab.appendTo(panel);
      $ul = $('<ul>').addClass('carousel');
      $('<div>').addClass('carousel-wrap carousel-themes punch-out').append($ul).appendTo($tab);
      tabs[types[i]] = $tab;
      carousels[types[i]] = $ul;
    }
    // Create a tile for each theme and put it in the proper carousel.
    for (i = 0; i < data.themes.length; i++) {
      theme = data.themes[i];
      $ul = carousels[theme.type];
      if (!$ul) {
        continue;
      }
      $li = this.getThemeTile(theme, this.selectedTheme === theme.system_name, data.published_theme === theme.system_name);
      $li.appendTo($ul);
    }
    // Initialize the carousels.
    this.carousels.base = $('#themebuilder-themes-featured .carousel').jcarousel();
    this.carousels.custom = $('#themebuilder-themes-mythemes .carousel').jcarousel();

    // Add the last class to the last carousel item
    this.carousels.base.find('li:last-child').addClass('last');
    this.carousels.custom.find('li:last-child').addClass('last');

    // Invoke the feature theme modal interaction. If the page was created from choosing a new
    // theme, this will cause the featured theme modal interaction to be active.
    ThemeBuilder.themes.FeaturedThemeInteraction.invoke();

    // Create the flyout links menu
    var themes = data.themes;
    var len = themes.length;
    for (i = 0; i < len; i++) {
      theme = themes[i];
      this.buildThemeActionList(theme);
    }
  },
  /**
   * Return markup for one entry in the theme carousel.
   */
  getThemeTile: function (theme, selected, live) {
    var $ = jQuery;
    var tile, img, authorLine, meta = '';
    var li = $('<li>', {
      id: 'themetile_' + theme.system_name
    });
    tile = $('<div>', {
      click: ThemeBuilder.bindIgnoreCallerArgs(this, this.switchTheme, theme.system_name)
    }).addClass('theme-shot')
      .appendTo(li);
    if (selected) {
      tile.addClass('applied');
    }
    if (live) {
      tile.addClass('live');
    }

    //Add the last modified info if this is not a base theme.
    if (!theme.is_base) {
      $('<div>', {
        html: Drupal.t('Saved ') + ThemeBuilder.util.niceTime(theme.time_current, theme.time_last_saved)
      }).addClass('last-saved')
        .appendTo(tile);
    }

    // Add the image.
    $('<img>', {
      src: theme.screenshot_url
    }).addClass('image')
      .appendTo(tile);

    // Add the flag.
    $('<div>')
      .addClass('flag')
      .appendTo(tile);

    // Add the preview hover div, and tile hover action.
    if (theme.is_base) {
      $('<div>')
        .addClass('preview')
        .append($("<span>", {
          html: Drupal.t("Preview")
        })).hide()
        .appendTo(tile);

      tile.mouseenter(function () {
          $(this).find(".preview").show();
        })
        .mouseleave(function () {
          $(this).find(".preview").hide();
        });
    }

    // Add the label.
    $('<div>', {
      html: theme.name,
      name: theme.system_name
    }).addClass('label')
      .appendTo(tile);

    return li;
  },
  show: function () {
    return true;
  },
  hide: function () {
    return true;
  },
  checkChanges: function () {
    var $ = jQuery;
    if (ThemeBuilder.Bar.getInstance().exitConfirm()) {
      $.ajax({
        url: Drupal.settings.themeSavePath,
        data: {
          'form_token': Drupal.settings.themeSaveToken,
          'discard': true
        },
        async: false,
        dataType: 'json',
        success: function (x) {
          if (x.error) {
            alert('An error occurred discarding session data: ' + x.error);
          }
        },
        error: function (x, status, error) {
          alert('Error: ' + status + error);
        },
        type: 'POST'
      });
    }
    else {
      return false;
    }
  },
  /**
   * Switch between view modes of this tab
   */
  chooseNewTheme: function (e, active) {
    e.preventDefault();
    // Make sure the user wants to discard any changes before entering the
    // featured themes modal.
    if (ThemeBuilder.Bar.getInstance().exitConfirm()) {
      this.modal = new ThemeBuilder.themes.FeaturedThemeInteraction();
      this.modal.start();
    }
  },
  switchTheme: function (theme) {
    if (this.themeSwitching) {
      return;
    }
    this.themeSwitching = true;

    // Switching themes takes a bit of time; display the loading image.
    var bar = ThemeBuilder.Bar.getInstance();
    bar.showWaitIndicator();
    // Ask for confirmation when discarding a dirty theme, unless we're in the
    // featured theme modal dialog, where the user has already confirmed.
    var inModal = (this.modal && this.modal.getCurrentState() === 'ready') ? true : false;
    if (!inModal && !bar.exitConfirm()) {
      bar.hideWaitIndicator();
      this.themeSwitching = false;
      return;
    }
    bar.disableThemebuilder();

    // Do the theme switch and the cache clear simultaneously, but
    // don't refresh the page until both are done.
    this._stacksCleared = false;
    this._themeChanged = false;
    this._newThemeName = theme;
    ThemeBuilder.postBack('themebuilder-start', {'theme_name': this._newThemeName}, ThemeBuilder.bind(this, this.themeChangeSuccess), ThemeBuilder.bind(this, this.themeChangeFail));
    ThemeBuilder.clearModificationStacks(this.currentTheme, ThemeBuilder.bind(this, this.stackClearSuccess), ThemeBuilder.bind(this, this.stackClearFail));
  },
  /**
   * Build the action list for a theme in the carousel. The action list for a
   * theme depends on what actions it is allowed to call.
   */
  buildThemeActionList: function (theme) {
    var $ = jQuery;
    var actions, $themeTile;
    // Create the flyout list menu
    if (!theme.is_base) {
      actions = this.getThemeActions(theme.system_name);
      $themeTile = $('#' + theme.dom_id);
      // Build and attach the flyout list to the UI
      var $flyoutList = $themeTile.flyoutList({
        items: actions
      }).find('.flyout-list');
      // Add a handler to the action list to deal with interactions
      this.registerActionList(theme.system_name, $themeTile, $flyoutList, actions);
    }
  },
  /**
   * The list of all actions that a theme might have access to.
   *
   * @param {String} theme_name
   *   The system name of the theme
   */
  getThemeActions: function (theme_name) {
    var actions = [];
    // Make live
    var bar = ThemeBuilder.Bar.getInstance();
    if (bar.userMayPublish()) {
      actions.push({
        label: Drupal.t('Publish'),
        linkClasses: [(theme_name === this.publishedTheme) ? 'disabled' : '', 'action-publish']
      });
    }
    // Duplicate
    actions.push({
      label: Drupal.t('Duplicate'),
      linkClasses: ['action-duplicate']
    });
    // Delete action
    actions.push({
      label: Drupal.t('Delete'),
      linkClasses: [((theme_name === this.publishedTheme) || (theme_name === this.selectedTheme)) ? 'disabled' : '', 'action-delete']
    });
    // Push more actions into the actions array as needed...

    return actions;
  },

  /**
   * Attaches handlers to the list of actions in a theme's action list
   *
   * @param {String} system_name
   *   The system name of the theme
   * @param {Array} $themeTile
   *    A jQuery object pointer to the theme tile in the theme carousel list
   * @param {Array} $flyoutList
   *    A jQuery object pointer to the flyout list being registered
   * @param {Array} actions
   *    A list of actions to be included in this flyout list
   */
  registerActionList: function (system_name, $themeTile, $flyoutList, actions) {
    var theme = ThemeBuilder.Theme.getTheme(system_name);
    $flyoutList.click(ThemeBuilder.bind(this, this.flyoutPanelClicked, theme, {tileId: $themeTile.attr('id'), actions: actions}));
  },

  /**
   * Called when any item on the flyout panel is clicked.
   *
   * @param {Event} event
   *   The click event
   * @param {Theme} theme
   *    The Theme instance that represents the theme associated with
   *    the clicked flyout menu.
   * @param {Object} info
   *    Additional information associated with the flyout menu that
   *    may be useful.
   */
  flyoutPanelClicked: function (event, theme, info) {
    var $ = jQuery;
    var $target = $(event.target);
    var actionCallbacks;
    var action;
    // isLive is the last attempt to stop a user from deleting their published
    // theme. The 'disabled' class on the delete action of a published theme
    // should prevent it from being clicked in the first place.
    var isLive = $target.closest('.flyout-list-context').find('.theme-shot').hasClass('live');
    if ($target.hasClass('disabled')) {
      return ThemeBuilder.util.stopEvent(event);
    }
    if ($target.hasClass('action-publish')) {
      actionCallbacks = {
        success: ThemeBuilder.bind(this, this.themePublished),
        fail: ThemeBuilder.bind(this, this.themePublishFailed)
      };
      action = ThemeBuilder.bind(theme, theme.publishTheme, actionCallbacks);
      ThemeBuilder.Bar.getInstance().showWaitIndicator();
      theme.publishTheme(actionCallbacks);
    }
    if ($target.hasClass('action-duplicate')) {
      // Ask the user for the new theme name.
      var duplicate = new ThemeBuilder.themes.DuplicateInteraction(theme);
      var newThemeName = Drupal.t('@theme copy', {'@theme': theme.getName()});
      var data = {
        name: newThemeName
      };
      duplicate.start(data);
    }
    if ($target.hasClass('action-delete') && !isLive) {
      // Confirm that the user wants to delete this theme
      var del = new ThemeBuilder.themes.DeleteInteraction(theme);
      del.start();
    }
    return ThemeBuilder.util.stopEvent(event);
  },

  /**
   * The callback for a successful theme publish.
   *
   * @param {Theme} theme
   *    The Theme instance that was published.
   */
  themePublished: function (theme) {
    var $ = jQuery;
    var bar = ThemeBuilder.Bar.getInstance();
    bar.themeChangeNotification('modify', theme.getSystemName());
    bar.hideWaitIndicator();
    bar.setStatus(Drupal.t('%theme is now the live theme.', {'%theme': theme.getName()}));
    //Mark the right tile as being live and having the delete function disabled.
    var oldPublishedThemeShot = $('#themebuilder-themes-mythemes .live');
    // Remove the live class
    oldPublishedThemeShot.removeClass('live');
    // Remove the disabled classes as appropriate
    var actionsList = oldPublishedThemeShot.parent().find('.flyout-list');
    actionsList.find('.action-publish').removeClass('disabled');
    if (!oldPublishedThemeShot.hasClass('applied')) {
      actionsList.find('.action-delete').removeClass('disabled');
    }
    // Process the active theme tile
    var newPublishedThemeTile = $('#themetile_' + theme.getSystemName());
    newPublishedThemeTile.find('.theme-shot').addClass('live');
    newPublishedThemeTile.find('.action-delete').addClass('disabled');
    newPublishedThemeTile.find('.action-publish').addClass('disabled');
    // If we published the active theme, reset the message
    ThemeBuilder.Bar.getInstance().setVisibilityText();
  },

  /**
   * The callback for a failed theme publish.
   *
   * @param {Theme} theme
   *    The Theme instance that could not be published.
   */
  themePublishFailed: function (theme) {
    var bar = ThemeBuilder.Bar.getInstance();
    bar.hideWaitIndicator();
    bar.setStatus(Drupal.t('Failed to make %theme the live theme.', {'%theme': theme.getName()}));
  },

  /**
   * Handle theme changes caused by a 'Save as'.
   *
   * @param {Theme} theme
   *    The Theme instance that was just created with 'Save as'.
   */
  themeSaved: function (theme) {
    var $ = jQuery;
    // Mark the right tile as being applied and having the delete function
    // disabled.
    var oldSavedThemeShot = $('#themebuilder-themes-mythemes .applied');
    oldSavedThemeShot.removeClass('applied');
    // Remove the disabled classes as appropriate.
    var actionsList = oldSavedThemeShot.parent().find('.flyout-list');
    if (!oldSavedThemeShot.hasClass('live')) {
      actionsList.find('.action-publish').removeClass('disabled');
      actionsList.find('.action-delete').removeClass('disabled');
    }
    // Process the active theme tile.
    var newSavedThemeTile = $('#themetile_' + theme.getSystemName());
    newSavedThemeTile.find('.theme-shot').addClass('applied');
    newSavedThemeTile.find('.action-delete').addClass('disabled');
    if (theme.isPublished()) {
      newSavedThemeTile.find('.action-publish').addClass('disabled');
    }
    // If we saved over the active theme, reset the message.
    ThemeBuilder.Bar.getInstance().setVisibilityText();
  },

  /**
   * Causes the themebuilder user interface to be refreshed after
   * application data has been updated.
   *
   * @param {Object} data
   *   An object containing the Application data fields that have been
   *   modified.
   */
  refreshUI: function (data) {
    var $ = jQuery;
    if (data.themes) {
      // The theme list has been modified.
      this.refreshThemes(data);
      this.refreshScreenshots(data);
    }
    // This is a temporary hack until we get the publish button in themebuilder_bar
    // working the same as the publish action on each theme tile.
    if (data.bar_published_theme) {
      this.themePublished(data.bar_published_theme);
    }
    // With 'Save as' the 'applied' state on the themelist changes.
    if (data.bar_saved_theme) {
      this.themeSaved(data.bar_saved_theme);
    }
  },

  /**
   * Refreshes the theme carousel by adding or removing theme tiles according
   * to the state of the application data.
   *
   * @param {Object} data
   *   The data that has recently been updated, including the theme list.
   */
  refreshThemes: function (data) {
    var $ = jQuery;
    var i = 0;
    var c = this.carousels.custom.data('jcarousel');
    var themes = data.themes;
    var themeIds = [];
    var mythemes = {};
    // Store the theme tile ids in an array
    for (i = 0; i < themes.length; i++) {
      // Don't consider base themes.
      if (!themes[i].is_base) {
        themeIds.push(themes[i].dom_id);
        mythemes[themes[i].dom_id] = themes[i];
      }
    }
    // Get all the carousel items in the DOM.
    var carouselItems = $('#themebuilder-themes-mythemes .carousel > li');
    var carouselItemIds = [];
    // Store the carousel item ids in an array
    carouselItems.each(function (index, value) {
      carouselItemIds[index] = $(value).attr('id');
    });
    // Find any carousel items that exist in the DOM, but not in the application data.
    var deletedThemeItemIndices = [];
    for (i = 0; i < carouselItemIds.length; i++) {
      // If the carousel item id isn't in the theme id list, mark it for deletion
      if ($.inArray(carouselItemIds[i], themeIds) < 0) {
        deletedThemeItemIndices.push(i);
      }
    }

    // Remove the deleted items from the jcarousel
    while (deletedThemeItemIndices.length > 0) {
      // The carousel is a 1 indexed list, not 0
      c.remove(deletedThemeItemIndices.shift() + 1);
    }
    // Find any themes that exist in the application data but not in the theme carousel
    var newThemeIndices = [];
    for (i = 0; i < themeIds.length; i++) {
      // If the theme id isn't in the carousel item list, mark it to be added.
      if ($.inArray(themeIds[i], carouselItemIds) < 0) {
        newThemeIndices.push(i);
      }
    }
    // Add the new items to the end of the carousel.
    while (newThemeIndices.length > 0) {
      var themeId = newThemeIndices.shift();
      var theme = mythemes[themeIds[themeId]];
      var newMarkup = this.getThemeTile(theme, false, false);
      // Add it to the carousel.
      c.add(newMarkup);
      // Create a flyout list for it.
      this.buildThemeActionList(theme);
    }

    // Move the "last" class to the last tile.
    $('#themebuilder-themes-mythemes .carousel > li.last').removeClass('last');
    $('#themebuilder-themes-mythemes .carousel > li:last-child').addClass('last');
  },
  /**
   * Force the theme screenshots to redownload.
   */
  refreshScreenshots: function (data) {
    var themes = data.themes;
    var $ = jQuery;
    var date = new Date();
    var i, img, theme;
    for (i = 0; i < themes.length; i++) {
      theme = themes[i];
      img = $('#themebuilder-themes-mythemes #' + theme.dom_id).find('img.image').get(0);
      if (img && img.src) {
        img.src = theme.screenshot_url + "?" + date.getTime();
      }
    }
  },
  /**
   * Called when the theme was successfully changed.
   */
  themeChangeSuccess: function () {
    this._themeChanged = true;
    this.reloadOnSuccess();
  },

  /**
   * Called when the theme change failed.
   *
   * This callback will attempt to fix any broken themes and try to
   * switch themes a second time.
   */
  themeChangeFail: function (data, type) {
    if (type === 'error') {
      // We tried to change themes but it failed.  Probably there is a
      // corrupted theme that could use a bit of attention.  Try to
      // fix the themes before continuing.
      ThemeBuilder.postBack('themebuilder-fix-themes', {}, ThemeBuilder.bind(this, this.recoverySuccess), ThemeBuilder.bind(this, this.recoveryFailed));
    }
  },

  /**
   * Called when the undo/redo stack has been cleared.
   */
  stackClearSuccess: function () {
    this._stacksCleared = true;
    this.reloadOnSuccess();
  },

  /**
   * Called if the undo/redo stack clear failed.
   *
   * @param {object} data
   *   The object passed back from the server containing any interesting
   *   information about the processing of the request.
   * @param {String} type
   *   A string that reveals the type of response.
   * @param {String} exception
   *   A string that indicates the cause of the problem.
   */
  stackClearFail: function (data, type, exception) {
    // We have been having occasional failures when clearing the
    // undo/redo stacks.  Start by getting more information about the
    // failures and don't treat this as a fatal error.  Once we better
    // understand the issue, we can determine whether this should be
    // fatal or not.
    var info = '';
    if (data) {
      info += ' status: ' + data.status + '; ';
    }
    if (type) {
      info += 'type: ' + type + '; ';
    }
    if (exception) {
      info += 'exception: ' + exception + '; ';
    }
    ThemeBuilder.Log.gardensError('AN-22430 - Failed to clear the undo/redo stack when changing themes.', info);
    // For now, rather than giving up on the failure, clear the client
    // side cache and continue.
    if (false) {
      this.giveUp();
    }
    else {
      ThemeBuilder.undoStack.clear();
      ThemeBuilder.redoStack.clear();
      this.stackClearSuccess();
    }
  },

  /**
   * Called when when the theme switch has been successful and when
   * the stack clear has been successful.  When both are done, this
   * method forces a page refresh so we can view the new theme.
   */
  reloadOnSuccess: function () {
    if (this._themeChanged && this._stacksCleared) {
      parent.location.reload();
    }
  },

  /**
   * Called when the theme recovery worked.
   */
  recoverySuccess: function () {
    // Managed to fix one or more themes.  Try to change themes again,
    // but don't create an infinite loop of requests.  If this fails,
    // give up.
    ThemeBuilder.postBack('themebuilder-start', {'theme_name': this._newThemeName}, ThemeBuilder.bind(this, this.themeChangeSuccess), ThemeBuilder.bind(this, this.recoveryFailed));
    // Send a theme notification.
    var bar = ThemeBuilder.Bar.getInstance();
    bar.themeChangeNotification('modify', this._newThemeName);
  },

  /**
   * Called when the theme recovery failed.
   */
  recoveryFailed: function (data) {
    ThemeBuilder.Log.gardensError('AN-22457 - Failed to switch themes, and running the theme elves did not help.');
    this.giveUp();
  },

  /**
   * We had no luck changing themes either because the the theme
   * switch didn't work or because the undo/redo stack clear failed.
   * Exit the themebuilder with a message to the user.
   */
  giveUp: function () {
    var bar = ThemeBuilder.Bar.getInstance();
    bar.recoveryFailed();
    bar.exit(false);
  }
};
