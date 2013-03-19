/**
 * @file
 * Admin-related JavaScript behaviors for the MASt module.
 */

(function ($) {

Drupal.behaviors.mastFieldsetSummary = {
  attach: function (context) {
    $('fieldset.mast-settings-form', context).drupalSetSummary(function (context) {
      if ($('.form-item-mast-enabled input', context).is(':checked')) {
        return Drupal.t('Enabled');
      }
      else {
        return Drupal.t('Disabled');
      }
    });
  }
};

})(jQuery);

