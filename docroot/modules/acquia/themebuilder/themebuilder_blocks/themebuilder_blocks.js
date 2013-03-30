/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true debug: true window: true*/
(function ($) {

  var omouseStart = $.ui.sortable.prototype._mouseStart;
  $.ui.sortable.prototype = $.extend($.ui.sortable.prototype, { 
    _mouseStart: function () {
      this.__mouseStart = omouseStart;
      this.__mouseStart.apply(this, arguments);
      this.offset.click.left = 50;
      this.offset.click.top = 50;
    }
  });

  /**
   * @class
   */
  Drupal.behaviors.editBlocks = {
    attach: function (context, settings) {
      // Add blocks palette component to page.
    },
  
    init: function () {
    },

    show: function () {
      var regions = $('div.block-region').parent().add('div.region');
      var that = this;
      regions.sortable({
        items: '>div.block',
        connectWith: regions,
        opacity: 0.8,
        cursor: 'move',
        appendTo: 'body',
        cursorAt: 'top',
        distance: 20,
        tolerance: 'pointer',
        forceHelperSize: false,
        placeholder: 'block-placeholder',
        helper: function (e, node) {
          var title = $('h2', node).html();
          if (!title) {
            title = $(node)[0].id.split('-').slice(1).join(' ').replace(new RegExp(' .', 'g'), function (x) {
              return ' ' + x[1].toUpperCase();
            });
            title = title[0].toUpperCase() + title.slice(1);
          }
          return $('<div class="dragger"></div>').html(title).appendTo('body');
        },
        stop: function (e, ui) {
          that.saveBlocks();
        }
      });
      Drupal.settings.editBlocks = true;
    },
    hide: function () {
      $('div.block-region').parent().sortable('destroy');
      Drupal.settings.editBlocks = false;
    },
  
    // Cancel blocks editing. First hide our controls, then
    // reload page. We just cannot revert to the previous state
    // cleanly otherwise.
    cancelBlocks: function () {
      Drupal.behaviors.editBlocks.toggleEditor();
      window.location.reload();
    },
  
    saveBlocks: function () {
      var regions = $('div.block-region').parent().add('div.region');
    
      var data = 'form_token=' + Drupal.encodeURIComponent(Drupal.settings.blocksSubmitToken);
      regions.each(function () {
        var that = $(this);
        while (!that.attr('id') && (that = that.parent())) {
        }
        var region = that.attr('id').replace('-region', '').replace(/-/g, '_');
        if (region === 'content_area') {
          region = 'content';
        }
        data += '&regions[' + region + ']=' + $(this).sortable('toArray').join(',');
      });
    
      $.ajax({
        type: "POST",
        url: Drupal.settings.blocksSubmitPath,
        // Send placement and order of blocks.
        data: data,
        success: function (data) {
        },
        error: function (xmlhttp) {
          alert(Drupal.t('An HTTP error @status occured.', { '@status': xmlhttp.status }));
        }
      });
    }
  };

  Drupal.behaviors.editBlocks.attach();

}(jQuery));
