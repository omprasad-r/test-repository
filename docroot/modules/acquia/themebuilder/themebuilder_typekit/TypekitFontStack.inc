<?php

class TypekitFontStack implements FontStack {

  public function __construct($name, $cssStack) {
    $this->name = $name;
    $names = explode(',',$cssStack);
    foreach ($names as &$name) {
      $name = trim($name, ' "');
      $name = trim($name, "'");
      $name = "'$name'";
    }
    $this->cssNames = $names;
  }

  public function getName() {
    return $this->name;
  }

  public function getFontFamily() {
    return implode(',', $this->cssNames);
  }

  public function getFontFaceRule() {
    return '';
  }

  public function isClientSide() {
    return FALSE;
  }
}
