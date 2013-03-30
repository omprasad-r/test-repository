(function ($) {

  function preventSubmit(event) {
    return false;
  }

  function handleInitialSubmit(event) {
    var f = $.proxy(preventSubmit, this);
    $(this)
    .unbind('.preventMultiSubmits')
    .bind('submit.preventMultiSubmits', f);
  }

  Drupal.behaviors.preventMultiSubmits = {
    attach: function (context, settings) {
      $('.comment-form').once('prevent-multi-submits', function (index) {
        var f = $.proxy(handleInitialSubmit, this);
        $(this).bind('submit.preventMultiSubmits', f);
      });
    }
  };
} (jQuery));
