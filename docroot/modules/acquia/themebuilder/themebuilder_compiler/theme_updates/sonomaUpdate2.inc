<?php

/**
 * This theme update will make changes to the css selectors in a customized
 * theme based on the sonoma theme when updating from a theme with a version
 * number of 0.10 or earlier to version 1.0 or later.
 */
class sonomaUpdate2 extends ThemeUpdate {
  protected $oldVersion = '0.10';
  protected $newVersion = '1.0';
  protected $selectorMap = array(
    '#page-wrapper' => '#page',
    '#page' => '.lining',
    '#navigation-wrapper' => '.tb-navigation-1',
    '#navigation-end-wrapper' => '.tb-navigation-2',
    '#navigation-region' => '.tb-navigation-3',
    '#header' => '#header-inner',
    '#branding' => '.tb-header-inner-1',
    '#name-and-slogan' => '.tb-header-inner-2',
    '#main-wrapper' => '#content',
    '#main' => '.tb-content-wrapper-1',
    '#body' => '.tb-content-wrapper-2',
    '#body-wrapper' => '.tb-content-wrapper-3',
    '#body-end-wrapper' => '.tb-content-wrapper-4',
    '#banner-wrapper' => '.tb-banner-1',
    '#banner-end-wrapper' => '.tb-banner-2',
    '#precontent-wrapper' => '.tb-precontent-1',
    '#precontent-end-wrapper' => '.tb-precontent-2',
    '#content-wrapper' => '#content-inner',
    '#content' => '#main',
    '#prefooter-wrapper' => '.tb-prefooter-1',
    '#prefooter-end-wrapper' => '.tb-prefooter-2', /* Triple */
    '.area' => '',
    '.section' => '',
    '.column' => ''
  );
}
