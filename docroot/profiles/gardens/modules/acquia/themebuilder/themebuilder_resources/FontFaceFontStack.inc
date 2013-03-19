<?php

class FontFaceFontStack implements FontStack, Resource {
  private $name;
  private $localStack;
  private $clientStack;
  private $attribution;
  private $modulePath;
  private $fontTypes;

  /**
   * Constructor.
   *
   * @param string $directory
   *   The directory within themebuilder_resources/fonts that the font files can
   *   be found in. This will also be used as the human-readable name in the UI,
   *   so name the directories nicely.
   * @param array $localAlternatives
   *   An array of alternative font names that the user may have on their system
   *   already, used in the 'local()' part of the @font-face src declaration.
   * @param array $clientAlternatives
   *   An array of alternate fonts for browsers that don't support @font-face.
   * @param string $attribution
   *   Optional. An attribution string for Creative Commons and similarly-
   *   licensed fonts. Will appear immediately after the @font-face rule.
   */
  public function __construct($directory, $localAlternatives, $clientAlternatives, $attribution = NULL) {
    $fontPath = drupal_get_path('module', 'themebuilder_resources') . '/fonts/' . $directory;
    if (!drupal_realpath($fontPath)) {
      throw new Exception("Font directory $fontpath does not exist.");
    }
    $this->name = $directory;
    $this->localStack = $localAlternatives;
    $this->clientStack = $clientAlternatives;
    $this->attribution = $attribution;
    $this->modulePath = $fontPath;
    $this->fontTypes = array('eot' => 'eot', 'ttf' => 'truetype', 'otf' => 'opentype', 'svg' => 'svg', 'woff' => 'woff');

    foreach (glob($fontPath . '/*') as $file) {
      $info = pathinfo($file);
      $type = $info['extension'];
      if (in_array($type, array_keys($this->fontTypes))) {
        $this->versions[$type] = $info['basename'];
      }
    }
  }

  /**
   * Determine whether the font referred to lives on the client's computer.
   */
  public function isClientSide() {
    // All @font-face fonts are server-side.
    return FALSE;
  }

  /**
   * Return the name of the font, which is also the directory name.
   */
  public function getName() {
    return $this->name;
  }

  /**
   * Return the font-family string for use in a CSS file.
   */
  public function getFontFamily() {
    $fonts = array($this->name) + (array) $this->clientStack;
    foreach ($fonts as &$font) {
      $font = "'$font'";
    }
    return implode(',', $fonts);
  }

  /**
   * Return a @font-face rule that will make this font available to the client.
   *
   * @param string $type
   *   The type of font-face rule to return. If 'themebuilder', will return
   *   a rule with an absolute URL pointing to Themebuilder's version of the
   *   fonts. If 'theme', will return a rule with a relative URL that will work
   *   as part of a theme's font-families.css.
   * @return string
   *   The @font-face rule, plus attribution if the license requires it.
   */
  public function getFontFaceRule($type = 'themebuilder') {
    switch ($type) {
      case 'theme':
        // Drupal CSS aggregation strips quotes from relative URLs
        // (http://drupal.org/node/854312). Until that is fixed, work around it
        // here by replacing spaces with %20 so that our paths will work even
        // without quotes.
        $path = str_replace(' ', '%20', $this->getDestinationPath());
        break;
      case 'themebuilder':
      default:
        $path = base_path() . $this->modulePath;
        break;
    }
    $family = $this->name;
    $ieSource = $this->getIeSource($path);
    $source = $this->getSource($path);
    $output = "@font-face {\n  font-family: '$family';\n  src: $ieSource;\n  src: $source;\n}\n";

    // Add an attribution line for fonts that require it.
    if ($this->attribution) {
      $output .= "/* " . $this->attribution . " */\n";
    }
    return $output;
  }

  public function getIeSource($path) {
    $file = empty($this->versions['eot']) ? '' : $this->versions['eot'];
    if ($file) {
      return "url('$path/$file')";
    }
    return '';
  }

  public function getSource($path) {
    $output = '';
    foreach ($this->localStack as $local) {
      $output .= "local('$local'), ";
    }
    $fontSources = array();
    foreach ($this->versions as $type => $filename) {
      $format = $this->fontTypes[$type];
      $fontSources[$type] = "url('$path/$filename') format('$format')";
    }
    // The IE-specific eot font must be included on a separate line, so we
    // will not include it here.  Also it is important to note that order
    // matters.  The browser will use the first font type encountered that it
    // can handle.  Thus it is important to put the most efficient fonts near
    // the beginning of the list, with the least efficient at the end.
    $versions = array();
    foreach (array('woff', 'otf', 'ttf', 'svg') as $type) {
      if (isset($fontSources[$type])) {
        $versions[] = $fontSources[$type];
      }
    }
    $output .= implode(",\n   ", $versions);
    return $output;
  }

  /**
   * The path from which the font files can be copied.
   */
  public function getSourcePath() {
    return $this->modulePath;
  }

  /**
   * The path where the font files should be copied within a given theme.
   */
  public function getDestinationPath() {
    return 'fonts/' . $this->name;
  }

}
