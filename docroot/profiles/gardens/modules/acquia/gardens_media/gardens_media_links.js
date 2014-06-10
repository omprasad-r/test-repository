(function ($) {

  /**
   * Add dialog behavior to /upload links for anonymous users.
   */
  Drupal.behaviors.gardensMediaLinks = {};
  Drupal.behaviors.gardensMediaLinks.attach = function (context, settings) {
    if (settings.gardensFeatures && settings.gardensFeatures.userIsAnonymous && settings.gardensFeatures.dialogUserEnabled) {
      var useCapture = settings.janrainCapture && settings.janrainCapture.enforce;
      var loginPath = useCapture ? '/user/login' : '/user/login/ajax';
      // Modify all /upload links so that anonymous users are presented
      // a login dialog.
      var links = $('a[href^="/upload"], a[href^="http://' + location.host + '/upload"]').once('user-dialog', function () {
        // Only act on the following types of links:
        // /upload, /upload/image, /upload/video
        // Ignore links that were already set up correctly on the server side.
        if ($(this).attr('href').indexOf('/nojs') === -1 && $(this).attr('href').indexOf('/ajax') === -1) {
          if ($(this).attr('href').match(/\/upload$/)) {
            $(this).attr('href', loginPath + '?destination=/upload');
            if (!useCapture) {
              $(this).addClass('use-ajax use-dialog');
            }
          }
          else if ($(this).attr('href').match(/\/upload\/(video|photo)$/)) {
            var newHref = $(this).attr('href').replace(/\/upload\/(video|photo)/,  loginPath + '?destination=/upload/$1');
            $(this).attr('href', newHref);
            if (!useCapture) {
              $(this).addClass('use-ajax use-dialog');
            }
          }
        }
      });
      if (links.length === 0) {
        return;
      }
      // The AJAX and dialog behaviors have already run; rerun them to pick up
      // newly ajaxified links.
      Drupal.behaviors.AJAX.attach(context, settings);
      Drupal.behaviors.dialog.attach(context, settings);
      if (useCapture) {
        Drupal.behaviors.janrainCapture.attach(context, settings);
        Drupal.behaviors.janrainCaptureUi.attach(context, settings);
      }
    }
  };

})(jQuery);