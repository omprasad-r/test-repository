/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global window: true define: true Drupal: true */

/**
 * Centers the rotating banner when it is too large for the window.
 */
(function (factory) {
  // Print warnings to the console if it exists.
  function logger(message) {
    if (typeof window.console === 'object' && typeof window.console.log === 'function') {
      console.log(message);
    }
  }
  // Load this plugin with require.js if available.
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define(['jquery', 'drupal'], factory);
  }
  else {
    var i,
    required = ['jQuery', 'Drupal'];
    // Continue only if the required libraries are present.
    for (i in required) {
      if (required[i].hasOwnProperty(i)) {
        if (window[required[i]] === undefined) {
          // If jQuery is not defined, warn the user and return.
          logger("\"fpmg-responsivizer\" failed to run because " + required[i] + " is not present.");
          return null;
        }
      }
    }
    // Call the plugin factory.
    factory();
  }
}
// The plugin factory function.
(function () {
  var plugin = 'fpmg-responsiviser';
  /**
   * Make the site look nice in mobile phones.
   */
  Drupal.behaviors[plugin] = {
    attach: function (context, settings) {
      // Local copy of jQuery.
      var $ = window.jQuery;
      var Drupal = window.Drupal;
      var $content = $('#content');
      var $main = $('#main', $content);
      var $box = $main.parent('.box');
      var responsiveHandler = {
        breakpoint: '',
        updated: false,
        /**
        * Get the screen width.
        */
        getScreenWidth: function () {
          return window.innerWidth || document.documentElement.offsetWidth || document.documentElement.clientWidth;
        },
        /**
        * Check what breakpoint the screen is in.
        */
        getBreakPoint: function () {
          var screen = this.getScreenWidth();
          if (screen <= 480) {
            return 'mobile';
          }
          if (screen <= 768) {
            return 'tablet';
          }
          // Return desktop as a default.
          return 'desktop';
        },
        breakChangeHandler: function () {
          if (this.breakpoint === 'mobile' && !this.updated) {
            this.mobileBreak();
          }
          if (this.breakpoint === 'tablet' && !this.updated) {
            this.tabletBreak();
          }
          if (this.breakpoint === 'desktop' && !this.updated) {
            this.desktopBreak();
          }
        },
        breakCheck: function () {
          if (this.breakpoint !== this.getBreakPoint()) {
            // Save the current breakpoint in this scope.
            this.breakpoint = this.getBreakPoint();
            this.updated = false;
            $(document).trigger('breakChanged');
          }
        },
        mobileBreak: function () {
          // Move the main content to the top of the content box.
          $main.prependTo($box);
          // Show any hidden sidebars.
          $('.sidebar:hidden').not('.tb-hidden').show();
          // Remove the horizontal class from the postfooter.
          $('.stack-postfooter').removeClass('horizontal');
          // Remove the main menu event handlers added by the superfish plugin.
          $('.pulldown-processed li').unbind('mouseover mouseout');
        },
        tabletBreak: function () {
          // Add the horizontal class to the postfooter.
          $('.stack-postfooter').addClass('horizontal');
          // Move the main content to the top of the content box.
          $main.prependTo($box);
        },
        desktopBreak: function () {
          // Move the main content below the sidebars.
          $main.appendTo($box);
          // Add the horizontal class to the postfooter.
          $('.stack-postfooter').addClass('horizontal');
        }
      };
      /**
       * Prep the main content.
       */
      $('body').once(plugin, function (index) {
        // Turn off automatic column resizing.
        if (Drupal.behaviors.acquia && Drupal.behaviors.acquia.equalCols) {
          Drupal.behaviors.acquia.equalCols = false;
        }
        // Remove the min height on columns if the resizing already ran.
        $('.tb-height-balance').css({
          'min-height': 0
        });
      });
      /**
       * Handle breakpoint changes.
       */
      // Register a custom 'breakChanged' event on the document.
      var f = $.proxy(responsiveHandler.breakChangeHandler, responsiveHandler);
      $(document).bind('breakChanged' + '.' + plugin, f);
      // Register a handler on the window resize event.
      f = $.proxy(responsiveHandler.breakCheck, responsiveHandler);
      $(window).bind('resize' + '.' + plugin, f);
      $(window).bind('load' + '.' + plugin, f);
    }
  };
}));
