(function ($) {

// Disable the vertical tabs summary for "our" fieldset, since we
// have no control over it.
Drupal.behaviors.rpxPublishSummaries = {
  attach: function (context) {
    // Make sure this behavior is processed only if drupalSetSummary is defined.
    if (typeof jQuery.fn.drupalSetSummary == 'undefined') {
      return;
    }

    $('fieldset#edit-options', context).drupalSetSummary(function (context) {
      return;
    })
  }
};

//
// Handles a "Publish to" checkbox.
//
// Accepts a form item-specific ID of the checkbox to drive.
function rpxPublishAttach(id, context, settings) {
  var list = $('#' + id + '-provider-list', context);
  var link = $('#' + id + '-edit', context);
  var options = $('#' + id + '-settings', context);
  var provider_inputs = $('#edit-' + id + '-provider-checkboxes', context);
  // The provider list should be static if no provider checkboxes were added.
  var use_inputs = provider_inputs.length;

  $('#edit-' + id, context).once('edit-' + id, function() {
    $(this).bind('click', function () {
      options.hide();
      if ($(this).attr('checked')) {
        list.show();
        link.show();
      }
      else {
        if (use_inputs) list.hide();
        link.hide();
      }
    });

    // Make sure we are OK on preview reloads.
    options.hide();
    if (use_inputs) list.html(enabledProviders());
    if($(this).attr('checked')) {
      list.show();
      link.show();
    }
    else {
      list.hide();
      link.hide();
    }
  });

  $('#' + id + '-edit', context).once(id + '-edit', function() {
    $(this).bind('click', function () {
      if (use_inputs) list.hide();
      link.hide();
      options.show();
      return false;
    });
  });

  // When a provider is enabled for publishing, update the "Publish to" checkbox label.
  // If last enabled provider is disabled, disable the parent checkbox.
  provider_inputs.find('input').once('edit-' + id + '-provider-checkboxes', function() {
    $(this).bind('click', function () {
      if (!provider_inputs.find('input:checked').length) {
        $('#edit-' + id, context).click();
        list.hide();
        link.hide();
      }
      if (use_inputs) {
        list.html(enabledProviders());
      }
    });
  });

  /**
   * Return a string listing enabled providers.
   */
  function enabledProviders() {
    var list = '';
    provider_inputs.find('input:checked').each(function () {
      list += settings.rpx_providers[this.value] + ', ';
    });
    list = list.slice(0, -2);
    return list ? '<em>' + list + '</em>' : '<em>none</em>';
  }
}

Drupal.behaviors.rpxPublish = {
  attach: function (context, settings) {
    rpxPublishAttach('rpx-site-publish', context, settings);
    rpxPublishAttach('rpx-user-publish', context, settings);
  }
};

})(jQuery);
