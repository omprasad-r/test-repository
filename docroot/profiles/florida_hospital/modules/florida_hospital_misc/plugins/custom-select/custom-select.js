/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true document: true */

/**
 * Creates a fake select box from spans. This provides a hook to styles select
 * boxes across browser platforms in a consistent way.
 */
(function ($, Drupal, window, document) {
  Drupal.behaviors.customSelect = {
    attach: function (context, settings) {
      if (!$.browser.msie || ($.browser.msie && $.browser.version > 6)) {
        $('.page').find('select').once('custom-select', function () {
          var $this = $(this),
          width = $this.width(),

          // Create a fake select box element. Inner element first...
          $inner = $('<span>', {
            text: $this.find(':selected').text()
          })
          .css({
            width: width,
            display: 'inline-block'
          })
          .addClass('.inner'),
          // ...then the outer element.
          $fakeSelect = $('<span>', {
            html: $inner
          })
          .css({
            display: 'inline-block'
          })
          .addClass('customstyle-select')
          .insertAfter($this);
          // Style the select element.
          $this
          .css({
            position: 'absolute',
            opacity: 0,
            height: $fakeSelect.outerHeight(true)
          });

          // Register a change event handler on the select element.
          $this.change(function (event) {
            $inner.text(
              $this
              .find(':selected')
              .text()
            );
          });
        });
      }
    }
  };
}(jQuery, Drupal, window, document));
