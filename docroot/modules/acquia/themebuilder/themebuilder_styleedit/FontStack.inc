<?php

interface FontStack {

  /**
   * Return the human-readable name of the font (i.e. "Verdana").
   */
  public function getName();

  /**
   * Return a font-family string for use in a CSS file.
   *
   * @return string
   *   The font-family string (e.g. "Verdana,Arial,sans-serif"), in the format
   *   in which getComputedStyle outputs its font families (i.e. no spaces or
   *   quotes). This is necessary so that Themebuilder can auto-discover
   *   which font is being used for the selected element.
   */
  public function getFontFamily();

  /**
   * Return a @font-face rule that will make this font available to the client.
   *
   * Fonts that do not need a @font-face rule should return an empty string.
   */
  public function getFontFaceRule();

  /**
   * Return TRUE if all fonts in the font-family string are expected to be
   * found on the end user's computer, FALSE for any fonts made available via
   * @font-face (including Typekit and Webfonts).
   */
  public function isClientSide();

}

/**
 * Represents a traditional font stack such as "Verdana, Arial, sans-serif",
 * with no @font-face declaration.
 */
class ClientFontStack implements FontStack {

  public function __construct($name, $fontnames) {
    $this->name = $name;
    $this->fontnames = $fontnames;
  }

  public function getName() {
    return $this->name;
  }

  public function getFontFamily() {
    $fonts = $this->fontnames;
    foreach ($fonts as &$font) {
      if ($font !== 'inherit') {
        $font = "'$font'";
      }
    }
    return implode(',', $fonts);
  }

  public function getFontFaceRule() {
    return "";
  }

  public function isClientSide() {
    return TRUE;
  }

}
