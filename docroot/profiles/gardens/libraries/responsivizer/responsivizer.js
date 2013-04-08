/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global window: true define: true Drupal: true */

/**
 *
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
          logger("\"responsivizer\" failed to run because " + required[i] + " is not present.");
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
  var plugin = 'responsiviser';
  // Local copy of jQuery.
  var $ = window.jQuery;
  var Drupal = window.Drupal;

  Drupal.behaviors[plugin] = {
    attach: function (context, settings) {
      $('body').once(plugin, function (index) {
        var $pageWidth = $('.page-width');
        var $stackWidth = $('.stack-width');
        var $content = $('#content');
        var $sidebars = $content.find('.sidebar');
        var $main = $content.find('#main');
        var $menu = $('.wrapper-header .block .menu');
        var $box = $main.parent('.box');
        var $menus = $('.horizontal.stack');
        // Breakpoint step functions.
        function mobileBreak() {
          // Make the page 100% width.
          $pageWidth.addClass('responsive-flex');
          $stackWidth.addClass('responsive-flex');
          $sidebars.addClass('responsive-sidebar');
          $menu.addClass('responsive-menu');
          // Move the main content to the top of the content box.
          $main.prependTo($box);
          // Show any hidden sidebars.
          $('.sidebar:hidden').not('.tb-hidden').show();
          // Remove the main menu event handlers added by the superfish plugin.
          $('.pulldown-processed li').unbind('mouseover mouseout');
        }
        function tabletBreak() {
          // Make the page 100% width.
          $pageWidth.addClass('responsive-flex');
          $stackWidth.addClass('responsive-flex');
          $sidebars.addClass('responsive-sidebar');
          $menu.addClass('responsive-menu');
          // Move the main content to the top of the content box.
          $main.prependTo($box);
        }
        function flexiBreak() {
          // Make the page max-width 960px.
          $pageWidth.addClass('responsive-flex');
          $stackWidth.addClass('responsive-flex');
          $sidebars.removeClass('responsive-sidebar');
          $menu.removeClass('responsive-menu');
          // Move the main content below the sidebars.
          $main.appendTo($box);
        }
        function desktopBreak() {
          // Make the page fixed width.
          $pageWidth.removeClass('responsive-flex');
          $stackWidth.removeClass('responsive-flex');
          $sidebars.removeClass('responsive-sidebar');
          $menu.removeClass('responsive-menu');
          // Move the main content below the sidebars.
          $main.appendTo($box);
        }
        // Turn off automatic column resizing.
        if (Drupal.behaviors.acquia && Drupal.behaviors.acquia.equalCols) {
          Drupal.behaviors.acquia.equalCols = false;
        }
        // Remove the min height on columns if the resizing already ran.
        $('.tb-height-balance').css({
          'min-height': 0
        });
        // Don't responsivize the theme is the viewport tag isn't present.
        if (!$('[name="viewport"]', 'head').length > 0) {
          return;
        }
        // Pass the step handlers to the BreakUp plugin.
        var responder = new $.BreakUp({
          '0': mobileBreak,
          '480': tabletBreak,
          '768': flexiBreak,
          '960': desktopBreak
        }, {'namespace': 'gardens_features'}, $(document));
      });
    }
  };
}));
