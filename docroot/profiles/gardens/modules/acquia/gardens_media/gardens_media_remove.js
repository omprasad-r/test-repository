(function ($) {

  Drupal.behaviors.gardens_media_remove = {
    attach: function (context, settings) {
      // If we have more than one media item hide the one that isn't the newly fetched one.
      // Also hide the remove button that came with the non saved one.
      if ($('.media-item').length > 1) {
        $('.media-item').not('#media-item-fetched > .media-item').hide();
        $('#remove-button').hide();
      }
      // If this is an embed clip we want to show the embed button
      // If we have a new fetched media clip DO NOT hide the field or embed button.
      if ($('#media-item-fetched').length == 0) {
        $('.gardens-media-embed-code-button').hide();
        $('#gardens-media-widget').parent().hide();
      }
      // If the remove button is clicked hide the existing thumbnail
      // and show the upload field
      $('#remove-button').click(function(e){
          $('.media-item').toggle();
          $('#gardens-media-widget').parent().toggle();
          $(this).toggle();
          $('.gardens-media-embed-code-button').toggle();
          // Hide the upload field initially
          e.preventDefault();
      });
    }
  }
})(jQuery);