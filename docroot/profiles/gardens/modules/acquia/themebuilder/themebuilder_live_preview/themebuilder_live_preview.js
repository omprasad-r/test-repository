Drupal.behaviors.themebuilderLivePreview = {
  attach: function (context, settings) {
    if (jQuery('body').hasClass('themebuilder-live-preview')) {
      jQuery('#toolbar-link-admin-appearance').removeAttr('href').css('cursor', 'pointer').click(function() {
        alert('You cannot use ThemeBuilder when viewing an unpublished theme.');
      });
    }
  }
};
