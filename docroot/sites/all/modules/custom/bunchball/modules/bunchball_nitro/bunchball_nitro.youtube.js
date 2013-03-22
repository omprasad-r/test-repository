(function ($) {
  Drupal.bunchball = Drupal.bunchball || {};
  Drupal.bunchball.userCommandsArray = Drupal.bunchball.userCommandsArray || [];
  Drupal.bunchball.playerPlayed = 0;

  Drupal.bunchball.youtubeActions = {
    /**
     * Temporary command storage, for the case when the Youtube video started
     * before the Nitro library could initialize.
     */
    commandsArray: [],

    /**
     * Youtube video event listeners.
     *
     * https://developers.google.com/youtube/js_api_reference#Events
     */
    nitroVideoStateChange: function(newState) {
      // empty action to prevent anything from going forward
      var youtubeData = Drupal.settings.bunchballNitroYoutube;
      var action = "";

      if (newState === 0) {
        // Video ended.
        action = youtubeData.artistEnd;
        Drupal.bunchball.playerPlayed = 0;
      } else if (newState === 1 && Drupal.bunchball.playerPlayed === 0) {
        // Video started.
        action = youtubeData.artistStart;
        Drupal.bunchball.playerPlayed = 1;
      }

      // only continue if there is something in Action
      if (action.length > 1) {
        action = action + ", Artist: " + youtubeData.artistName + ", Category: " + youtubeData.artistCategory;

        var inObj = {};
        inObj.tags = action;
        inObj.ses = '';

        if (typeof Drupal.bunchball.currentUserId === 'undefined') {
          inObj.uid = '';
          this.commandsArray.push(inObj);
        }
        else {
          inObj.uid = Drupal.bunchball.currentUserId;
          Drupal.bunchball.userCommandsArray.push(inObj);
        }

        this.youtubeWaitForBunchballNitroInit();
      }
    },

    youtubeWaitForBunchballNitroInit: function() {
      if (typeof Drupal.bunchball.nitro !== 'undefined' && typeof Drupal.bunchball.currentUserId !== 'undefined' && typeof Drupal.bunchball.WorkerQueue.nitroIterateQueue === 'function') {
        if (this.commandsArray.length !== 0) {
          for (var inObj in Drupal.bunchball.userCommandsArray) {
            Drupal.bunchball.userCommandsArray[inObj].uid = Drupal.bunchball.currentUserId;
          }
          Drupal.bunchball.userCommandsArray.push.apply(this.commandsArray);
          this.commandsArray = [];
        }
        Drupal.bunchball.WorkerQueue.nitroIterateQueue();
      }
      else {
        setTimeout(this.youtubeWaitForBunchballNitroInit, 1000);
      }
    }
  };
})(jQuery);

/**
 * Your HTML pages that display the chromeless player must implement
 * a callback function named onYouTubePlayerReady.
 *
 * The API will call this function when the player is fully loaded
 * and the API is ready to receive calls.
 *
 * https://developers.google.com/youtube/js_api_reference#EventHandlers
 */
function onYouTubePlayerReady(playerId) {
  // attach the listener
  (function ($) {
    $("div.oembed-video .oembed-content object embed").each(function (i) {
      this.addEventListener("onStateChange", "Drupal.bunchball.youtubeActions.nitroVideoStateChange");
    });
  })(jQuery);
}
