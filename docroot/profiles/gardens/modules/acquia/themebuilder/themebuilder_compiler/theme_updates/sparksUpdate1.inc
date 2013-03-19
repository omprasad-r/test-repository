<?php

/**
 * This theme update will make changes to the palette order in a customized
 * theme based on the sparks theme when updating from a theme with a version
 * number of 0.9 or earlier to version 0.10 or later.
 */
class sparksUpdate1 extends ThemeUpdate {
  protected $oldVersion = '0.9';
  protected $newVersion = '0.10';
  protected $paletteMap = array(
    'a' => 'd',
    'b' => 'e',
    'c' => 'a',
    'd' => 'c',

  );
}
