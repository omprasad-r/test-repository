<?php

/**
 * This theme update will make changes to the css selectors in a customized
 * theme based on the kenwood theme when updating from a theme with a version
 * number of 0.10 or earlier to version 1.0 or later.
 */
class kenwoodUpdate1 extends ThemeUpdate {
  protected $oldVersion = '0.10';
  protected $newVersion = '1.0';
  protected $selectorMap = array(
    '#page-wrapper' => '#page',
    '#page' => '.lining',
    '#header-wrapper' => '#header-inner',
    '#header' => '.tb-header-inner-1',
    '#branding' => '.tb-header-inner-2',
    '#name-and-slogan' => '.tb-header-inner-3',
    '#main-wrapper' => '#content',
    '#main' => '.tb-content-wrapper-1',
    '#banner-region' => '#banner',
    '#precontent-wrapper' => '.tb-precontent-1',
    '#precontent-end-wrapper' => '.tb-precontent-2',
    '#body' => '#content-inner',
    '#content-wrapper' => '#content-inner .inset-1',
    '#content' => '#main',
    '#prefooter-wrapper' => '.tb-prefooter-1',
    '#prefooter-end-wrapper' => '.tb-prefooter-2',
    '.area' => '',
    '.section' => '',
    '.column' => ''
  );

  /**
   * Updates the given palette info according to the palette map.
   *
   * @param <Array> $paletteInfo
   *   The palette section of the .info file array.
   * @return <Array>
   *   An updated version of the original palette info.
   */
  public function updatePaletteInfo($paletteInfo) {
    // Whether the palette in the .info file is the legacy Kenwood palette
    // (which it will be for pre-beta-8 themes), or the new Kenwood palette
    // (which it will be for beta-8), replace it with the legacy Kenwood
    // palette plus a new UUID.
    $newInfo['id'] = '19F6A878-9120-DD68-4FC2B0640C419BF4';
    $newInfo['name'] = 'Legacy';
    $newInfo['white']['hex'] = 'FFFFFF';
    $newInfo['black']['hex'] = '333333';
    $newInfo['a']['hex'] = '5D0604';
    $newInfo['b']['hex'] = '1D1D2A';
    $newInfo['c']['hex'] = 'EBEADE';
    $newInfo['d']['hex'] = 'E6DBC8';
    $newInfo['e']['hex'] = 'FFFDFB';
    return $newInfo;
  }
}
