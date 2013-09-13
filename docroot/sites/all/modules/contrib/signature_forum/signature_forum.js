(function ($) {

Drupal.behaviors.signatureForum = {
  attach: function (context) {
    // Provide a vertical tab summary for short content behaviour.
    $('fieldset#edit-signature-forum-short-content', context).drupalSetSummary(function (context) {
      var action = $('input[name="signature_forum_short_content_action"]:checked', context).val(),
          min = parseInt($('#edit-signature-forum-short-content-length', context).val());

      if (action == -1) {
        return Drupal.t('None');
      } else if (action == 0) {
        return Drupal.t('Hide, if the content is shorter than %min characters', {'%min': min});
      } else {
        var format = $('#edit-signature-forum-short-content-format option:selected').text();
        return Drupal.t('Run through %format, if the content is shorter than %min characters', {'%min': min, '%format': format});
      }
    });

    // Provide a vertical tab summary for per-conversation signature settings.
    $('fieldset#edit-signature-forum-show-once', context).drupalSetSummary(function (context) {
      var showOnceOptions = $('input[name="signature_forum_show_once_options"]:checked', context).val();

      if (showOnceOptions == 1) {
        return Drupal.t('Show each signature once');
      } else {
        return Drupal.t('Show signatures for every post');
      }
    });

    // Provide a vertical tab summary for per-post signature settings.
    $('fieldset#edit-signature-forum-defaults', context).drupalSetSummary(function (context) {
      var sel = $('input[name="signature_forum_defaults_mode"]:checked', context).val();
      if (sel == 0) {
        return Drupal.t('None');
      } else if (sel == 1) {
        return Drupal.t('Global');
      } else {
        return Drupal.t('Per user');
      }
    });

    // Provide a vertical tab summary for limit settings.
    $('fieldset#edit-signature-forum-max', context).drupalSetSummary(function (context) {
      var chars = parseInt($('#edit-signature-forum-max-characters', context).val()),
          lines = parseInt($('#edit-signature-forum-max-lines', context).val()),
          vals = [];
      vals.push(Drupal.t('%count characters', {'%count': chars}));
      if (!isNaN(lines) && lines != 0) {
        vals.push(Drupal.t('%count lines', {'%count': lines}));
      }
      return vals.join(', ');
    });
  }
};

})(jQuery);
