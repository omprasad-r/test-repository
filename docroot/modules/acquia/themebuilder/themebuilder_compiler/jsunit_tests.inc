<?php

/**
 * Test configuration for coreUtil jsunit tests.
 */
function themebuilder_compiler_jsunit_core_util(&$jsunit) {
  $path = drupal_get_path('module', 'themebuilder_compiler') . '/js';
  $jsunit->setTitle('Core util unit test');
  $jsunit->addJavaScriptFile('core.js', $path);
  $jsunit->addJavaScriptFile('coreUtil.js', $path);
  $jsunit->addJavaScriptTestFile("$path/test/coreUtil.js");
}
