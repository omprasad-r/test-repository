
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true assertEquals: true*/

function setUp() {
}

/**
 * @ignore
 */
function tearDown() {
}

/**
 * Verifies the ThemeBuilder.util.trim function.
 */
function test_trim() {
  var testString = 'This is my test string.';
  assertEquals(testString, ThemeBuilder.util.trim(testString));
  assertEquals(testString, ThemeBuilder.util.trim(' ' + testString));
  assertEquals(testString, ThemeBuilder.util.trim(testString + ' '));
  assertEquals(testString, ThemeBuilder.util.trim(' ' + testString + ' '));
  assertEquals(testString, ThemeBuilder.util.trim('   \n     ' + testString + '     \n'));
  assertEquals('', ThemeBuilder.util.trim('              '));
}

/**
 * Verifies the ThemeBuilder.util.isHtmlMarkup function.
 */
function test_isHtmlMarkup() {
  var markup = '<div class="bogus">Hello</div>';
  var object = '{"key":"value","key2":"value2"}';
  assertEquals(true, ThemeBuilder.util.isHtmlMarkup(markup));
  assertEquals(true, ThemeBuilder.util.isHtmlMarkup('\n  ' + markup + '  \n'));
  assertEquals(false, ThemeBuilder.util.isHtmlMarkup(object));
  assertEquals(false, ThemeBuilder.util.isHtmlMarkup('\n  ' + object + '  \n'));
}
