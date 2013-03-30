
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

function get_sample_css_unix() {
  return "body {\n  color: red;\n}\n\np {\n  font-weight: bold;\n}\n";
}

function get_sample_css_windows() {
  var css = get_sample_css_unix();
  var re = new RegExp("\\n");
  return css.replace(re, '\r\n');
}

function test_line_endings() {
  var codeEditor = ThemeBuilder.CodeEditor.getInstance();
  var unix_css = get_sample_css_unix();
  var windows_css = get_sample_css_windows();

  var unix_css_clean = codeEditor._standardizeLineEndings(unix_css);
  assertEquals(unix_css, unix_css_clean);

  var windows_css_clean = codeEditor._standardizeLineEndings(windows_css);
  assertEquals(unix_css, windows_css_clean);
}
