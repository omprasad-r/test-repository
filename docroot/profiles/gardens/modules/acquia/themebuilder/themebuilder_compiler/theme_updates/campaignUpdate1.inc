<?php

/**
 * This theme update will make changes to the css selectors in a customized
 * theme based on the campaign theme when updating from a theme with a version
 * number of 0.6 or earlier to version 0.7 or later.
 */
class campaignUpdate1 extends ThemeUpdate {
  protected $oldVersion = '0.6';
  protected $newVersion = '0.7';
  protected $selectorMap = array(
    '#full-page' => '#page',
    '#page-wrapper' => '#page-temp', /* Hook for campaignUpdate2.inc, goes to #page .lining */
    '#pre-page-wrapper' => '#header',
    '#header' => '#header-inner',
    '#name-and-slogan' => '.tb-header-inner-col-c',
    '#page' => '#content',
    '#main-wrapper' => '.tb-content-wrapper-1',
    '#main' => '.tb-content-wrapper-2',
    '#banner-region' => '#banner',
    '#precontent-wrapper' => '.tb-precontent-1',
    '#precontent-end-wrapper' => '.tb-precontent-2',
    '#body' => '#main',
    '#content-wrapper' => '#main-inner',
    '#content' => '#main-inner .brick',
    '#prefooter-wrapper' => '.tb-prefooter-1',
    '#prefooter-end-wrapper' => '.tb-prefooter-2',
    '#footer' => '#footer-inner',
    '.area' => '',
    '.section' => '',
    '.column' => ''
  );
}
