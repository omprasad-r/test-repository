/**
 * JavaScript behaviors related to Colorbox module for Drupal Gardens sites.
 */

(function ($) {

/**
 * Resizes Colorbox for responsive.
 *
 * - Ensures Colorbox displays images at their proper dimensions for desktop
 *   browsers.
 * - Shrinks Colorbox and the contained image if the window is smaller than
 *   Colorbox, maintaining aspect ratio.
 * - Resizes Colorbox and the contained image on orientation change and resize
 *   events.
 */
Drupal.behaviors.GardensFeaturesColorboxResize = {
  attach: function (context, settings) {

    $(document).unbind('cbox_complete.gardens_features');
    $(document).bind('cbox_complete.gardens_features', function() {
      var img = new Image(),
          loadedImage = $('#cboxLoadedContent').find('img'),
          resizeHandlerClass,
          resizeHandler,
          colorbox = $.colorbox;

      // Create a new DOM element so we can extract the real image dimensions.
      img.src = loadedImage.attr('src');

      // Avoid unwanted scrollbars.
      $('#cboxLoadedContent').css('overflow', 'hidden');

      // Undo Responsivizer's fluid images technique.
      loadedImage.css('max-width', 'none');

      // Set up a resize handler to resize Colorbox.
      resizeHandlerClass = new Drupal.gardensFeaturesColorbox(img, loadedImage, colorbox);
      resizeHandler = $.proxy(resizeHandlerClass.delayUpdate, resizeHandlerClass);

      // Update the size of Colorbox on initial load.
      resizeHandlerClass.update(img, loadedImage, colorbox);

      // If the orientation changes, resize Colorbox.
      if (window.orientation !== undefined) {
        window.onorientationchange = resizeHandler;
      }

      // If the window is resized, resize Colorbox.
      $(window).unbind('resize.gardens_features');
      $(window).bind('resize.gardens_features', resizeHandler);
    });
  }
};

/**
 * Resize handler class.
 *
 * Captures the arguments in a closure so that they are available for use in
 * the orientation change and resize event handlers.
 */
Drupal.gardensFeaturesColorbox = function(img, loadedImage, colorbox) {
  this.$image = img;
  this.$loadedImage = loadedImage;
  this.$colorbox = colorbox;
};

/**
 * Dymanically resizes Colorbox to fit within the window.
 *
 * If the image is larger than the window minus Colorbox's chrome, calculate the
 * scale (quotient) by which we need to reduce the size of the image and
 * Colorbox overlay (whilst maintaining aspect ratio) to fit the overlay
 * completely inside the window.
 *
 * If the image is smaller than the window minus Colorbox's chrome, resize based
 * on a scale (quotient) of 1. This ensures Colorbox displays images in their
 * proper dimensions.
 */
Drupal.gardensFeaturesColorbox.prototype.update = function() {
  if (this.$image === undefined) {
    return;
  }

  var quotient = 1,
      scaleWidth,
      scaleHeight,
      horizontalChrome = 42,
      verticalChrome = 100;

  // Both width and height are larger than the window.
  if (this.$image.width  >= (window.innerWidth  - horizontalChrome) &&
      this.$image.height >= (window.innerHeight - verticalChrome)) {
    scaleWidth = this.$image.width / (window.innerWidth - horizontalChrome);
    scaleHeight = this.$image.height / (window.innerHeight - verticalChrome);

    // Find the largest quotient to ensure we scale down enough.
    quotient = Math.max(scaleWidth, scaleHeight);
  }

  // The image is wider than the window minus the chrome.
  else if (this.$image.width >= (window.innerWidth - horizontalChrome)) {
    quotient = this.$image.width / (window.innerWidth - horizontalChrome);
  }

  // The image is taller than the window minus the chrome.
  else if (this.$image.height >= (window.innerHeight - verticalChrome)) {
    quotient = this.$image.height / (window.innerHeight - verticalChrome);
  }

  // Scale the width and height of the image using the quotient and round down
  // to a whole pixel value.
  scaleWidth = Math.floor(this.$image.width / quotient);
  scaleHeight = Math.floor(this.$image.height / quotient);

  this.$loadedImage.css({
    'width': scaleWidth,
    'height': scaleHeight
  });

  this.$colorbox.resize({
    width: scaleWidth + horizontalChrome,
    height: scaleHeight + verticalChrome
  });
};

/**
 * Throttles resize events to once every 150ms to avoid stampedes on colorbox
 * resize (the resize event otherwise fires continuously during dragging).
 */
Drupal.gardensFeaturesColorbox.prototype.delayUpdate = function() {
  clearTimeout(this.timer);
  this.timer = setTimeout($.proxy(this.update, this), 150);
};

})(jQuery);
