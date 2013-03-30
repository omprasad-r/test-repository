
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global window: true jQuery: true Drupal: true ThemeBuilder: true debug: true*/

/**
 * Sets the specified handler to handle changes for the specified
 * modification type.
 *
 * @param {String} type
 *   A string identifier that indicates the modification type.
 * @param {Object} handler
 *   An object that will manage changes for the specified modification type.
 */
ThemeBuilder.addModificationHandler = function (type, handler) {
  if (!ThemeBuilder.modificationHandlers) {
    ThemeBuilder.modificationHandlers = {};
  }
  ThemeBuilder.modificationHandlers[type] = handler;
};

/**
 * Removes the specified handler from the list of handlers that manage
 * modifications of the specified type.
 *
 * @param {String} type
 *   A string identifier that indicates the modification type.
 * @param {Object} handler
 *   An object that will manage changes for the specified modification type.
 */
ThemeBuilder.removeModificationHandler = function (type, handler) {
  if (ThemeBuilder.modificationHandlers &&
    ThemeBuilder.modificationHandlers[type] === handler) {
    delete ThemeBuilder.modificationHandlers[type];
  }
};

/**
 * Returns the registered handler that manages changes for the
 * specified modification.
 *
 * @param {String} type
 *   A string identifier that indicates the modification type.
 *
 * @return {Object}
 *   The handler object associated with the specified type, or undefined if
 *   the handler has not been registered.
 */
ThemeBuilder.getModificationHandler = function (type) {
  var handler = undefined;
  if (ThemeBuilder.modificationHandlers) {
    handler = ThemeBuilder.modificationHandlers[type];
  }
  return handler;
};

/**
 * Registers a new modification class.  Registering the class is necessary in
 * order for the system to create new instances of the modification.  The
 * class must have a TYPE field that provides the string that identifies the
 * type of modification being registered.
 *
 * @param {String} classname
 *   The name of the modification class being registered.
 */
ThemeBuilder.registerModificationClass = function (classname) {
  if (!ThemeBuilder._modificationTypes) {
    ThemeBuilder._modificationTypes = {};
  }
  if (!ThemeBuilder[classname].TYPE) {
    throw classname + " is not a recognized Modification subclass.";
  }
  ThemeBuilder._modificationTypes[ThemeBuilder[classname].TYPE] = classname;
};

/**
 * Retrieves the modification class for the specified modification type.  The
 * modification class can be used to instantiate a Modification instance
 * suitable for recording property changes for the specified modification
 * type.
 *
 * @param {String} type
 *   The modification type, which indicates what kind of modification needs to
 *   be stored.
 */
ThemeBuilder.getModificationClassForType = function (type) {
  var classname = undefined;
  if (ThemeBuilder._modificationTypes) {
    classname = ThemeBuilder._modificationTypes[type];
  }
  return classname;
};

/**
 * This class contains a single modification.  The Modification instance captures
 * a delta between a previous value and the next value and can facilitate apply
 * and undo operations by keeping track of both states for every change in a
 * stack.
 * @class
 * @constructor
 */
ThemeBuilder.Modification = ThemeBuilder.initClass();

/**
 * The constructor for the Modification class.  A modification object has the
 * modification type and the selector in common for both the apply and revert
 * states of the change.  The type indicates what kind of change this modification
 * instance holds (css, layout, palette etc.) while the selector indicates the
 * particular entity being modified.
 *
 * @param selector string
 *   The selector of the property being modified.
 */
ThemeBuilder.Modification.prototype.initialize = function (selector) {
  this.selector = selector;
  this.priorState = null;
  this.newState = null;
  this.type = null;
};

/**
 * Creates a new modification from the specified states.
 *
 * @param priorState object
 *   The priorState indicates the state of the property being modified
 *   before the change is applied.
 * @param newState object
 *   The newState indicates the state of the property being modified
 *   after the change is applied.
 * @return
 *   A Modification instance that represents the specified change.
 */
ThemeBuilder.Modification.create = function (priorState, newState) {
  if (!priorState) {
    throw 'The priorState must be specified when calling Modification.create';
  }
  var modification;
  var classname = ThemeBuilder.getModificationClassForType(priorState.type);
  if (classname) {
    modification = ThemeBuilder[classname].create(priorState, newState);
  }
  else {
    throw "Unexpected modification type: " + priorState.type;
  }
  return modification;
};

/**
 * Creates a new modification instance from the specified description.  The
 * description should be an object that contains all of the properties of
 * a Modification instance, but is not an instance of Modification.
 *
 * @param desc object
 *   An object identifying the modification.  This would generally come from
 *   the database.
 * @return
 *   The new Modification instance.
 */
ThemeBuilder.Modification.fromDescription = function (desc) {
  if (!desc) {
    throw 'The description must be specified.';
  }
  var modification;
  if (desc.type === ThemeBuilder.GroupedModification.TYPE) {
    var children = {};
    for (var child in desc.children) {
      if (typeof(desc.children[child]) !== 'function') {
        children[child] = (ThemeBuilder.Modification.fromDescription(desc.children[child]));
      }
    }
    modification = ThemeBuilder.GroupedModification.create(children);
  }
  else {
    var classname = ThemeBuilder.getModificationClassForType(desc.type);
    if (classname) {
      modification = new ThemeBuilder[classname](desc.selector);
    }
    else {
      throw 'Unknown modification type ' + desc.type;
    }
  }
  modification.priorState = desc.priorState;
  modification.newState = desc.newState;
  return modification;
};

/**
 * Returns the selector of the entity being modified.  For CSS, this would be
 * the CSS selector.  For layout, it could be a string that represents when the
 * layout would be applied (page, global, etc.).
 *
 * @return
 *   The selector associated with this modification.
 */
ThemeBuilder.Modification.prototype.getSelector = function () {
  return this.selector;
};

/**
 * Returns the type of modification this instance represents.
 *
 * @return
 *   The type associated with this modification.
 */
ThemeBuilder.Modification.prototype.getType = function () {
  return this.type;
};

/**
 * Configures into the modification instance the state of the property as it
 * existed before the modification was applied.  This is the state to which it
 * will return should the ThemeBuilder.undo() function be called.  Note that
 * the prior state is specific to the particular class of Modification being
 * used.  The arguments will be passed to the createState method.
 */
ThemeBuilder.Modification.prototype.setPriorState = function () {
  this.priorState = this.createState.apply(this, arguments);
};

/**
 * Configures into the modification instance the state of the property as it
 * should be after the modification is applied.  This is the state to which it
 * will be set after the modification is applied, or after a call to
 * ThemeBuilder.redo() if this modification instance is at the top of the undo
 * stack.  Note that the new state parameters are specific to the particular
 * class of Modification being used.  The arguments will be passed to the
 * createState method.
 */
ThemeBuilder.Modification.prototype.setNewState = function () {
  this.newState = this.createState.apply(this, arguments);
};

/**
 * This function will return an object that represents the current state.
 *
 * @return object
 *   An object containing the properties of the state being created.
 */
ThemeBuilder.Modification.prototype.createState = function () {
  throw "The Modification.createState is abstract and must be overridden.";
};

/**
 * Returns a description of what needs to change if this modification instance
 * is applied.
 *
 * @return
 *   An object that contains fields that fully describe this modification when
 *   it is applied.  This object should be used to perform the actual changes.
 */
ThemeBuilder.Modification.prototype.getNewState = function () {
  if (this.newState === null) {
    throw "The Modification instance has not been initialized before apply.";
  }
  var update = ThemeBuilder.clone(this.newState);
  update.type = this.getType();
  update.selector = this.selector;
  return update;
};

/**
 * Returns a description of what needs to change if this modification instance
 * is reverted.
 *
 * @return
 *   An object that contains fields that fully describe this modification when
 *   it is reverted.  This object should be used to perform the actual changes.
 */
ThemeBuilder.Modification.prototype.getPriorState = function () {
  if (this.priorState === null) {
    throw "The Modification instance has not been initialized before revert.";
  }
  var update = ThemeBuilder.clone(this.priorState);
  update.type = this.getType();
  update.selector = this.selector;
  return update;
};

/**
 * Returns a new Modification instance that represents a fresh modification
 * using the new state from the current modification.  This step is required
 * whenever a modification has been committed so the following modification
 * is a different instance and has the correct starting point.
 *
 * @return object
 *   The new Modification instance.
 */
ThemeBuilder.Modification.prototype.getFreshModification = function () {
  return ThemeBuilder.Modification.create(this.getNewState());
};

/**
 * Determines whether the value of the specified modification has changed.
 * This method essentially compares the value in the prior state to the value
 * in the new state to try to detect a change.
 *
 * @return {boolean}
 *   true if the modification represents a change; false otherwise.
 */
ThemeBuilder.Modification.prototype.hasChanged = function () {
  var before = this.getPriorState();
  var after = this.getNewState();
  for (var property in before) {
    if (property && before.hasOwnProperty(property)) {
      if (after[property] !== before[property]) {
        return true;
      }
    }
  }
  return false;
};

/**
 * Indicates the number of Modification instances are represented.
 *
 * @return {int}
 *   The total number of modifications.
 */
ThemeBuilder.Modification.prototype.getCount = function () {
  return 1;
};

/**
 * The CssModification is a subclass of the abstract Modification class.  An
 * instance of this class can hold a modification to a CSS property such that
 * it can be applied and reverted.
 *
 * @class
 * @extends ThemeBuilder.Modification
 * @constructor
 * @param selector string
 *   The selector for the CSS modification.
 */
ThemeBuilder.CssModification = ThemeBuilder.initClass();

// Subclass the Modification class.
ThemeBuilder.CssModification.prototype = new ThemeBuilder.Modification();

/**
 * The type string that indicates this is a css modification.
 */
ThemeBuilder.CssModification.TYPE = 'css';

ThemeBuilder.registerModificationClass('CssModification');

/**
 * This static method returns a correctly initialized CssModification instance
 * that contains the specified prior state and new state.  Enough checking is
 * performed to ensure that the newly instantiated object is valid.
 *
 * @return
 *   A new instance of CssModification that contains the specified prior
 *   state and new state.
 */
ThemeBuilder.CssModification.create = function (priorState, newState) {
  if (ThemeBuilder.CssModification.TYPE !== priorState.type) {
    throw 'Cannot create a CssModification from state type ' + priorState.type;
  }

  var instance = new ThemeBuilder.CssModification(priorState.selector, priorState.undofunction);
  instance.setPriorState(priorState.property, priorState.value);
  if (newState) {
    instance.setNewState(newState.property, newState.value);
  }
  return instance;
};

/**
 * The constructor for the CssModification class.  This initializes the type
 * and selector for the modification.  You should never call this method
 * directly, but rather use code such as:
 * <pre>
 *   var modification = new CssModification('h1');
 * </pre>
 *
 * @param selector string
 *   The selector that describes the element(s) that the property and values
 *   associated with this Modification instance would apply to.
 */
ThemeBuilder.CssModification.prototype.initialize = function (selector, undofunction) {
  ThemeBuilder.Modification.prototype.initialize.call(this, selector);
  this.type = ThemeBuilder.CssModification.TYPE;
  this.undofunction = undofunction;
};

/**
 * Creates a simple object that encapsulates a state (either a prior state or
 * a new state) which will be associated with this modification instance.
 *
 * @param property string
 *   The property name.
 * @param value string
 *   The value associated with the property.
 * @param resources string
 *   Any resources needed for this property (such as a font or image).
 */
ThemeBuilder.CssModification.prototype.createState = function (property, value, resources) {
  return {
    property : property,
    value : value,
    resources: resources
  };
};

/**
 * The LayoutModification is a subclass of the abstract Modification class.  An
 * instance of this class can hold a modification to the layout such that
 * it can be applied and reverted.
 *
 * @class
 * @extends ThemeBuilder.Modification
 * @param selector string
 *   The selector for the layout modification.  Use 'global' if the layout
 *   should apply to the entire site.
 */
ThemeBuilder.layoutEditorModification = ThemeBuilder.initClass();

// Subclass the Modification class.
ThemeBuilder.layoutEditorModification.prototype = new ThemeBuilder.Modification();

/**
 * The type string that indicates this is a layout modification.
 */
ThemeBuilder.layoutEditorModification.TYPE = 'layout';
ThemeBuilder.registerModificationClass('layoutEditorModification');
/**
 * This static method returns a correctly initialized LayoutModification instance
 * that contains the specified prior state and new state.  Enough checking is
 * performed to ensure that the newly instantiated object is valid.
 *
 * @return
 *   A new instance of LayoutModification that contains the specified prior
 *   state and new state.
 */
ThemeBuilder.layoutEditorModification.create = function (priorState, newState) {
  if (ThemeBuilder.layoutEditorModification.TYPE !== priorState.type) {
    throw 'Cannot create a LayoutModification from state type ' + priorState.type;
  }

  var instance = new ThemeBuilder.layoutEditorModification(priorState.selector);
  instance.setPriorState(priorState.layout);
  if (newState) {
    instance.setNewState(newState.layout);
  }
  return instance;
};

/**
 * The constructor for the LayoutModification class.  This initializes the type
 * of the modification.  You should never call this method directly, but rather
 * use code such as:
 * <pre>
 *   var modification = new LayoutModification(selector);
 * </pre>
 *
 * @param selector
 *   Where to apply this layout change.  For the entire site, this should be
 *   '<global>'.
 */
ThemeBuilder.layoutEditorModification.prototype.initialize = function (selector) {
  ThemeBuilder.Modification.prototype.initialize.call(this, selector);
  this.type = ThemeBuilder.layoutEditorModification.TYPE;
};

/**
 * Creates a simple object that encapsulates a state (either a prior state or
 * a new state) which will be associated with this modification instance.
 *
 * @param layoutName string
 *   The the name of the layout.
 * @param {String} urlPattern A regex to match for selecting this layout.
 */
ThemeBuilder.layoutEditorModification.prototype.createState = function (layoutName) {
  return {
    layout: layoutName
  };
};

/**
 * The GroupedModification is a subclass of the abstract Modification class.  An
 * instance of this class can hold several modifications of any type that can
 * be applied, undone, or redone all at once.
 * @class
 * @extends ThemeBuilder.Modification
 */
ThemeBuilder.GroupedModification = ThemeBuilder.initClass();

// Subclass the Modification class.
ThemeBuilder.GroupedModification.prototype = new ThemeBuilder.Modification();

/**
 * The type string that indicates this is a group modification.
 */
ThemeBuilder.GroupedModification.TYPE = 'grouped';
ThemeBuilder.registerModificationClass('GroupedModification');

ThemeBuilder.GroupedModification.create = function (children) {
  var result = new ThemeBuilder.GroupedModification();
  for (var key in children) {
    if (typeof(children[key]) !== 'function') {
      result.addChild(key, children[key]);
    }
  }
  return result;
};

/**
 * The constructor for the GroupedModification class.  This initializes the type
 * of the modification.  You should never call this method directly, but rather
 * use code such as:
 * <pre>
 *   var modification = new GroupedModification();
 * </pre>
 */
ThemeBuilder.GroupedModification.prototype.initialize = function () {
  ThemeBuilder.Modification.prototype.initialize.call(this, 'group');
  this.type = ThemeBuilder.GroupedModification.TYPE;
  this.children = {};
};

ThemeBuilder.GroupedModification.prototype.getNewState = function () {
  var result = [];
  for (var attribute in this.children) {
    result.push(this.children[attribute].getNewState());
  }
  return result;
};

ThemeBuilder.GroupedModification.prototype.getPriorState = function () {
  var result = [];
  for (var attribute in this.children) {
    result.push(this.children[attribute].getPriorState());
  }
  return result;
};


/**
 * // TODO: Not sure what to do with this yet...
 * Creates a simple object that encapsulates a state (either a prior state or
 * a new state) which will be associated with this modification instance.
 *
 * @param layoutName string
 *   The the name of the layout.
 */
ThemeBuilder.GroupedModification.prototype.createState = function (layoutName) {
  return {
    layout: layoutName
  };
};

ThemeBuilder.GroupedModification.prototype.addChild = function (name, modification) {
  this.children[name] = modification;
};

ThemeBuilder.GroupedModification.prototype.getChild = function (name) {
  return this.children[name];
};

/**
 * Returns all children from this GroupedModification instance.
 *
 * @return {Associative array}
 *   The children.
 */
ThemeBuilder.GroupedModification.prototype.getChildren = function () {
  return this.children;
};

/**
 * Indicates the number of Modification instances are represented.
 *
 * @return {int}
 *   The total number of modifications.
 */
ThemeBuilder.GroupedModification.prototype.getCount = function () {
  var count = 0;
  for (var name in this.children) {
    if (this.children.hasOwnProperty(name) &&
       this.children[name].getCount) {
      count += this.children[name].getCount();
    }
  }
  return count;
};

/**
 * Make a child modification last in the group.
 *
 * This will fail in Chrome if the child modification's name is an integer.
 * The order of properties in an ECMAScript object is implementation-dependent.
 * Most browsers respect their order, but Chrome treats them more like PHP
 * arrays; it respects the order of string keys but reorders integer keys.
 * See http://ejohn.org/blog/javascript-in-chrome/ and
 * http://code.google.com/p/chromium/issues/detail?id=20144.
 *
 * The Chrome "bug" is slated to be fixed. In the meantime, callers need to
 * make sure that their modification names are not integers. '1' is a bad idea.
 *
 * @param name string
 *   The name of the modification to be moved.
 */
ThemeBuilder.GroupedModification.prototype.bumpChild = function (name) {
  if (this.children[name]) {
    var lastChild = ThemeBuilder.clone(this.children[name]);
    var newChildren = {};
    var i;
    for (i in this.children) {
      if (i !== name && this.children[i].priorState) {
        newChildren[i] = this.children[i];
      }
    }
    this.children = newChildren;
    this.addChild(name, lastChild);
    return true;
  }
  return false;
};



/**
 * The codeModification is a subclass of the abstract Modification class.  An
 * instance of this class can hold a modification to the code such that
 * it can be applied and reverted.
 *
 * @class
 * @extends ThemeBuilder.Modification
 * @param selector string
 *   The selector for the code modification.  Use 'global' if the code
 *   should apply to the entire site.
 */
ThemeBuilder.codeEditorModification = ThemeBuilder.initClass();

// Subclass the Modification class.
ThemeBuilder.codeEditorModification.prototype = new ThemeBuilder.Modification();

/**
 * The type string that indicates this is a code modification.
 */
ThemeBuilder.codeEditorModification.TYPE = 'code';
ThemeBuilder.registerModificationClass('codeEditorModification');

/**
 * This static method returns a correctly initialized codeModification instance
 * that contains the specified prior state and new state.  Enough checking is
 * performed to ensure that the newly instantiated object is valid.
 *
 * @return
 *   A new instance of codeModification that contains the specified prior
 *   state and new state.
 */
ThemeBuilder.codeEditorModification.create = function (priorState, newState) {
  if (ThemeBuilder.codeEditorModification.TYPE !== priorState.type) {
    throw 'Cannot create a codeModification from state type ' + priorState.type;
  }

  var instance = new ThemeBuilder.codeEditorModification(priorState.selector);
  instance.setPriorState(priorState.code);
  if (newState) {
    instance.setNewState(newState.code);
  }
  return instance;
};

/**
 * The constructor for the codeModification class.  This initializes the type
 * of the modification.  You should never call this method directly, but rather
 * use code such as:
 * <pre>
 *   var modification = new codeModification(selector);
 * </pre>
 *
 * @param selector
 *   Where to apply this code change.  For the entire site, this should be
 *   'global'.
 */
ThemeBuilder.codeEditorModification.prototype.initialize = function (selector) {
  ThemeBuilder.Modification.prototype.initialize.call(this, selector);
  this.type = ThemeBuilder.codeEditorModification.TYPE;
};

/**
 * Creates a simple object that encapsulates a state (either a prior state or
 * a new state) which will be associated with this modification instance.
 *
 * @param codeBuffer string
 *   The buffer of code.
 */
ThemeBuilder.codeEditorModification.prototype.createState = function (codeBuffer) {
  return {
    code: codeBuffer
  };
};

/**
 * The undo stack.  When a modification is performed, it should be placed on
 * the undo stack.  If the user would like to revert the change, the item can
 * be popped from the stack and reverted.  At this point it should be placed
 * on the redo stack.
 */
ThemeBuilder.undoStack = new ThemeBuilder.Stack();

/**
 * The redo stack.  When a modification is reverted, the modification should
 * be placed on the redo stack.  Subsequently, if another modification is
 * performed (and placed on the undo stack), the redo stack should be cleared.
 */
ThemeBuilder.redoStack = new ThemeBuilder.Stack();

/**
 * Meant for debugging purposes, logs the stack
 */
ThemeBuilder.showStack = function (stack) {
  for (var i = 0; i < stack.size(); i++) {
    debug.log(stack._data[i]);
  }
};


// Constant values indicating the type of modification being committed to the server.
ThemeBuilder.APPLY_MODIFICATION = "apply";
ThemeBuilder.UNDO_MODIFICATION = "undo";
ThemeBuilder.REDO_MODIFICATION = "redo";

/**
 * Sends the specified modification to the server and commits it.  The operation
 * must be specified, and can be one of 'apply', 'undo', or 'redo'.
 *
 * @param modification object
 *   The object that encapsulates the modification to be applied.
 * @param operation string
 *   Indicates how the modification should be applied - 'apply', 'undo', or
 *   'redo'.
 */
ThemeBuilder.commitModification = function (modification, operation) {
  var $ = jQuery;

  // If this is a large modification, show the spinner.
  if (modification.getCount() > 8) {
    ThemeBuilder.Bar.getInstance().showWaitIndicator();
  }

  // Do not allow undo/redo operations while commiting changes.
  var key = ThemeBuilder.undoButtons.disable();
  ThemeBuilder.postBack(Drupal.settings.themebuilderCommitPath,
  {
    'operation': operation,
    'modification': JSON.stringify(modification),
    'statusKey' : key
  }, ThemeBuilder.bindIgnoreCallerArgs(ThemeBuilder, ThemeBuilder._commitSuccess, modification, operation), ThemeBuilder.bindIgnoreCallerArgs(ThemeBuilder, ThemeBuilder._commitFail, modification, operation));

  // Trigger the ModificationCommitted event to cause the UI to update
  // while the asynchronous server call is processed.
  $(window).trigger('ModificationCommitted', [modification, operation]);
  ThemeBuilder.Bar.getInstance().setChanged(true);
};

/**
 * Called when a modification is successfully committed.
 *
 * @param {Modification} modification
 *   The modification that was committed.
 * @param {String} operation
 *   "apply" if the modification was applied for the first time,
 *   "undo" if the modification was undone, "redo" if the modification
 *   was redone.
 */
ThemeBuilder._commitSuccess = function (modification, operation) {
  if (modification.getCount() > 8) {
    ThemeBuilder.Bar.getInstance().hideWaitIndicator();
  }
  jQuery(window).trigger('ModificationCompleted', [modification, operation]);
};

/**
 * Called when a modification fails to commit.
 *
 * @param {Modification} modification
 *   The modification that was committed.
 * @param {String} operation
 *   "apply" if the modification was applied for the first time,
 *   "undo" if the modification was undone, "redo" if the modification
 *   was redone.
 */
ThemeBuilder._commitFail = function (modification, operation) {
  // The commit failed.  In order to get in perfect sync with the
  // server, reload the page.
  ThemeBuilder.Bar.getInstance().reloadPage();
};

/**
 * Retrieves the undo and redo stacks from the server and populates the client
 * side stacks.  This is needed only if a page refresh is done.
 */
ThemeBuilder.populateUndoStack = function () {
  ThemeBuilder.getApplicationInstance().addApplicationInitializer(ThemeBuilder._populateUndoStackCallback);
};

/**
 * This is the function that is called when the undo stack data is retrieved
 * from the server.  This function takes the data and populates the undo and
 * redo stacks.
 *
 * @param {object} data
 *   The data returned from the server.
 */
ThemeBuilder._populateUndoStackCallback = function (data) {
  var undoArray = [];
  var redoArray = [];
  var i;
  var modification;
  if (data.undo) {
    undoArray = data.undo;
  }
  if (data.redo) {
    redoArray = data.redo;
  }
  for (i = 0; i < undoArray.length; i++) {
    modification = ThemeBuilder.Modification.fromDescription(undoArray[i]);
    ThemeBuilder.undoStack.push(modification);
  }
  for (i = redoArray.length - 1; i >= 0; i--) {
    modification = ThemeBuilder.Modification.fromDescription(redoArray[i]);
    ThemeBuilder.redoStack.push(modification);
  }
};

/**
 * Applies the specified modification.  The modification will be applied to the
 * browser (usually a CSS change), and the change will be pushed to the server
 * so a subsequent request will result in the theme being rendered with the
 * new modification applied.
 *
 * @param modification object
 *   The modification to apply.
 */
ThemeBuilder.applyModification = function (modification) {
  ThemeBuilder.preview(modification);
  ThemeBuilder.undoStack.push(ThemeBuilder.clone(modification));
  ThemeBuilder.redoStack.clear();
  ThemeBuilder.commitModification(modification, ThemeBuilder.APPLY_MODIFICATION);
};

/**
 * Causes the last committed modification to be reverted.  The modification
 * will be pushed onto the redo stack as a result.
 */
ThemeBuilder.undo = function () {
  var modification = ThemeBuilder.undoStack.pop();
  if (!modification) {
    return;
  }

  ThemeBuilder.preview(modification, false);
  var handler;
  var state = modification.getPriorState();
  if (ThemeBuilder.isArray(state)) {
    for (var i = 0; i < state.length; i++) {
      handler = ThemeBuilder.getModificationHandler(state[i].type);
      if (handler && handler.processModification) {
        handler.processModification(modification, state[i]);
      }
    }
  }
  else {
    handler = ThemeBuilder.getModificationHandler(modification.type);
    if (handler && handler.processModification) {
      handler.processModification(modification, state);
    }
  }

  ThemeBuilder.redoStack.push(modification);
  ThemeBuilder.commitModification(modification, ThemeBuilder.UNDO_MODIFICATION);
};

/**
 * Causes the last modification that was reverted to be applied again.  The
 * modification will be pushed onto the undo stack as a result.
 */
ThemeBuilder.redo = function () {
  var modification = ThemeBuilder.redoStack.pop();
  if (!modification) {
    return;
  }

  ThemeBuilder.preview(modification);
  var handler;
  var state = modification.getNewState();
  if (ThemeBuilder.isArray(state)) {
    for (var i = 0; i < state.length; i++) {
      handler = ThemeBuilder.getModificationHandler(state[i].type);
      if (handler && handler.processModification) {
        handler.processModification(modification, state[i]);
      }
    }
  }
  else {
    handler = ThemeBuilder.getModificationHandler(modification.type);
    if (handler && handler.processModification) {
      handler.processModification(modification, state);
    }
  }

  ThemeBuilder.undoStack.push(modification);
  ThemeBuilder.commitModification(modification, ThemeBuilder.REDO_MODIFICATION);
};

/**
 * Clears both the undo and redo modification stacks on both the client and
 * server.
 *
 * @param {String} theme
 *   The name of the theme for which the stacks should be cleared.
 * @param {function} success
 *   The success callback.
 * @param {function} fail
 *   The fail callback.
 */
ThemeBuilder.clearModificationStacks = function (theme, success, fail) {
  ThemeBuilder.postBack(Drupal.settings.themebuilderClearModificationStacks, {theme: theme}, ThemeBuilder.bind(this, ThemeBuilder._clearModificationStacksSuccess, success), fail);
};

/**
 * This callback is invoked when the undo and redo stacks have been successfully cleared.
 *
 * @param {object} data
 *   The object passed back from the server containing any interesting
 *   information about the processing of the request.
 * @param {String} type
 *   A string that reveals the type of response.  Usually 'success'.
 * @param {Function} callback
 *   The callback that has been registered to let the caller know when
 *   the request was successfully completed.
 */
ThemeBuilder._clearModificationStacksSuccess = function (data, type, callback) {
  ThemeBuilder.undoStack.clear();
  ThemeBuilder.redoStack.clear();
  if (callback) {
    callback();
  }
};

/**
 * Previews a state within a modification.  The state is the result of calling
 * either modification.getPriorState (to preview after reverting the modification)
 * or modification.getNextState (to preview after the modification is applied).
 *
 * @param state object
 *   The state of a modification to preview.
 */
ThemeBuilder._preview = function (state, modification) {
  if (ThemeBuilder.isArray(state)) {
    for (var i = 0; i < state.length; i++) {
      ThemeBuilder._preview(state[i], modification);
    }
  }
  else {
    var handler = ThemeBuilder.getModificationHandler(state.type);
    if (handler) {
      handler.preview(state, modification);
    }
    else {
      throw Drupal.t('Unknown modification type ') + state.type;
    }
  }
};

/**
 * Previews a modification.  A preview consists of showing the modification in
 * the client without committing the change.  If after previewing a modification
 * the user refreshes the display, the change will no longer be apparent.
 *
 * @param {object} modification
 *   The modification to preview.
 * @param {boolean} apply
 *   Optional - indicates whether the modification should be applied (true)
 *   or reverted (false).  By default, apply is true.
 */
ThemeBuilder.preview = function (modification, apply) {
  if (apply === undefined) {
    apply = true;
  }
  var state = apply === true ? modification.getNewState() : modification.getPriorState();
  ThemeBuilder._preview(state, modification);
};
