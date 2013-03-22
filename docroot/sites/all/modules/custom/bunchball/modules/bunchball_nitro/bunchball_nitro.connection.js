(function ($) {
  Drupal.bunchball = Drupal.bunchball || {};
  Drupal.bunchball.userCommandsArray = Drupal.bunchball.userCommandsArray || [];

  Drupal.behaviors.bunchballNitroConnection = {
    attach: function (context, settings) {
      var connectionParams = Drupal.settings.bunchballNitroConnection.connectionParams;
      Drupal.bunchball.nitro = new Nitro(connectionParams);
    }
  };

  Drupal.bunchball.setUserId = function() {
    var userId = Drupal.bunchball.nitro.connectionParams.userId;
    Drupal.bunchball.nitro.setUserId(userId);
    Drupal.bunchball.nitro.getUserId(Drupal.bunchball.gotCurrentUserId);
  };

  // Callback function for acquiring the User ID of the current user.
  Drupal.bunchball.gotCurrentUserId = function(inUserId) {
    Drupal.bunchball.currentUserId = inUserId;
  };

  Drupal.bunchball.WorkerQueue = {
    submitNitroAPICall: function (tags) {
      var params = new Array();

      params[0] = 'method=' + encodeURIComponent('user.logAction');
      params[1] = 'sessionKey=' + encodeURIComponent(Drupal.bunchball.userCommandsArray[0].ses);
      params[2] = 'tags=' + encodeURIComponent(tags);

      var queryString = params.join('&');

      this.nitroCallback("data", "token");
      Drupal.bunchball.nitro.callAPI(queryString, "Drupal.bunchball.WorkerQueue.nitroCallback");
    },

    nitroCallback: function (data, token) {
      // remove from array
      if (Drupal.bunchball.userCommandsArray.length > 0) {
        Drupal.bunchball.userCommandsArray.splice(0, 1);
      }

      this.nitroIterateQueue();
    },

    nitroLogin: function (userId) {
      var connectionParams = Drupal.bunchball.nitro.connectionParams;
      var params = new Array();

      params[0] = 'method=' + encodeURIComponent('user.login');
      params[1] = 'apiKey=' + encodeURIComponent(connectionParams.apiKey);
      params[2] = 'userId=' + encodeURIComponent(userId);
      params[3] = 'ts=' + encodeURIComponent(connectionParams.timeStamp);
      params[4] = 'sig=' + encodeURIComponent(connectionParams.signature);

      var loginQuery = params.join('&');

      Drupal.bunchball.nitro.callAPI(loginQuery, "Drupal.bunchball.WorkerQueue.nitroLoginCallback");
    },

    nitroLoginCallback: function (data, token) {
      // this is a stub that can be used later to track responses from the server.
      Drupal.bunchball.userCommandsArray[0].ses = data['Nitro']['Login']['sessionKey'];

      // do the nitro API call..
      this.submitNitroAPICall(Drupal.bunchball.userCommandsArray[0].tags);
    },

    nitroIterateQueue: function () {
      // proceed to log in next user
      if (Drupal.bunchball.userCommandsArray.length > 0) {
        this.nitroLogin(Drupal.bunchball.userCommandsArray[0].uid);
      }
    }
  };
})(jQuery);
