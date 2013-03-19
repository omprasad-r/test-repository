<?php

/**
 * This theme update will make changes to the palette order in a customized
 * theme based on the campaign theme when updating from a theme with a version
 * number of 0.9 or earlier to version 0.10 or later.
 */
class campaignUpdate2 extends ThemeUpdate {
  protected $oldVersion = '0.9';
  protected $newVersion = '0.10';
  protected $paletteMap = array(
    'a' => 'c',
    'b' => 'a',
    'c' => 'd',
    'd' => 'e',
    'e' => 'b',
  );
}
