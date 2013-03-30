
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

/**
 * The SwatchModification is a subclass of the abstract Modification class.  An
 * instance of this class can hold a modification to a swatch in a palette such
 * that it can be applied and reverted.
 * @class
 * @extends ThemeBuilder.Modification
 */
ThemeBuilder.SwatchModification = ThemeBuilder.initClass();

// Subclass the Modification class.
ThemeBuilder.SwatchModification.prototype = new ThemeBuilder.Modification();

/**
 * The type string that indicates this is a swatch modification.
 */
ThemeBuilder.SwatchModification.TYPE = 'swatch';
ThemeBuilder.registerModificationClass('SwatchModification');

/**
 * This static method returns a correctly initialized SwatchModification
 * instance that contains the specified prior state and new state.  Enough
 * checking is performed to ensure that the newly instantiated object is valid.
 *
 * @return
 *   A new instance of SwatchModification that contains the specified prior
 *   state and new state.
 */
ThemeBuilder.SwatchModification.create = function (priorState, newState) {
  if (ThemeBuilder.SwatchModification.TYPE !== priorState.type) {
    throw 'Cannot create a SwatchModification from state type ' + priorState.type;
  }

  var instance = new ThemeBuilder.SwatchModification(priorState.index, priorState.hex);
  instance.setPriorState(priorState.index, priorState.hex);
  if (newState) {
    instance.setNewState(newState.index, newState.hex);
  }
  return instance;
};

/**
 * The constructor for the SwatchModification class.  This initializes the type
 * and palette id for the modification.  You should never call this method
 * directly, but rather use code such as:
 * <pre>
 *   var modification = new PaletteModification();
 * </pre>
 *
 * @param selector
 *   Where to apply the palette change. For the entire site, this should be
 *   'global'.
 */
ThemeBuilder.SwatchModification.prototype.initialize = function (selector) {
  ThemeBuilder.Modification.prototype.initialize.call(this, selector);
  this.type = ThemeBuilder.SwatchModification.TYPE;
};

/**
 * Creates a simple object that encapsulates a state (either a prior state or
 * a new state) which will be associated with this modification instance.
 *
 * @param property string
 *   The property name.
 *
 * @param value string
 *   The value associated with the property.
 */
ThemeBuilder.SwatchModification.prototype.createState = function (index, hex) {
  return {
    'index' : index,
    'hex' : hex
  };
};
