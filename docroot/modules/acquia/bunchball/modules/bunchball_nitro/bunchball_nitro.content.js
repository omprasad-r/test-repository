(function($) {
  Drupal.bunchball = Drupal.bunchball || {};
  Drupal.bunchball.userCommandsArray = Drupal.bunchball.userCommandsArray || [];

  Drupal.behaviors.bunchballNitroContent = {
    attach: function (context, settings) {
      if (typeof Drupal.bunchball.nitro !== 'undefined') {
        Drupal.bunchball.setUserId();
      }

      // User viewed content. We are in a node.
      if (typeof Drupal.settings.bunchballNitroNode.nodeID !== 'undefined') {
        var waitForCurrentUserId = function() {
          if (typeof Drupal.bunchball.currentUserId === 'undefined') {
            setTimeout(waitForCurrentUserId, 1000)
          }
          else {
            Drupal.bunchball.nodeActions.userViewedContent();
          }
        }
        waitForCurrentUserId();
      }
    }
  };

  Drupal.bunchball.nodeActions = {
    // ViewedContent is called because the user is currently viewing content.
    userViewedContent: function() {
      var nodeData = Drupal.settings.bunchballNitroNode;
      if (nodeData.viewAction) {
        var sentTags = nodeData.viewAction + ', Title: ' + nodeData.nodeTitle + ', Category: ' + nodeData.nodeCategory;

        var inObj = {};
        inObj.uid = Drupal.bunchball.currentUserId;
        inObj.tags = sentTags;
        inObj.ses = '';
        Drupal.bunchball.userCommandsArray.push(inObj);
      }

      if (nodeData.viewReceiveAction) {
        var sentTags = nodeData.viewReceiveAction + ', Title: ' + nodeData.nodeTitle + ', Category: ' + nodeData.nodeCategory;

        var inObj = {};
        inObj.uid = nodeData.nodeUID;
        inObj.tags = sentTags;
        inObj.ses = '';
        Drupal.bunchball.userCommandsArray.push(inObj);
      }

      this.nodeWaitForBunchballNitroInit();
    },

    nitroSocialShareClicked: function(network) {
      if(network.length > 0) {
        var action = "Share_Link, Network: " + network;

        var inObj = {};
        inObj.uid = Drupal.bunchball.currentUserId;
        inObj.tags = action;
        inObj.ses = '';
        Drupal.bunchball.userCommandsArray.push(inObj);

        Drupal.bunchball.WorkerQueue.nitroIterateQueue();
      }
    },

    nodeWaitForBunchballNitroInit: function() {
      if (typeof Drupal.bunchball.nitro !== 'undefined' && typeof Drupal.bunchball.WorkerQueue.nitroIterateQueue === 'function') {
        Drupal.bunchball.WorkerQueue.nitroIterateQueue();
      }
      else {
        setTimeout(this.nodeWaitForBunchballNitroInit, 1500);
      }
    }
  };
}) (jQuery);
