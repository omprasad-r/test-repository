/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global window: true document: true jQuery: true Drupal: true janrain: true */

(function ($, window, document) {

  /**
   * Loads the janrain engage.js and in turn, the engage_signin.js script.
   */
  function loadJanrainEngage(event) {
    var data = event.data;

    window.janrain = window.janrain || {};
    window.janrain.settings = window.janrain.settings || {};

    window.janrain.settings.tokenUrl = data.settings.janrainClient.token_url;
    window.janrain.settings.showAttribution = false;
    window.janrain.settings.tokenAction = 'event';
    window.janrain.settings.ready = true;

    var e = document.createElement('script');
    e.type = 'text/javascript';
    e.id = 'janrainAuthWidget';

    if (document.location.protocol === 'https:') {
      e.src = 'https://rpxnow.com/js/lib/' + data.settings.janrainClient.rpx_realm + '/engage.js';
    } else {
      e.src = 'http://widget-cdn.rpxnow.com/js/lib/' + data.settings.janrainClient.rpx_realm + '/engage.js';
    }

    var s = document.getElementsByTagName('script')[0];
    s.parentNode.insertBefore(e, s);
  }
  /**
   * Utility function to remove 'px' string from css values in jQuery
   */
  function stripPX(value) {
    if (typeof(value) === 'string') {
      var index = value.indexOf('px');
      if (index === -1) {
        return Number(value);
      }
      else {
        return Number(value.substring(0, index));
      }
    }
    return 0;
  }

  /**
   * Respond to the jQuery UI dialogopen event.
   *
   * Adds the janrain engage widget to the user login and register dialogs.
   */
  function insertWidget(event) {
    // The login form container.
    var $this = $(this);

    // Theoretically this is responsible for initializing the Janrain Engage
    // widget. Suddenly we just had to start calling it ourselves.
    janrain.engage.signin.widget.init();

    //store a reference to widget container
    var $rpxWidgetEmbed = $this.find('#rpx-widget-embed'),
      $janrainView = $('#janrainEngageEmbed').find("#janrainView");

    if ($janrainView.next("div").length) {
      $janrainView.next("div").remove();
    }

    // If engage.js fails to load, then window.janrain.engage.signin.widget.init()
    // will throw an error.
    if (window.janrain && window.janrain.engage) {
      // Login Container
      var $methods = $('.auth-methods', $this);
      // Move the rpx-signin element into the auth-methods element.
      var $rpx = $this.find('.rpx-signin').appendTo($methods);
      var $userMethod = $('.user-fields', $methods),
      $rpxMethod = $('.rpx-signin', $methods);
      // Create the rpx-or element.
      var $rpxOr = $('<div>', {
        html: $('<span>', {
          text: Drupal.t('or')
        })
      }).addClass('rpx-or'),
      rpxOrHeight = Drupal.behaviors.rpxLoadJanrainEngage.getRPXOrHeight($methods);
      // Add the or text to the RPX container.
      $rpxMethod.once('rpx-or', function (index) {
        $(this).prepend(
          $rpxOr
          .css({
            height: rpxOrHeight || 'auto',
            'min-width': '1em'
          })
        );
      });

      // Display the signin widget and trigger contentupdate to resize it.
      $rpxWidgetEmbed
      .empty()
      .append($('#janrainEngageEmbed'))
      .end()
      .trigger('contentupdate');

      // DG-9879: silly hack to make sure the widget gets updated so that it
      // shows when a user has previously signed in. The duplicate call, above,
      // is required prior to shifting around the login form elements. However,
      // the call above is too early to set up the "returnExperience" properly
      // for users who had previously used a social login and logged out again,
      // so we need to call it again.
      janrain.engage.signin.widget.init();
    }
  }


  /**
   * Responds to the jQuery UI dialogclose event.
   *
   * Removes the janrain engage widget from the user login and register dialogs.
   */
  function removeWidget(event) {
    if (window.janrain) {
      // Hide the signin widget.
      $('#rpx-widget-store').append($('#janrainEngageEmbed'));
    }
  }

  /**
   * Load the engage.js file, which in turn loads engage_signin.js when the
   * document is ready. The binding is created with one() so that this event
   * handler is fired only once.
   *
   * Bind handlers to the user login and register dialogs in order to inject the
   * janrain engage widget.
   */
  Drupal.behaviors.rpxLoadJanrainEngage = {
    attach: function (context, settings) {
      var rpxNameSpace = 'rpxLoadJanrainEngage';
      // Load the janrain engage scripts just once on the document ready event.
      $(document).one('ready.rpxLoadJanrainEngage', {context: context, settings: settings}, loadJanrainEngage);
      if (settings.rpxSuppress) {
        // rpxSuppress may be set if e.g. the user is in the middle
        // of the registration flow.
        $('.user-login-dialog, .user-register-form-dialog')
        .find('.rpx-signin')
        // Remove inlined display and width styles.
        .removeAttr('style')
        .hide()
        .end()
        // Don't let dialogopen events within this namespace fire.
        .unbind('.' + rpxNameSpace)
        // We'll want to rebind the dialog events once the rpxSuppress is removed.
        .removeClass(rpxNameSpace + '-processed')
        // Resize the dialog.
        .trigger('contentupdate');
        return;
      }
      // Prepare the login and register dialogs.
      $('.user-login-dialog, .user-register-form-dialog').once(rpxNameSpace, function (context, settings) {
        $(this)
        .bind('dialogopen.' + rpxNameSpace, {context: context, settings: settings}, insertWidget)
        .bind('dialogclose.' + rpxNameSpace, {context: context, settings: settings}, removeWidget);
      });
      // Insert the or divider on pages where the janrain widget is loaded.
      $(document).ready(function (event) {
        // Create the rpx-or element.
        $('.rpx-signin', '.block').once('rpx-or', function (index) {
          $(this).prepend($('<div>', {
              text: Drupal.t('or')
            }).addClass('rpx-or')
          );
        });
      });
    },
    getRPXOrHeight: function ($methods) {
      var $userMethod = $('.user-fields', $methods),
      orHeight = $.map($userMethod, function (index, element) {
        var $this = $(this),
        height = $userMethod.height(),
        actionsHeight = $userMethod.find('.form-actions').outerHeight(true),
        tosHeight = $userMethod.find('.tos-and-disclaimer').outerHeight(true);

        return (height - actionsHeight - tosHeight) || false;
      });
      return orHeight;
    }
  };

  /**
   * ??
   */
  Drupal.behaviors.rpxPathTree = {
    attach: function (context, settings) {
      $('table.rpx-path-tree', context).once('rpx-path-tree', function () {
        $(this).treeTable();
      });
    }
  };

  /**
   * ??
   */
  Drupal.behaviors.rpxPathInsert = {
    attach: function (context, settings) {
      Drupal.settings.rpxPathInput = $('.rpx-path-input', context).eq(0);

      $('.rpx-path-click-insert .rpx-path', context).once('rpx-path-click-insert', function () {
        var newThis = $('<a href="javascript:void(0);" title="' + Drupal.t('Insert this path into your form') + '">' + $(this).html() + '</a>').click(function () {
          Drupal.settings.rpxPathInput.val($(this).text());
          // Compensation for the toolbar's height.
          var scrollCorrection = $('#toolbar') ? $('#toolbar').height() + 120 : 0;
          $('html,body').animate({scrollTop: $('.rpx-field-title-input').offset().top - scrollCorrection}, 500);
          return false;
        });
        $(this).html(newThis);
      });
    }
  };

}(jQuery, window, document));

/**
 * @WARNING
 *
 * This function needs global scope because janrain_engage.js simply calls it
 * bare from the window.
 */
function janrainWidgetOnload() {
  // For the widgets on the /user page, call init().  Overlays are handled
  // within insertWidget(), above.
  jQuery('#block-system-main .rpx-signin').once('rpx-init', function() {
    janrain.engage.signin.widget.init();
  });

  janrain.events.onProviderLoginToken.addHandler(function (tokenResponse) {
    jQuery.ajax({
      type: 'POST',
      url: window.janrain.settings.tokenUrl,//'/janrain/token_handler',
      data: {'token': tokenResponse.token, 'ajax': 1},
      dataType: 'json',
      success: function (signinResponse) {
        var link;
        switch (signinResponse.status) {
        case 'signup_failed':
          // Load the registration form and let the user provide
          // required missing data.
          // If there is a dialog_user.module link on the page, click that to go to the 
          // registration form in a dialog overlay.
          if ((link = jQuery('a[href^="/user/register/nojs"]')) && link.length) {
            link.first().click(); 
          }
          else {
            // If there is no dialog_user signup link on the page, redirect to the 
            // registration form.  <link>.click() doesn't work in all browsers for 
            // actually following links (only triggers bound event listeners.)
            window.location = '/user/register';
          }
          break;
        case 'signin_failed':
          // Load the login form and let the user know what
          // happenned.
          if ((link = jQuery('a[href^="/user/login/nojs"]')) && link.length) {
            link.first().click();
          }
          else {
            window.location = '/user/login';
          }
          break;
        case 'signup':
          // Close the dialog by reloading the page.
          window.location.reload(true);
          break;
        case 'signin':
          // Close the dialog by reloading the page.
          window.location.reload(true);
          break;
        case 'error':
          // Close the dialog by reloading the page.
          window.location.reload(true);
          break;
        default:
          // Close the dialog by reloading the page.
          window.location.reload(true);
        }
      }
    });
  });
}
