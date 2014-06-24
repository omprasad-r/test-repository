/**
 * @file visitor_actions_ui.editor.js
 */
(function (Drupal, $, _, Backbone) {

"use strict";

// If we have a version of jQuery > 1.7, then use $.proxy.
// $.proxy_visitor_actions_ui will be defined when jQuery < 1.7.
$.proxy_visitor_actions_ui = $.proxy_visitor_actions_ui || $.proxy;

Drupal.behaviors.visitorActionsUIEditor = {
  attach: function () {
    var model = Drupal.visitorActions.ui.models.appModel || new Drupal.visitorActions.ui.AppModel({
      editMode: Drupal.settings.visitor_actions.edit_mode
    });
    if (!Drupal.visitorActions.ui.views.appView) {
      Drupal.visitorActions.ui.models.appModel = model;
      Drupal.visitorActions.ui.views.appView = new Drupal.visitorActions.ui.AppView({
        el: Drupal.settings.visitor_actions.content_wrapper || 'body',
        model: model,
        actionableElements: new Backbone.Collection([], {
          model: Drupal.visitorActions.ui.ActionModel
        }),
        ActionView: Drupal.visitorActions.ui.ActionView,
        ActionDialogView: Drupal.visitorActions.ui.ActionDialogVisualView
      });
    }
    // Destroy the toggle views and rebuild them in each pass.
    _.each(Drupal.visitorActions.ui.views.toggleViews, function (view) {
      view.remove();
      view = null;
    });
    var toggleViews = Drupal.visitorActions.ui.views.toggleViews = [];
    // Process the visitor actions edit mode toggle.
    $('[href="/admin/structure/visitor_actions/add"]')
      .each(function (index, element) {
        toggleViews.push(
          (new Drupal.visitorActions.ui.AppToggleView({
            el: element,
            model: model
          }))
        );
      });
    Drupal.visitorActions.ui.views.toggleViews = toggleViews;
    // Process actionable elements on every invocation of attach().
    var view = Drupal.visitorActions.ui.views.appView;
    var modelDefinitions = view.buildActionableElementModels(Drupal.settings.visitor_actions.actionableElementTypes, view.el);
    view.actionableElements.add(modelDefinitions);

    // Close the editor dialog if the the overlay is opened.
    $(document).bind('drupalOverlayOpen.visitoractionsui', function () {
      _.each(view.actionableElements.where({active: true}), function (model) {
        model.set('active', false);
      });
    });
  }
};

Drupal.visitorActions = Drupal.visitorActions || {};
Drupal.visitorActions.ui = {

  // A hash of View instances.
  views: {},

  // A hash of Model instances.
  models: {},

  /**
   * Backbone model for the context page.
   */
  AppModel: Backbone.Model.extend({
    defaults: {
      // If this app is being loaded, it is because it is being launched into
      // an edit mode.
      editMode: true
    },

    /**
     * {@inheritdoc}
     */
    destroy: function (options) {
      this.trigger('destroy', this, this.collection, options);
    }
  }),

  /**
   * Backbone controller view for page-level interactions.
   */
  AppView: Backbone.View.extend({

    /**
     * {@inheritdoc}
     */
    initialize: function (options) {

      this.actionableElements = options.actionableElements;
      this.$pageActionsContainer = $();

      this.model.on('change:editMode', this.render, this);
      this.model.on('change:editMode', this.toggleEditMode, this);

      // When actionable elements are added to the collection,
      var decorate = $.proxy_visitor_actions_ui(this.decorateActionableElement, this, options.ActionView, options.ActionDialogView);
      this.actionableElements.on('add', decorate, this);
      this.actionableElements.on('change:active', this.switchActiveElement, this);

      // Run the setup methods once on initialization.
      for (var i = 0, methods = ['render', 'toggleEditMode'], len = methods.length; i < len; i++) {
        this[methods[i]](this.model, this.model.get('editMode'));
      }
    },

    /**
     * {@inheritdoc}
     */
    render: function (model, editMode) {
      this.$pageActionsContainer.toggle(editMode);
    },

    /**
     * {@inheritdoc}
     */
    remove: function () {
      this.actionableElements.each(function (model) {
        model.set({
          enabled: false,
          active: false
        });
        model.destroy();
      });
      this.setElement(null);
      Backbone.View.prototype.remove.call(this);
    },

    /**
     * Toggles the app on and off.
     *
     * @param Backbone.Model model
     * @param Boolean editMode
     *   true is active and false is inactive.
     */
    toggleEditMode: function (model, editMode) {
      this.actionableElements.each(function (model) {
        if (editMode) {
          model.set({
            enabled: editMode
          });
        }
        else {
          model.set({
            enabled: false,
            active: false
          });
        }
      });
    },

    /**
     *
     */
    buildActionableElementModels: function (settings, context) {
      var definitions = [];
      var instanceCount = 0;
      var item, type, selector;
      context = context || 'body';

      /**
       * Returns a page-unique ID for the provided element.
       *
       * @param DOM element
       *
       * @return String
       */
      function getElementID (element) {
        // Use an id to reference the HTML element. If the element does not have
        // an id attribute, create one from the current time. This value will
        // never be stored, so it does not need to be globally unique.
        var id;
        if (element.id) {
          id = element.id;
        }
        else {
          id = 'visitorActionsUI-' + (new Date()).getTime() + '-' + (instanceCount++);
          element.id = id;
        }
        return id;
      }

      /**
       * Creates and inserts a trigger of adding actions that have no
       * indentifier.
       */
      function insertErsatzLink(view, context, item) {
        var $container = $('#visitor-actions-ui-actionable-elements-without-identifiers');
        if (!$container.length) {
          $container = view.$pageActionsContainer = $(Drupal.theme('visitorActionsUIActionableElementsWithoutIdentifiers', {
            id: 'visitor-actions-ui-actionable-elements-without-identifiers'
          })).prependTo(context);
        }
        var $button = $(Drupal.theme('visitorActionsUIButton', {
          text: Drupal.t('Add @type action', {'@type': item.type})
        })).appendTo($container);
        // Give this element an id.
        item.id = getElementID($button.get(0));
        item.selector = '#' + item.id;
        return item;
      }


      /**
       * Returns a definition structure for an ActionModel.
       *
       * @param String type
       *   The type of ActionModel e.g. 'link'
       * @param String selector
       *   A query selector to find this element.
       * @param DOM element
       *   A DOM element associated with this ActionModel. The association is
       *   maintained by the element's ID.
       *
       * @return Object
       */
      function defineModel (type, selector, element) {
        return {
          id: getElementID(element),
          type: type,
          selector: selector
        };
      }

      for (var i = 0, len = settings.length; i < len; i++) {
        item = settings[i];
        type = item.type;
        selector = item.selector;
        // Create a temporary link for this actionable element.
        if (!selector) {
          item = insertErsatzLink(this, context, item);
          selector = item.selector;
        }
        // Create standard model definitions for the actionable elements.
        $(context).find(selector)
          .once('visitor-actions-ui')
          // Give developers a chance to opt out on a one-by-one basis.
          .not('.visitor-actions-ui-ignore')
          // Try to eliminate administration elements.
          .filter(function () {
            var el = this;
            // Filter on the id for blacklisted components.
            var id = el.id || '';
            // Filter for blacklisted class name components.
            var className = typeof this.className === 'string' && this.className || '';
            // Filter for blacklisted href components.
            var href = this.attributes['href'] && this.attributes['href'].value || '';
            // Eliminate any visitor actions components.
            var rVA = /^visitor-actions/;
            // Eliminate local tasks and contextual links.
            var rTask = /local-task|contextual/;
            // Eliminate admin links.
            var rAdmin = /^\/?admin/;
            // Eliminate node action links.
            var rNode = /^\/?node(\/)?(\d)*(?=\/add|\/edit|\/delete)/;
            // Reject the element if any tests match.
            if (rVA.test(id) || rTask.test(className) || rAdmin.test(href) || rNode.test(href)) {
              return false;
            }
            // Keep the element as the default.
            return true;
          })
          .each(function (index, element) {
            definitions.push(defineModel(type, selector, element));
          });
      }

      return definitions;
    },

    /**
     * Associates an ActionView with an ActionModel.
     */
    decorateActionableElement: function (ActionView, ActionDialogView, model) {
      // Create an ActionView and ActionDialogView for the model.
      var options = {
        el: document.getElementById(model.id),
        model: model
      };
      (new ActionView(options));
      (new ActionDialogView(options));
      model.set('enabled', this.model.get('editMode'));
    },

    /**
     * Ensures that only one actionable element is active at a time.
     */
    switchActiveElement: function (model, active) {
      if (!active) {
        return;
      }
      _.chain(this.actionableElements.models)
        .reject(function (m) {
          return m.id === model.id;
        })
        .each(function (m) {
          m.set('active', false);
        });
    }
  }),

  /**
   *
   */
  AppToggleView: Backbone.View.extend({

    /**
     * {@inheritdoc}
     */
    initialize: function () {
      this.model.on('change:editMode', this.render, this);
      this.model.on('destroy', this.remove, this);
      // Set the toggle based on the initial value of the AppModel editMode.
      this.render(this.model, this.model.get('editMode'));
    },

    /**
     * {@inheritdoc}
     */
    render: function (model, editMode) {
      this.$el.toggleClass('visitor-actions-ui-toggle-active', editMode);
      var text = (editMode) ? Drupal.t('Exit add goal mode') : Drupal.t('Add goal');
      this.$el.text(text);
    },

    /**
     * {@inheritdoc}
     */
    remove: function () {
      this.undelegateEvents();
      this.$el.removeData().off();
      this.setElement(null);
      Backbone.View.prototype.remove.call(this);
    }
  }),

  /**
   * The model for an actionable element.
   */
  ActionModel: Backbone.Model.extend({
    defaults: {
      // True if the Action element is available for activation.
      enabled: false,
      // True if the Action element is currently being configured.
      active: false,
      // A type of element, e.g. 'link', or 'form'.
      type: null,
      // A query selector to find this item.
      selector: null
    },

    /**
     * {@inheritdoc}
     */
    destroy: function (options) {
      this.trigger('destroy', this, this.collection, options);
    }
  }),

  /**
   * Backbone View/Controller for creating Actions.
   */
  ActionView: Backbone.View.extend({

    events: {
      'click': 'onClick'
    },

    /**
     * {@inheritdoc}
     */
    initialize: function (options) {
      this.model.on('change', this.render, this);
      this.model.on('destroy', this.remove, this);

      this.render();
    },

    /**
     * {@inheritdoc}
     */
    render: function (model) {
      var enabled = this.model.get('enabled');
      this.$el.toggleClass('visitor-actions-ui-enabled', enabled);
      this[(enabled) ? 'delegateEvents' : 'undelegateEvents']();
      this.$el.toggleClass('visitor-actions-ui-active', this.model.get('active'));
    },

    /**
     * {@inheritdoc}
     */
    remove: function () {
      this.setElement(null);
      // Remove the processed marker.
      this.$el.removeClass('visitor-actions-ui-processed');
      Backbone.View.prototype.remove.call(this);
    },

    /**
     * Responds to clicks.
     *
     * @param jQuery.Event event
     */
    onClick: function (event) {
      this.model.set('active', true);
      event.preventDefault();
      event.stopPropagation();
      event.stopImmediatePropagation();
    }
  }),

  /**
   * Backbone view for the dialog element that contains the interaction UI.
   */
  ActionDialogVisualView: Backbone.View.extend({

    /**
     * {@inheritdoc}
     */
    initialize: function (options) {
      // Save the actionable element as the anchor for this view.
      this.anchor = this.el;

      this.model.on('change:active', this.render, this);
      this.model.on('change:active', this.deactivate, this);
      this.model.on('destroy', this.remove, this);
    },

    /**
     * {@inheritdoc}
     */
    render: function (model, active) {
      if (!active) {
        return;
      }
      var that = this;
      var type = this.model.get('type');
      // A bound function to trigger this' position method.
      var posFn = $.proxy(this.position, this);
      // Set the new dialog fragment as the el of this view.
      this.setElement($(Drupal.theme('visitorActionsUIAssociativeDialog', {
          id: this.model.id
        }))
        .appendTo('body'));
      // Reposition the dialog on window scroll and resize.
      $(window).off('.visitorActionsUI.actionDialogVisualView');
      $(window).on('resize.visitorActionsUI.actionDialogVisualView scroll.visitorActionsUI.actionDialogVisualView', function (event) {
        that.position();
      });

      /**
       * Dismisses this instance of ActionDialogVisualView view.
       */
      Drupal.ajax.prototype.commands.visitor_actions_ui_dismiss = function (ajax, response, status) {
        Drupal.ajax.prototype.commands.visitor_actions_ui_dismiss = null;
        // Clean up the Drupal.ajax object reference for this element.
        Drupal.ajax[that.anchor.id] = null;
        that.model.set('active', false);
      };

      /**
       * Redirects to the callback that disables edit mode.
       *
       * This in turn redirects to where the user came from before they enabled
       * edit mode.
       */
      Drupal.ajax.prototype.commands.visitor_actions_ui_command_redirect = function (ajax, response, status) {
        Drupal.ajax.prototype.commands.visitor_actions_ui_command_redirect = null;
        var destination = document.location.href;
        document.location.href = response.redirect_url + '?destination=' + destination;
      };

      /**
       * Prefills the form with known values after it is loaded into the DOM.
       */
      Drupal.ajax.prototype.commands.visitor_actions_ui_prefill_form = function (ajax, response, status) {
        Drupal.ajax.prototype.commands.visitor_actions_ui_prefill_form = null;
        // Prefill the title for the action.
        var $item = $(ajax.selector);
        var $dialog = $(ajax.selector + '-dialog');
        var title = '';
        switch (that.model.get('type')) {
          case 'form':
            if (ajax.element && ajax.element.id) {
              title = ajax.element.id
                .replace(/-/g, ' ')
                .replace(/^[a-z]/, function (char) {
                  return char.toUpperCase();
                })
                .replace(/\s?form/, '')
                .trim();
              title = Drupal.t('@title form', {'@title': title});
            }
            break;
          case 'page':
            title = /^(.*)\s\|.*/.exec(document.title)[1].trim();
            title = Drupal.t('@title page', {'@title': title});
            break;
          default:
            if (ajax.element && ajax.element.innerText && ajax.element.innerText.length > 0) {
              title = ajax.element.innerText.replace(/^[a-z]/, function (char) {
                return char.toUpperCase();
              }).trim();
              title = Drupal.t('@title link', {'@title': title});
            }
            break;
        }
        title = Drupal.t('@title action', {'@title': title});
        $dialog
          .find('[name="title"]')
          .val(title)
          // Trigger a keyup to produce a machine name.
          .trigger('keyup');


        // Prefill the selector for the element.
        var selector = that.defineUniqueSelector(ajax.selector, type);
        $dialog.find('[name="identifier[' + type + ']"]').val(Drupal.formatString(selector));
      };

      /**
       * Override the Drupal.ajax error handler for the form redirection error.
       *
       * Remove the alert() call.
       */
      var ajaxError = Drupal.ajax.prototype.error;
      Drupal.ajax.prototype.error = function (response, uri) {
        // Remove the progress element.
        if (this.progress.element) {
          $(this.progress.element).remove();
        }
        if (this.progress.object) {
          this.progress.object.stopMonitoring();
        }
        // Undo hide.
        $(this.wrapper).show();
        // Re-enable the element.
        $(this.element).removeClass('progress-disabled').removeAttr('disabled');
        // Reattach behaviors, if they were detached in beforeSerialize().
        if (this.form) {
          var settings = response.settings || this.settings || Drupal.settings;
          Drupal.attachBehaviors(this.form, settings);
        }
      };

      // We need to know when the insert command is called and position the
      // dialog after the form DOM elements have been inserted.
      var insert = Drupal.ajax.prototype.commands.insert;
      /**
       * Hooks into the Drupal.ajax insert command.
       */
      Drupal.ajax.prototype.commands.insert = function (ajax, response, status) {
        // Deal with incremented form IDs.
        if (ajax.wrapper === '#visitor-actions-form') {
          ajax.wrapper = '#' + ajax.form[0].id;
        }
        // Call the original insert command.
        insert.call(this, ajax, response, status);
        // Call the position method.
        if (ajax.event === 'newAction.visitorActionsUI') {
          that.position(function () {
            that.show();
          });
        }
        // Put the original insert command back.
        Drupal.ajax.prototype.commands.insert = insert;
      }

      // Perform an AJAX request to get the add action form.
      Drupal.ajax[this.anchor.id] = new Drupal.ajax(this.anchor.id, this.anchor, {
        url: Drupal.settings.basePath +
          'visitor_actions/add/' +
          Drupal.encodePath(type) +
          '?path=' + Drupal.encodePath(Drupal.settings.visitor_actions.currentPath),
        event: 'newAction.visitorActionsUI',
        wrapper: that.model.id + '-dialog .visitor-actions-ui-placeholder',
        progress: {
          type: null
        },
        success: function (response, status) {
          $('#' + that.anchor.id).off('newAction.visitorActionsUI');
          Drupal.ajax.prototype.success.call(this, response, status);
        },
        complete: function () {
          // Put the original Drupal.ajax error handler back.
          Drupal.ajax.prototype.error = ajaxError;
          ajaxError = null;
        }
      });

      // Trigger the form load after the dialog has rendered.
      $('#' + this.anchor.id).trigger('newAction.visitorActionsUI');
    },

    /**
     * Removes the form dialog from the DOM.
     *
     * @param Backbone.Model model
     * @param Boolean active
     */
    deactivate: function (model, active) {
      if (active) {
        return;
      }
      this.el.parentNode.removeChild(this.el);
    },

    /**
     * {@inheritdoc}
     */
    remove: function (model) {
      $(window).off('.visitorActionsUI.actionDialogVisualView');
      this.setElement(null);
      Backbone.View.prototype.remove.call(this);
    },

    /**
     * Uses the jQuery.ui.position() method to position the dialog.
     *
     * @param function callback
     *   (optional) A function to invoke after positioning has finished.
     */
    position: function (callback) {
      clearTimeout(this.timer);

      var that = this;
      // Vary the edge of the positioning according to the direction of language
      // in the document.
      var edge = (document.documentElement.dir === 'rtl') ? 'right' : 'left';
      // A time unit to wait until the entity toolbar is repositioned.
      var delay = 100;
      // Alighn the dialog with the edge of the highlighted element outline.
      var horizontalPadding = -4;

      /**
       * Refines the positioning algorithm of jquery.ui.position().
       *
       * Invoked as the 'using' callback of jquery.ui.position() in
       * positionToolbar().
       *
       * @param Object suggested
       *   A hash of top and left values for the position that should be set. It
       *   can be forwarded to .css() or .animate().
       * @param Object info
       *   The position and dimensions of both the 'my' element and the 'of'
       *   elements, as well as calculations to their relative position. This
       *   object contains the following properties:
       *     - Object element: A hash that contains information about the HTML
       *     element that will be positioned. Also known as the 'my' element.
       *     - Object target: A hash that contains information about the HTML
       *     element that the 'my' element will be positioned against. Also known
       *     as the 'of' element.
       */
      function refinePosition (suggested, info) {
        var $pointer = info.element.element.find('.visitor-actions-ui-dialog-pointer');
        var coords = {
          left: Math.floor(suggested.left),
          top: Math.floor(suggested.top)
        };

        /**
         * Calculates the position of the pointer in relation to the target.
         */
        function pointerPosition () {
          var swag = info.target.left - info.element.left;
          var element = info.element.element;
          var elWidth = element.outerWidth();
          var pointerWidth = $pointer.outerWidth();
          var gutter = parseInt(element.css('padding-left').slice(0, -2), 10) + parseInt(element.css('padding-right').slice(0, -2), 10);
          // Don't let the value be less than zero.
          swag = (swag > gutter) ? swag : gutter;
          // Don't let the value be greater than the width of the element.
          swag = (swag + pointerWidth < (elWidth - gutter)) ? swag : elWidth - pointerWidth - gutter;
          return swag;
        }

        // Determine if the pointer should be on the top or bottom.
        info.element.element.toggleClass('visitor-actions-ui-dialog-pointer-top', info.vertical === 'top');
        // Determine if the pointer should be on the left or right.
        $pointer.css('left', pointerPosition());
        // Apply the positioning.
        info.element.element.css(coords);
      }

      /**
       * Calls the jquery.ui.position() method on the $el of this view.
       *
       * @param function callback
       *   (optional) A function to invoke after positioning has finished.
       */
      function positionToolbar (callback) {
        that.$el
          .position_visitor_actions_ui({
            my: edge + ' bottom',
            // Move the toolbar 1px towards the start edge of the 'of' element,
            // plus any horizontal padding that may have been added to the element
            // that is being added, to prevent unwanted horizontal movement.
            at: edge + '+' + (1 + horizontalPadding) + ' top',
            of: that.anchor,
            collision: 'flipfit',
            using: refinePosition
          });
        // Invoke an optional callback after the positioning has finished.
        if (callback) {
          callback();
        }
      }

      // Uses the jQuery.ui.position() method. Use a timeout to move the toolbar
      // only after the user has focused on an editable for 250ms. This prevents
      // the toolbar from jumping around the screen.
      this.timer = setTimeout(function () {
        // Render the position in the next execution cycle, so that animations on
        // the field have time to process. This is not strictly speaking, a
        // guarantee that all animations will be finished, but it's a simple way
        // to get better positioning without too much additional code.
        _.defer(positionToolbar, callback);
      }, delay);
    },

    /**
     * Reveals the add action dialog after it has been positioned.
     *
     * Called as part of the render sequence.
     */
    show: function () {
      this.el.style.display = 'block';
    },

    /**
     * Creates a page-unique selector for the selected DOM element.
     *
     * @param String id
     *   A selector string e.g. '#some-element'.
     * @param String type
     *   The type of actionable element e.g. 'link' or 'form'.
     *
     * @return String
     *   A unique selector for this element.
     */
    defineUniqueSelector: function (id, type) {
      var selector = '';

      /**
       * Indicates whether the selector string represents a unique DOM element.
       *
       * @param String selector
       *   A string selector that can be used to query a DOM element.
       *
       * @return Boolean
       *   Whether or not the selector string represents a unique DOM element.
       */
      function isUniquePath (selector) {
        return $(selector).length === 1;
      }

      /**
       * Creates a selector from the element's id attribute.
       *
       * Temporary IDs created by the module are excluded.
       *
       * @param DOM element
       *
       * @return String
       *   An id selector or an empty string.
       */
      function applyID (element) {
        var selector = '';
        var id = element.id;
        if (id.length > 0 && !/visitorActions/.test(id)) {
          selector = '#' + id;
        }
        return selector;
      }

      /**
       * Creates a selector from classes on the element.
       *
       * Classes with known functional components like the word 'active' are
       * excluded because these often denote state, not identity.
       *
       * @param DOM element
       *
       * @return String
       *   A selector of classes or an empty string.
       */
      function applyClasses (element) {
        var selector = '';
        // Try to make a selector from the element's classes.
        var classes = element.className || '';
        if (classes.length > 0) {
          classes = classes.split(/\s+/);
          // Filter out classes that might represent state.
          classes = _.reject(classes, function (cl) {
            return /active|enabled|disabled|first|last|only|collapsed|open|clearfix|processed/.test(cl);
          });
          if (classes.length > 0) {
            return '.' + classes.join('.');
          }
        }
        return selector;
      }

      /**
       * Finds attributes on the element and creates a selector from them.
       *
       * @param DOM element
       *
       * @return String
       *   A selector of attributes or an empty string.
       */
      function applyAttributes (element) {
        var selector = '';
        var attributes = ['href', 'type'];
        var value;
        // Try to make a selector from the element's classes.
        for (var i = 0, len = attributes.length; i < len; i++) {
          value = element.attributes[attributes[i]] && element.attributes[attributes[i]].value;
          if (value) {
            // If the attr is href and it points to a specific user account,
            // just tack on the attr name and not the value.
            if (attributes[i] === 'href' && /user\/\d+/.test(value)) {
              selector += '[' + attributes[i] + ']'
            }
            else {
              selector += '[' + attributes[i] + '="' + value + '"]';
            }
          }
        }
        return selector;
      }

      /**
       * Creates a unique selector using id, classes and attributes.
       *
       * It is possible that the selector will not be unique if there is no
       * unique description using only ids, classes and attributes of an
       * element that exist on the page already. If uniqueness cannot be
       * determined and is required, you will need to add a unique identifier
       * to the element through theming development.
       *
       * @param DOM element
       *
       * @return String
       *   A unique selector for the element.
       */
      function genderateSelector (element) {
        var selector = '';
        var scopeSelector = '';
        var pseudoUnique = false;
        var firstPass = true;

        do {
          scopeSelector = '';
          // Try to apply an ID.
          if ((scopeSelector = applyID(element)).length > 0) {
            selector = scopeSelector + ' ' + selector;
            // Assume that a selector with an ID in the string is unique.
            break;
          }

          // Try to apply classes.
          if (!pseudoUnique && (scopeSelector = applyClasses(element)).length > 0) {
            // If the classes don't create a unique path, tack them on and
            // continue.
            selector = scopeSelector + ' ' + selector;
            // If the classes do create a unique path, mark this selector as
            // pseudo unique. We will keep attempting to find an ID to really
            // guarantee uniqueness.
            if (isUniquePath(selector)) {
              pseudoUnique = true;
            }
          }

          // Process the original element.
          if (firstPass) {
            // Try to add attributes.
            if ((scopeSelector = applyAttributes(element)).length > 0) {
              // Do not include a space because the attributes qualify the
              // element. Append classes if they exist.
              selector = scopeSelector + selector;
            }

            // Add the element nodeName.
            selector = element.nodeName.toLowerCase() + selector;

            // The original element has been processed.
            firstPass = false;
          }

          // Try the parent element to apply some scope.
          element = element.parentNode;
        } while (element && element.nodeType === 1 && element.nodeName !== 'BODY' && element.nodeName !== 'HTML');

        return selector.trim();
      }

      // The method for determining a unique selector depends on the type of
      // element.
      switch (type) {
        case 'form':
          selector = id;
          break;
        case 'page':
          selector = id;
          break;
        case 'link':
        default:
          selector = genderateSelector($(id)[0]);
          break;
      }
      return selector;
    }
  })
};

/**
 * Theme function for an associative dialog.
 *
 * @param Object options
 *   Contains the following key:
 *   - id: The id associated with the actionable element.
 *
 * @return String
 *   The corresponding HTML.
 */
Drupal.theme.visitorActionsUIAssociativeDialog = function (options) {
  var html = '';
  html += '<div id="' + options.id + '-dialog" class="visitor-actions-ui-dialog clearfix" style="display:none;">';
  html += '<i class="visitor-actions-ui-dialog-pointer"></i>';
  html += '<div class="visitor-actions-ui-dialog-content">';
  html += '<div class="visitor-actions-ui-placeholder"></div>';
  html += '</div>';
  return html;
};

/**
 * Theme function for a container to present actionable elements without IDs.
 *
 * @param Object options
 *   Contains the following key:
 *   - id: The id associated with the container.
 *
 * @return String
 *   The corresponding HTML.
 */
Drupal.theme.visitorActionsUIActionableElementsWithoutIdentifiers = function (options) {
  return '<div id="' + options.id + '" class="visitor-actions-ui-actionable-elements-without-identifiers clearfix"></div>';
};

/**
 * Theme function for a simple button.
 *
 * @param Object options
 *   Contains the following key:
 *   - text: The button text.
 *
 * @return String
 *   The corresponding HTML.
 */
Drupal.theme.visitorActionsUIButton = function (options) {
  return '<button>' + options.text + '</button>';
};

}(Drupal, jQuery, _, Backbone));
