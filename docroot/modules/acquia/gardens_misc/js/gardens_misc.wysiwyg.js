(function ($) {

Drupal.gardens_misc = Drupal.gardens_misc || {};

Drupal.behaviors.gardens_misc = {
  attach: function(context, settings) {
    if (typeof CKEDITOR != 'undefined') {
      // Bind events to handle enabling and disabling of the WYSIWYG.
      CKEDITOR.on('instanceReady', Drupal.behaviors.gardens_misc.instanceLoaded);
    }
    
    $('.wysiwyg', context).closest('.text-format-wrapper').once('wysiwyg-tabs', function() {
      var $tabs = $('<div class="wysiwyg-tab-wrapper"></div>').insertBefore($(this).find('.form-textarea-wrapper'));
      $('<div class="wysiwyg-tab enable first">' + Drupal.t('WYSIWYG') + '</div>')
        .bind('click', this, Drupal.behaviors.gardens_misc.tabToggle)
        .appendTo($tabs);
      $('<div class="wysiwyg-tab disable last">' + Drupal.t('HTML') + '</div>')
        .bind('click', this, Drupal.behaviors.gardens_misc.tabToggle)
        .appendTo($tabs);
    });
  },
  instanceLoaded: function(event) {
    var $wrapper = $(event.editor.container.$).closest('.text-format-wrapper');
    var $original = $wrapper.find('.wysiwyg');
    var $clone = $original.clone().val($original.val());
    $clone.bind('change', function(event) {
      $original.val($clone.val()).change();
    });
    $wrapper.find('.cke_toolbox').removeAttr('onmousedown').prepend($clone);
    $wrapper.find('.wysiwyg-tab.enable').addClass('wysiwyg-active');
    $wrapper.find('.wysiwyg-tab.disable').removeClass('wysiwyg-active');
  },
  tabToggle: function (event) {
    if (!$(this).hasClass('wysiwyg-active')) {
      $(this).closest('.text-format-wrapper').find('.wysiwyg-toggle-wrapper a').click();
    }
  }
};

/**
 * Overides the default attach behavior for text formats without an editor
 */
Drupal.wysiwyg.editor.attach.none = function(context, params, settings) {
  var $field = $('#' + params.field);
  var $original = $('#' + params.trigger);
  var $clone = $original.clone().val($original.val());
  var $container = $('<div class="wysiwyg-none-header"></div>');
  $container.append($clone).insertBefore($field);
  $clone.bind('change', function(event) {
    $original.val($clone.val()).change();
  });
  
  // @todo: Need to display the allowed tags here
  var tags = Drupal.settings.wysiwyg_formats[$original.val()];
  $('<span><b>' + Drupal.t('Allowed tags: ') + '</b>' + tags + '</span>').appendTo($container);
  
  
  // Original behavior
  if (params.resizable) {
    var $wrapper = $field.closest('.form-textarea-wrapper');
    $wrapper.addClass('resizable');
    if (Drupal.behaviors.textarea.attach) {
      Drupal.behaviors.textarea.attach();
    }
  }
  
  $field.closest('.text-format-wrapper').find('.wysiwyg-tab.enable').removeClass('wysiwyg-active');
  $field.closest('.text-format-wrapper').find('.wysiwyg-tab.disable').addClass('wysiwyg-active');
};

/**
 * Overides the default detach behavior for text formats without an editor
 */
Drupal.wysiwyg.editor.detach.none = function(context, params) {
  $('#' + params.field).siblings('.wysiwyg-none-header').remove();
  
  // Original behavior
  if (typeof params != 'undefined') {
    var $wrapper = $('#' + params.field).parents('.form-textarea-wrapper:first');
    $wrapper.removeOnce('textarea').removeClass('.resizable-textarea')
      .find('.grippie').remove();
  }
};

})(jQuery);