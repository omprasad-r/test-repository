<?php
/*
 * class ThemeBuilderConfig
 */

class ThemeBuilderConfig {
  
  private $conf;
  
  function __set($name, $value) {
    $this->conf[$name] = $value;
  }
  
  function __get($name) {
    return $this->conf[$name];
  }
  
  public static function get() {
    static $config;
    if (!$config) {
      $config =  new ThemeBuilderConfig();
    }
    return $config;
  }
}
?>