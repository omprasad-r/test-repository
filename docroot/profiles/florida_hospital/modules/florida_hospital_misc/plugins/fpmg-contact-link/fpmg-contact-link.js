/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true document: true */

/**
 * Pulls the contact and promo links out of the sidebar wrapper
 */

(function ($, Drupal, window, document) {

  Drupal.behaviors.fpmgContactLink = {
    attach: function (context, settings) {
      $('.block-contact-link, .block-promo').once('fpmg-contact-link', function (index) {
        var $this = $(this),
        $sidebar = $this.closest('.sidebar');
        $this
        .show()
        .detach()
        .hide()
        .appendTo($sidebar)
        .fadeIn();
        // Remove the sidebar if it is now empty.
        var $blocks = $sidebar.find('.region .block');
        if ($blocks.length === 0) {
          $sidebar.children('.region').remove();
        }
      });
    }
  };
}(jQuery, Drupal, window, document));
