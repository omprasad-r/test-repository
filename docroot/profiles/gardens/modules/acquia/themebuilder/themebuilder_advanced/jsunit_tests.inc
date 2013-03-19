<?php

function themebuilder_advanced_jsunit_code_editor(&$jsunit) {
  $tb_advanced = drupal_get_path('module', 'themebuilder_advanced');
  $tb_compiler = drupal_get_path('module', 'themebuilder_compiler');
  $tb_styles = drupal_get_path('module', 'themebuilder_styleedit');
  $jsunit->setTitle('Code editor unit test');
  $jsunit->addJavaScriptFile('jquery.js', 'misc');
  $jsunit->addJavaScriptFile('drupal.js', 'misc');
  $jsunit->addJavaScriptFile('core.js', "$tb_compiler/js");
  $jsunit->addJavaScriptFile('CodeEditor.js', "$tb_advanced/js");
  $jsunit->addJavaScriptFile('PowerNavigator.js', "$tb_styles/js");
  $jsunit->addJavaScriptFile('History.js', "$tb_advanced/js");
  $jsunit->addJavaScriptTestFile("$tb_advanced/js/test/codeEditor.js");
}

function themebuilder_advanced_jsunit_edit_pane(&$jsunit) {
  $tb_advanced = drupal_get_path('module', 'themebuilder_advanced');
  $tb_compiler = drupal_get_path('module', 'themebuilder_compiler');
  $tb_styles = drupal_get_path('module', 'themebuilder_styleedit');
  $jsunit->setTitle('Edit pane unit test');
  $jsunit->addJavaScriptFile('jquery.js', 'misc');
  $jsunit->addJavaScriptFile('drupal.js', 'misc');
  $jsunit->addJavaScriptFile('core.js', "$tb_compiler/js");
  $jsunit->addJavaScriptFile('CodeEditor.js', "$tb_advanced/js");
  $jsunit->addJavaScriptFile('PowerNavigator.js', "$tb_styles/js");
  $jsunit->addJavaScriptFile('History.js', "$tb_advanced/js");
  $jsunit->addJavaScriptFile('EditPane.js', "$tb_advanced/js");
  $jsunit->addJavaScriptTestFile("$tb_advanced/js/test/editPane.js");
}

function themebuilder_advanced_jsunit_specificity(&$jsunit) {
  $tb_advanced = drupal_get_path('module', 'themebuilder_advanced');
  $tb_compiler = drupal_get_path('module', 'themebuilder_compiler');
  $jsunit->setTitle('Selector specificity calculator');
  $jsunit->addJavaScriptFile('jquery.js', 'misc');
  $jsunit->addJavaScriptFile('drupal.js', 'misc');
  $jsunit->addJavaScriptFile('core.js', "$tb_compiler/js");
  $jsunit->addJavaScriptFile('CustomStyles.js', "$tb_advanced/js");
  $jsunit->addJavaScriptTestFile("$tb_advanced/js/test/specificity.js");
}

function themebuilder_advanced_jsunit_selectors(&$jsunit) {
  $tb_advanced = drupal_get_path('module', 'themebuilder_advanced');
  $tb_compiler = drupal_get_path('module', 'themebuilder_compiler');
  $jsunit->setTitle('Selector specificity calculator');
  $jsunit->addJavaScriptFile('jquery.js', 'misc');
  $jsunit->addJavaScriptFile('drupal.js', 'misc');
  $jsunit->addJavaScriptFile('core.js', "$tb_compiler/js");
  $jsunit->addJavaScriptFile('CustomStyles.js', "$tb_advanced/js");
  $jsunit->addJavaScriptTestFile("$tb_advanced/js/test/selectors.js");
}
