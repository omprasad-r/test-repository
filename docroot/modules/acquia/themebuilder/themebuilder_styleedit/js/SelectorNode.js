/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

ThemeBuilder.styles = ThemeBuilder.styles || {};

/**
 * The SelectorNode object represents the visual display of part
 * of a css selector.  The user can interact with each node, toggling
 * it on or off to enable or disable that part of the css selector,
 * and change the specificity for nodes representing html elements
 * that have multiple specificity options, such as tag, id, and
 * classes.
 * @class
 */
ThemeBuilder.styles.SelectorNode = ThemeBuilder.initClass();

/**
 * The constructor for the SelectorNode class.
 *
 * @param {SelectorEditor} editor
 *   The SelectorEditor instance that caused this object to be
 *   instantiated.
 * @param {int} index
 *   The index within the Selector corresponding to this editor node.
 * @param {PathElement} pathElement
 *   The PathElement instance this object should manipulate as the
 *   user interacts with it.
 * @param {boolean} isLast
 *   Indicates whether this is instance corresponds to the last item
 *   in the path.  This flag is used to manipulate the style of the
 *   editor node.
 */
ThemeBuilder.styles.SelectorNode.prototype.initialize = function (editor, index, pathElement, isLast) {
  this.editor = editor;
  this.id = 'path-element-' + index;
  this.pathElement = pathElement;
  this.toggleNode = ThemeBuilder.bind(this, this._toggleNode);
  this.specificityOptionsVisible = false;
  
  this.toggleSpecificityOptions = ThemeBuilder.bind(this, this._toggleSpecificityOptions);
  this.showSpecificityOptions = ThemeBuilder.bind(this, this._showSpecificityOptions);
  this.selectSpecificity = ThemeBuilder.bind(this, this._selectSpecificity);
  this.cancelSpecificity = ThemeBuilder.bind(this, this._cancelSpecificity);

  this.isFirst = (index === 0);
  this.isLast = isLast;
};

/**
 * Causes the markup representing this editor node to be generated and
 * added to the DOM.
 *
 * @param {DomElement} element
 *   The DomElement within which this editor markup should be placed.
 */
ThemeBuilder.styles.SelectorNode.prototype.create = function (element) {
  var $ = jQuery;
  var body = $('<div id="' + this.id + '" class="path-element' +
           (this.isFirst ? ' first' : '') +
           (this.isLast ? ' last' : '') +
    '">');
  //.append('<div class="path-element-left"></div>');
  var middle = $('<div class="path-element-inner">');
  var options = this.pathElement.getSpecificityOptions();

  if (options.identification.length + options.pseudoclass.length > 1) {
    // We need a pulldown list for this element.
    var pulldown = $('<div class="path-element-pulldown-button">')
    .click(ThemeBuilder.bind(this, this.toggleSpecificityOptions));
    middle.append(pulldown);
  }
  middle.append('<div class="path-element-label"></div>');
  body.append(middle);
  //.append('<div class="path-element-right">' + (this.isLast ? '' : '<div class="path-element-expand"></div>') + '</div>');
  $(element).append(body);
  body.click(this.toggleNode);
  this.refresh();
};

/**
 * Causes the markup for this editor to be removed from the DOM.
 */
ThemeBuilder.styles.SelectorNode.prototype.destroy = function () {
  var $ = jQuery;
  var body = $('#' + this.id);
  if (body) {
    body.unbind('click', this.onClick)
      .remove();
  }
};

/**
 *  Refreshes the display of this editor.  This should be called when
 *  the corresponding node is enabled or disabled and when a different
 *  specificity is selected.
 */
ThemeBuilder.styles.SelectorNode.prototype.refresh = function () {
  var $ = jQuery;
  var body = $('#' + this.id);
  if (body) {
    if (this.pathElement.enabled === true) {
      body.removeClass('disabled')
        .attr({title: Drupal.t('Click to broaden your styling')})
        .find('.path-element-pulldown-button').css({display: 'inline-block'});
    }
    else {
      body.addClass('disabled')
        .attr({title: Drupal.t('Click to narrow your styling')})
        .find('.path-element-pulldown-button').css({display: 'none'});
    }
    var settings = ThemeBuilder.getApplicationInstance().getSettings();
    var naturalLanguageEnabled = settings.naturalLanguageEnabled();
    if (naturalLanguageEnabled === true) {
      var text = ThemeBuilder.util.capitalize(this.pathElement.getHumanReadableLabelFromSelector(this.pathElement.getCssSelector()));
    }
    else {
      text = this.pathElement.getCssSelector();
    }
    $('#' + this.id + ' .path-element-label')
      .text(text);
  }
};

/**
 * This callback method is used to toggle this node instance on and
 * off.
 *
 * @param {Event} event
 *   The event.
 */
ThemeBuilder.styles.SelectorNode.prototype._toggleNode = function (event) {
  if (this.editor.selector.path.length === 1) {
    // Do not allow the only node to be toggled.
    return false;
  }
  this.pathElement.setEnabled(!this.pathElement.enabled);
  this.editor.pathSettingsModified();
  return false;
};

/**
 * Determines if the specificity panel should be shown or hidden when the dropdown trigger is clicked
 *
 * @param {Event} event
 *   The event.
 */
ThemeBuilder.styles.SelectorNode.prototype._toggleSpecificityOptions = function (event) {
  event.stopImmediatePropagation(); // Prevents the puck from being deselected.
  
  if (!this.specificityOptionsVisible) {
    this.showSpecificityOptions(event);
  } else {
    this.cancelSpecificity(event);
  }
};

/**
 * This callback is used to display the dropdown containing
 * specificity options associated with this node.  While the dropdown
 * is displayed, the user will not be able to interact with anything
 * but the specificity panel.  Clicks outside of this panel will cause
 * the panel to be dismissed.
 *
 * @param {Event} event
 *   The event.
 */
ThemeBuilder.styles.SelectorNode.prototype._showSpecificityOptions = function (event) {
  var $ = jQuery;
  var veil = $(this.editor.veilSelector)
    .removeClass('show')
    .unbind('click');
  var dropdown = $(this.editor.specificitySelector)
  .removeClass('show');
  var dropdownCenter = $(this.editor.specificityCenterSelector)
  .unbind('click')
  .html('');
  var options = this.pathElement.getSpecificityOptions();
  if (options.identification.length + options.pseudoclass.length > 1) {
    var $panel = $('<div>');
    $panel.append(this._getSpecificityPanel(options, 'identification').click(this.selectSpecificity));
    $panel.append(this._getSpecificityPanel(options, 'pseudoclass').click(this.selectSpecificity));
    dropdownCenter.append($panel);
    veil.attr('style', 'width: ' + $(document).width() + 'px; ' +
      'height: ' + $(document).height() + 'px;')
      .click(this.cancelSpecificity)
      .addClass('show');
    // Position the panel so it makes sense to the user.
    var pos = $(event.currentTarget).position();

    $('#' + this.id + ' .path-element-pulldown-button')
    .addClass('open');
    
    dropdown.addClass('show'); // Show must be called before positioning or the dropdown won't have a size yet
    
    this.specificityOptionsVisible = true;
    
    // A function to position the specificity panel inside the document.width and prevent collisions with it
    var dropdownPos = this._positionSpecificityOptions(pos.left, pos.top);
    
    // Position the dropdown
    dropdown
    .css('left', (dropdownPos.left))
    .css('top', (dropdownPos.top));
  }
  return false;
};

/**
 * Creates a specificity selection panel for the specified options of the
 * specified type.
 *
 * @param {Array} options
 *   An array of objects that contain the specificity options that should be
 *   included in the panel.
 *
 * @param {String} type
 *   Either 'identification' or 'pseudoclass', depending on which panel needs
 *   to be created.  The 'identification' panel allows selection of the
 *   element's id, tag, or any of its classes.  'pseudoclass' allows selection
 *   of any of the available pseudoclasses.
 *
 * @return
 *   A jQuery object representing the newly created panel.
 */
ThemeBuilder.styles.SelectorNode.prototype._getSpecificityPanel = function (options, type) {
  var $ = jQuery;
  var $panel = $('<div class="tb-type-' + type + '">');
  for (var i = 0; i < options[type].length; i++) {
    var classes = 'option';
    if (i === 0) {
      classes += ' first';
    }
    if (i === options[type].length - 1) {
      classes += ' last';
    }
    if (options[type][i].selected === true) {
      classes += ' selected';
    }

    var labelText = options[type][i].name;
    $panel.append('<div class="' + classes + '"><span/>' +
      labelText + '</div>');
  }
  return $panel;
};

/**
 * This callback is invoked when the user selects a specificity within
 * the dropdown box.  This causes the corresponding specifitity to be
 * selected and the dropdown box is dismissed.
 *
 * @param {Event} event
 *   The event.
 */
ThemeBuilder.styles.SelectorNode.prototype._selectSpecificity = function (event) {
  var $ = jQuery;
  var originalTarget = event.srcElement || event.originalTarget || event.target;
  var type = 'identification';
  var $target = $(originalTarget);
  if ($target.parent().hasClass('tb-type-pseudoclass')) {
    type = 'pseudoclass';
  }
  for (var i = 0; i < event.currentTarget.childNodes.length; i++) {
    if (originalTarget === event.currentTarget.childNodes[i]) {
      this.pathElement.setSpecificity(type, i);
      this.editor.pathSettingsModified();
      break;
    }
  }
  return this.cancelSpecificity(event);
};

/**
 * This callback is invoked when the user clicks outside of the
 * specificity dropdown box when the box is being displayed.  This
 * action effectively dismisses the specificity box with no change to
 * the specificity of the corresponding node.
 *
 * @param {Event} event
 *   The event.
 */
ThemeBuilder.styles.SelectorNode.prototype._cancelSpecificity = function (event) {
  var $ = jQuery;
  $(this.editor.specificitySelector)
  .removeClass('show')
  .unbind('click');
  $(this.editor.veilSelector)
    .removeClass('show')
    .unbind('click');
  $('#' + this.id + ' .path-element-pulldown-button')
  .removeClass('open');
  this.specificityOptionsVisible = false;
  return false;
};

/**
 * Provides window edge detection for the Specificity Options Pulldown
 * 
 * @param {Number} posLeft
 *   Original left position of the dropdown before repositioning
 *
 * @param {Number} posTop
 *   Original top position of the dropdown before repositioning. Not really used at the moment except
 *   as a simple pass through.
 */
 
ThemeBuilder.styles.SelectorNode.prototype._positionSpecificityOptions = function (posLeft, posTop) {
  
  var $ = jQuery;
  ThemeBuilder.bind(this, ThemeBuilder.styles.SelectorEditor); // Get the editor.
  var contentPos = {left: 0, top: 0};
  var pos = {}; // The position for the dropdown that will be calculated.
  
  // If we're using a widget, the positioning is more complicated because the pucks could
  // could be shifted left relative to its initial rendered origin.
  if (this.editor && this.editor.widgets && this.editor.widgets.PathSelector) {
    var contentPosObj = this.editor.widgets.PathSelector.getContentPos();
    contentPos.left = contentPosObj.left;
    contentPos.top = contentPosObj.top; 
  }
  
  var docWidth = document.width;
  var dropdown = $(this.editor.specificitySelector);
  var dropdownCenter = $(this.editor.specificityCenterSelector);
  
  // Get the width of the options lists because the option lists will wrap
  // if the options pulldown hits the edge of the document.
  // We need to get the width of each object and add them together to know what the width
  // would have been if the lists hadn't wrapped.
  
  // Get the width of the option lists individually
  var typeIdWidth = dropdownCenter.find('.tb-type-identification').outerWidth();
  var typePseudoWidth = dropdownCenter.find('.tb-type-pseudoclass').outerWidth();
  
  // Get the width of the images to the left and right of the pulldown
  var dropdownLeftWidth = $(this.editor.specificitySelectorLeft).outerWidth();
  var dropdownRightWidth = $(this.editor.specificitySelectorRight).outerWidth();
  
  //
  var realWidth = typeIdWidth + typePseudoWidth + dropdownLeftWidth + dropdownRightWidth;
  var realSpace = realWidth + posLeft + contentPos.left;
  
  if (realSpace > docWidth) {
    pos.left = docWidth - realWidth - 20;
  } else {
    pos.left = posLeft + contentPos.left;
  }
  
  pos.top = posTop + 14;
  
  return pos;
};
