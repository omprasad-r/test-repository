/**
 * @file views_load_more.js
 *
 * Handles the AJAX pager for the view_load_more plugin.
 */
(function ($) {

  // Avoid to declare viewsLoadMoreAppend function if Drupal.Ajax is not available.
  // Known error with JavaScript aggregation http://drupal.org/node/1703436.
  if (Drupal.ajax != undefined) {
    /**
    * Provide a series of commands that the server can request the client perform.
    */
    Drupal.ajax.prototype.commands.viewsLoadMoreAppend = function (ajax, response, status) {
      // Get information from the response. If it is not there, default to
      // our presets.
      var wrapper = response.selector ? $(response.selector) : $(ajax.wrapper);
      var method = response.method || ajax.method;
      var targetList = response.targetList || '';
      var effect = ajax.getEffect(response);

      // We don't know what response.data contains: it might be a string of text
      // without HTML, so don't rely on jQuery correctly iterpreting
      // $(response.data) as new HTML rather than a CSS selector. Also, if
      // response.data contains top-level text nodes, they get lost with either
      // $(response.data) or $('<div></div>').replaceWith(response.data).
      var new_content_wrapped = $('<div></div>').html(response.data);
      var new_content = new_content_wrapped.contents();

      // For legacy reasons, the effects processing code assumes that new_content
      // consists of a single top-level element. Also, it has not been
      // sufficiently tested whether attachBehaviors() can be successfully called
      // with a context object that includes top-level text nodes. However, to
      // give developers full control of the HTML appearing in the page, and to
      // enable Ajax content to be inserted in places where DIV elements are not
      // allowed (e.g., within TABLE, TR, and SPAN parents), we check if the new
      // content satisfies the requirement of a single top-level element, and
      // only use the container DIV created above when it doesn't. For more
      // information, please see http://drupal.org/node/736066.
      if (new_content.length != 1 || new_content.get(0).nodeType != 1) {
        new_content = new_content_wrapped;
      }
      // If removing content from the wrapper, detach behaviors first.
      var settings = response.settings || ajax.settings || Drupal.settings;
      Drupal.detachBehaviors(wrapper, settings);

      // Add the new content to the page.
      var $pager = $('.pager', wrapper),
          $newPager = $('.pager', new_content);
      $('a', $pager).remove();
      $pager.html($newPager);
      if (targetList) {
        $('.view-content ' + targetList, wrapper)[method]($('.views-row', new_content));
      }
      else {
        $('.view-content', wrapper)[method]($('.views-row', new_content));
      }
      // Renumber and reclass the items
      $('.views-row', wrapper)
      .removeClass('views-row-first views-row-last views-row-odd views-row-even')
      .filter(':first')
      .addClass('views-row-first')
      .end()
      .filter(':last')
      .addClass('views-row-last')
      .end()
      // :even and :odd are zero-based, so even starts at the first (odd) element.
      .filter(':even')
      .addClass('views-row-odd')
      .end()
      .filter(':odd')
      .addClass('views-row-even')
      .end();
      // @todo Still need to renumber the items so the ordinal views-row classes
      // are correct.


      // Attach all JavaScript behaviors to the new content
      // Remove the Jquery once Class, TODO: There needs to be a better
      // way of doing this, look at .removeOnce() :-/
      var classes = wrapper.attr('class');
      var onceClass = classes.match(/jquery-once-[0-9]*-[a-z]*/);
      wrapper.removeClass(onceClass[0]);
      var settings = response.settings || ajax.settings || Drupal.settings;
      Drupal.attachBehaviors(wrapper, settings);

      if (new_content.parents('html').length > 0) {
        // Apply any settings from the returned JSON if available.
      }
    }
  }
  /**
   * Attaches the AJAX behavior to Views Load More waypoint support.
   */
  Drupal.behaviors.ViewsLoadMore = {};
  Drupal.behaviors.ViewsLoadMore.attach = function() {
    if (Drupal.settings && Drupal.settings.viewsLoadMore && Drupal.settings.views.ajaxViews) {
      opts = {
        offset: '100%'
      };
      $.each(Drupal.settings.viewsLoadMore, function(i, settings) {
        var view = '.view-' + settings.view_name + '.view-display-id-' + settings.view_display_id + ' .pager-next a';
        $(view).bind('waypoint.reached', function(event, direction) {
           $(view).click();
        });
        $(view).waypoint(opts);
      });
    }
    $('li.pager-next > a.active').removeClass('active');
  };

})(jQuery);
