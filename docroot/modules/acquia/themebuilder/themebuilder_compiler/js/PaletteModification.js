
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

/**
 * The PaletteModification is a subclass of the abstract Modification class.  An
 * instance of this class can hold a modification to the global palette such
 * that it can be applied and reverted.
 * @class
 * @extends ThemeBuilder.Modification
 */
ThemeBuilder.PaletteModification = ThemeBuilder.initClass();

// Subclass the Modification class.
ThemeBuilder.PaletteModification.prototype = new ThemeBuilder.Modification();

/**
 * The type string that indicates this is a palette modification.
 */
ThemeBuilder.PaletteModification.TYPE = 'palette';
ThemeBuilder.registerModificationClass('PaletteModification');

/**
 * This static method returns a correctly initialized PaletteModification
 * instance that contains the specified prior state and new state.  Enough
 * checking is performed to ensure that the newly instantiated object is valid.
 *
 * @return
 *   A new instance of PaletteModification that contains the specified prior
 *   state and new state.
 */
ThemeBuilder.PaletteModification.create = function (priorState, newState) {
  if (ThemeBuilder.PaletteModification.TYPE !== priorState.type) {
    throw 'Cannot create a PaletteModification from state type ' + priorState.type;
  }

  var instance = new ThemeBuilder.PaletteModification(priorState.paletteId);
  instance.setPriorState(priorState.paletteId);
  if (newState) {
    instance.setNewState(newState.paletteId);
  }
  return instance;
};

/**
 * The constructor for the PaletteModification class.  This initializes the type
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
ThemeBuilder.PaletteModification.prototype.initialize = function (selector) {
  ThemeBuilder.Modification.prototype.initialize.call(this, selector);
  this.type = ThemeBuilder.PaletteModification.TYPE;
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
ThemeBuilder.PaletteModification.prototype.createState = function (paletteId) {
  return {
    'paletteId' : paletteId
  };
};
