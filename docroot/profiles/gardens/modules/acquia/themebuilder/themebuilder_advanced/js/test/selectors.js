
/*jslint bitwise: true, eqeqeq: true, immed: true, newcap: true, nomen: false,
 onevar: false, plusplus: false, regexp: true, undef: true, white: true, indent: 2
 browser: true */

/*global jQuery: true Drupal: true ThemeBuilder: true assertEquals: true assertArrayEquals: true*/

function getData() {
  var data = {
    "#id": {
      "text-decoration": "underline",
      "font-size": "66px",
      "color": "{e}"
    },
    "#id-2": {
      "text-decoration": "underline",
      "font-size": "66px",
      "color": "{e}"
    },
    ".active a": {
      "font-size": "30px",
      "font-weight": "bold",
      "color": "{black-67-white}"
    },
    "a": {
      "background-color": "{e-33-white}"
    }
  };
  return data;
}

function test_simple_selector_sort_asc() {
  var m = new ThemeBuilder.CustomStyles(getData());
  var i = m.getIterator();
  var v;
  var sorted = [];
  while (i.hasNext()) {
    v = i.next();
    sorted.push(v.selector);
  }
  
  assertArrayEquals(['a', '.active a', '#id', '#id-2'], sorted);
}

function test_simple_selector_sort_desc() {
  var m = new ThemeBuilder.CustomStyles(getData());
  var i = m.getIterator(false);
  var v;
  var sorted = [];
  while (i.hasNext()) {
    v = i.next();
    sorted.push(v.selector);
  }
  
  assertArrayEquals(['#id-2', '#id', '.active a', 'a'], sorted);
}

function test_property_sort() {
  var data = getData();
  var sorted = [];
  var v;
  var i = new ThemeBuilder.PropertyIterator(data['.active a']);
  while (i.hasNext()) {
    v = i.next();
    var str = v.name + ': ' + v.value;
    sorted.push(str);
  }
  assertArrayEquals(['color: {black-67-white}', 'font-size: 30px', 'font-weight: bold'], sorted);
}