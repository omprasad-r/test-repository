/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true document: true */

/**
 * Adds a breadcrumb of the current page title
 */

(function ($, Drupal, window, document) {

  Drupal.behaviors.fpmgPageTitleBreadcrumb = {
    attach: function (context, settings) {
      $('.stack-content-inner').once('fpmg-page-title-breadcrumb', function (index) {
        var $this = $(this),
        $breadcrumb = $('.breadcrumb');

        // Check to see if the breadcrumb is active. If it is, find the page title
        // and append it to the breadcrumbs.
        if ($breadcrumb.length > 0) {
          // Grab the page title
          var $pageTitle = $this
          .find('#page-title')
          .text();
          // Append the page title to the end of the breadcrumb.
          $('<span>', {
            text: ' » ' + $pageTitle
          })
          .appendTo($breadcrumb);
        }
      });
    }
  };
}(jQuery, Drupal, window, document));
