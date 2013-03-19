<?php

/**
 * This theme update will make changes to the palette in a customized
 * theme based on the impact theme when updating from a theme with a version
 * number of 0.3 or earlier to version 0.4 or later.
 */
class impactUpdate1 extends ThemeUpdate {
  protected $oldVersion = '0.3';
  protected $newVersion = '0.4';
  protected $paletteMap = array(
    'b' => 'd',
    'c' => 'e',
    'd' => 'b',
    'e' => 'c',
  );
}
