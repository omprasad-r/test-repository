
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */
/*global assertEquals: true assertTrue: true*/


var atLeastOneTestHasRun = false;
var aVariable = null;

function setUp() {
  aVariable = "foo";
}

function tearDown() {
  atLeastOneTestHasRun = true;
  aVariable = null;
}

function testEmpty1() {
}

function testSetUp() {
  assertEquals("foo", aVariable);
}

function testEmpty2() {
}

function testTearDown() {
  assertTrue(atLeastOneTestHasRun);
}

function testEmpty3() {
}
