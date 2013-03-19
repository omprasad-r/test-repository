<?php
// $Id:$
/**
 * @file
 * Advanced CSS and JS Editor
 */

require_once 'code.inc';

class AdvancedCssAsset extends CodeAsset {
  public function __construct($theme) {
    parent::__construct($theme);
    $this->asset_name = 'advanced.css';
  }
}

class AdvancedJsAsset extends CodeAsset {
  public function __construct($theme) {
    parent::__construct($theme);
    $this->asset_name = 'advanced.js';
  }
}
