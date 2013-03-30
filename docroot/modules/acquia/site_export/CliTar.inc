<?php

/**
 * Creates .tar or .tar.gz files using cli tar.
 *
 * @author jacob
 */
class CliTar {

  public $tarFile;
  public $options = array();
  public $mode = 'c';
  // z, j or ''
  public $compress_mode = '';
  private $template = 'tar -%mode%%compress_mode%f %tarFile% %options% %paths%';
  public $paths = array();

  function __construct($tarFile) {
    $this->tarFile = $tarFile;
  }

  static function getVersion() {
    $output = '';
    exec('tar --version', $output);
    $version_line = $output[0];
    preg_match('/^(.*?) (?:|\(.+\)) ([0-9a-z\.-]+)/', $version_line, $reg);
    if (count($reg) == 3) {
      return array('type' => $reg[1], 'version' => $reg[2]);
    }
  }

  function setMode($mode = 'c') {
    $this->mode = $mode;
  }

  function setOption($option, $value, $multiple = FALSE) {
    if ($multiple) {
      $this->options[$option][] = $value;
    } 
    else {
      $this->options[$option] = $value;
    }
  }

  function add($path) {
    $this->paths[$path] = $path;
  }

  function optionsToStr($options) {
    $output = '';
    foreach ($options as $option => $value) {
      if (is_array($value)) {
        foreach ($value as $foo) {
          $output .= $this->optionsToStr(array($option => $foo));
        }
      }
      else {
        $output .= $option;
        if ($this->isLongOption($option) && $value) {
          $output .= '=';
        }
        else {
          $output .= ' ';
        }
        if ($value) {
          $output .= escapeshellarg($value) . ' ';
        }
      }
    }
    return $output;
  }

  private function isLongOption($str) {
    if (strpos($str, '--') === 0) {
      return TRUE;
    }
  }

  public function __toString() {
    $replacements = array(
      '%tarFile%' => $this->tarFile,
      '%mode%' => $this->mode,
      '%compress_mode%' => $this->compress_mode,
      '%options%' => $this->optionsToStr($this->options),
      '%paths%' => implode(' ', $this->paths),
    );
    return strtr($this->template, $replacements);
  }
}