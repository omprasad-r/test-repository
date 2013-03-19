/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global window : true jQuery: true Drupal: true ThemeBuilder: true*/

ThemeBuilder = ThemeBuilder || {};

ThemeBuilder.ui = ThemeBuilder.ui || {};

/**
 * The horizontal slider wraps a specified element with a left/right scrolling controlling
 * that employs buttons to slide the content rather than a scroll bar.
 * @class
 */
ThemeBuilder.ui.HorizontalCarousel = ThemeBuilder.initClass();

/**
 * The constructor of the HorizontalCarousel class.
 *
 * @param {DomElement} element
 *   The element is a pointer to the jQuery object that will be wrapped in the 
 *   horizontal carousel.
 * @param {int} [optional] steps
 *   A Number indicating the number of clicks a user will need to make in order
 *   scroll the carousel from one end of the content items to the other
 * @param {int} [optional] tolerance
 *   A pixel distance.  The amount of distance from the ends of the scroller that should
 *   disregarded when a user has scrolled near the extreme left or right of the content.
 * @param {int} [optional] duration
 *   The number of milliseconds that the slide animation should last.
 * @return {Boolean}
 *   Returns true if the carousel initializes
 */
ThemeBuilder.ui.HorizontalCarousel.prototype.initialize = function (element, steps, tolerance, duration) {
  
  var $ = jQuery;
  
  this._element = {};
  this._type = '';
  this._steps = steps || 3;
  this._tolerance = tolerance || 20;
  this._animationDuration = duration || 220;
  this._currentOffset = NaN;
  this._scrollContentWidth = 0;
  this._scrollPaneWidth = 0;
  this._scrollRemainder = 0;
  // Private functions
  this.stripPX = ThemeBuilder.bind(this, this._stripPX);
  
  if (element) {
    this._element = element;
  } 
  else {
    return false;
  }
  
  this._type = 'HorizontalCarousel';

  this._element
    .addClass('scroll-content ui-widget ui-widget-header ui-corner-all');
  this._scrollContent = $('.scroll-content');
  this._scrollContent
    .addClass('clearfix');
  
  this._scrollPane = this._scrollContent
    .wrap('<div class="scroll-pane"></div>')
    .parent()
    .addClass('clearfix');
    
  this._horizontalCarousel = this._scrollPane
    .wrap('<div class="horizontal-carousel"></div>')
    .parent()
    .addClass('clearfix');
  
  this._decrementButton = this._horizontalCarousel
    .prepend('<a href="#" class="decrement button"></a>')
    .children().first()
    .bind('click', ThemeBuilder.bind(this, this.slideCarousel));
    
  this._incrementButton = this._horizontalCarousel
    .prepend('<a href="#" class="increment button"></a>')
    .children().first()
    .bind('click', ThemeBuilder.bind(this, this.slideCarousel));
  
  this._handleHelper = $('.ui-handle-helper-parent', this._horizontalCarousel);
  this._buttons = $('.button', this._horizontalCarousel);
  
  this._scrollPane.css({
    overflow: 'hidden' // change overflow to hidden now that the carousel handles the scrolling
  });
  
  this._trackWindowSize(); // update the UI when the window size changes
  return true;
};

/**
 * Handles the click event from the carousel controls.
 *
 * @param {Event} event
 *   Event object from the click of the carousel controls.
 * @return {Boolean} false
 *   Returns false to prevent default anchor tag behavior.
 */
ThemeBuilder.ui.HorizontalCarousel.prototype.slideCarousel = function (event) {
  var $ = jQuery;
  event.preventDefault();
  
  this._scrollContentWidth = this._getContentWidth();
  this._scrollPaneWidth = this._getPaneWidth();
  this._scrollRemainder = this._scrollPaneWidth - this._scrollContentWidth;
  var increment = Math.floor(this._scrollContentWidth / this._steps);
  var trigger = $(event.currentTarget);
  
  if (trigger.hasClass('increment')) { 
    if (trigger.hasClass('disabled')) {
      return false;
    }
    this._shiftContent((this._currentOffset - increment), true, 'increment');
    return false;
  }
  if (trigger.hasClass('decrement')) {
    if (trigger.hasClass('disabled')) {
      return false;
    }
    this._shiftContent((this._currentOffset + increment), true, 'decrement');
    return false;
  }
  return false;
};

/**
 * Rerenders the carousel.
 * Called externally to recreate the content of the carousel.
 *
 * @return {Boolean} true
 *   Returns true in call cases.
 */
ThemeBuilder.ui.HorizontalCarousel.prototype.updateUI = function () {
  var $ = jQuery;
  
  var scrollContentWidthOrig = this._scrollContentWidth;
  var scrollPaneWidthOrig = this._scrollPaneWidth;
  var currentOffsetOrig = this._currentOffset;
  var scrollRemainderOrig = this._scrollRemainder;

  this._scrollPaneWidth = this._getPaneWidth();
  this._scrollPane.add(this._scrollContent).width('auto');
  this._scrollContentWidth = this._getContentWidth();
  
  if (this._scrollContentWidth === 'auto') {
    return false;  // don't update the UI if it doesn't have content
  }
  //Set the widths of the content and pane
  this._scrollContent.width(this._scrollContentWidth);
  this._scrollPane.width(this._scrollPaneWidth);
  this._scrollRemainder = this._scrollPaneWidth - this._scrollContentWidth;
  this._scrollRemainder = typeof(this._scrollRemainder) === 'number' ? this._scrollRemainder : 0;
  // When the user selects a new element, shift the scrollContent to maximum offset.
  if (isNaN(this._currentOffset)) { // Initial updateUI call.
    this._shiftContent(this._scrollRemainder, false);
    return true;
  }
  if (scrollPaneWidthOrig > scrollContentWidthOrig) { // They weren't scrolling, so shift to the left.
    this._shiftContent(this._scrollRemainder, false);
    return true;
  }
  if (this._currentOffset === 0) { // They scrolled all the way left, keep this offset of zero.
    this._shiftContent(0, false);
    return true;
  }
  if (currentOffsetOrig > scrollRemainderOrig) { // They scrolled somewhere in between the ends of the content.
    var shift = currentOffsetOrig + (scrollContentWidthOrig - this._scrollContentWidth);
    this._shiftContent(shift, false);
    return true;
  }
  this._shiftContent(this._scrollRemainder, false); // Just shift it completely to the left.
  return true;
};

/**
 * Returns a jQuery pointer to this object
 *
 * @return {object}
 *   Returns null if the carousel has no pointer.
 */
ThemeBuilder.ui.HorizontalCarousel.prototype.getPointer = function () {
  if (this._horizontalCarousel) {
    return this._horizontalCarousel;
  } 
  else {
    return null;
  }
};

/**
 * Content position relative to the offset parent.
 *
 * @return {Object}
 *   position contains left and top position values
 */
ThemeBuilder.ui.HorizontalCarousel.prototype.getContentPos = function () {
  var contentPositionLeft = this._stripPX(this._scrollContent.css('left'));
  var contentPositionTop = this._stripPX(this._scrollContent.css('top'));
  return {left: contentPositionLeft, top: contentPositionTop};
};

/**
 * Handles window resizing 
 */
ThemeBuilder.ui.HorizontalCarousel.prototype._reFlow = function () {
  var $ = jQuery;
  
  var scrollRemainderCurrent = this._scrollRemainder || 0;
  this._scrollPaneWidth = this._getPaneWidth();
  this._scrollPane.width(this._scrollPaneWidth);
  this._scrollRemainder = this._scrollPaneWidth - this._scrollContentWidth;
  if (isNaN(this._scrollRemainder)) {
    this._scrollRemainder = 0;
    this._currentOffset = 0;
  }
  var delta = this._scrollRemainder - scrollRemainderCurrent;
  var shift = this._currentOffset + delta;
  this._shiftContent(shift, false);
};

/**
 * Content width is calculated as the sum of the outerWidths of all child elements.
 *
 * @return {int}
 *   Returns the width of the content, or zero if the content does not have width.
 */
ThemeBuilder.ui.HorizontalCarousel.prototype._getContentWidth = function () {

  var scrollContentItems = this._scrollContent.children();
  var scrollContentItemNum = scrollContentItems.size();
  
  if (scrollContentItemNum > 0 && scrollContentItems.eq(0).is(':visible')) {
    var computedSize = 0;
    while (scrollContentItemNum--) {
      var item = scrollContentItems.eq(scrollContentItemNum);
      var marginLeft = this._stripPX(item.css('margin-left'));
      var marginRight = this._stripPX(item.css('margin-right'));
      computedSize += Math.ceil(item.outerWidth() + Number(marginLeft) + Number(marginRight));
    }
    // Add 2px for the angels - JBeach.
    return Number(computedSize) + 2;
  }
  return 'auto';
};

/**
 * Pane width is calculated as the space inside the carousel excluding
 * the width of the controls.
 *
 * @return {int}
 *   Returns the width of the scrollPane.
 */
ThemeBuilder.ui.HorizontalCarousel.prototype._getPaneWidth = function () {
  return this._horizontalCarousel.width() - (this._incrementButton.outerWidth(true) + this._decrementButton.outerWidth(true));
};

/**
 * Utility function to set the offset of scrollContent
 *
 * @param {int} offset
 *   The offset value to move scrollContent.
 * @param {Boolean} animate
 *   Whether or not setting the offset should animate the movement of the content.
 */
ThemeBuilder.ui.HorizontalCarousel.prototype._setOffset = function (offset, animate) {
  if (animate) {
    this._scrollContent.animate({
      'left': offset + 'px'
    }, this._animationDuration, 'swing');
  }
  else {
    this._scrollContent.css({
      'left': offset + 'px'
    });
  }
  this._currentOffset = offset;
};

/**
 * Utility function to remove 'px' from calculated values.  The function assumes that
 * that unit 'value' is pixels.
 *
 * @param {String} value
 *   The String containing the CSS value that includes px.
 * @return {int}
 *   Value stripped of 'px' and casted as a number or NaN if 'px' is not found in the string.
 */
ThemeBuilder.ui.HorizontalCarousel.prototype._stripPX = function (value) {
  var index = value.indexOf('px');
  if (index === -1) {
    return NaN;
  }
  else {
    return Number(value.substring(0, index));
  }
};
 
 
 /**
  * Controls the update of the UI after a shifting event such as a click or reFlow
  *
  * @param {int} offset
  *   The offset value to move scrollContent.
  * @param {Boolean} animate
  *   Whether or not setting the offset should animate the movement of the content.
  * @param {string} direction
  *   'increment' or 'decrement'.
  */
ThemeBuilder.ui.HorizontalCarousel.prototype._shiftContent = function (offset, animate, direction) {
  this._snapScroll(offset, animate, direction);
  this._setButtonState();
};
 
/**
 * Snaps the scroll content to the edges of the visible pane area when the content
 * is displaced outside the visible pane area by less than the provided tolerance
 * value. Tolerance is set to 20 by default.  Snap scroll is also responsible
 * for keeping the scroll pane from exceeding its boundaries.  The left edge of the content
 * pane cannot scroll farther right than a zero offset and the right edge
 * of the content scroll pane cannot be translated farther left than the delta
 * between of the content scroll pane and the containing pane.
 *
 *   Illegal Cases - snapping must occur regardless of tolerance
 *   case 1: left edge of content is at a positive offset or the scrollRemainder is positive
 *   case 2: left edge of content is at a negative offset, greater than the scrollRemainer
 *
 *   Snap Cases - Snapping might occur depending on the value of tolerance (default 10px)
 *   case 3: the absolute value of offset is at tolerance or less
 *   case 4: the absolute difference between offset and scrollRemainder is at tolerance or less.
 *
 * @param {int} offset
 *   The offset value to move scrollContent.
 * @param {Boolean} animate
 *   Whether or not setting the offset should animate the movement of the content.
 * @param {string} direction
 *   'increment' or 'decrement'.
 * @return {Boolean}
 *   Returns true of the offset was altered before being sent to setOffset
 */
ThemeBuilder.ui.HorizontalCarousel.prototype._snapScroll = function (offset, animate, direction) {
  var tol = this._tolerance;
  // Correct illegal cases first.  We can quit the method when either case 3 or case 4 are resolved
  // after updating button state.
  // case 1
  if (this._scrollRemainder > 0) {
    this._setOffset(0, animate);
    return true;
  }
  // case 1
  if (offset > 0) {
    this._setOffset(0, animate);
    return true;
  }
  //case 2
  if ((this._scrollRemainder - offset) >= 0) {
    this._setOffset(this._scrollRemainder, animate);
    return true;
  }
  // The direction that the scrollContent is traveling is important because we only
  // want to snap the edge of the content that was just acted on, i.e. increment side
  // or decrement side.
  if (direction) {
    switch (direction) {
    case 'decrement': // <| decrement
      // case 3
      if (Math.abs(offset) <= tol) {
        this._setOffset(0, animate);
        return true;
      }
      // if no case applies, pass offset through to setOffset
      this._setOffset(offset, animate);
      return false;
    case 'increment': // increment |>
      //case 4
      if ((Math.abs(this._scrollRemainder - offset)) <= tol) {
        this._setOffset(this._scrollRemainder, animate);
        return true;
      }
      // if no case applies, pass offset through to setOffset
      this._setOffset(offset, animate);
      return false;
    default:
      break;
    }
  }
  // case 3
  // We want the left edge of the scrollContent to snap to zero in the default case
  if (tol >= Math.abs(offset) > 0) {
    this._setOffset(0, animate);
    return true;
  }
  // if no case applies, pass offset through to setOffset
  this._setOffset(offset, animate);
  return false;
};

/**
 * Set the slider button enabled state.  The state is determined
 * by the position of the content container inside the scroll pane.
 *
 *   case 1: left edge of content is at offset 0, and scrollContent length is less than scrollPane
 *      <| Decrement (disabled) ... Increment (disabled) |>
 *   case 2: left edge of content is at offset 0, and scrollContent length is greater than scrollPane
 *      <| Decrement (disabled) ... Increment |>
 *   case 3: left edge of content is at a negative offset, less than the scrollRemainder,
        and the scrollPaneWidth is bigger than scrollContentWidth
 *      <| Decrement ... Increment |>
 *   case 4: left edge of content is at a negative offset, equal to the scrollRemainder
 *      <| Decrement ... Increment (disabled) |>
 */
ThemeBuilder.ui.HorizontalCarousel.prototype._setButtonState = function () {
  var $ = jQuery;
  
  // decrement button
  //case 1 and case 2
  if (this._currentOffset === 0 || this._scrollRemainder > 0) {
    this._decrementButton.addClass('disabled');
  }
  else {
    this._decrementButton.removeClass('disabled');
  }
  // increment button
  //case 1
  if (this._scrollRemainder > 0) { // content has sufficient space
    this._incrementButton.addClass('disabled');
  }
  //case 4
  else if (this._currentOffset === this._scrollRemainder) { // shifted completely left
    this._incrementButton.addClass('disabled');
  }
  else {
    this._incrementButton.removeClass('disabled');
  }
};

/**
 * Causes window resizes to be detected, and resizes the themebuilder panel
 * accordingly.
 */
ThemeBuilder.ui.HorizontalCarousel.prototype._trackWindowSize = function () {
  var $ = jQuery;
  $(window).resize(ThemeBuilder.bind(this, this._windowSizeChanged));
  this._windowSizeChanged();
};

/**
 * When the window size changes, reset the max-width property of the
 * themebuilder.  Certain properties applied to the body tag will have an
 * effect on the layout of the themebuilder.  These include padding and
 * margin.  Because they change the size of the actual window, this type of
 * CSS "leakage" could not be fixed by the css insulator or iframe alone.
 *
 * @param {Event} event
 *   The window resize event.
 */
ThemeBuilder.ui.HorizontalCarousel.prototype._windowSizeChanged = function (event) {
  this._reFlow();
};