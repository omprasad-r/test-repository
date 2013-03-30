<?php
// $Id:$

/**
 * @file
 * The template asset, which allows selection of the various template files.
 */

require_once(dirname(__FILE__) . '/../asset.inc');
require_once(dirname(__FILE__) . '/../editable.inc');

/**
 * Template asset class
 *
 */
class TemplateAsset extends ThemebuilderAsset implements UserSelectable {

  public function __construct($theme_name, $selector_name) {
    parent::__construct($theme_name);
    $this->asset_name = $selector_name;
  }

  public function getFilename() {
    return $this->asset_name .'.tpl.php';
  }

  public function renderString() {
    $templates = $this->getAllProperties();
    $output = '';
    foreach ($templates as $template) {
      $output .= $template->code;
    }

    return $output;
  }

  protected function save() {
    die('not yet implemented');
  }

  /*
   */
  public function getAllProperties() {
    $result = array();
    /*
    $query = db_select('themebuilder_template', 't');
    $query->innerJoin('themebuilder_markup', 'm', 'm.tid = t.tid');
    $properties = $query
      ->fields('t', array('selector'))
      ->fields('m', array('tid', 'code'))
      ->condition('t.theme_name', $this->theme_name)
      ->execute();
    foreach ($properties as $property) {
      $result[] = $property;
    }
    */
    return $result;
  }

  public function getSelection($selection_name) {
    /*
    $query = db_select('themebuilder_template', 't');
    $query->innerJoin('themebuilder_markup', 'm', 'm.tid = t.tid');
    $code = $query
      ->fields('m', array('code'))
      ->condition('t.theme_name', $this->theme_name)
      ->condition('t.selector', $selection_name)
      ->fetchField();
    return $code ? $code : NULL;
    */
    return NULL;
  }

  public function setSelection($selection_name, $selection_id) {
    /*
    $this->_setSelection($this->theme_name, $selection_name, $selection_id);
     */
  }

  private function _setSelection($theme_name, $selection_name, $selection_id) {
    /*
    db_merge('themebuilder_template')
      ->key(array(
        'theme_name' => $theme_name,
        'selector' => $selection_name,
      ))
      ->fields(array('tid' => $selection_id))
      ->execute();
    */
  }

  public static function getAssets($theme_name) {
    $result = array();
    $templates = db_select('themebuilder_template', 't')
      ->fields('t', array('selector', 'tid'))
      ->condition('theme_name', $theme_name)
      ->execute();
    foreach ($templates as $template) {
      $result[] = new TemplateAsset($theme_name, $template->selector);
    }
    return $result;
  }

  public function copy($new_theme) {
    /*
    $templates = $this->getAllProperties();
    foreach ($templates as $template) {
      $this->_setSelection($new_theme->getName(), $template->selector, $template->tid);
    }
    $asset = new TemplateAsset($new_theme->getName(), $this->asset_name);
    $asset->renderFile($new_theme->getPath());
     */
    $filename = $new_theme->getPath() .'/template.php';
    if ($filename) {
      $contents = file_get_contents($filename);
      $x = 0;
    }
  }

  public function delete() {
    db_delete('themebuilder_template')
      ->condition('theme_name', $this->theme->getName())
      ->execute();
    $path = $this->theme->getPath() .'/' .$this->getFilename();
    @unlink($path);
  }
}
