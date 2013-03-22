(function ($) {
  Drupal.behaviors.request_queue_runner = {
    attach: function(context, settings) {
      var queues, queue_url;
      if (Drupal.settings.request_queue && Drupal.settings.request_queue.queues) {
        queues = Drupal.settings.request_queue;
      }
      else {
        queues = {'queues': ['request_queue']};
      }
      queue_url = Drupal.settings.basePath + 'request_queue';
      $.ajax({
        url: queue_url,
        type: "POST",
        data: queues,
      });
    }
  };
}(jQuery));