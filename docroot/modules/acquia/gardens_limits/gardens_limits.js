(function($) {

Drupal.behaviors.gardens_limits = {
  attach: function (context) {
    // Add behaviors to plupload.
    if (typeof plupload != "undefined") {
      var uploader = $('.plupload-element').pluploadQueue();
      if (uploader) {
        uploader.bind('QueueChanged', Drupal.gardens_limits.checkLimit);
        uploader.bind('Error', Drupal.gardens_limits.fileUploadErrorHandler);
      }
    }

    // Resize the iframe.
    Drupal.media.browser.resizeIframe();
  }
};

Drupal.gardens_limits = {};

Drupal.gardens_limits.checkLimit = function (up) {
  if ((Drupal.settings.gardens_limits.storage.unlimited != 1) && (this.total.size > Drupal.settings.gardens_limits.storage.remaining)) {
    // Stop the upload in progress
    up.stop();

    // Remove the item added last.  This will trigger QueueChanged event which
    // will cause this function to recurse until the file size total is under
    // the maximum additional disk space allowed.
    up.removeFile(this.files[this.files.length - 1]);

    // Display an error message if applicable.
    if ($('.messages.warning').length == 0) {
      $('#media-browser-page').prepend(Drupal.gardens_limits.messageText());
      Drupal.media.browser.resizeIframe();
    }
  }
};

Drupal.gardens_limits.fileUploadErrorHandler = function (up, args) {
  // If the server denied access to our file upload (likely because we hit the
  // file storage limit; see gardens_limits_plupload_handle_uploads()), stop
  // uploading and submit the form immediately. This allows the server to
  // process any valid files that were uploaded before this one, and also to
  // give the user immediate feedback on the failure.
  // @todo: This doesn't seem to work in the Flash runtime (the handler does
  //   not get triggered). However, in that case we still prevent the file from
  //   being uploaded server side, and since this is just a UI improvement for
  //   a situation that is a pretty extreme edge case (the site got pushed over
  //   the file storage limit after the files were queued in plupload but
  //   before they were actually uploaded), it's not that big of a deal.
  if (args.status && args.status == 403) {
    up.stop();
    var $form = $('#media-add-upload-multiple');
    // The standard plupload submit handler waits for all files to be uploaded
    // before the form is submitted. That will never happen in this case, so we
    // have to unbind it before proceeding.
    $form.unbind('submit');
    $form.trigger('submit');
  }
};

Drupal.gardens_limits.messageText = function () {
  var text = $('<div class="messages warning"></div>');
  text.html(Drupal.settings.gardens_limits.storage.message);
  return text;
};

})(jQuery);
