<?php

define('DRUPAL_ROOT', getcwd());
include_once (DRUPAL_ROOT . '/sites/g/sites.inc');

$unsed_modules = new unusedModules();
$unsed_modules->parseUnusedModules();

class unusedModules {

  public function parseUnusedModules() {

    $output = '';
    $site_list = $this->sitesList();
    foreach ($site_list as $key => $site_url) {
      $output .= '=======================' . $key . '=====================';
      $output .= "\n";
      $output .= $this->getDrushPMlist($key);
      //echo "<pre>$output</pre>";
      $output .= "\n";
      $output .= "\n";
    }
    file_put_contents('./pm-unused-module-list_' . date("j.n.Y") . '.txt', $output, FILE_APPEND);
  }

  /**
   * unsed and disabled modules
   * @param type $sites
   * @return type
   */
  public static function getDrushPMlist($sites = NULL) {
    $uri = NULL;
    if (!empty($sites)) {
      $uri = ' --uri=' . $sites;
    }
    $output = shell_exec('drush pm-list --type=Module —no-core --pipe --status="disabled,not installed"' . $uri);
    return $output;
  }

  /**
   * @return Sites List
   */
  public static function sitesList() {
    $site_list = gardens_site_data_load_file();
    return $site_list['sites'];
  }

}

?>
