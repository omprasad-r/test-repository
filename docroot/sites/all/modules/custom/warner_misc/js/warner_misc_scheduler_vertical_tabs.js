(function ($) {

  /**
   * Provide summary information for vertical tabs.
   *
   * Overrides the original behavior of 'scheduler' module. Adds date info to
   * the vertical tab summary.
   */
  Drupal.behaviors.scheduler_settings = {
    attach: function (context) {

      // Add the theme name as an additional class to the vertical-tabs div. This can then be used
      // in scheduler.css to rectify the style for collapsible fieldsets where different themes
      // need slightly different fixes. The theme is available in ajaxPageState.
      var theme = Drupal.settings.ajaxPageState['theme'];
      $("div.vertical-tabs").addClass(theme);

      // Provide summary when editting a node.
      $('fieldset#edit-scheduler-settings', context).drupalSetSummary(function (context) {
        var vals = [];
        if ($('#edit-publish-on').val() || $('#edit-publish-on-datepicker-popup-0').val()) {
          vals.push(Drupal.t('Scheduled publishing on !date !time !timezone', {
            '!date': $('#edit-publish-on-datepicker-popup-0').val(),
            '!time': $('#edit-publish-on-timepicker-popup-1').val(),
            '!timezone': Drupal.settings.warnerMiscScheduler.userTimezoneOffset
          }));
        }
        if ($('#edit-unpublish-on').val() || $('#edit-unpublish-on-datepicker-popup-0').val()) {
          vals.push(Drupal.t('Scheduled unpublishing on !date !time !timezone', {
            '!date': $('#edit-unpublish-on-datepicker-popup-0').val(),
            '!time': $('#edit-unpublish-on-timepicker-popup-1').val(),
            '!timezone': Drupal.settings.warnerMiscScheduler.userTimezoneOffset
          }));
        }
        if (!vals.length) {
          vals.push(Drupal.t('Not scheduled'));
        }
        return vals.join('<br/>');
      });

      // Provide summary during content type configuration.
      $('fieldset#edit-scheduler', context).drupalSetSummary(function (context) {
        var vals = [];
        if ($('#edit-scheduler-publish-enable', context).is(':checked')) {
          vals.push(Drupal.t('Publishing enabled'));
        }
        if ($('#edit-scheduler-unpublish-enable', context).is(':checked')) {
          vals.push(Drupal.t('Unpublishing enabled'));
        }
        return vals.join('<br/>');
      });

    }
  };

})(jQuery);