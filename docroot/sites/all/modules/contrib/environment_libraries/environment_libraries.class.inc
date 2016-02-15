<?php
/**
 * @file
 * Utility functions
 */

// # Utility Functions
namespace Environment_Libraries;
/**
 * Class LibraryUtils
 *
 * @package Environment_Libraries
 */
class LibraryUtils {
  // ## Default environments
  // These should probably be overridden in a custom module via
  // `hook_environment_libraries_environments_alter`.
  // **Note:** Environments should be defined in highest to lowest priority
  // (production => staging => dev); the first environment will be considered
  // the default / production environment.
  public static $default_environments = array(
    'prod' => 'Production',
    'stage' => 'Staging',
    'dev' => 'Development',
  );

  /**
   * Get a list of the available environments.
   *
   * @return array
   *   An array of the environments used by the site.
   */
  public static function get_environments() {
    // ## Get the list of environments.
    $environments = &drupal_static(__FUNCTION__, array());
    if (empty($environments)) {
      $environments = LibraryUtils::$default_environments;
      // Environments can be overridden by modules using a hook_alter function.
      drupal_alter('environment_libraries_environments', $environments);
    }
    return $environments;
  }

  /**
   * Get the current environment.
   *
   * @param $library
   *   A library which may be set to an explicit environment
   *
   * @returns string
   *   The current environment name.
   */
  public static function current_environment($library = NULL) {
    // ## Get the current environment
    if (isset($library) && !empty($library->environment)) {
      // If the library has a specified environment then use that.
      $environments = environment_libraries_environments();
      if (in_array($library->environment, array_keys($environments))) {
        return $library->environment;
      }
    }
    // The environment defaults to production.
    $environment = variable_get('environment_libraries_environment_current', 'prod');
    // Allow any custom modules to override the current environment.
    drupal_alter('environment_libraries_environment_current', $environment);

    // Only users with this permission can view non-production libraries.
    if ($environment != LibraryUtils::default_environment() && !user_access('view non-production environment_libraries')) {
      $environment = LibraryUtils::default_environment();
    }
    return $environment;
  }

  /**
   * Get the default environment, generally production that will be used as a fallback.
   */
  public static function default_environment() {
    // Get the first environment in the assoc array.
    // TODO should there be some other source of production environment(s)?
    $env_default = &drupal_static(__FUNCTION__, NULL);
    if (isset($env_default)) {
      return $env_default;
    }
    $environments = LibraryUtils::get_environments();
    reset($environments);
    $env_default = key($environments);
    return $env_default;
  }

  /**
   * Check if a file has any set urls.
   *
   * @param array $file
   *   Environment / file settings.
   *
   * @return bool
   *   If the library file url is set.
   */
  public static function is_empty_file(array $file) {
    $environments = LibraryUtils::get_environments();
    // Check to see if there are any urls set for any environment.
    foreach ($environments as $env => $environment) {
      if (!empty($file[$env]['url'])) {
        return TRUE;
      }
    }
  }

  /**
   * Returns the number of files in the library entity / element
   *
   * @param $library
   *   The environment_library entity.
   * @param array $element
   *   The library form element.
   *
   * @return int
   *   The number of files in the library.
   */
  public static function count_files($library, $element = NULL) {
    // ## Count files for library form element
    if (isset($element)) {
      // ### If the element exists then it is being edited
      $file_count = 0;
      $environments = LibraryUtils::get_environments();
      foreach (element_children($element['files']) as $i => $file) {
        // then count how many files have urls;
        foreach ($environments as $env => $environment) {
          if (!empty($file[$i][$env]['url'])) {
            $file_count++;
            break;
          }
        }
      }
      return max($file_count, 1);
    }
    // otherwise, check the library default state.
    $lib_file_counts = drupal_static(__FUNCTION__, array());
    if (empty($library->lid)) {
      return 1;
    }
    elseif (empty($lib_file_counts[$library->lid])) {
      $file_count = 1;
      if (!empty($library->config) && !empty($library->config['files'])) {
        $file_count = count($library->config['files']);
      }
      $lib_file_counts[$library->lid] = $file_count;
    }
    return $lib_file_counts[$library->lid];
  }

  /**
   * Get a list of the files used by this library in order to delete them on cron / cache clear.
   *
   * @param $library
   *   The environment_library entity.
   *
   * @return array
   *   The files in the environment_library.
   */
  public static function list_files($library) {
    // ## Find the file urls set for the library.
    $environments = LibraryUtils::get_environments();
    $environment_current = LibraryUtils::current_environment();

    $current_files = array();
    // Check if the library has an explicit environment override.
    if (isset($library->environment) && $library->environment != "default") {
      $env = $library->environment;
    }
    else {
      $env = $environment_current;
    }

    foreach ($library->config['files'] as $library_file) {
      if (isset($library_file[$env])) {
        $current_files[] = $library_file[$env];
      }
      else {
        // If there is no file for the current environment then fallback to a higher environment by:
        // copying the environment array,
        $_env = $environments;
        // advancing $_env to the current environment,
        while (key($_env) !== $env) {
          if (!next($_env)) {
            break;
          }
        }
        // and then walking back up to the next highest defined environment.
        while (!isset($library_file[key($_env)])) {
          if (!prev($_env)) {
            break;
          }
        };
        $current_files[] = $library_file[key($_env)];
      }
    }
    return $current_files;
  }

  /**
   * Parse the file url by adding a protocol if scheme-less.
   *
   * @param $url
   *   The file url.
   *
   * @return mixed
   *   The parsed url.
   */
  public static function parse_url($url) {
    // Allow scheme-less urls by adding an http(s):
    if (strpos($url, '//') === 0) {
      $protocol = stripos($_SERVER['SERVER_PROTOCOL'], 'https') === TRUE ? 'https:' : 'http:';
      $url = $protocol . $url;
    }
    return parse_url($url);
  }

  /**
   * @param $url
   *   The file url.
   *
   * @return string
   *   The url with a protocol.
   */
  public static function http_url($url) {
    // Allow scheme-less urls by adding an http.
    if (strpos($url, '//') === 0) {
      $url = 'http:' . $url;
    }
    return $url;
  }

}
