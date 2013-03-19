(function ($) {

Drupal.behaviors.extlink_extra = {
  attach: function(context){
    //Unbind extlink's click handler and add our own
    // @todo: extlink.js only adds the 'ext' class to links that do not contain images!  We need to find another way to intercept those links...
    jQuery('a.' + Drupal.settings.extlink.extClass).unbind('click').not('.ext-override').click(function(e) {
      //This is what extlink does by default (except
      if (Drupal.settings.extlink_extra.extlink_alert_type == 'confirm') {
        return confirm(Drupal.settings.extlink.extAlertText.value);
      }

      var external_url = jQuery(this).attr('href');
      var back_url = window.location.href;//window.location.protocol + window.location.hostname + window.location.pathname;
      var alerturl = Drupal.settings.extlink_extra.extlink_alert_url;
      $.cookie("external_url", external_url, { path: '/' });
	    $.cookie("back_url", back_url, { path: '/' });

	    if (Drupal.settings.extlink_extra.extlink_alert_type == 'colorbox') {
        jQuery.colorbox({
          href: alerturl + ' .extlink-extra-leaving',
          height: '50%',
          width: '50%',
          initialWidth: '50%',
          initialHeight: '50%',
          onComplete: function() { //Note - drupal colorbox module automatically attaches drupal behaviors to loaded content
            //Allow our cancel link to close the colorbox
            jQuery('div.extlink-extra-back-action a').click(function(e) {jQuery.colorbox.close(); return false;});
            extlink_extra_timer();
          },
          onClosed: extlink_stop_timer
        });
        return false;
	    }

	    if (Drupal.settings.extlink_extra.extlink_alert_type == 'page') {
	      //If we're here, alert text is on but pop-up is off; we should redirect to an intermediate confirm page
	      window.location = alerturl;
	      return false;
	    }
    });

    //Dynamically replace hrefs of back and external links on page load.  This is to compensate for aggressive caching situations
    //where the now-leaving is returning cached results
    if (Drupal.settings.extlink_extra.extlink_cache_fix == 1) {
      if (jQuery('.extlink-extra-leaving').length > 0) {
        //grab our cookies
        var external_url = $.cookie("external_url");
        var back_url = $.cookie("back_url");

        //if there are any places where the urls were rendered as placeholders because the aggressive cache setting is on, replace them:
        var html = jQuery('.extlink-extra-leaving').html();
        html = html.replace(/external-url-placeholder/gi, external_url);
        html = html.replace(/back-url-placeholder/gi, back_url);
        jQuery('.extlink-extra-leaving').html(html);

        //Adding these 2 lines that specifically set the href seems a little bit overkill, but seems to be necessary for IE7
        //We can't 100% rely on the existance of the two link classes, but they should be there in most cases.
        //It seems IE7 writes the domain directly into a relative link when parsing the page, so the straight regex replacement
        //ends up being href="http://thecurrentdomain.com/http://thebacklink.com/link
        //Either that or I was compensating for some other weird JS behavior, either way, this fixes it.
        jQuery('.extlink-extra-back-action a').attr('href', back_url);
        jQuery('.extlink-extra-go-action a').attr('href', external_url);
      }
    }

    //If the timer is configured, we'll call it for the intermediate page
    if (Drupal.settings.extlink_extra.extlink_alert_type == 'page') {
      if (jQuery('.extlink-extra-leaving').length > 0) {
        extlink_extra_timer();
      }
    }
  }
}

})(jQuery);

//Global var that will be our JS interval
var extlink_int;

function extlink_extra_timer() {
  if (Drupal.settings.extlink_extra.extlink_alert_timer == 0 || Drupal.settings.extlink_extra.extlink_alert_timer ==  null) {
    return;
  }
  extlink_int = setInterval(function() {
    var container = jQuery('.automatic-redirect-countdown');
    var count = container.attr('rel');
    if (count == null) {
      count = Drupal.settings.extlink_extra.extlink_alert_timer;
    }
    if (count >= 0) {
      container.html('<span class="extlink-timer-text">Automatically redirecting in: </span><span class="extlink-count">'+count+'</span><span class="extlink-timer-text"> seconds.</span>');
      container.attr('rel',--count);
    }
    else {
      extlink_stop_timer();
      container.remove();
      window.location = jQuery('div.extlink-extra-go-action a').attr('href');
    }
  }, 1000);
}

function extlink_stop_timer() {
  clearInterval(extlink_int);
}
