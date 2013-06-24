/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true*/

/**
 * The NestedThemeSettingModification is a subclass of the abstract Modification class.
 *
 * An instance of this class can hold a modification to the theme settings such
 * that it can be applied and reverted. A nested theme setting can be also handled.
 *
 * @class
 * @extends ThemeBuilder.Modification
 */
ThemeBuilder.NestedThemeSettingModification = ThemeBuilder.initClass();

// Subclass the Modification class.
ThemeBuilder.NestedThemeSettingModification.prototype = new ThemeBuilder.Modification();

/**
 * The type string that indicates this is a theme setting modification.
 */
ThemeBuilder.NestedThemeSettingModification.TYPE = 'nestedThemeSetting';
ThemeBuilder.registerModificationClass('NestedThemeSettingModification');

/**
 * Creates a new NestedThemeSettingModification instance.
 *
 * This static method returns a correctly initialized NestedThemeSettingModification
 * instance that contains the specified prior state and new state. Enough
 * checking is performed to ensure that the newly instantiated object is valid.
 *
 * @return
 *   A new instance of NestedThemeSettingModification that contains the specified
 *   prior state and new state.
 */
ThemeBuilder.NestedThemeSettingModification.create = function (priorState, newState) {
  var instance;

  if (ThemeBuilder.NestedThemeSettingModification.TYPE !== priorState.type) {
    throw 'Cannot create a NestedThemeSettingModification from state type ' + priorState.type;
  }

  // Instantiate a new NestedThemeSettingModification instance.
  instance = new ThemeBuilder.NestedThemeSettingModification(priorState.selector);

  // Set the prior and new states.
  instance.setPriorState(priorState.parents, priorState.value);

  if (newState) {
    instance.setNewState(newState.parents, newState.value);
  }

  return instance;
};

/**
 * The constructor for the NestedThemeSettingModification class. You should never call
 * this method directly, but rather use code such as:
 * <pre>
 *   var modification = new NestedThemeSettingModification();
 * </pre>
 */
ThemeBuilder.NestedThemeSettingModification.prototype.initialize = function (key) {
  ThemeBuilder.Modification.prototype.initialize.call(this, key);
  this.type = ThemeBuilder.NestedThemeSettingModification.TYPE;
};

/**
 * Creates a simple object that encapsulates a state.
 *
 * The state is either a prior state or a new state which will be associated
 * with this modification instance.
 *
 * @param {Array} parents
 *   The value of the theme setting modification.
 * @param value
 *   The value of the theme setting modification.
 */
ThemeBuilder.NestedThemeSettingModification.prototype.createState = function (parents, value) {
  return {
    'parents': parents,
    'value': value
  };
};

/**
 * Determines whether the value of the specified modification has changed.
 * This method essentially compares the value in the prior state to the value
 * in the new state to try to detect a change.
 *
 * @return {boolean}
 *   true if the modification represents a change; false otherwise.
 */
ThemeBuilder.NestedThemeSettingModification.prototype.hasChanged = function () {
  var before = this.getPriorState();
  var after = this.getNewState();
  for (var property in before) {
    if (property && before.hasOwnProperty(property)) {
      if (after[property] instanceof Array && before[property] instanceof Array) {
        if (JSON.stringify(after[property]) !== JSON.stringify(before[property])) {
          return true
        }
      }
      else {
        if (after[property] !== before[property]) {
          return true;
        }
      }

    }
  }
  return false;
};
