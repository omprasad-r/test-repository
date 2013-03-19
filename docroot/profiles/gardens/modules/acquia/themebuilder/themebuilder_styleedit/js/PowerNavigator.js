
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true window: true ThemeBuilder: true */

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * The PowerNavigator allows the user to traverse through the DOM by
 * clicking on arrows on each side of a box that surrounds the selected
 * element.  This has been abstracted out of the ElementPicker class so we
 * could implement multiple strategies for selected element highlight and DOM
 * navigation, and bind the strategy to the version of the theme it was meant
 * to work with.
 * @class
 */
ThemeBuilder.styles.PowerNavigator = ThemeBuilder.initClass();
ThemeBuilder.styles.PowerNavigator.prototype.initialize = function () {
  this.arrowClicked = ThemeBuilder.bind(this, this._arrowClicked);
  this.selected = null;
  this.advanced = false;

  // Add the Power navigator markup
  var $ = jQuery;
  this.left = $('<div class="tb-nav tb-no-select"></div>').append('<div class="arrow  tb-no-select" title="Select the previous sibling element"></div>').addClass('left').appendTo('body');
  this.right = $('<div class="tb-nav tb-no-select"></div>').append('<div class="arrow  tb-no-select" title="Select the next sibling element"></div>').addClass('right').appendTo('body');
  this.top = $('<div class="tb-nav tb-no-select"></div>').append('<div class="arrow  tb-no-select" title="Select the parent element"></div>').addClass('top').appendTo('body');
  this.bottom = $('<div class="tb-nav tb-no-select"></div>').append('<div class="arrow  tb-no-select" title="Select the first child element"></div>').addClass('bottom').appendTo('body');
  $('.arrow').bind('click', this.arrowClicked);
};

/**
 * Causes the element(s) identified by the specified selector to be highlighted.
 *
 * @param {String} selector
 *   The selector that describes the set of selected elements.
 */
ThemeBuilder.styles.PowerNavigator.prototype.highlightSelection = function (selector) {
  var $ = jQuery;
  this.unhighlightSelection();
  if (!selector) { // If the highlighted element does not match the element selector, hide the Navigator arrows
    $('.tb-nav, .tb-hover').hide();
    return;
  }
  if (selector) {
    selector = ThemeBuilder.util.removeStatePseudoClasses(selector);
    
    // Wrap matching, non-selected elements in a secondary highlight.
    $(selector).not('.selected, .selected *, .tb-no-select').addClass('selection').append('<div class="selection-highlight tb-no-select"><div class="highlight-inner"></div></div>');

    if ($(selector).is('.selected')) {
      $('.tb-nav').show();
      $('.link-hover').show();
    } else {
      $('.tb-nav').hide();
      $('.link-hover').hide();
    }
  }
};

/**
 * Causes the entire navigator to be removed from the dom.
 */
ThemeBuilder.styles.PowerNavigator.prototype.deleteNavigator = function () {
  var $ = jQuery;
  this.unhighlightSelection();
  this.remove();
};

/**
 * Remove the highlight from the elements identified by the current selector.
 */
ThemeBuilder.styles.PowerNavigator.prototype.unhighlightSelection = function () {
  var $ = jQuery;
  $('.selection').removeClass('selection');
  $('.selection-highlight').remove();
};

/**
 * Highlight the selected element itself and add the controls for moving
 * around the DOM.
 *
 * @param {jQuery element} $element
 *   The selected element.
 */
ThemeBuilder.styles.PowerNavigator.prototype.highlightClicked = function ($element) {
  var $ = jQuery;

  $('.selected').removeClass('selected');
  $('.tb-inset').removeClass('tb-inset');

  if ($element) {
    this.selected = $element;
    $element.addClass('selected');
  }
  this.updateDisplay();
};

/**
 * Called when one of the navigator arrows is clicked.
 *
 * @private
 *
 * @param {Event} event
 *   The click event.
 */
ThemeBuilder.styles.PowerNavigator.prototype._arrowClicked = function (event) {
  var $ = jQuery;
  event.stopPropagation();
  event.preventDefault();
  var direction = this._getDirection($(event.currentTarget));

  switch (direction) {
  case 'up':
    this.selected.parent().closest('.style-clickable:visible').click();
    break;
  case 'down':
    this.selected.find('.style-clickable:visible').first().click();
    break;
  case 'right':
    if (this.selected.next('.style-clickable:visible').length > 0) {
      this.selected.next('.style-clickable:visible').click();
    } else {
      this.selected.nextUntil('.style-clickable:visible').next('.style-clickable:visible').click();
    }
    break;
  case 'left':
    if (this.selected.prev('.style-clickable:visible').length > 0) {
      this.selected.prev('.style-clickable:visible').click();
    } else {
      this.selected.prevUntil('.style-clickable:visible').prev('.style-clickable:visible').click();
    }
    break;
  }
};

/**
 * Returns a string the identifies the direction that the user clicked.
 *
 * @private
 *
 * @param {jQuery element} $currentTarget
 *   The element representing the arrow the user clicked on.
 *
 * @return
 *   A string indicating the direction within the DOM the user would like to
 *   navigate to.  It will be one of 'up', 'down', 'left' or 'right'.
 */
ThemeBuilder.styles.PowerNavigator.prototype._getDirection = function ($currentTarget) {
  var direction = 'up';
  var $ = jQuery;
  if ($currentTarget.is('.top .arrow')) {
    direction = 'up';
  } 
  else if ($currentTarget.is('.bottom .arrow')) {
    direction = 'down';
  } 
  else if ($currentTarget.is('.right .arrow')) {
    direction = 'right';
  } 
  else if ($currentTarget.is('.left .arrow')) {
    direction = 'left';
  }
  return direction;
};

ThemeBuilder.styles.PowerNavigator.prototype.updateDisplay = function () {
  var $ = jQuery;

  if (!this.selected) {
    return;
  }
  var selectedOffset = this.selected.offset();
  selectedOffset.height = this.selected.outerHeight(false);
  selectedOffset.width = this.selected.outerWidth(false);

  if (selectedOffset.height === 0) {
    var next = this.selected.children('.style-clickable:visible').first();
    while (selectedOffset.height === 0) {
      var prev = selectedOffset.height;
      selectedOffset.height = $(next).outerHeight(false);
      next = $(next).children('.style-clickable:visible').first();
    }
  }
  if (this.selected.is('.deco-bottom') && this.selected.outerHeight(false) === 0) {
    selectedOffset.top = selectedOffset.top - selectedOffset.height;
  }

  selectedOffset.right = selectedOffset.left + selectedOffset.width;
  selectedOffset.bottom = selectedOffset.top + selectedOffset.height;

  var pageWidth = $('body').width();
  var pageHeight = $('body').height();

  if (selectedOffset.right >= pageWidth - 4) {
    selectedOffset.right = selectedOffset.right - 4;
    this.right.addClass('tb-inset');
  }
  if (selectedOffset.bottom + 292 > pageHeight - 2) {
    selectedOffset.bottom = selectedOffset.bottom - 2;
  }
  if (selectedOffset.left === 0) {
    selectedOffset.left = 4;
    this.left.addClass('tb-inset');
  }
  if (selectedOffset.width >= pageWidth) {
    selectedOffset.width = selectedOffset.width - 8;
  }

  if (selectedOffset.top <= 40) {
    selectedOffset.top = selectedOffset.top + 8;
  }

  this.top.css({'top' : selectedOffset.top, 'left' : selectedOffset.left, 'width' : selectedOffset.width});
  this.right.css({'top' : selectedOffset.top, 'left' : selectedOffset.right, 'height' : selectedOffset.height});
  this.bottom.css({'top' : selectedOffset.bottom, 'left' : selectedOffset.left, 'width' : selectedOffset.width});
  this.left.css({'top' : selectedOffset.top, 'left' : selectedOffset.left, 'height' : selectedOffset.height});

  $('.tb-nav-enabled').removeClass('tb-nav-enabled');
  if (this.advanced) {
    if (this.selected.parent().closest('.style-clickable:visible').length > 0) {
      this.top.addClass('tb-nav-enabled');
    }
    if (this.selected.find('.style-clickable:visible').length > 0) {
      this.bottom.addClass('tb-nav-enabled');
    }
    if (this.selected.nextUntil('.style-clickable:visible').next('.style-clickable:visible').length > 0 || this.selected.next('.style-clickable:visible').length > 0) {
      this.right.addClass('tb-nav-enabled');
    }
    if (this.selected.prevUntil('.style-clickable:visible').prev('.style-clickable:visible').length > 0 || this.selected.prev('.style-clickable:visible').length > 0) {
      this.left.addClass('tb-nav-enabled');
    }
  }
};
