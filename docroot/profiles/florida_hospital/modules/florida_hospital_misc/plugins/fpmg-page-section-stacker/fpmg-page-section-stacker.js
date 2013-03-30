/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true document: true */

/**
 * Moves the page title into its own stack above the content.
 */
(function ($, Drupal, window, document) {

  Drupal.behaviors.pageSectionStacker = {
    attach: function (context, settings) {
      $('body').once('page-section-stacker', function (index) {
        if ($.isFunction($.pageBanner)) {
          $.pageBanner({
            sections: [
              {
                path: 'medical-team',
                label: Drupal.t('Medical Team')
              },
              {
                path: 'programs-and-specialties',
                label: Drupal.t('Programs and Specialties')
              },
              {
                path: 'services-and-specialties',
                label: Drupal.t('Services and Specialties')
              },
              {
                path: 'news-room',
                label: Drupal.t('News Room')
              },
              {
                path: 'locations',
                label: Drupal.t('Locations and Directions')
              },
              {
                path: 'locations-and-directions',
                label: Drupal.t('Locations and Directions')
              },
              {
                path: 'locations-directions',
                label: Drupal.t('Locations and Directions')
              },
              {
                path: 'patient-resources',
                label: Drupal.t('Patient Resources')
              },
              {
                path: 'contact-us',
                label: Drupal.t('Contact us')
              },
              {
                path: 'contact',
                label: Drupal.t('Contact us')
              },
              {
                path: 'testimonials',
                label: Drupal.t('Testimonials')
              }
            ]
          })
          .insertBefore('.stack-content-inner');
        }
      });
    }
  };
}(jQuery, Drupal, window, document));