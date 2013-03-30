function janrainCaptureWidgetOnLoad() {
    function handleCaptureLogin(result) {

        //console.log ("exchanging code for token...");
        getTokenForCode(result.authorizationCode, janrain.settings.capture.redirectUri);
    }
    janrain.events.onCaptureSessionFound.addHandler(function(result){
	    //console.log ("capture session found");
	    janrainSignOut();
    });

    janrain.events.onCaptureSessionNotFound.addHandler(function(result){
	    //console.log ("capture session not found");
    });

    janrain.events.onCaptureLoginSuccess.addHandler(handleCaptureLogin);
    janrain.events.onCaptureRegistrationSuccess.addHandler(handleCaptureLogin);

    janrain.capture.ui.start();
}
