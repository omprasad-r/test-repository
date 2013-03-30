<?php
// $Id$

/**
 * Implements hook_install_tasks_alter().
 */
function gardens_verification_install_tasks_alter(&$tasks, $install_state) {
  // Perform the same modifications as the Gardens install profile.
  include_once DRUPAL_ROOT . '/profiles/gardens/gardens.profile';
  gardens_install_tasks_alter($tasks, $install_state);
  // And then perform some more of our own.
  $tasks['install_profile_modules']['function'] = 'gardens_verification_install_profile_modules';
}

/**
 * Installation task; install the default Gardens modules.
 *
 * We use this function to add Gardens modules to the batch installation,
 * rather than including them as dependencies in this profile's .info file,
 * because we do not want to have to change our .info file each time the main
 * Gardens profile is updated to include a new module.
 */
function gardens_verification_install_profile_modules(&$install_state) {
  // Get the initial batch for this profile and a list of all possible modules.
  $batch = install_profile_modules($install_state);
  $files = system_rebuild_module_data();

  // Install this profile's dependencies, then the dependencies of the main
  // Gardens profile we'll be building off of, and finally the profile itself.
  // Note: This assumes the profile .info file orders the modules in the
  // correct dependency order, which Drupal no longer actually requires.
  $gardens_info = install_profile_info('gardens');
  $modules = array_values(array_unique(array_merge($install_state['profile_info']['dependencies'], $gardens_info['dependencies'], array('gardens_verification'))));
  $batch['operations'] = array();
  foreach ($modules as $module) {
    $batch['operations'][] = array('_install_module_batch', array($module, $files[$module]->info['name']));
  }
  return $batch;

// Removed: We no longer need to install every single module in this profile.

/*
  // Load the Scarecrow module (even though it isn't installed yet). This is
  // for the sole purpose of calling scarecrow_disallowed_modules() below.
  drupal_load('module', 'scarecrow');

  // Next list every other module that Gardens users might choose to turn on.
  // (Do this as a separate step from the above, since the Gardens modules
  // might need to be installed in a specific order that is contained within
  // the 'dependencies' array.)
  foreach ($files as $module => $module_data) {
    if (
        // Skip modules already in the above lists.
        !in_array($module, $gardens_info['dependencies']) &&
        !in_array($module, $gardens_verification_info['dependencies']) &&
        // Skip already-installed and hidden modules.
        empty($module_data->status) &&
        empty($module_data->info['hidden']) &&
        // Skip incompatible modules.
        $module_data->info['core'] == '7.x' &&
        // Skip forbidden modules.
        !in_array($module, scarecrow_disallowed_modules()) &&
        // Skip this profile "module" and any modules it contains.
        strpos($module_data->filename, 'gardens_verification') === FALSE
      ) {
      $batch['operations'][] = array('_install_module_batch', array($module, $module_data->info['name']));
    }
  }
*/
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function gardens_verification_form_install_configure_form_alter(&$form, $form_state) {
  // Perform the same modifications as the Gardens install profile.
  include_once DRUPAL_ROOT . '/profiles/gardens/gardens.profile';
  gardens_form_install_configure_form_alter($form, $form_state);

  // Default to installing the product site template.
  $form['acquia_gardens']['site_template']['#default_value'] = 'product';

// We no longer need these custom options since the product template fills most
// block regions on the page for us, and the others we can do manually.

/*
  // Add our own custom options.
  $form['acquia_gardens_development']['acquia_gardens_advanced_block_mode'] = array(
    '#type' => 'checkbox',
    '#title' => t('Create sample blocks in all possible regions'),
    '#description' => t('Selecting this will install the site with at least two blocks created in every region on the page. Otherwise, a more limited set of blocks will be created.'),
    '#default_value' => FALSE,
  );
*/

  $form['#submit'][] = 'gardens_verification_configure_form_submit';

  // Optimize for developers.
  $form['acquia_gardens_development']['acquia_gardens_basic_developer_features']['#default_value'] = TRUE;
}

/**
 * Custom submit handler for this profile's site configuration options.
 */
function gardens_verification_configure_form_submit($form, &$form_state) {
  gardens_verification_fill_block_regions();
  gardens_verification_add_style_guide();

// We no longer need these custom options since the product template fills most
// block regions on the page for us, and the others we can do manually.
/*
  module_load_install('gardens_verification');
  if (empty($form_state['values']['acquia_gardens_advanced_block_mode'])) {
    gardens_verification_configure_limited_blocks();
  }
  else {
    gardens_verification_configure_all_blocks();
  }
*/
}

// We no longer need the function below since the product template fills most
// block regions on the page for us, and the others we can do manually.

/**
 * Implements hook_block_view_alter().
 */
/*
function gardens_verification_block_view_alter(&$data, $block) {
  // Make sure that empty blocks will still appear on the page.
  if (empty($data['content'])) {
    $data['content'] = array('#markup' => t('This is the @delta block provided by the @module module.', array('@delta' => $block->delta, '@module' => $block->module)));
  }
}
*/

/**
 * Manually add a block to each page region that doesn't have one.
 */

function gardens_verification_fill_block_regions() {
  module_load_install('gardens_verification');
  gardens_verification_configure_all_blocks(1, TRUE);
}

/**
 * Add a page node with HTML for the style guide.
 */
function gardens_verification_add_style_guide() {
  $content = <<<EOF
<p>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris sed nisi ut odio dignissim tristique. Suspendisse tempor mattis justo, et rhoncus nulla semper facilisis. In hac habitasse platea dictumst. Maecenas dignissim scelerisque massa in pellentesque</p><h1>Heading 1 - Lorem ipsum dolor sit amet, consectetur adipiscing elit. Heading 1 - Lorem ipsum dolor sit amet, consectetur adipiscing elit.</h1><p>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris sed nisi ut odio dignissim tristique. Suspendisse tempor mattis justo, et rhoncus nulla semper facilisis. In hac habitasse platea dictumst. Maecenas dignissim scelerisque massa in pellentesque</p><h2>Heading 2 - Lorem ipsum dolor sit amet, consectetur adipiscing elit. Heading 2 - Lorem ipsum dolor sit amet, consectetur adipiscing elit.</h2><p>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris sed nisi ut odio dignissim tristique. Suspendisse tempor mattis justo, et rhoncus nulla semper facilisis. In hac habitasse platea dictumst. Maecenas dignissim scelerisque massa in pellentesque</p><h3>Heading 3 - Lorem ipsum dolor sit amet, consectetur adipiscing elit. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Lorem ipsum dolor sit amet, consectetur adipiscing elit.</h3><p>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris sed nisi ut odio dignissim tristique. Suspendisse tempor mattis justo, et rhoncus nulla semper facilisis. In hac habitasse platea dictumst. Maecenas dignissim scelerisque massa in pellentesque</p><h4>Heading 4 - Lorem ipsum dolor sit amet, consectetur adipiscing elit. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Lorem ipsum dolor sit amet, consectetur adipiscing elit.</h4><p>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris sed nisi ut odio dignissim tristique. Suspendisse tempor mattis justo, et rhoncus nulla semper facilisis. In hac habitasse platea dictumst. Maecenas dignissim scelerisque massa in pellentesque</p><h5>Heading 5 - Lorem ipsum dolor sit amet, consectetur adipiscing elit. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Lorem ipsum dolor sit amet, consectetur adipiscing elit.</h5><p>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris sed nisi ut odio dignissim tristique. Suspendisse tempor mattis justo, et rhoncus nulla semper facilisis. In hac habitasse platea dictumst. Maecenas dignissim scelerisque massa in pellentesque</p><blockquote>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris sed nisi ut odio dignissim tristique. Suspendisse tempor mattis justo, et rhoncus nulla semper facilisis. In hac habitasse platea dictumst. Maecenas dignissim scelerisque massa in pellentesque<cite>Me!</cite></blockquote><dl><dt>Do</dt><dd>A deer, a female deer</dd><dt>Re</dt><dd>A drop of golden sun</dd><dt>Mi</dt><dd>A name I call myself</dd><dt>Fa</dt><dd>A long, long way to run!</dd></dl><ol><li>Apples</li><li>Oranges</li><li>Pears<ol><li>Apples</li><li>Oranges</li><li>Pears</li><li>Bananas<ol><li>Apples</li><li>Oranges</li><li>Pears</li><li>Bananas</li><li>Shoe Polish</li></ol></li><li>Shoe Polish</li></ol></li><li>Bananas</li><li>Shoe Polish</li></ol><ul><li>Apples<ul><li>Apples<ul><li>Apples</li><li>Oranges</li><li>Pears</li><li>Bananas</li><li>Shoe Polish</li></ul></li><li>Oranges</li><li>Pears</li><li>Bananas</li><li>Shoe Polish</li></ul></li><li>Oranges</li><li>Pears</li><li>Bananas</li><li>Shoe Polish</li></ul>
<pre>function f = x.n();
</pre>
<table><thead><tr><th>hello</th><th>hello</th><th>hello</th></tr></thead><tbody><tr><td>Lorem</td><td>Ipsum</td><td>Dolor</td></tr><tr><td>Lorem</td><td>Ipsum</td><td>Dolor</td></tr><tr><td>Lorem</td><td>Ipsum</td><td>Dolor</td></tr><tr><td>Lorem</td><td>Ipsum</td><td>Dolor</td></tr></tbody></table>
EOF;

  $full_html = db_query("SELECT format FROM {filter_format} WHERE name = 'Full HTML'")->fetchField();
  $data = array(
    'title' => 'Style guide',
    'body' => $content,
    'format' => $full_html,
    'menu_link' => array(
      'menu_name' => 'main-menu',
      'link_title' => 'Style guide',
      'weight' => 50,
    ),
    'alias' => 'style-guide',
  );
  site_template_add_basic_node($data);
}
