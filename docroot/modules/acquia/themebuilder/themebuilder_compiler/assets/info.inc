<?php

include_once dirname(__FILE__) . '/../asset.inc';

class ThemebuilderInfo extends ThemebuilderAssetNew {
  private $contents = NULL;

  public function write($filename) {
    parent::write($filename);

    // Clear the static cache or layout changes will not be used on
    // subsequent page requests.
    drupal_static_reset('drupal_parse_info_file');
  }

  public static function loadFromFile($filename) {
    $ret = new self();
    $ret->contents = drupal_parse_info_file($filename);
    return $ret;
  }

  /**
   * Returns the contents of the .info file.
   *
   * @return
   *   An array containing the key/value pairs in the .info file.
   */
  public function getContents() {
    return $this->contents;
  }

  /**
   * Sets new contents into the .info file.  Note that the .info file has to
   * be rendered after new contents have been set in order to see them in the
   * file.
   *
   * @param $contents
   *   An array containing the new set of key/value pairs that comprise the
   *   .info file.
   */
  public function setContents($contents) {
    $this->contents = $contents;
  }

  function changeName($theme_friendly_name) {
    $this->contents['name'] = $theme_friendly_name;
  }

  function changeSetting($key, $value, $theme) {
    switch ($key) {
      // Special handling for logo files.
      case 'default_logo_path':
        if ($value) {
          $this->contents['settings'][$key] = $value;
        }
        else {
          unset($this->contents['settings'][$key]);
        }
        break;

      default:
        // Settings can be objects or simple values. If we have an object,
        // convert it to an array.
        if (is_object($value)) {
          $value = (array) $value;
        }

        $this->contents['settings'][$key] = $value;
        break;
    }
  }

  function changeColorset($newColorset) {
    $paletteType = $newColorset->getType();
    $this->contents[$paletteType] = $newColorset->getInfoArray();
  }

  function renderString() {
    return drupal_info_file_to_string($this->contents);
  }

  /**
   * This is a magic __get method, but we call it directly from many places in
   * the codebase, which seems wrong.
   *
   * Also, for some unknown reason, a side effect of calling this function is
   * that $this->contents[$key] gets *set* (e.g. to an empty string) if it
   * didn't exist before, which means calling this function can actually
   * trigger the previously-nonexistent property to be *written* to the info
   * file when it is eventually saved.
   *
   * TODO: Transition most callers to use the get() method below, instead of
   * this.
   */
  function &__get($key) {
    if (!$this->contents) {
      throw new Exception("Info object is not initialized.");
    }
    return $this->contents[$key];
  }

  /**
   * This method returns a property's value from the info file, or NULL if the
   * the property isn't present.
   *
   * TODO: We should probably transition most uses of __get() to this instead.
   *
   * @param $key
   *   The key name representing the property.
   */
  function get($key) {
    if (!$this->contents) {
      throw new Exception("Info object is not initialized.");
    }
    if (isset($this->contents[$key])) {
      return $this->contents[$key];
    }
  }

  /**
   * This method sets a new property/value pair into the info file, or updates
   * an existing property.
   *
   * @param $key
   *   The key name representing the property.
   * @param $value
   *   The value of the property.
   */
  function set($key, $value) {
    if (!$this->contents) {
      throw new Exception('Info object is not initialized.');
    }
    $this->contents[$key] = $value;
  }
}

/**
 * Generates an info file that can be parsed by drupal_parse_info_file.
 *
 * TODO: Either rename this functions to be in the correct namespace, or
 * delete it from this file if it is committed to Drupal core (see
 * http://drupal.org/node/537332#comment-1877348).
 *
 * @param array $info_array what is returned from drupal_parse_info_file()
 * @return string
 */
function drupal_info_file_to_string($info_array) {
  $output = "";
  foreach ($info_array as $key => $value) {
    if (is_array($value)) {
      $output .= "\n";
      $output .= drupal_info_block_to_string($value, $key);
    }
    else {
      $output .= sprintf("%s = '%s'\n", $key, str_replace("'", "\'", $value));
    }
  }
  return $output;
}

/**
 * Helper function for drupal_info_file_to_string.
 * @see drupal_info_file_to_string
 *
 * TODO: Either rename this functions to be in the correct namespace, or
 * delete it from this file if it is committed to Drupal core (see
 * http://drupal.org/node/537332#comment-1877348).
 *
 * @param array $block Configuration block to write.
 * @param string $prefix A string to prefix each entry with.
 * @return string
 */
function drupal_info_block_to_string($block, $prefix) {
  $output = '';
  $is_assoc = array_keys($block) !== range(0, count($block) - 1);
  foreach ($block as $key => $value) {
    if (!$is_assoc) {
      $key = ''; // Wipe out the key if this is an iterative array.
    }
    if (is_array($value)) {
      $output .= drupal_info_block_to_string($value, $prefix . "[$key]");
    }
    else {
      $output .= sprintf("%s[%s] = '%s'\n", $prefix, $key, str_replace("'", "\'", $value));
    }
  }
  return $output;
}
