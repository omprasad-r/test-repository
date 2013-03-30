(function($) {
	Drupal.behaviors.fpmgGoogleAnalytics = {
		attach: function (context, settings) {
			if (_gaq && $.isFunction(_gaq.push)) {
				$('a[href]', '.page').once('fpmgGoogleAnalytics-link', function (index) {
					var $link = $(this),
					href = $link.attr('href');
					$link[0].setAttribute("onclick", "_gaq.push(['_link', '" + href + "']);");
				});
			}
		}
	};
}(jQuery));