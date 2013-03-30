/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true document: true */

/**
 * Inserts the footer logo into the copyright region of the theme.
 */
(function ($, Drupal, window, document) {

  function stackTemplate() {
    var $tmpl = $('<div>', {
      html: $('<div>', {
        html: $('<div>', {
          html: $('<div>', {
            html: $('<div>').addClass('region region-copyright')
          }).addClass('box')
        }).addClass('stack-width')
      }).addClass('page-width inner')
    }).addClass('stack-copyright stack clearfix tb-scope');
    return $tmpl;
  }

  function getLogo() {
    var $logo = $('<div>', {
      html: $('<span>', {
        text: Drupal.t('Florida Hospital Medical Group (FHMG)'),
      }).addClass('fpmg-footer-logo')
    }).addClass('fpmg-footer');
    return $logo;
  }

  Drupal.behaviors.insertFooterLogo = {
    attach: function (context, settings) {
      $('body').once('fpmg-footer-logo', function (index) {
        var $logo = getLogo();
        // If the copyright stack is not present, create it.
        if ($('.stack-copyright').length === 0) {
          stackTemplate().insertAfter('.page');
        }
        // Insert the logo.
        $logo
        .hide()
        .prependTo('.region-copyright')
        .slideDown();
      });
    }
  };
}(jQuery, Drupal, window, document));
