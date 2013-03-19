
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true ThemeBuilder: true*/

function jQueryPlugin(name, defaults, helpers, init, allforone) {
  var $ = jQuery;
  $.fn[name] = function (options) {
    if (typeof(options) === 'string') {
      if (typeof(helpers[options]) !== 'undefined') {
        var args = arguments;
        if (allforone) {
          return helpers[options].apply($.data(this[0], name), Array.prototype.slice.call(args, 1));
        }
        else {
          return this.each(function () {
            return helpers[options].apply($.data(this, name), Array.prototype.slice.call(args, 1));
          });
        }
      }
    }
    if (allforone) {
      var widget = {};
      widget.settings = $.extend({}, defaults, options);
      widget.node = $(this); // the individual node
      $.data(widget.node[0], name, widget);
      init.apply(widget, [widget.settings]);
      return this;
    }
    else {
      return this.each(function () {
        var widget = {};
        widget.settings = $.extend({}, defaults, options);
        widget.node = $(this); // the individual node
        $.data(this, name, widget);

        init.apply(widget, [widget.settings]);
        return this;
      });
    }
  };
}

  // actually make the plugin
jQueryPlugin('inputslider',
  {
    onShow: function (islider, target) {},
    onStart: function (islider, event, value, target) {},
    onSlide: function (islider, event, value, target) {},
    onStop: function (islider, event, value, target) {},
    modify: function (x) {
      return x; 
    },
    min: 0,
    max: 10,
    step: 1,
    value: 0,
    autofocus: true
  },
  { // helpers
    set: function (attr, value) {
      return this.set(attr, value);
    },
    get: function (attr) {
      return this.get(attr);
    }
  },
  function (settings) { // init
    var $ = jQuery;
    var current = null;
    var down = false;
    var that = this;
    // TODO: this won't scroll...it probably should -- look into how pallettepicker does it
    this.slider = $('<div class="slider-container"><div class="slider"></div></div>')
      .appendTo('#themebuilder-wrapper').children().eq(0);
    this.slider.slider({
      min: settings.min,
      max: settings.max,
      step: settings.step,
      slide: function (event, ui) {
        if (false === settings.onSlide.call(that, that, event, ui.value, current)) {
          return false;
        }
        if (settings.autofocus !== false) {
          current.focus();
        }
      },
      init: function (event, slider) {
        that._slider = slider;
      },
      stop: function (event, ui) {
        if (down) {
          return;
        }
        settings.onStop.call(that, that, event, ui.value, current);
        if (settings.autofocus !== false) {
          current.focus();
        }
      }
    });
      
    this.set = function (attr, value) {
      switch (attr) {
      case 'min':
      case 'max':
      case 'step':
        that.slider.slider('option', attr, value);
        break;

      case 'value':
        value = parseInt(value, 10);
        value = isNaN(value) ? 0 : value;
        var max = that.get('max');
        var min = that.get('min');
        if (value > max) {
          value = max;
        }
        else if (value < min) {
          value = min;
        }
        that.slider.slider('value', value);
        break;
  
      case 'autofocus':
        this.settings.autofocus = value;
        break;
      }
    };
      
    this.get = function (attr, value) {
      switch (attr) {
      case 'min':
      case 'max':
      case 'step':
        return that.slider.slider('option', attr);

      case 'value':
        return that.slider.slider('value');

      case 'autofocus':
        return this.settings.autofocus;
      }
    };
      
    $(that.node).mousedown(function (e) { // this == current
      if (false === settings.onShow.call(that, that, this)) {
        return;
      }
      current = this;
      down = true;
      var max = that.get('max');
      var min = that.get('min');

      /* Is this needed? --prefill slider w/ value -- probably handled in onShow
       that.set('value', $(this).val());
      */
        
      // position the slider
      var left = e.pageX - 20 - 209 * ((that.slider.slider('value') - min) / (max - min));
      var top = $(this).offset().top + this.offsetHeight - $('#themebuilder-wrapper').offset().top;

      // Make sure the slider doesn't slide off of the bottom of the page.
      var sliderHeight = 50;
      top = Math.min(top, $('#themebuilder-wrapper').height() - sliderHeight);

      that.slider.parent().css('left', left).css('top', top).show();
        
      settings.onStart.apply(that, [that, e, that.get('value'), current]);
        
      that._slider._mouseCapture(e);
      var mm;
      var mu = function (e) {
        down = false;
        $('body').unbind('mousemove', mm).unbind('mouseup', mu);
        settings.onStop.apply(that, [that, e, that.get('value'), current]);
        that.slider.parent().hide();
      };
      mm = function (e) {
        e.stopPropagation();
        e.preventDefault();
        that._slider._mouseDrag(e);
      };
      $('body').mousemove(mm).mouseup(mu);
    });
  },
  true // allforone
);

/**
 * small jQuery hack to enable the customized ui slider
 */
(function () {
  var oinit = jQuery.ui.slider.prototype._init;
  jQuery.ui.slider.prototype = jQuery.extend(jQuery.ui.slider.prototype, {
    _init: function () {
      this.__init = oinit;
      this.__init();
      this._trigger('init', {}, this);
    }
  });
}());
