(function ($) {

Drupal.site_export = Drupal.site_export || {};
  
Drupal.behaviors.site_export = {
  attach: function (context, settings) {
    $('select[name=reason]').bind('change', Drupal.site_export.toggle).trigger('change');
  }
};

Drupal.site_export.toggle = function (event) {
  var $textarea = $('textarea[name=explanation]').closest('.form-item');
  var value = $(this).val();
  if (value) {
    $textarea.show();
    $textarea.find('label').text(Drupal.settings.site_export[value]);
  } else {
    $textarea.hide();
    $textarea.find('label').text(Drupal.settings.site_export.default);
  }
};
  
})(jQuery);