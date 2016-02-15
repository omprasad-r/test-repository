<?php
/**
 * @file
 * Define form environment_library elements
 */

// # Custom Form Element
// @see https://www.drupal.org/node/169815
module_load_include('inc', 'environment_libraries', 'environment_libraries.class');
use Environment_Libraries\LibraryUtils;

/**
 * Define the environment_libraries_libraries form field elements.
 */
function environment_libraries_element_info() {
   $params = drupal_get_query_parameters(arg());
  // ### Setup the field schema
   $readonly_machine_name = NULL;
   $add_library = 'add';
   if ($params[4] != 'add') {
     $readonly_machine_name = array('readonly' => 'readonly');
     $add_library = 'edit';
    // echo '<pre>'; print_r($readonly_machine_name); die;
   }
  $environment_library = array(
    '#input' => TRUE,
    '#tree' => TRUE,
    '#theme' => array('environment_library'),
    '#theme_wrappers' => array('form_element'),
    "#title_display" => 'before',
    '#process' => array('environment_libraries_environment_library_process'),
    '#element_validate' => array('environment_libraries_environment_library_validate'),
        // sub-elements
    'name' => array(
      '#type' => 'textfield',
      '#title' => t('Library Name'),
      '#placeholder' => t('Library Name'),
      '#required' => TRUE,
    ),
    
    'machine_name' => array(
      '#type' => 'machine_name',
      '#title' => t('Machine Name'),
      '#description' => '',
      '#placeholder' => t('library_name'). $readonly_machine_name,
      '#size' => 30,
      '#attributes' => $readonly_machine_name,
      '#required' => TRUE,
      '#machine_name' => array(
        'exists' => 'environment_libraries_machinename_exists',
        'source' => array('name', $add_library),
      ),
      
    ),
    'environment' => array(
      '#type' => 'select',
      '#title' => t('Environment'),
      '#options' => array(
        'default' => t('Default'),
      ),
      '#default_value' => 'default',
      '#attributes' => array(
        'title' => t('Force this library to use the selected environment.'),
      ),
    ),
    'dependencies' => array(
      '#type' => 'select',
      '#multiple' => TRUE,
      '#title' => t('Dependencies'),
      '#options' => array(),
      '#chosen' => TRUE,
      '#placeholder' => t('(Optional) Select library dependencies'),
    ),
    'expose_block' => array(
      '#type' => 'checkbox',
      '#title' => t('Expose block'),
      '#description' => '<em>(Optional)</em> It is recommended to add libraries via <a href="https://www.drupal.org/project/context" target="_blank">context</a> but libraries can also be exposed as blocks.',
    ),
    'description' => array(
      '#type' => 'textarea',
      '#title' => t('Description'),
      '#placeholder' => t('Describe this library'),
      '#rows' => 5,
      '#required' => FALSE,
    ),
    '#attributes' => array(
      'class' => array('form-item-type-environment-library'),
    ),
  );

  // ### Specify Dependencies
  $module_dependencies = explode(',', variable_get('environment_libraries_module_dependencies', 'environment_libraries,system'));
  foreach ($module_dependencies as $module_name) {
    $module_name = trim($module_name);
    $mod_libs = drupal_get_library($module_name);
    foreach ($mod_libs as $lib_name => $library) {
      $environment_library['dependencies']['#options'][$lib_name . '::' . $module_name] = $lib_name . '::' . $module_name;
      // t($library['title']);
    }
  }

  // ### Add Environment Options
  $environments = LibraryUtils::get_environments();
  $environment_library['environment']['#options'] = array_merge($environment_library['environment']['#options'], $environments);
  // How many files should each library expose?
  // ### Add Files.
  $environment_library['files'] = array(
    '#type' => 'fieldset',
        // '#title' => t('Files'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
  );
  $file_count = 1;
  for ($f = 0; $f < $file_count; $f++) {
    $environment_library['files'][$f] = environment_libraries_file_element($f);
  }

  return array('environment_library' => $environment_library);
}

/**
 * ## File form elements.
 */
function environment_libraries_file_element($f) {
  $file = array(
    '#type' => 'fieldset',
    '#title' => t('File #') . ($f + 1),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#attributes' => array('class' => array('environment-library--file')),
    '#tree' => TRUE,
  );
  $environments = environment_libraries_environments();
  $last_env = NULL;
  foreach ($environments as $env => $environment) {
    $environment_lcase = strtolower($environment);
    $file[$env] = array(
      'url' => array(
        '#type' => 'textfield',
        // '#title' => t('Url'),
        // '#title_display' => 'before',
        '#size' => 120,
        '#maxlength' => 1024,
        '#attributes' => array('placeholder' => t("Enter a url on the $environment_lcase environment" . (isset($last_env) ? " or fallback to $last_env" : ''))),
        '#prefix' => "<div class=\"url-wrapper\"><span class=\"environment--tag $env\" >$env</span>",
        '#suffix' => '</div>',
      ),
      'cache' => array(
        '#type' => 'checkbox',
        '#title' => t('cache'),
        // '#description' => 'cache files locally',
        '#attributes' => array(
          'title' => t('Serve the file locally instead of, required for aggregation & minification'),
        ),
      ),
      // The aggregate option sets every_page flag in the library options.
      'aggregate' => array(
        '#type' => 'checkbox',
        '#title' => t('global'),
        '#attributes' => array(
          'title' => t('Combine this file along with others used on every page in the aggregated css/js'),
        ),
      ),
      // TODO add minification support.
      'minify' => array(
        '#type' => 'checkbox',
        '#title' => t('minify'),
        '#attributes' => array(),
        // '#description' => 'cache files locally',
        '#attributes' => array(
          'title' => t('Use UglifyJS to minify the file.'),
        ),
      ),
    );
    $last_env = $environment_lcase;
  }

  $file['region'] = array(
    '#type' => 'select',
    '#title' => t('Region'),
    '#options' => array(
      'header' => t('Header'),
      'footer' => t('Footer'),
    ),
    '#default_value' => 'header',
  );

  // TODO weight not rendered via AJAX.
  $file['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => 0,
  );

  return $file;
}

/**
 * Validate the library machine_name.
 */
function environment_libraries_machinename_exists($name, $add_library) {
  if ($add_library == 'add') {
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'environment_libraries_library')
        ->propertyCondition('machine_name', $name);
    $result = $query->execute();
    if (isset($result['environment_libraries_library'])) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
  else {
    return FALSE;
  }
}
