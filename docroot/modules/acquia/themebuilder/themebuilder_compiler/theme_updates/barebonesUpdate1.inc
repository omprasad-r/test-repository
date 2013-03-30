<?php

/**
 * This theme update will make changes to the palette order in a customized
 * theme based on the barebones theme when updating from a theme with a version
 * number of 1.1 or earlier to version 1.2 or later.
 */
class barebonesUpdate1 extends ThemeUpdate {
  protected $oldVersion = '1.1';
  protected $newVersion = '1.2';
  protected $paletteMap = array(
    'c' => 'd',
    'd' => 'c',
  );
}
