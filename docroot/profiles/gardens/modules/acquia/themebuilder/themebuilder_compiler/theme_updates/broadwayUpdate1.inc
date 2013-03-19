<?php

/**
 * This theme update will make changes to the palette order in a customized
 * theme based on the broadway theme when updating from a theme with a version
 * number of 0.6 or earlier to version 0.7 or later.
 */
class broadwayUpdate1 extends ThemeUpdate {
  protected $oldVersion = '0.6';
  protected $newVersion = '0.7';
  protected $paletteMap = array(
    'a' => 'e',
    'b' => 'c',
    'c' => 'b',
    'd' => 'a',
    'e' => 'd',
  );
}