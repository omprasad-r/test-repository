
(function ($) {

Drupal.toolbar_prototype = Drupal.toolbar_prototype || {};

/**
 * Attach toggling behavior and notify the overlay of the toolbar.
 */
Drupal.behaviors.toolbar_prototype = {
  attach: function(context) {
    var $drawer_links = $('#toolbar .drawer-links a:not(#toolbar-link-admin-appearance)', context)
    $('#toolbar .toolbar-menu li').bind('mouseenter', Drupal.toolbar_prototype.drawer_toggle);
    
    Drupal.toolbar_prototype.path = $('#toolbar .active');
    Drupal.toolbar_prototype.original = Drupal.settings.toolbar.tooltips.default;

    $(window).bind('hashchange.drupal-overlay', Drupal.toolbar_prototype.activeTrail);
    $(document).bind('drupalOverlayLoad', Drupal.toolbar_prototype.activeTrail);
    $(document).bind('drupalOverlayClose', Drupal.toolbar_prototype.drawer_close);
    
    $('#toolbar li').bind('mouseenter', Drupal.toolbar_prototype.tooltipShow);
    $('#toolbar').bind('mouseleave', Drupal.toolbar_prototype.tooltipHide);
    $('#toolbar').bind('mouseleave', Drupal.toolbar_prototype.drawer_close);
    $('#tooltip .toolbar-menu li').bind('mouseleave', Drupal.toolbar_prototype.tooltipHide);
    
  }
};

Drupal.toolbar_prototype.drawer_toggle = function (event) {
  event.preventDefault();
  event.stopPropagation();
  
  var $this = $(this).children('a:first');
  var $drawer = $('#' + $this.attr('drawer'));
  
  if (!$drawer.length) {
    // Let normal interaction proceed
    Drupal.toolbar_prototype.drawer_close();
    $(this).addClass('active').blur();
    return;
  }
  
  if ($drawer.hasClass('active-path') && $this.hasClass('active-path')) {
    Drupal.toolbar_prototype.drawer_close();
    Drupal.toolbar_prototype.path.addClass('active');
    Drupal.toolbar_prototype.path.parentsUntil('#toolbar').addClass('active-path');
    $('#toolbar [drawer=' + $('.toolbar-drawer .drawer.active-path').attr('id') + ']').addClass('active-path');
  } else {
    $('#toolbar .active').removeClass('active');
    $('#toolbar .active-path').removeClass('active-path');
    $this.addClass('active-path').blur();
    $drawer.addClass('active-path');
    $drawer.parentsUntil('#toolbar').addClass('active-path');
    $('body').css('paddingTop', Drupal.toolbar.height());
  }
  
  Drupal.overlay.eventhandlerAlterDisplacedElements();
};

Drupal.toolbar_prototype.drawer_close = function (event) {
  $('#toolbar .active').removeClass('active').blur();
  $('#toolbar .active-path').removeClass('active-path').blur();
  $('body').css('paddingTop', Drupal.toolbar.height());
};

Drupal.toolbar_prototype.activeTrail = function (event) {
  var path = Drupal.settings.basePath + $.bbq.getState('overlay');
  $('#toolbar a.active').removeClass('active');
  $('#toolbar .active-path').removeClass('active-path');
  var $link = $('#toolbar a[href=' + path + ']');
  $link.addClass('active');
  $link.parentsUntil('#toolbar').addClass('active-path');
  $('#toolbar [drawer=' + $('.toolbar-drawer .drawer.active-path').attr('id') + ']').addClass('active-path');
  Drupal.toolbar_prototype.path = $('#toolbar .active');
  Drupal.overlay.eventhandlerAlterDisplacedElements();
};

Drupal.toolbar_prototype.tooltipShow = function (event) {
  var index = $(this).find('a').attr('href');
  if ($(this).is('.toolbar-menu li')) {
    index = 'href' + index;
  }
  if (index == 'hrefundefined') {
    index = 'href/admin/appearance';
  }
  var text = Drupal.settings.toolbar.tooltips.default;
  if (event.type == 'mouseenter' && Drupal.settings.toolbar.tooltips[index] != '') {
    text = Drupal.settings.toolbar.tooltips[index]
  }
  
  if (!$('.toolbar-drawer').hasClass('active') && event.type == 'mouseleave') {
    Drupal.toolbar_prototype.tooltipHide();
  }
  
  $('.toolbar-tooltips').html(text);
  $('.toolbar-tooltips').show(500);
};

Drupal.toolbar_prototype.tooltipHide = function (event) {
  event.stopPropagation();
  event.preventDefault();
  if ($(this).attr('id') == 'toolbar' || !$('#toolbar .toolbar-drawer').hasClass('active')) {
    $('.toolbar-tooltips').hide(500);
  }
};

})(jQuery);
