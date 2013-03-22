(function ($) {

Drupal.janrainCapture = {
  closeProfileEditor: function() {
    window.location.href = Drupal.settings.janrainCapture.profile_sync_url;
  },
  closeRecoverPassword: function() {
    window.location.reload();
  },
  token_expired: function() {
    window.location.href = Drupal.settings.janrainCapture.token_expired_url;
  },
  bp_ready: function() {   
      var ssojs = null;
      var ssotrue = false;
  	  var channelId = Backplane.getChannelID();
	  jQuery('script').each(function() {
		if(jQuery(this).attr('src')) {
		  ssojs = jQuery(this).attr('src');
	      if ( undefined != ssojs && ssojs.search(/sso.js/i) > 0 ) { 
	    	  ssotrue = true;
	    	  return false;
	      }
		}
      });
	  if (ssotrue) { 
	    // do sso - 
		console.log('Federated');
		JANRAIN.SSO.CAPTURE.check_login({
			  sso_server: "https://" + Drupal.settings.janrainCapture.sso_address,
			  client_id: janrainCaptureClientId,
			  redirect_uri: janrainCaptureRedirectUri,
			  logout_uri: janrainCaptureLogoutUri,
			  xd_receiver: janrainCaptureXdReceiver,
			  bp_channel: channelId
			});
	  } else {
	    // do non sso
      } 
	  jQuery("a.janrain_capture_signin").each(function(){
	    jQuery(this).attr("href", $(this).attr("href") + "&bp_channel=" + channelId).click(function(){
		  Backplane.expectMessages("identity/login");
		});  
      });
  },
  logout: function() {
    if (typeof(Drupal.settings.janrainCapture.sso_address) != 'undefined') {
      JANRAIN.SSO.CAPTURE.logout({
        sso_server: "https://" + Drupal.settings.janrainCapture.sso_address,
        logout_uri: Drupal.settings.janrainCapture.logout_url
      });
    }
  },
  addDestination: function(destination) {
    if ($.bbq) {
      $.bbq.pushState({"destination": destination});
    }
  },
  removeDestination: function() {
    if ($.bbq) {
      $.bbq.removeState('destination');
    }
  }
};

Drupal.janrainCapture.prototype = {
  passwordRecover: function(url) {
    // Placeholder to be overwritten by the CaptureUI Module
  },
  resize: function(jargs) {
    // Placeholder to be overwritten by the CaptureUI Module
  }
}

Drupal.behaviors.janrainCapture = {
  attach: function(context, settings) {
    if (settings.janrainCapture.enforce) {
      // Modify all /user/login and /user/register links to use Capture.
      var links = $('a[href^="/user/login"], a[href^="/user/register"]').once('janrain-capture');
      var length = links.length;
      if (links.length !== 0) {
        var i, link;
        for (i = 0; i < length; i++) {
          link = links[i];
          $(link).addClass('janrain_capture_anchor janrain_capture_signin');
          // If our login/register link has a destination param, we need to store
          // this in its data property so that we can use the jQuery BBQ plugin
          // to add it as a hash on the url.
          var regex = /(?:\?|&)destination\=([^\&]*)/;
          var match = regex.exec($(link).attr('href'));
          if (match && match.length == 2) {
            var destination = match[1];
            $(link).data("destination", destination);
          }
        }
      }
    }
    if (typeof(settings.janrainCapture.backplane_server) != 'undefined'
      && typeof(settings.janrainCapture.backplane_bus_name) != 'undefined') {
      Backplane(Drupal.janrainCapture.bp_ready);
      Backplane.init({
        serverBaseURL: settings.janrainCapture.backplane_server,
        busName: settings.janrainCapture.backplane_bus_name
      });
    }
    window.CAPTURE = Drupal.janrainCapture;
  }
};

})(jQuery);
