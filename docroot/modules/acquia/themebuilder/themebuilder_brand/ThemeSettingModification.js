
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

/**
 * The ThemeSettingModification is a subclass of the abstract Modification class.
 *
 * An instance of this class can hold a modification to the theme settings such
 * that it can be applied and reverted.
 *
 * @class
 * @extends ThemeBuilder.Modification
 */
ThemeBuilder.ThemeSettingModification = ThemeBuilder.initClass();

// Subclass the Modification class.
ThemeBuilder.ThemeSettingModification.prototype = new ThemeBuilder.Modification();

/**
 * The type string that indicates this is a theme setting modification.
 */
ThemeBuilder.ThemeSettingModification.TYPE = 'themeSetting';
ThemeBuilder.registerModificationClass('ThemeSettingModification');

/**
 * Creates a new ThemeSettingModification instance.
 *
 * This static method returns a correctly initialized ThemeSettingModification
 * instance that contains the specified prior state and new state. Enough
 * checking is performed to ensure that the newly instantiated object is valid.
 *
 * @return
 *   A new instance of ThemeSettingModification that contains the specified
 *   prior state and new state.
 */
ThemeBuilder.ThemeSettingModification.create = function (priorState, newState) {
  var instance;

  if (ThemeBuilder.ThemeSettingModification.TYPE !== priorState.type) {
    throw 'Cannot create a ThemeSettingModification from state type ' + priorState.type;
  }

  // Instantiate a new ThemeSettingModification instance.
  instance = new ThemeBuilder.ThemeSettingModification(priorState.selector);

  // Set the prior and new states.
  instance.setPriorState(priorState);

  if (newState) {
    // Handle the viewport theme settings differently to normal theme settings.
    if (priorState.selector !== 'viewport') {
      newState = newState.value;
    }

    instance.setNewState(newState);
  }

  return instance;
};

/**
 * The constructor for the ThemeSettingModification class. You should never call
 * this method directly, but rather use code such as:
 * <pre>
 *   var modification = new ThemeSettingModification();
 * </pre>
 */
ThemeBuilder.ThemeSettingModification.prototype.initialize = function (key) {
  ThemeBuilder.Modification.prototype.initialize.call(this, key);
  this.type = ThemeBuilder.ThemeSettingModification.TYPE;
};

/**
 * Creates a simple object that encapsulates a state.
 *
 * The state is either a prior state or a new state which will be associated
 * with this modification instance.
 *
 * @param value
 *   The value of the theme setting modification.
 */
ThemeBuilder.ThemeSettingModification.prototype.createState = function (value) {
  // Allow viewport theme settings to be an object of settings, rather than
  // simple values.
  if (this.selector === 'viewport') {
    return value;
  }

  return {
    'value': value
  };
};

