/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true debug: true window: true*/
/*
 * Turns a list of links into a contextual flyout menu.
 * The menu is associated with an element on the page.
 * 
 * Author: Acquia, @jessebeach
 * Website: http://acquia.com, http://qemist.us
 *
 * items = {
 *  [
 *    {
 *      label: string,
 *      href: string (optional),
 *      itemClasses: array (optional),
 *      linkClasses: array (optional),
 *      linkWrapper: string (default: <a>, optional)
 *    }
 *  ]
 *  ...
 * }
 *
 */
(function ($) {

  // replace 'pluginName' with the name of your plugin
  $.fn.flyoutList = function (options) {

    // debug(this);

    // build main options before element iteration
    var opts = $.extend({}, $.fn.flyoutList.defaults, options);

    // iterate over matched elements
    return this.each(function () {
      var $this = $(this);
      // build element specific options. Uses the Metadata plugin if available
      // @see http://docs.jquery.com/Plugins/Metadata/metadata
      var o = $.meta ? $.extend({}, opts, $this.data()) : opts;
      // implementations
      
      if (o.items) {
        var $flyoutList = $.fn.flyoutList.buildFlyoutList(o.items).prependTo($this);
      
        var $context = o.context ? $(o.context) : $this;
        $context.css({
          position: 'relative'
        })
        .addClass('flyout-list-context');
      
        // Place the dotted outline just outside the context element
        $.fn.flyoutList.buildContextOutline($context);
      }
    });
  };
    
  // plugin defaults
  $.fn.flyoutList.defaults = {};

  // private functions definition
  $.fn.flyoutList.buildFlyoutList = function (items) {
    var $list = $('<ul>').addClass('flyout-list clearfix');
    var len = items.length;
    for (var i = 0; i < len; i++) {
      var itemClasses = ['item-' + i];
      if (items[i].itemClasses) {
        $.merge(itemClasses, items[i].itemClasses);
      }
      var linkClasses = ['action', $.fn.flyoutList.makeSafeClass('action-' + items[i].label)];
      if (items[i].linkClasses) {
        $.merge(linkClasses, items[i].linkClasses);
      }
      var linkWrapper = (items[i].linkWrapper) ? '<' + items[i].linkWrapper + '>' : '<a>';
      var linkProperties = {};
      if (items[i].label) {
        linkProperties.text = items[i].label;
      }
      else {
        linkProperties.text = "missing label";
      }
      if (items[i].href) {
        linkProperties.href = items[i].href;
      }
      
      $list.append($('<li>', {
        html: $(linkWrapper, linkProperties).addClass(linkClasses.join(' '))
      }).addClass(itemClasses.join(' ')));
    }
    return $list;
  };
  
  $.fn.flyoutList.makeSafeClass = function (s) {
    var className = s.toString().replace(new RegExp("[^a-zA-Z0-9_-]", 'g'), "-").toLowerCase();
    return className;
  };
  
  $.fn.flyoutList.buildContextOutline = function ($context) {
    $('<div>').addClass('flyout-list-outline top').prependTo($context);
    $('<div>').addClass('flyout-list-outline right').prependTo($context);
    $('<div>').addClass('flyout-list-outline bottom').prependTo($context);
    $('<div>').addClass('flyout-list-outline left').prependTo($context);
  };

  // private function for debugging
  function debug() {
    var $this = $(this);
    if (window.console && window.console.log) {
      window.console.log('selection count: ' + $this.size());
    }
  }

}(jQuery));