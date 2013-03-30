/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true document: true */

/**
 * Moves the page title into its own stack above the content.
 */
(function ($, Drupal, window, document) {
	var MINIMUM_COLUMN_WIDTH = 125;
  /**
   * Creates a fake stack structure.
   */
  function stackTemplate(needsWrapper) {
    var $tmpl = $('<div>', {
      id: 'postfooter',
      html: $('<div>', {
        html: $('<div>', {
          html: $('<div>').addClass('col-first col only last tb-height-balance tb-terminal')
        }).addClass('box col-1 clearfix tb-terminal')
      }).addClass('stack-width tb-terminal inset inset-1')
    }).addClass('stack-postfooter stack-post stack horizontal extended-menu clearfix tb-scope');

    if (needsWrapper) {
      $tmpl = $tmpl
      .wrapAll($('<div>', {
        id: 'footer'
      }).addClass('wrapper-footer wrapper clearfix tb-scope'))
      .closest('.wrapper');
    }
    // Adding this tracer is ugly, but I cannot think up a better way to find
    // the outer wrapper later without an immutable hook.
    return $tmpl.addClass('tmpl-tracer');
  }

  /**
   * This should eventually just be a destroy method on the dropdown plugin.
   */
  function destroyDropdowns() {
    return this.each(function () {
      $(this)
      .removeClass('menu-dropdown')
      .find('.content > .menu')
      .removeClass('pulldown pulldown-processed menu-dropdown-js-enabled')
      .addClass('root')
      .find('[style]')
      .removeAttr('style')
      .end()
      .find('li')
      .removeClass('expanded collapsed leaf active-trail')
      .children('a')
      .removeClass('active-trail active')
      .end()
      .end()
      .find('.more-indicator')
      .remove()
      .end()
      .end();
    });
  }
  /**
   * Clone the main menu links and force them into a footer stack.
   */
  Drupal.behaviors.fpmgMainMenuCloner = {
    attach: function (context, settings) {
      $('.stack-navigation').once('main-menu-cloner', function (index) {
        var $target, $footer;
        // Clone the menu.
        var $menu = $(this)
        .find('#block-system-main-menu')
        .clone();
        // If the wrapper-footer exists, append the menu to it, otherwise just
        // dump it at the bottom of the page lining.
        $footer = $('.wrapper-footer');
        $target = ($footer.length > 0) ? $footer : $('.page .lining');
        // Destroy the dropdown menu, wrap it and append it.
        $menu = destroyDropdowns.call($menu)
        // Wrap the menus in a stack and append it.
        .wrapAll(stackTemplate(($footer.length === 0)))
        .closest('.tmpl-tracer')
        .removeClass('.tmpl-tracer')
        .hide()
        .appendTo($target)
        .fadeIn();
      });
    }
  };
}(jQuery, Drupal, window, document));
