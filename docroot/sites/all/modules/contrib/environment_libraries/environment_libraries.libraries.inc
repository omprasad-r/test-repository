<?php
/**
 * @file
 * Libraries API integration.
 */

// # Libraries API Integration
module_load_include('inc', 'environment_libraries', 'environment_libraries.class');
use Environment_Libraries\LibraryUtils;

/**
 * Define the Drupal libraries from the environment_libraries_library entities.
 *
 * Implements hook_library().
 */
function environment_libraries_library() {
  // ## Create Libraries
  // Expose libraries defined in ui as Drupal libraries, this allows them to be
  // used as dependencies. However this does create a risk of missing
  // dependencies if they are removed from the UI. It is probably best to use
  // features if you plan on using environment_libraries as dependencies so they
  // exist in code.
  $libraries = array();
  $environments = LibraryUtils::get_environments();
  $environment_current = LibraryUtils::current_environment();

  // TODO cache libraries list?
  // TODO clear on new library add/save or environment change.
  $environment_libraries = environment_libraries_library_load_multiple();

  foreach ($environment_libraries as $library) {
    $library_file_count = LibraryUtils::count_files($library);
    $lib = $library->machine_name;
    // Create some empty values for unused properties.
    $libraries[$lib] = array(
      'title' => $library->title,
      'version' => NULL,
      'website' => NULL,
      'variants' => array(),
    );

    $library_config = $library->config;
    $library_env = NULL;
    // ### Get the current environment setting.
    if (isset($library->environment) && $library->environment != "default") {
      $library_env = $library->environment;
      $env = $library_env;
    }
    else {
      $env = $environment_current;
    }

    // ### Add Environment files
    // For each file in this library,
    for ($f = 0; $f < $library_file_count; $f++) {
      // if there is a file specified
      if (!isset($library->config['files']) || empty($library->config['files'][$f])) {
        continue;
      }
      // add the library js / css file & options.
      // #### Get the file url of the library.
      $library_file = $library_config['files'][$f];
      if (isset($library_file[$env])) {
        $library_current = $library_file[$env];
      }
      else {
        // If there is no file for the current environment then fallback to a
        // higher environment by: copying the environment array,
        $_env = $environments;
        // Advancing $_env to the current environment,
        while (key($_env) !== $env) {
          if (!next($_env)) {
            break;
          }
        }
        // And then walking back up to the next highest defined environment.
        while (!isset($library_file[key($_env)])) {
          if (!prev($_env)) {
            break;
          }
        };
        $env = key($_env);
        $library_current = $library_file[$env];
      }
      $libraries[$lib]['environment'] = $library_env ?: $env;

      // #### Check file options.
      if ($library_current['cache'] == 1) {
        // If the file is cached copy it locally;
        $library_url = environment_libraries_cache($library_current['url'], $library->lid);
        // TODO optionally minify & aggregate the file.
        $_type = 'file';
      }
      else {
        // otherwise, use the external url.
        $library_url = $library_current['url'];
        $_type = 'external';
      }

      // ### Create the Drupal library entry
      $libraries[$lib][$library_file['type']][$library_url] = array(
        'type' => $_type,
        'scope' => $library_file['region'],
        'every_page' => $library_current['aggregate'] == 1 ? TRUE : FALSE,
        'cache' => $library_current['cache'] == 1 ? TRUE : FALSE,
        'weight' => $library_file['weight'],
      );

      // ### Create variants for each environment on the library
      foreach ($environments as $variant_env => $v) {
        // Only create variants for environments w/ urls.
        if (!isset($library_config['files'][$f][$variant_env]) || !isset($library_config['files'][$f][$variant_env]['url'])) {
          continue;
        }
        $variant_file = $library_config['files'][$f][$variant_env];

        // Get the url & filetype the same as before.
        if ($variant_file['cache'] == 1) {
          $variant_url = environment_libraries_cache($variant_file['url'], $library->lid);
          $variant_type = 'file';
        }
        else {
          $variant_url = $variant_file['url'];
          $variant_type = 'external';
        }
        // Create the variant, only setting properties to override.
        $libraries[$lib]['variants'][$variant_env][$library_file['type']][$variant_url] = array(
          'type' => $variant_type,
          'every_page' => $variant_file['aggregate'] == 1 ? TRUE : FALSE,
          'cache' => $variant_file['cache'] == 1 ? TRUE : FALSE,
          'environment' => $variant_env,
          'scope' => $library_file['region'],
          'weight' => $library_file['weight'],
        );
      }
    }
    $libraries[$lib]['dependencies'] = environment_libraries_get_dependencies($library);
  }

  return $libraries;
}

/**
 * Build library dependencies from environment_libraries_library entity.
 */
function environment_libraries_get_dependencies($library) {
  // ## Get the dependencies for the library.
  $dependencies = array();
  if (!empty($library->dependencies)) {
    $deps_list = explode(',', $library->dependencies);
    foreach ($deps_list as $dep) {
      $dep_parts = explode('::', $dep);
      // By default, any dependencies without a module will be assumed to come
      // from environment_libraries.
      $dep_mod = 'environment_libraries';
      if (count($dep_parts) == 2) {
        $dep_mod = $dep_parts[1];
        $dep_lib = $dep_parts[0];
      }
      else {
        $dep_lib = $dep;
      }
      if ($dep_mod != $library->machine_name) {
        $dependencies[] = array($dep_mod, $dep_lib);
      }
      else {
        // Warn of self-depencency.
        drupal_set_message(t("The Environment Library @dep_mod has an invalid depencency on itself.", array('@dep_mod' => $dep_mod)), 'error');
      }
    }
  }
  return $dependencies;
}

/**
 * Update a library with a variant override.
 */
function _environment_libraries_load_variant(&$library, $variant = NULL) {
  $environments = LibraryUtils::get_environments();
  if (isset($variant)) {
    $env = NULL;
    // ## find the closest specified variant...
    $_env = $environments;
    // Advancing $_env to the current environment,
    while (key($_env) !== $variant) {
      if (!next($_env)) {
        break;
      }
    }
    // and then walking back up to the next highest defined environment.
    while (!isset($library['variants'][key($_env)])) {
      if (!prev($_env)) {
        break;
      }
    };
    $env = key($_env) ?: NULL;
    // ## If we have a matching variant then
    if (isset($env)) {
      // merge in the variant settings.
      $library = array_merge($library, $library['variants'][$env]);
      $library['environment'] = $env;
    }

  }
}

/**
 * A mashup of drupal_add_library and libraries_load to support adding libraries with variants.
 *
 * Copied from drupal_add_library.
 *
 * @see https://api.drupal.org/api/drupal/includes%21common.inc/function/drupal_add_library/7
 * Here there be dragons.
 */
function environment_libraries_load($module, $name, $environment = NULL, $every_page = NULL) {
  // ## Add the Library js / css Files
  // $added = &drupal_static(__FUNCTION__, array());
  // Use the same static cache as drupal_add_library to prevent duplicates from
  // environment_libraries_load and drupal_add_library
  $added = &drupal_static('drupal_add_library', array());
  if ($environment == "default") {
    $environment = NULL;
  }
  if ($environment == NULL) {
    $environment = LibraryUtils::current_environment();
  }
  // ### Load a library from a module with an optional forced environment
  // Only process the library if it exists and it was not added already.
  if (!isset($added[$module][$name])) {
    if ($library = drupal_get_library($module, $name)) {
      $dependencies = $library['dependencies'] ?: array();
      foreach ($dependencies as $dep) {
        environment_libraries_load($dep[0], $dep[1], $environment, $every_page);
      }
      // Find the version for this environment or the closest fallback
      _environment_libraries_load_variant($library, $environment);

      // and add the library's JS and css.
      $elements['#attached'] = array(
        'library' => $library['dependencies'],
        'js' => $library['js'],
        'css' => $library['css'],
      );
      // @see https://api.drupal.org/api/drupal/includes%21common.inc/function/drupal_process_attached/7
      $added[$module][$name] = drupal_process_attached($elements, JS_DEFAULT, TRUE, $every_page);
    }
    else {
      // The requested library does not exist.
      $added[$module][$name] = FALSE;
    }
  }

  return $added[$module][$name];
}

/**
 * Same as environment_libraries_load matching drupal_add_library syntax.
 *
 * @see https://api.drupal.org/api/drupal/includes%21common.inc/function/drupal_add_library/7
 */
function environment_libraries_add_library($module, $name, $every_page = NULL, $environment = NULL) {
  return environment_libraries_load($module, $name, $environment, $every_page);
}
