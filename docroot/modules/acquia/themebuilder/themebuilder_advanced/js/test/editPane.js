
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true assertEquals: true*/

function setUp() {
  // Create a textarea to work on.
  var $ = jQuery;
  var textarea = $('<textarea id="editor"/>');
  $('body').append(textarea);
}

/**
 * @ignore
 */
function tearDown() {
  var $ = jQuery;
  $('#editor').remove();
}

/**
 * Test whether the insertAtCursor method works on an empty textarea.
 */
function test_insert_when_empty() {
  var $ = jQuery;
  var textarea = $('#editor');
  var codeEditor = ThemeBuilder.CodeEditor.getInstance();
  var editPane = new ThemeBuilder.EditPane(textarea, codeEditor);
  var sampleText = "hello world";
  editPane.insertAtCursor(sampleText);
  assertEquals(textarea.val(), sampleText);
}
