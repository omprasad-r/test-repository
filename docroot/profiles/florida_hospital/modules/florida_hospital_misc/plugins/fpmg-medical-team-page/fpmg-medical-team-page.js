/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true document: true */

/**
 * Places the h1 tag on a medical team member page inside the bio-wrapper.
 */
(function ($, Drupal, window, document) {

  Drupal.behaviors.fpmgMedicalTeamPage = {
    attach: function (context, settings) {
      if ($('body').hasClass('node-type-medical-team')) {
        $('.wrapper-content h1').once('medical-team-page', function (index) {
          $(this).prependTo('.bio-wrapper-right', '.node-medical-team').show(0);
        });
      }
    }
  };
}(jQuery, Drupal, window, document));
