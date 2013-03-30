/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true document: true */

/**
 * Centers the rotating banner when it is too large for the window.
 */
(function ($, Drupal, window, document) {

  function reposition() {
    var $this = $(this),
    delta = (document.documentElement.clientWidth - $this.width()) / 2;
    if (delta <= 0) {
      $this.css({left: delta});
    }
  }

  Drupal.behaviors.centerRotatingBanner = {
    attach: function (context, settings) {
      $('.rotating-banner').once('center-rotating-banner', function (index) {
        $(this).css({'position': 'relative'});
        var func = $.proxy(reposition, this);
        $(window).resize(func);
        $(document).ready(func);
      });
    }
  };
}(jQuery, Drupal, window, document));
