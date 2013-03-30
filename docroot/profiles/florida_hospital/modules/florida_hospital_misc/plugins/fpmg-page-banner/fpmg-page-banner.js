/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true window: true document: true */

(function ($, window, document) {
  // Replace 'pluginName' with the name of your plugin.
  var plugin = 'pageBanner',
  // A private reference to this $plugin object.
  $plugin;

  // Private function definitions.
  /**
   * Wraps an object in a stack structure.
   */
  function stackTemplate(classes) {
    var $tmpl = $('<div>', {
      html: $('<div>', {
        html: $('<div>', {
          html: $('<div>').addClass('col-first col only last tb-height-balance tb-terminal')
        }).addClass('box col-1 clearfix tb-terminal')
      }).addClass('stack-width tb-terminal inset inset-1')
    }).addClass('stack clearfix tb-scope' + ' ' + classes);
    return $tmpl;
  }
  function getSection(sections) {
    var pathSection = window.location.pathname.replace(/^\/([^\/]*).*$/, '$1');
    for (var i = 0; i < sections.length; i++) {
      var section = sections[i];
      if (section.path === pathSection) {
        return section;
      }
    }
    return null;
  }

  // Plugins should not declare more than one namespace in the $.fn object.
  // So we declare methods in a methods array
  var methods = {
    init : function (options) {
      // Build main options before element iteration.
      var opts = $.extend({}, $[plugin].defaults, options),
      // Iterate over matched elements.
      section = getSection(opts.sections);
      // If a site section is matched, create a stacker for it.
      if (section) {
        var $stack = $('<div>', {
          text: section.label
        })
        .addClass('section-banner')
        .wrap(stackTemplate('stack-section-banner'))
        .closest('.stack')
        .addClass('section-' + section.path);
        // Return the stack
        return $stack;
      }
      // Return an empty jQuery object if no section banner was created.
      return $();
    }
  };

  // Add the plugin to the jQuery fn object.
  $plugin = $[plugin] = function (method) {
    // Method calling logic
    if (methods[method]) {
      return methods[method].apply(this, Array.prototype.slice.call(arguments, 1));
    } else if (typeof method === 'object' || ! method) {
      return methods.init.apply(this, arguments);
    } else {
      $.error('Method ' +  method + ' does not exist on jQuery.' + plugin);
    }
  };

  // plugin defaults
  $[plugin].defaults = {};
}
// Pass jQuery as the param to the preceding anonymous function
(jQuery, window, document));
