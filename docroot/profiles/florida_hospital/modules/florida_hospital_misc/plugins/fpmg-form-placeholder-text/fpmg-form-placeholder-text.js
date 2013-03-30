/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true document: true */

/**
 * Adds placeholder text to the Search box
 */

(function ($, Drupal, window, document) {

  Drupal.behaviors.fpmgFormPlaceholderText = {
    attach: function (context, settings) {
      // Find the argument, and apply the placeholder attribute and content
      $('.block-search .form-text').once('fpmg-form-placeholder-text', function (index) {
        $(this).attr({
          placeholder: Drupal.t('I\'m Looking For...')
        })
      });
    }
  };
}(jQuery, Drupal, window, document));
