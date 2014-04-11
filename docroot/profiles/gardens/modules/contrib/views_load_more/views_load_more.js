/**
 * @file views_load_more.js
 *
 * Handles the AJAX pager for the view_load_more plugin.
 */
(function ($) {

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
    if ($.waypoints != undefined) {
      $.waypoints('refresh');
    }

    // Set up our default query options. This is for advance users that might
    // change there views layout classes. This allows them to write there own
    // jquery selector to replace the content with.
    // Provide sensible defaults for unordered list, ordered list and table
    // view styles.
    var content_query = targetList && !response.options.content ? '.view-content ' + targetList : response.options.content || '.view-content';

    // If we're using any effects. Hide the new content before adding it to the DOM.
    if (effect.showEffect != 'show') {
      new_content.find(content_query).children().hide();
    }

    // Remove old pager link, then add the new one. If there are attachments
    // with enabled pager inheritance, insert pager only once.
    wrapper.find('.pager a').remove();
    if (!response.attachment_inherit_pager) {
      wrapper.find('.pager').parent('.item-list').html(new_content.find('.pager'));
    }
    else {
      wrapper.find('.pager').parent('.item-list').html(new_content.find('.pager').last());
    }

    // Add new content with paying attention to attachments.
    var new_content_result = new_content.find(content_query);
    wrapper.find(content_query).each(function(i, settings) {
      // If this is an attachment, append new elements only if it has pager
      // inheritance enabled.
      if ($(new_content_result[i]).parents('div.attachment').length == 1) {
        var this_wrapper = this;
        if (response.attachment_inherit_pager) {
          $.each(response.attachment_inherit_pager, function(index, item) {
            if (new_content_result[i].parentElement.classList.contains(item)) {
              $(this_wrapper)[method](new_content_result[i].children);
            }
          });
        }
      }
      else {
        // Add new content.
        $(this)[method](new_content_result[i].children);
      }

      // Re-class new content.
      var row = 1;
      $(this).children()
        .removeClass('views-row-first views-row-last views-row-odd views-row-even')
        .filter(':first')
          .addClass('views-row-first')
          .end()
        .filter(':last')
          .addClass('views-row-last')
          .end()
        .filter(':even')
          .addClass('views-row-odd')
          .end()
        .filter(':odd')
          .addClass('views-row-even')
          .end()
        .each(function() {
          var classes = $(this).attr('class');
          var indexClass = classes.match(/views-row-[0-9]+/);
          $(this).removeClass(indexClass[0]).addClass('views-row-' + row++);
      });
    });

    if (effect.showEffect != 'show') {
      wrapper.find(content_query).children(':not(:visible)')[effect.showEffect](effect.showSpeed);
    }

    // Additional processing over new content
    wrapper.trigger('views_load_more.new_content', new_content.clone());

    // Attach all JavaScript behaviors to the new content
    // Remove the Jquery once Class, TODO: There needs to be a better
    // way of doing this, look at .removeOnce() :-/
    var classes = wrapper.attr('class');
    var onceClass = classes.match(/jquery-once-[0-9]*-[a-z]*/);
    wrapper.removeClass(onceClass[0]);
    var settings = response.settings || ajax.settings || Drupal.settings;
    Drupal.attachBehaviors(wrapper, settings);
  }

  /**
   * Attaches the AJAX behavior to Views Load More waypoint support.
   */
  Drupal.behaviors.ViewsLoadMore = {
    attach: function (context, settings) {
      if (settings && settings.viewsLoadMore && settings.views.ajaxViews) {
        opts = {
          offset: '100%'
        };
        $.each(settings.viewsLoadMore, function(i, setting) {
          var view = '.view-id-' + setting.view_name + '.view-display-id-' + setting.view_display_id + ' .pager-next a';
          $(view).waypoint(function(event, direction) {
            $(view).waypoint('remove');
            $(view).click();
          }, opts);
        });
      }
    },
    detach: function (context, settings, trigger) {
      if (settings && Drupal.settings.viewsLoadMore && settings.views.ajaxViews) {
        $.each(settings.viewsLoadMore, function(i, setting) {
          var view = '.view-id-' + setting.view_name + '.view-display-id-' + setting.view_display_id + ' .pager-next a';
          $(view, context).waypoint('destroy');
        });
      }
    }
     };
})(jQuery);
