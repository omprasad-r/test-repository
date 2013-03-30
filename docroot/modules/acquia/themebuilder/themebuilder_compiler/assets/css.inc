<?php
// $Id:$

/**
 * @file
 * The CSS asset, which allows modification of css properties.
 */


require_once(dirname(__FILE__) . '/../asset.inc');
require_once(dirname(__FILE__) . '/../editable.inc');
require_once(dirname(__FILE__) . '/../lib/csstidy/class.csstidy.php');

/**
 * CSS asset class
 */
class CssAsset extends ThemebuilderAsset implements UserEditable {
  private $css_table = 'themebuilder_css';
  protected $properties;

  public function __construct($theme) {
    parent::__construct($theme);
    $this->asset_name = 'custom.css';
    $this->properties = array(self::GENERATED => array(), self::CUSTOM => array());
  }
  
  public function load() {
    themebuilder_log_trace_enter(__METHOD__);
    if ($this->aid) {
      $result = array();
      $fields = array('selector', 'property_name', 'value', 'generated');
      timer_start(__METHOD__ . '_query');
      $rules = db_select('themebuilder_css', 'c')
        ->fields('c', $fields)
        ->condition('aid', $this->aid)
        ->orderBy('pid')
        ->execute();
      themebuilder_log_code_time(__METHOD__, __LINE__, timer_stop(__METHOD__ . '_query'));
      timer_start(__METHOD__ . '_setProperty');
      foreach($rules as $rule) {
        $this->setProperty($rule->selector, $rule->property_name, $rule->value, $rule->generated);
      }
      themebuilder_log_code_time(__METHOD__, __LINE__ - 2, timer_stop(__METHOD__ . '_setProperty'));
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  public function importData() {
    // If the theme being imported has a json file containing the CSS
    // properties and their metadata for the imported theme, set the CSS
    // properties based on that.
    $filename = $this->jsonMetadataFile($this->theme->getPath());
    $css = file_get_contents($filename);
    if ($css) {
      $css = drupal_json_decode($css);
      foreach ($css as $generated => $section) {
        foreach ($section as $rule => $properties) {
          foreach ($properties as $name => $value) {
            $this->setProperty($rule, $name, $value, $generated);
          }
        }
      }
    }
  }

  public function export($directory) {
    // Save the CSS properties (including their metadata, such as "generated"
    // status) to a json file on theme export, so they can be read in again on
    // theme import in $this->importData().
    $output = drupal_json_encode($this->getSortedProperties());
    $filename = $this->jsonMetadataFile($directory);
    file_put_contents($filename, $output);
  }

  /**
   * Returns the name of the (optional) json metadata file that this CSS asset is associated with.
   *
   * @param $directory
   *   The directory in which the CSS asset lives.
   */
  private function jsonMetadataFile($directory) {
    return $directory . '/' . $this->getFilename() . '.import.json';
  }
  
  public function renderString() {
    $css = $this->getSortedProperties();
    $output = '';
    foreach ($css as $type => $section) {
      $output .= "\n";
      foreach ($section as $rule => $properties) {
        $output .= $rule ." {\n";
        foreach ($properties as $name => $value) {
          $output .= '  ' .$name .': ' .$value .";\n";
        }
        $output .= "}\n\n";
      }
    }
    return $output;
  }

  public function getAllProperties() {
    return $this->properties;
  }

  /*
   * Return an array of CSS properties, with custom properties sorted such that
   * pseudoclasses appear in LVHA order.
   */
  public function getSortedProperties() {
    themebuilder_log_trace_enter(__METHOD__);
    $css = $this->getAllProperties();
    $customCss = $css[self::CUSTOM];
    $this->addMetaInfo($customCss);
    uasort($customCss, 'CssAsset::sortRules');
    $this->removeMetaInfo($customCss);
    $css[self::CUSTOM] = $customCss;
    themebuilder_log_trace_exit(__METHOD__);
    return $css;
  }

  /**
   * Adds metadata to each element in the specified css array.  The metadata
   * will indicate the weight each rule should have (used for rendering
   * purposes to make sure rules that include pseudoclasses come out in the
   * right order).
   *
   * @param {array} $css
   *   The array that contains all of the css rule data.  This includes a
   *   selector and associated propertys and values.
   */
  private function addMetaInfo(&$css) {
    foreach ($css as $selector => &$info) {
      $info['metadata'] = array('weight' => $this->getSelectorWeight($selector));
    }
  }

  /**
   * Removes the metadata from each element in the specified css array.  The
   * metadata indicates the weight each rule should have (used for rendering
   * purposes to make sure rules that include pseudoclasses come out in the
   * right order).  This information needs to be removed before rendering the
   * css rule into a file.
   *
   * @param {array} $css
   *   The array that contains all of the css rule data.  This includes a
   *   selector and associated propertys and values.
   */
  private function removeMetaInfo(&$css) {
    foreach ($css as $selector => &$info) {
      unset($info['metadata']);
    }
    return $css;
  }

  /**
   * Sort function used with uasort() that sorts css rules based on their
   * weight.  You have to add weight metadata to the css rules before this can
   * possibly work.  See CssAsset::addMetaInfo().
   *
   * @param (array) $a
   *   The first item to compare.  This item has the property/value and
   *   metadata that includes the selector weight.
   * @param (array) $b
   *   The second item to compare.  This item has the property/value and
   *   metadata that includes the selector weight.
   * @return
   *   An integer that describes which of the two items should appear before
   *   the other in a sorted list.  The selector weight is the only factor
   *   that determines the outcome.  A return value of 0 indicates the items
   *   have equal weight and it does not matter which appears first.  A value
   *   of -1 indicates $b should precede $a in the sorted list.  1 indicates
   *   $a should precede $b in the sorted list.
   */
  public static function sortRules($a, $b) {
    $weightA = intval($a['metadata']['weight']);
    $weightB = intval($b['metadata']['weight']);
    if ($weightA === $weightB) {
      return 0;
    }
    return ($weightA < $weightB) ? -1 : 1;
  }

  /**
   * Determines the weight that should be given to the specified selector for
   * rendering purposes.  The point is that all selectors that include a
   * pseudoclass should be rendered in the proper order (LVHA) or the desired
   * styles will not come through.
   *
   * @param {String} $selector
   *   The css selector.
   * @return
   *   An integer that indicates the weight that should be applied to the
   *   specified selector.
   */
  private function getSelectorWeight($selector) {
    $result = 0;
    if (strpos($selector, ':') != -1) {
      // We don't look for the 'hover' pseudoclass.  We consider that as
      // having a weight of 0 and we weight other pseudoclasses around it so
      // combinations of pseudoclasses don't throw off the order.
      $patterns = array(
        'active' => array('pattern' => '/(\:active)/', 'weight' => 25),
        'visited' => array('pattern' => '/(\:visited)/', 'weight' => -25),
        'link' => array('pattern' => '/(\:link)/', 'weight' => -50),
                       );
      foreach ($patterns as $pseudoclass => $query) {
        $hasPseudoclass = preg_match($query['pattern'], $selector, $matches);
        if ($hasPseudoclass) {
          $result = $query['weight'];
          break;
        }
      }
    }
    return $result;
  }

  public function getProperty($selector, $property_name, $generated = self::CUSTOM) {
    $query = db_select('themebuilder_css', 'c')
      ->fields('c', array('value'))
      ->condition('theme_name', $this->theme_name)
      ->condition('selector', $selector)
      ->condition('property_name', $property_name)
      ->condition('generated', $generated);
    $result = $query->execute();
    if (!empty($result)) {
      return $result->value;
    }
    return NULL;
  }

  public function setProperty($selector, $property_name, $value, $generated = self::CUSTOM) {
    // If the value has a url() in it, make sure to handle spaces correctly.
    // TODO AN-11360: More robust algorithm.
    if (strpos($value, "url(") !== FALSE) {
      $value = str_replace(" ", "%20", $value);
    }

    if (!isset($this->properties[$generated][$selector])) {
      $this->properties[$generated][$selector] = array();
    }
    $this->properties[$generated][$selector][$property_name] = $value;
  }

  public function removeProperty($selector, $property_name, $generated = self::CUSTOM) {
    if (isset($this->properties[$generated][$selector][$property_name])) {
      unset($this->properties[$generated][$selector][$property_name]);
    }
    if (empty($this->properties[$generated][$selector])) {
      unset($this->properties[$generated][$selector]);
    }
  }
  
  function save() {
    themebuilder_log_trace_enter(__METHOD__);
    db_delete('themebuilder_css')
      ->condition('aid', $this->aid)
      ->execute();

    foreach ($this->properties as $generated => $section) {
      foreach ($section as $selector => $properties) {
        foreach ($properties as $name => $value) {
          $this->_saveProperty($this->aid, $selector, $name, $value, $generated);
        }
      }
    }
    themebuilder_log_trace_exit(__METHOD__);
  }

  public function copy($new_theme) {
    die("Copy not yet implemented");
    $css = $this->getAllProperties();
    foreach ($css as $generated => $section) {
      foreach ($section as $rule => $properties) {
        foreach ($properties as $property) {
          $this->_setProperty($new_theme->getName(), $rule, $property->name, $property->value, $generated);
        }
      }
    }
    $asset = new CssAsset($new_theme->getName());
    $asset->renderFile($new_theme->getPath());
  }

  /**
   * Save a CSS property to the database.
   *
   * @param {integer} $asset_id
   * @param {string} $selector
   * @param {string} $property_name
   * @param {string} $value
   * @param {boolean} $generated
   */
  public function _saveProperty($asset_id, $selector, $property_name, $value, $generated) {
    $fields = array(
      'aid' => $asset_id,
      'selector' => $selector,
      'property_name' => $property_name,
      'value' => $value,
      'generated' => $generated,
    );
    db_insert('themebuilder_css')
      ->fields($fields)
      ->execute();
  }

  public function delete() {
    db_delete('themebuilder_css')
      ->condition('aid', $this->aid)
      ->execute();
    $path = $this->theme->getPath() .'/' .$this->getFilename();
    @unlink($path);
  }
}
