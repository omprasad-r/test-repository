
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true assertEquals: true*/

function test_element() {
  var s = ThemeBuilder.Specificity.getScore('h1');
  assertEquals(s.compare({a: 0, b: 0, c: 0, d: 1}), 0);
}

function test_class() {
  var s = ThemeBuilder.Specificity.getScore('.active');
  assertEquals(s.compare({a: 0, b: 0, c: 1, d: 0}), 0);
}

function test_id() {
  var s = ThemeBuilder.Specificity.getScore('#header');
  assertEquals(s.compare({a: 0, b: 1, c: 0, d: 0}), 0);
}

function test_id_then_tag() {
  var s = ThemeBuilder.Specificity.getScore('#header table');
  assertEquals(s.compare({a: 0, b: 1, c: 0, d: 1}), 0);
}

function test_id_then_class() {
  var s = ThemeBuilder.Specificity.getScore('#header .active');
  assertEquals(s.compare({a: 0, b: 1, c: 1, d: 0}), 0);
}

function test_id_then_class_then_tag() {
  var s = ThemeBuilder.Specificity.getScore('#header .active table');
  assertEquals(s.compare({a: 0, b: 1, c: 1, d: 1}), 0);
}

function test_id_and_class() {
  var s = ThemeBuilder.Specificity.getScore('#header.active');
  assertEquals(s.compare({a: 0, b: 1, c: 1, d: 0}), 0);
}

function test_tag_and_id_and_class() {
  var s = ThemeBuilder.Specificity.getScore('table#header.active');
  assertEquals(s.compare({a: 0, b: 1, c: 1, d: 1}), 0);
}

function test_tag_and_id_and_class_then_class_and_id() {
  var s = ThemeBuilder.Specificity.getScore('table#header.active .tb-inactive#themebuilder');
  assertEquals(s.compare({a: 0, b: 2, c: 2, d: 1}), 0);
}

function test_id_and_pseudoclass() {
  var s = ThemeBuilder.Specificity.getScore('#header:hover');
  assertEquals(s.compare({a: 0, b: 1, c: 1, d: 0}), 0);
}

function test_id_and_pseudoelement() {
  var s = ThemeBuilder.Specificity.getScore('#header::first-line');
  assertEquals(s.compare({a: 0, b: 1, c: 0, d: 1}), 0);
}

function test_wildcard() {
  var s = ThemeBuilder.Specificity.getScore('*');
  assertEquals(s.compare({a: 0, b: 0, c: 0, d: 0}), 0);
}

function test_adjacent_sibling_combinator() {
  var s = ThemeBuilder.Specificity.getScore('div ol+li');
  assertEquals(s.compare({a: 0, b: 0, c: 0, d: 3}), 0);
}

function test_general_sibling_combinator() {
  var s = ThemeBuilder.Specificity.getScore('div ol~li');
  assertEquals(s.compare({a: 0, b: 0, c: 0, d: 3}), 0);
}

function test_child_combinator() {
  var s = ThemeBuilder.Specificity.getScore('ol > li');
  assertEquals(s.compare({a: 0, b: 0, c: 0, d: 2}), 0);
}

function test_to_string() {
  var s = new ThemeBuilder.SpecificityScore(0, 1, 1, 2);
  assertEquals(s.toString(), '000,001,001,002');
}
