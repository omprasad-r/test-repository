(function ($) {
// The remove and format functions do not work properly in the jcarousel
// plugin. So we overwrite them.
if ($.isFunction($.jcarousel)) {
  $.jcarousel.fn.extend({
    // Add is broken in the plugin. We fix it here. 
    add: function(s, i) {
      var pivot, carousel = this.list, items = carousel.find('.jcarousel-item'), item = $(s);
      // Prepend the item before the index passed in
      if (i && i >= 0) {
        pivot = items.get(i);
        pivot.before(item);
        item.css({visibility: 'hidden'});
      }
      // Otherwise tack it on the end if no index is passed in
      else {
        items.filter(':last').after(item);
        item.css({visibility: 'hidden'});
      }
      // Renumber the carousel items
      var _renumerate = ThemeBuilder.bind(this, this.renumerate, 'add', item);
      // Scroll to the new item
      // this.scroll((items.length + 1), true);
      // Make the item visible, then hide it, then show it slowly.
      item.css({visibility: 'visible'}).hide().show('slow', _renumerate);
    },
    remove: function (i) {
      var e = this.get(i);

      // The actual remove() call happens in renumerate because the hide() call takes time
      // and the renumeration can't happen until this operation completes. So it's necessary
      // to call remove() from the hide() callback.
      var _renumerate = ThemeBuilder.bind(this, this.renumerate, 'remove', e);
      e.hide('slow', _renumerate);
    },
    // This is a new function that correctly sets the number of carousel items and
    // re-classes them according to their updated index positions.
    renumerate: function (action, e) {
      // Lock the scrolling buttons
      this.lock();

      if (typeof action === "string") {
        switch (action) {
        case "remove":
          e.remove();
          this.options.size--;
          break;
        case "add":
          this.options.size++;
          break;
        default:
          break;
        }
      }

      var li = this.list.children('li');
      var self = this;

      if (li.length > 0) {
        var wh = 0, i = this.options.offset;
        li.each(function () {
          self.format(this, i++);
          wh += self.dimension(this, null);
        });

        this.list.css(this.wh, wh + 'px');
      }
      li.filter(':last-child').addClass('last');
      // Unlock the scrolling buttons
      this.unlock();
    },
    format: function (e, i) {
      // remove all class names matches 'jcarousel-item' at the start
      $(e)[0].className = $(e)[0].className.replace(/\bjcarousel\-item\-\d+\b/g, '');
      // add new class names  
      var $e = $(e).addClass('jcarousel-item').addClass('jcarousel-item-' + i);
      $e.attr('jcarouselindex', i);
      return $e;
    },
    // The scroll function is overridden because there is no way to pass in 
    // the second parameter (true) to the pos() function to force it to show
    // the complete item that is scrolled to
    scroll: function(i, a) {
      if (this.locked || this.animating) {
        return;
      }

      this.pauseAuto();
      this.animate(this.pos(i, true), a);
    }
  });
}
})(jQuery);