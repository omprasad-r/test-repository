<?php

include_once dirname(__FILE__) . '/../asset.inc';

class HtaccessAsset extends ThemebuilderAssetNew {
  private $contents = NULL;

  function renderString() {
    return <<<EOF
<IfModule mod_headers.c>
  <FilesMatch ".css$">
    Header set Cache-Control "no-cache, no-store, must-revalidate, max-age=0"
  </FilesMatch>
</IfModule>

EOF;
  }

  public static function loadFromFile($filename) {
    themebuilder_log_trace_enter(__METHOD__);
    $ret = new self();
    $ret->contents = '';
    if (file_exists($filename)) {
      $ret->contents = file_get_contents($filename);
    }
    themebuilder_log_trace_exit(__METHOD__);
    return $ret;
  }
}
