<?php

function themebuilder_styleedit_jsunit_selector(&$jsunit) {
  $path = drupal_get_path('module', 'themebuilder_styleedit');
  $core_path = drupal_get_path('module', 'themebuilder_compiler') . '/js';
  $jsunit->setTitle('Element selector unit test');
  $jsunit->addJavaScriptFile('jquery.js', 'misc');
  $jsunit->addJavaScriptFile('jquery.ui.core.min.js', 'misc/ui');
  $jsunit->addJavaScriptFile('jquery.ui.widget.min.js', 'misc/ui');
  $jsunit->addJavaScriptFile('jquery.ui.slider.min.js', 'misc/ui');
  $jsunit->addJavaScriptFile('drupal.js', 'misc');
  $jsunit->addJavaScriptFile('core.js', $core_path);
  $jsunit->addJavaScriptFile('coreUtil.js', $core_path);
  $jsunit->addJavaScriptFile('Selector.js', "$path/js");
  $jsunit->addJavaScriptTestFile("$path/js/test/selector.js");
}

function themebuilder_styleedit_jsunit_pathelement(&$jsunit) {
  $path = drupal_get_path('module', 'themebuilder_styleedit');
  $core_path = drupal_get_path('module', 'themebuilder_compiler') . '/js';
  $jsunit->setTitle("PathElement class unit test");
  $jsunit->addJavaScriptFile('jquery.js', 'misc');
  $jsunit->addJavaScriptFile('jquery.ui.core.min.js', 'misc/ui');
  $jsunit->addJavaScriptFile('jquery.ui.widget.min.js', 'misc/ui');
  $jsunit->addJavaScriptFile('jquery.ui.slider.min.js', 'misc/ui');
  $jsunit->addJavaScriptFile('drupal.js', 'misc');
  $jsunit->addJavaScriptFile('core.js', $core_path);
  $jsunit->addJavaScriptFile('coreUtil.js', $core_path);
  $jsunit->addJavaScriptFile('Selector.js', "$path/js");
  $jsunit->addJavaScriptTestFile("$path/js/test/pathelement.js");
}

function themebuilder_styleedit_jsunit_filter(&$jsunit) {
  $path = drupal_get_path('module', 'themebuilder_styleedit');
  $core_path = drupal_get_path('module', 'themebuilder_compiler') . '/js';
  $jsunit->setTitle("Filter class unit test");
  $jsunit->addJavaScriptFile('jquery.js', 'misc');
  $jsunit->addJavaScriptFile('jquery.ui.core.min.js', 'misc/ui');
  $jsunit->addJavaScriptFile('jquery.ui.widget.min.js', 'misc/ui');
  $jsunit->addJavaScriptFile('jquery.ui.slider.min.js', 'misc/ui');
  $jsunit->addJavaScriptFile('drupal.js', 'misc');
  $jsunit->addJavaScriptFile('core.js', $core_path);
  $jsunit->addJavaScriptFile('coreUtil.js', $core_path);
  $jsunit->addJavaScriptFile('Selector.js', "$path/js");
  $jsunit->addJavaScriptFile('ThemeMarkup1Filter.js', "$path/js");
  $jsunit->addJavaScriptTestFile("$path/js/test/filter.js");
}
