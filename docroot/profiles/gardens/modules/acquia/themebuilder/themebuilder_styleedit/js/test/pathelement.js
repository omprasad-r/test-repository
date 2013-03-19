
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true assertEquals: true*/

/**
 * This set of tests is designed to test the API of the PathElement class.
 * @ignore
 */
function setUp() {
  // Create an interesting DOM hierarchy to try.
  var $ = jQuery;
  var topElement = $('<div id="styleedit_test" class="one two three"><div><div><table><tr><td><h3><img id="styleedit_test_find_me" class="four five six">');
  $('body').append(topElement);
}

/**
 * @ignore
 */
function tearDown() {
  var $ = jQuery;
  $('#styleedit_test').remove();
}

/**
 * Returns the DOM element that each of the tests will operate on.
 *
 * @ignore
 * @return
 *   The test element.
 */
function getTestElement(id) {
  if (!id) {
    id = 'styleedit_test_find_me';
  }
  var element = document.getElementById(id);
  return new ThemeBuilder.styles.PathElement(element);
}

/**
 * Test the tag functionality of the PathElement class.
 * @ignore
 */
function testGetTag() {
  var pathElement = getTestElement();
  assertEquals('img', pathElement.getTag());

  // Verify that disabling the element tag works correctly.  Start by
  // verifying the full css selector and the enabled css selector.
  assertEquals('img#styleedit_test_find_me.four.five.six', pathElement.getFullCssSelector());
  assertEquals('img#styleedit_test_find_me.four.five.six', pathElement.getEnabledCssSelector());

  // Now exclude the tag and test again.
  pathElement.setTagEnabled(false);
  assertEquals('img#styleedit_test_find_me.four.five.six', pathElement.getFullCssSelector());
  assertEquals('#styleedit_test_find_me.four.five.six', pathElement.getEnabledCssSelector());
}

/**
 * Test the id functionality of the PathElement class.
 * @ignore
 */
function testGetId() {
  var pathElement = getTestElement();
  assertEquals('styleedit_test_find_me', pathElement.getId());

  // Verify that disabling the element id works correctly.  Start by
  // verifying the full css selector and the enabled css selector.
  assertEquals('img#styleedit_test_find_me.four.five.six', pathElement.getFullCssSelector());
  assertEquals('img#styleedit_test_find_me.four.five.six', pathElement.getEnabledCssSelector());

  // Now exclude the id and test again.
  pathElement.setIdEnabled(false);
  assertEquals('img#styleedit_test_find_me.four.five.six', pathElement.getFullCssSelector());
  assertEquals('img.four.five.six', pathElement.getEnabledCssSelector());
}

/**
 * Test the classes functionality of the PathElement class.
 * @ignore
 */
function testGetClasses() {
  var pathElement = getTestElement();
  var classes = pathElement.getClasses();
  assertEquals(3, classes.length);

  var enabledClasses = pathElement.getEnabledClasses();
  assertEquals(3, classes.length);
  
  // Now disable one of the classes.
  pathElement.setClassEnabled('five', false);
  assertEquals('img#styleedit_test_find_me.four.six', pathElement.getEnabledCssSelector());

  // Reenable the class.
  pathElement.setClassEnabled('five', true);
  assertEquals('img#styleedit_test_find_me.four.five.six', pathElement.getEnabledCssSelector());

  // Disable using an array.
  pathElement.setClassEnabled(['five', 'four'], false);
  assertEquals('img#styleedit_test_find_me.six', pathElement.getEnabledCssSelector());
  pathElement.setClassEnabled(['six'], false);
  assertEquals('img#styleedit_test_find_me', pathElement.getEnabledCssSelector());

  // Reenable all of the classes using an array.
  pathElement.setClassEnabled(['six', 'five', 'four'], true);
  assertEquals('img#styleedit_test_find_me.four.five.six', pathElement.getEnabledCssSelector());
}

/**
 * Tests whether or not it is possible to enable/disable an instance of
 * the PathElement class.
 * @ignore
 */
function testSetEnabled() {
  var pathElement = getTestElement();
  assertEquals('img#styleedit_test_find_me.four.five.six', pathElement.getEnabledCssSelector());

  // Disable and test.
  pathElement.setEnabled(false);
  assertEquals('', pathElement.getEnabledCssSelector());

  // Reenable and test.
  pathElement.setEnabled(true);
  assertEquals('img#styleedit_test_find_me.four.five.six', pathElement.getEnabledCssSelector());
}

/**
 * @ignore
 */
function testBlacklistedClasses() {
  var $ = jQuery;
  var topElement = $('<div id="styleedit_test_red" class="style-clickable red"><div id="styleedit_test_blue" class="overlay-processed blue">');
  $('#styleedit_test').append(topElement);
  var pathElement = getTestElement('styleedit_test_red');
  assertEquals('div#styleedit_test_red.red', pathElement.getEnabledCssSelector());
  pathElement = getTestElement('styleedit_test_blue');
  assertEquals('div#styleedit_test_blue.blue', pathElement.getEnabledCssSelector());
}

/**
 * @ignore
 */
function testGreyListedClasses() {
  var $ = jQuery;
  var topElement = $('<div id="styleedit_test_red" class="first last red"><div id="styleedit_test_blue" class="leaf area blue">');
  $('#styleedit_test').append(topElement);
  var pathElement = getTestElement('styleedit_test_red');
  assertEquals('div#styleedit_test_red.red.first.last', pathElement.getEnabledCssSelector());
  pathElement = getTestElement('styleedit_test_blue');
  assertEquals('div#styleedit_test_blue.blue.leaf.area', pathElement.getEnabledCssSelector());
}

/**
 * @ignore
 */
function testGetAllClasses() {
  var $ = jQuery;
  var topElement = $('<div id="styleedit_test_red" class="style-clickable first overlay-processed last area leaf red">');
  $('#styleedit_test').append(topElement);
  var pathElement = getTestElement('styleedit_test_red');
  assertEquals('red, first, last, area, leaf', pathElement.getAllClasses().join(', '));
}

/**
 * @ignore
 */
function testDisableAllClasses() {
  var $ = jQuery;
  var topElement = $('<div id="styleedit_test_red" class="style-clickable first overlay-processed last area leaf red">');
  $('#styleedit_test').append(topElement);
  var pathElement = getTestElement('styleedit_test_red');
  pathElement.disableAllClasses();
  assertEquals('div#styleedit_test_red', pathElement.getEnabledCssSelector());
}

/**
 * @ignore
 */
function testDisableAllButOneClass() {
  var $ = jQuery;
  var topElement = $('<div id="styleedit_test_red" class="style-clickable first overlay-processed last area leaf red">');
  $('#styleedit_test').append(topElement);
  var pathElement = getTestElement('styleedit_test_red');
  pathElement.disableAllButOneClass('first');
  assertEquals('div#styleedit_test_red.first', pathElement.getEnabledCssSelector());
}

/**
 * @ignore
 */
function testGetFullCssSelector() {
  var $ = jQuery;
  var topElement = $('<div id="styleedit_test_red" class="style-clickable first overlay-processed last area leaf red">');
  $('#styleedit_test').append(topElement);
  var pathElement = getTestElement('styleedit_test_red');
  pathElement.disableAllButOneClass('first');
  assertEquals('div#styleedit_test_red.red.first.last.area.leaf', pathElement.getFullCssSelector());
}

/**
 * @ignore
 */
function testGetCssSelector() {
  var $ = jQuery;
  var topElement = $('<div id="styleedit_test_red" class="style-clickable first overlay-processed last area leaf red">');
  $('#styleedit_test').append(topElement);
  var pathElement = getTestElement('styleedit_test_red');
  pathElement.disableAllButOneClass('first');
  pathElement.setIdEnabled(false);
  pathElement.setTagEnabled(false);
  assertEquals('.first', pathElement.getCssSelector());
  pathElement.setEnabled(false);
  assertEquals('.first', pathElement.getCssSelector());
  assertEquals('', pathElement.getEnabledCssSelector());
}

/**
 * @ignore
 */
function testGetTagName() {
  var pathElement = getTestElement();
  assertEquals('images', pathElement.getTagName());
}

/**
 * @ignore
 */
function testGetSpecificityOptions() {
  var pathElement = getTestElement();
  var map = pathElement.getSpecificityOptions();
  assertEquals(5, map.identification.length);
  assertEquals('id', map.identification[0].use);
  assertEquals('class', map.identification[1].use);
  assertEquals('tag', map.identification[map.identification.length - 1].use);
}

/**
 * @ignore
 */
function testSetSpecificityOptions() {
  var pathElement = getTestElement();
  var map = pathElement.getSpecificityOptions();
  pathElement.setSpecificity('identification', 0);
  assertEquals('#styleedit_test_find_me', pathElement.getEnabledCssSelector());
  pathElement.setSpecificity('identification', 1);
  assertEquals('.four', pathElement.getEnabledCssSelector());
  pathElement.setSpecificity('identification', map.identification.length - 1);
  assertEquals('img', pathElement.getEnabledCssSelector());
}
