<?php

class MonotypeFontStack implements FontStack {

  /**
   * A hard-coded list of popular Monotype fonts, along with the CSS stack
   * that's most appropriate for them.
   */
  protected static $hints = array(
    'Helvetica' => 'sans',
    'Frutiger' => 'sans',
    'Univers' => 'sans',
    'Franklin Gothic' => 'sans',
    'Avant Garde' => 'sans',
    'Clarendon' => 'serif',
    'Rockwell' => 'serif',
    'Calvert' => 'serif',
  );

  /**
   * A hard-coded list of acceptable default font stacks.
   */
  protected static $stacks = array(
    'sans' => array('Arial', 'Helvetica Neue', 'Helvetica', 'sans-serif'),
    'serif' => array('Georgia', 'Palatino', 'Palatino Linotype', 'Times', 'Times New Roman', 'serif'),
    'condensed' => array('Arial Narrow', 'Arial', 'sans-serif'),
  );

  public function __construct($name, $cssName) {
    $this->name = $name;
    $this->cssNames = MonotypeFontStack::getStackSuggestions($cssName);
  }

  public function getName() {
    return $this->name;
  }

  public function getFontFamily() {
    $fonts = array();
    foreach ($this->cssNames as $font) {
      $fonts[] = "'$font'";
    }
    return implode(',', $fonts);
  }

  public function getFontFaceRule() {
    return '';
  }

  public function isClientSide() {
    return FALSE;
  }

  /**
   * Make our best guess at a list of acceptable fallback fonts for a given
   * Monotype font. Note that Typekit has a UI to let users provide their own
   * fallbacks, which is why there's no similar method in our Typekit module.
   */
  protected static function getStackSuggestions($cssName) {
    $suggestions = array();
    // See if this is one of the popular fonts we have hints for.
    foreach (MonotypeFontStack::$hints as $font => $type) {
      if (stripos($cssName, $font) !== FALSE) {
        $suggestions = MonotypeFontStack::$stacks[$type];
        break;
      }
    }
    // If we don't have any hints for this font, search its name for clues like
    // 'sans' or 'serif'.
    if (empty($suggestions)) {
      foreach (MonotypeFontStack::$stacks as $type => $stack) {
        if (stripos($cssName, $type) !== FALSE) {
          $suggestions = $stack;
          break;
        }
      }
    }
    // Most important step: Add the actual name of the font family at the
    // beginning of the stack.
    array_unshift($suggestions, $cssName);
    return $suggestions;
  }
}
