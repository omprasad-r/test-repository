
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true assertEquals: true*/

/**
 * This set of tests is designed to test the API of the PathElement class.
 */

function addBodyClasses() {
  var $ = jQuery;
  $('body').addClass('page front logged-in page-node no-sidebars toolbar toolbar-shortcuts body-layout-fixed-acb');
}

function removeBodyClasses() {
  var $ = jQuery;
  $('body').removeClass('page front logged-in page-node no-sidebars toolbar toolbar-shortcuts body-layout-fixed-acb');
}

/**
 * @ignore
 */
function setUp() {
  // Create an interesting DOM hierarchy to try.
  var $ = jQuery;
  var topElement = $('<div id="styleedit_test" class="one two three"><div><div><table><tr><td><h3><img id="styleedit_test_find_me" class="four five six">');
  $('body').append(topElement);
  var otherTopElement = $('<div id="page-wrapper"><div id="page"><div id=main-wrapper"><div id="main" class="clearfix"><div id="body" class="clearfix"><div id="content-wrapper"><div id="sidebar-a" class="column sidebar"><div class="section region"><div id="block-user-online" class="block block-user"><h2>Who\'s online</h2>');
  $('body').append(otherTopElement);
}

/**
 * @ignore
 */
function tearDown() {
  var $ = jQuery;
  $('#styleedit_test').remove();
  $('#page-wrapper').remove();
  removeBodyClasses();
}

/**
 * Returns the DOM element that each of the tests will operate on.
 *
 * @ignore
 * @return
 *   The test element.
 */
function getTestElement() {
  return document.getElementById('styleedit_test_find_me');
}

/**
 * @ignore
 */
function getH2Element() {
  var element = document.getElementById('block-user-online');
  return element.firstChild;
}

/**
 * Test the tag functionality of the PathElement class.
 * @ignore
 */
function testNakedDivFilter() {
  var selector = new ThemeBuilder.styles.Selector(new ThemeBuilder.styles.Filter());
  selector.setElement(getTestElement());
  assertEquals('html.js body div#styleedit_test.one.two.three table tbody tr td h3 img#styleedit_test_find_me.four.five.six', selector.getCssSelector());
}

/**
 * @ignore
 */
function testDrupalRecommendedBlockPath() {
  addBodyClasses();
  var element = getH2Element();
  var selector = new ThemeBuilder.styles.Selector(new ThemeBuilder.styles.ThemeMarkup1Filter());
  selector.setElement(element);
  assertEquals('.block h2',
    selector.getCssSelector());
}
