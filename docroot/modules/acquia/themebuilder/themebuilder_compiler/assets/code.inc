<?php

/**
 * @file
 * The basic interface for assets which don't do anything but r/w text.
 */

class CodeAsset extends ThemebuilderAssetNew {
  public $contents;

  public function renderString() {
    return $this->contents;
  }

  public function setContents($value) {
    // We also get U+00A0 (194 160 in bytes) to signify a nobreak-space for
    // every second space in the submitted code sample. This breaks the CSS
    // code in various ways. Should convert to a single regular space.
    $this->contents = str_replace("\xc2\xa0", ' ', $value);
  }

  public static function loadFromFile($filename) {
    themebuilder_log_trace_enter(__METHOD__);
    $ret = new self();
    $ret->contents = '';
    if (file_exists($filename)) {
      $ret->contents = file_get_contents($filename);
    }
    themebuilder_log_trace_exit(__METHOD__);
    return $ret;
  }
}
