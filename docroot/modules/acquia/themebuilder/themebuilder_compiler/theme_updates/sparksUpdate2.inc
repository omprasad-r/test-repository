<?php

/**
 * This theme update will make changes to the css selectors in a customized
 * theme based on the sparks theme when updating from a theme with a version
 * number of 0.10 or earlier to version 1.0 or later.
 */
class sparksUpdate2 extends ThemeUpdate {
  protected $oldVersion = '0.10';
  protected $newVersion = '1.0';
  protected $selectorMap = array(
    '#page-wrapper' => '#page',
    '#page' => '.lining',
    '#header-wrapper' => '#header',
    '#header' => '.tb-header-wrapper-1',
    '#branding' => '#header-inner',
    '#name-and-slogan' => '.tb-header-inner-2',
    '#main-wrapper' => '#content',
    '#main' => '.tb-content-wrapper-1',
    '#banner-region' => '#banner',
    '#precontent-wrapper' => '.tb-precontent-1',
    '#precontent-end-wrapper' => '.tb-precontent-2',
    '#body' => '#content-inner',
    '#content-wrapper' => '.tb-main-1',
    '#content' => '#main',
    '#prefooter-wrapper' => '.tb-prefooter-1',
    '#prefooter-end-wrapper' => '.tb-prefooter-2',
    '.area' => '',
    '.section' => '',
    '.column' => ''
  );
}
