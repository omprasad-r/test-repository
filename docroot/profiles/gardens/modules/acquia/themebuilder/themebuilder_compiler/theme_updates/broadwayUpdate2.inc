<?php

/**
 * This theme update will make changes to the css selectors in a customized
 * theme based on the broadway theme when updating from a theme with a version
 * number of 0.10 or earlier to version 1.0.
 */
class broadwayUpdate2 extends ThemeUpdate {
  protected $oldVersion = '0.10';
  protected $newVersion = '1.0';
  protected $selectorMap = array(
    '#page-wrapper' => '#page',
    '#page' => '.lining',
    '#header' => '#header-inner',
    '#branding' => '.tb-header-inner-1',
    '#name-and-slogan' => '.tb-header-inner-2', 
    '#main-wrapper' => '#content',
    '#main' => '.tb-content-wrapper-1',
    '#banner-region' => '#banner',
    '#precontent-wrapper' => '.tb-precontent-1',
    '#precontent-end-wrapper' => '.tb-precontent-2',
    '#content-wrapper' => '#content-inner',
    '#content' => '#main',
    '#prefooter-wrapper' => '.tb-prefooter-1',
    '#prefooter-end-wrapper' => '.tb-prefooter-2',
    '.area' => '',
    '.section' => '',
    '.column' => ''
  );
}