(function ($) {
  // Plugins should not declare more than one namespace in the $.fn object.
  // So we declare methods in a methods array
    var methods = {
      init : function (options) {
        // build main options before element iteration
        var o = mergeOptions(options);
        // iterate over matched elements
        return this.each(function () {
          // implementations
          var visible = (!$(this).hasClass('smart-hidden'));
          if (visible) {
            hide(this, o);
          }
          else {
            show(this, o);
          }
        });
      },
      show : function (options) {
        // build main options before element iteration
        var o = mergeOptions(options);
        // iterate over matched elements
        return this.each(function () {
          show(this, o);
        });
      },
      hide : function (options) {
        // build main options before element iteration
        var o = mergeOptions(options);
        // iterate over matched elements
        return this.each(function () {
          hide(this, o);
        });
      }
    };

    // replace 'smartToggle' with the name of your plugin
    $.fn.smartToggle = function (method) {
  
      // debug(this);
      
      // Method calling logic
      if (methods[method]) {
        return methods[method].apply(this, Array.prototype.slice.call(arguments, 1));
      } else if (typeof method === 'object' || ! method) {
        return methods.init.apply(this, arguments);
      } else {
        $.error('Method ' +  method + ' does not exist on jQuery.smartToggle');
      }
    };
    
    // plugin defaults
    $.fn.smartToggle.defaults = {
      vClass: "smart-visible",
      hClass: "smart-hidden",
      speed: 750
    };
    
    // private functions definition
    function mergeOptions (options) {
      return $.extend({}, $.fn.smartToggle.defaults, options);
    }
    function show (element, options) {
      var $this = $(element);
      $this.hide(0).removeClass('smart-hidden').fadeIn(options.speed);
    }
    function hide (element, options) {
      var $this = $(element);
      $this.fadeOut(options.speed).addClass('smart-hidden').show(0);
    }
  
    // private function for debugging
    function debug() {
      var $this = $(this);
      if (window.console && window.console.log) {
        window.console.log('selection count: ' + $this.size());
      }
    }
  }
)(jQuery);