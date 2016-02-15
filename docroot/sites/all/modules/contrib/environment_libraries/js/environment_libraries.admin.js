/**
 * @file
 * Administrative UI JS.
 */

(function($){
	"use strict";
	$(function(){
		// On document ready.
		var $name = $('#edit-environment-libraries-library .form-item-environment-libraries-library-name input'),
			$machine_name = $('#edit-environment-libraries-library .form-item-environment-libraries-library-machine-name input');

		$name.blur(function(){
			$machine_name.val($name.val().toLowerCase().replace(/[^a-z0-9_]+/g,'_'));
		});
	});

	$('.form-type-environment-library fieldset.environment-library--file').each(function(){
		var $fieldset = $(this);

		// Minification & Aggregation require caching, auto-check this dependency.
		function check($cache, $opt){
			return function(){
				if ($opt.find("input").is(":checked")) {
					$cache.find("input").attr("checked", true);
				}
			};
		}
		function uncheck($cache, $opt){
			return function(){
				if ($cache.find("input").is(":checked")) {
					$opt.find("input").attr("checked", false);
				}
			};
		}
		$fieldset.find(".environment").each(function(){
			var $env = $(this),
				$options = $env.find(".form-type-checkbox"),
				$cache = $($options[0]),
				$agg = $($options[1]),
				$minify = $($options[2]);
			$minify.on("click", check($cache, $minify));
			$agg.on("click", check($cache, $agg));
			$cache.on("click", uncheck($cache, $agg));
			$cache.on("click", uncheck($cache, $minify));

			// Copy the title attribute to the checkbox labels.
			$options.each(function(){
				var $this = $(this),
					$input = $this.find("input"),
					$lbl = $this.find("label");
				$lbl.attr("title", $input.attr("title"));
			});
		});
	});
})(jQuery);
