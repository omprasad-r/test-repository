<?php

define('DRUPAL_ROOT', getcwd());
include_once (DRUPAL_ROOT . '/sites/g/sites.inc');
$log_path = drush_get_option('log-path');
$log_path_exists = !empty($log_path) ? $log_path : NULL;


while ($arg = drush_shift()) {
        //  drush_print($arg);
        }

$unsed_modules = new unusedModules();
$unsed_modules->parseUnusedModules($log_path_exists);

class unusedModules {

  public function parseUnusedModules($log_file_location) {

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
    if($log_file_location) {
      file_put_contents($log_file_location . '/pm-unused-module-list_' . date("j.n.Y") . '.txt', $output, FILE_APPEND);
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
