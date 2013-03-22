(function ($, window, document) {

	function getFakeWidget () {
		var $widget = $('<div>', {
			style: "width: 220px !important; overflow: hidden; display: block; float: left;",
			html: $('<div>', {
				id: 'rpx-widget-embed',
				style: 'width: 200px; overflow: hidden;',
				html: $('<div>', {
					id: 'janrainEngageEmbed',
					html: $('<div>', {
						style: "width: 204px; height: 232px; padding-left: 5px; padding-right: 5px; box-sizing: content-box; background-color: rgb(255, 255, 255); border-top-width: 1px; border-right-width: 1px; border-bottom-width: 1px; border-left-width: 1px; border-top-style: solid; border-right-style: solid; border-bottom-style: solid; border-left-style: solid; border-top-color: rgb(255, 255, 255); border-right-color: rgb(255, 255, 255); border-bottom-color: rgb(255, 255, 255); border-left-color: rgb(255, 255, 255); border-image: initial; border-top-left-radius: 5px; border-top-right-radius: 5px; border-bottom-right-radius: 5px; border-bottom-left-radius: 5px; overflow-x: hidden; overflow-y: hidden; position: relative; ",
						html: $('<div>', {
							id: 'janrainView',
							html: $('<div>', {
								id: 'janrainProviderPages',
								style: "padding-top: 5px; left: 5px; top: 0px; position: absolute;",
								html: $('<div>', {
									html: $('<ul>', {
										id: 'janrainProviders_0',
										style: "list-style-type: none; margin-top: 0px; margin-left: 0px; margin-right: 0px; margin-bottom: 0px; padding-top: 0px; padding-right: 0px; padding-bottom: 0px; padding-left: 0px;",
										html: $('<li>', {
											id: 'janrain-twitter',
											role: 'button',
											style: "list-style-type: none; list-style-position: initial; list-style-image: initial; text-align: center; height: 30px; margin-top: 0px; width: 201.5px; margin-left: 0px; margin-bottom: 5px; position: relative; border-top-width: 1px; border-right-width: 1px; border-bottom-width: 1px; border-left-width: 1px; border-top-style: solid; border-right-style: solid; border-bottom-style: solid; border-left-style: solid; border-top-color: rgb(204, 204, 204); border-right-color: rgb(204, 204, 204); border-bottom-color: rgb(204, 204, 204); border-left-color: rgb(204, 204, 204); border-image: initial; border-top-left-radius: 5px; border-top-right-radius: 5px; border-bottom-right-radius: 5px; border-bottom-left-radius: 5px; cursor: pointer; background-color: rgb(227, 227, 227); background-image: -webkit-linear-gradient(bottom, rgb(238, 238, 238), rgb(255, 255, 255));",
											html: $('<img>', {
												src: "http://cdn.rpxnow.com/rel/img/09476f90c30519fa2ecbd94aa6419c6c.png",
												style: "background-color: transparent;"
											})
										})
									}).addClass('providers')
								}).addClass('janrainPage')
							})
						})
					}).addClass('janrainContent')
				})
			})
		}).addClass('rpx-signin');
		return $widget;
	}

	function insertFakeWidget(event) {
		// The login form container.
		var $this = $(this);
		// Re-init the signin widget to get rid of the 'Signing in' message
		// that may be there from the previous attempt to sign in.
		var children = $('#janrainEngageEmbed').children();
		if (children.length > 0) {
		  children.remove();
		}
		
		// Login Container
		var $methods = $('.auth-methods', $this);
		// Move the rpx-signin element into the auth-methods element.
		var $rpx = getFakeWidget().appendTo($methods);
		var $userMethod = $('.user-fields', $methods),
		$rpxMethod = $('.rpx-signin', $methods);
		// Create the rpx-or element.
		var $rpxOr = $('<div>', {
		  html: $('<span>', {
		    text: Drupal.t('or')
		  })
		}).addClass('rpx-or'),
		rpxOrHeight = $.map($userMethod, function (index, element) {
		  var $this = $(this),
		  height = $userMethod.height(),
		  actionsHeight = $userMethod.find('.form-actions').outerHeight(true),
		  tosHeight = $userMethod.find('.tos-and-disclaimer').outerHeight(true);
		
		  return (height - actionsHeight - tosHeight) || false;
		});
		// Add the or text to the RPX container.
		$rpxMethod.once('rpx-or', function (index) {
		  $(this).prepend(
		    $rpxOr
		    .css({
		      'height': rpxOrHeight || 'auto',
		      'width': '20px',
		      'float': 'left'
		    })
		  );
		});
		
		// Trigger contentupdate to resize it.
		$this
		.trigger('contentupdate');
  }
  
	function removeFakeWidget(event) {
		if (window.janrain) {
			// Hide the signin widget.
			$('#rpx-widget-store').append($('#janrainEngageEmbed'));
		}
	}

	Drupal.behaviors.fakeRain = {
		attach: function (context, settings) {
			// Prepare the login and register dialogs.
			$('.user-login-dialog, .user-register-form-dialog').once('janrainEnhancedDialog', function (context, settings) {
				$(this)
				.bind('dialogopen.rpxLoadJanrainEngage', {context: context, settings: settings}, insertFakeWidget)
				.bind('dialogclose.rpxLoadJanrainEngage', {context: context, settings: settings}, removeFakeWidget);
			});
		}
	};
}(jQuery, window, document));