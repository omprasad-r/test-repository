<?php
// $Id:$

/**
 * @file
 * The function asset, which allows modification of the template.php file.
 */

require_once(dirname(__FILE__) . '/../asset.inc');
require_once(dirname(__FILE__) . '/../editable.inc');

/**
 * function asset class
 *
 */
class FunctionAsset extends ThemebuilderAsset {
  private $table = 'themebuilder_function';
  public $content;
  public $layout;

  public function __construct($theme) {
    parent::__construct($theme);
    $this->asset_name = "template.php";
  }
  
  public function setTheme($theme) {
    themebuilder_log_trace_enter(__METHOD__);
    if (isset($this->theme)) {
      if ($this->theme->getName() != $theme->getName()) {
        $this->content = $this->modifyFunctionNames($this->content, $theme);
      }
    }
    parent::setTheme($theme);
    themebuilder_log_trace_exit(__METHOD__);
  }

  public function renderString() {
    $theme_name = $this->theme_name;
    $code = $this->content;
    $layout = $this->theme->getLayout();
    //Not sure about this here...
    if (NULL != $layout) {
      $code = $this->modifyLayout($code, $layout);
    }
    return $code;
  }

  public function getAllProperties() {
    $result = "<?php\\n";
    $filename = $this->theme->getPath() .'/' .$this->getFilename();
    if (file_exists($filename)) {
      $result = file_get_contents($filename);
    }
    return $result;
  }
  
  public static function getAssets($theme_name) {
    $result = array(new FunctionAsset($theme_name));
    return $result;
  }
  
  /**
   * Returns the layout name from the specified code.
   * 
   * @param string $code
   *   The full text of the code in template.php
   * @return string
   *   The layout name or 'unknown' if it could not be determined.
   */
  public static function getLayoutFromCode($code) {
    $matches = array();
    $result = preg_match('/(return[ \t]*\')(body-layout-[^\']*)/m', $code, $matches);
    if ($result) {
      return $matches[2];
    }
    return 'unknown';
  }

  private function modifyLayout($code, $layout) {
    $modified_code = preg_replace('/[\'](body-layout[^\']*)\'/m',
      "'$layout'", $code);
    return $modified_code;
  }

  private function modifyFunctionNames($code, $new_theme) {
    $modified_code = preg_replace('/(function[ \t]+)([_])*(' .$this->theme_name .')/m',
      '${1}${2}' .$new_theme->getName(), $code);
    $modified_code = preg_replace('/(\$vars*[^=]*[ \t]*=[ \t]*)([_])*(' .$this->theme_name .')/m',
      '${1}${2}' .$new_theme->getName(), $modified_code);
    return $modified_code;
  }
  
  protected function save() {
    //I guess no DB operation, but should probably be saving layout instead of theme
  }
  
  function load() {
    themebuilder_log_trace_enter(__METHOD__);
    $this->content = $this->getAllProperties();
    if (!$this->theme->getLayout()) {
      $this->layout = self::getLayoutFromCode($this->content);
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  // TODO: Need to be able to write this file from scratch as well.  This needs to be done every time the layout changes.

  protected function delete() {
    db_delete('themebuilder_function')
      ->condition('theme_name', $this->theme->getName())
      ->execute();
    $path = $this->theme->getPath() .'/' .$this->getFilename();
  }
}

class FunctionAssetNew extends ThemebuilderAssetNew {
  public $contents;

  public static function loadFromFile($filename) {
    themebuilder_log_trace_enter(__METHOD__);
    $ret = new self();
    $ret->contents = file_get_contents($filename);
    themebuilder_log_trace_exit(__METHOD__);
    return $ret;
  }

  public function renderString() {
    return $this->contents;
  }

  public function renameFunctions($old_name, $new_name) {
    themebuilder_log_trace_enter(__METHOD__);
    // If you change this regular expression, make sure you also
    // review the one located in
    // themebuilder_compiler/theme_elves/InfoFilenameThemeElf:modifyFunctionNames().
    $this->contents = preg_replace('/([ \t=])([_])*(' . $old_name .')([_])/m',
      '${1}${2}' .$new_name .'${4}', $this->contents);
    themebuilder_log_trace_exit(__METHOD__);
  }
}
