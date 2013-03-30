/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true document: true */

/*
 * Formats and styles the blog view
 */

(function ($, Drupal, window, document) {

  Drupal.behaviors.fpmgMediaItem = {
    attach: function (context, settings) {
      $('.node-health-blog, .node-blog, .node-article, .node-events-calendar').once('fpmg-media-item', function (index) {
        var $this = $(this);
        // Set up a media item float/overflow layout.
        $this
        .find('.field-name-field-thumbnail')
        .addClass('media-item-thumbnail')
        .insertBefore(
          $this.children('.float-overflow')
        );
      });
    }
  };
}(jQuery, Drupal, window, document));
