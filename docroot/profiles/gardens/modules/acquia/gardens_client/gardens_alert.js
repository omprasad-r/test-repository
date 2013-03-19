
Drupal.gardensClient = Drupal.gardensClient || {};

/**
 * Success callback fired after the nag box has been faded out.
 */
Drupal.gardensClient.hideMessagesRegion = function () {
  var $ = jQuery;
  var messages = $(this).siblings(':visible');
  if (!messages.length) {
    $("#messages").hide();
  }
};

/**
 * Ajax callback triggered when an alert is successfully dismissed
 */
Drupal.ajax.prototype.commands.fadeAlert = function (ajax, response, status) {
  var $ = jQuery;
  $(ajax.wrapper).fadeOut('slow', Drupal.gardensClient.hideMessagesRegion);
};
