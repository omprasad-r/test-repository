<?php

/**
 * Implement hook_preprocess_html().
 */
function sprouts_preprocess_html(&$vars) {
  $vars['classes_array'][] = 'theme-markup-2';
  $vars['classes_array'][] = _sprouts_get_layout();
  
  $theme_path = path_to_theme();
  // Knock out core stylesheets
  _sprouts_clean_stylesheets();
  // Add CSS
  // Give IE6 and below a basic typography stylesheet. No need to worry about this browser any further
  drupal_add_css('http://universal-ie6-css.googlecode.com/files/ie6.0.3.css', array('type' => 'external', 'group' => CSS_THEME, 'media' => 'all', 'browsers' => array('IE' => 'IE 6', '!IE' => FALSE),));
  // Screen and print
  drupal_add_css($theme_path . '/css/screen.css', array('group' => CSS_THEME, 'media' => 'screen, handheld, projection, tv', 'browsers' => array('IE' => 'gte IE 7', '!IE' => true),));
  drupal_add_css($theme_path . '/css/print.css', array('group' => CSS_THEME, 'media' => 'print', 'browsers' => array('IE' => 'gte IE 7', '!IE' => true),));
  // Aural and tactile
  drupal_add_css($theme_path . '/css/aural.css', array('group' => CSS_THEME, 'media' => 'aural, speech, tty',));
  drupal_add_css($theme_path . '/css/tactile.css', array('group' => CSS_THEME, 'media' => 'braille, embossed',));
  // Drupal core replacement stylesheets
  drupal_add_css($theme_path . '/css/drupal/system.menus.css', array('group' => CSS_THEME, 'media' => 'screen, handheld, projection, tv', 'browsers' => array('IE' => 'gte IE 7', '!IE' => true),));
  //drupal_add_css($theme_path . '/css/drupal/system.theme.css', array('group' => CSS_THEME, 'media' => 'screen, handheld, projection, tv', 'browsers' => array('IE' => 'gte IE 7', '!IE' => true),));
  // Typography
  drupal_add_css($theme_path . '/css/typography.css', array('group' => CSS_THEME, 'media' => 'all', 'browsers' => array('IE' => 'gte IE 7', '!IE' => true),));
  // Grid Fixed 960
  // drupal_add_css($theme_path . '/plugins/960grid/code/css/960_24_col.css', array('group' => CSS_THEME, 'media' => 'screen and (min-width : 960px), handheld and (min-width : 960px), projection and (min-width : 960px), tv and (min-width : 960px)', 'browsers' => array('IE' => 'gte IE 7', '!IE' => true),));
  // Grid Fluid
  drupal_add_css($theme_path . '/plugins/fluid960grid/css/grid.css', array('group' => CSS_THEME, 'media' => 'screen and (min-width : 960px), handheld and (min-width : 960px), projection and (min-width : 960px), tv and (min-width : 960px)', 'browsers' => array('IE' => 'gte IE 7', '!IE' => true),));
  // Plugin stylesheets
  drupal_add_css($theme_path . '/plugins/dev/wireframe.css', array('group' => CSS_THEME, 'media' => 'screen, handheld, projection, tv', 'browsers' => array('IE' => 'gte IE 7', '!IE' => true),));
  // drupal_add_css($theme_path . '/plugins/dev/typology.css', array('group' => CSS_THEME, 'media' => 'screen, handheld, projection, tv', 'browsers' => array('IE' => 'gte IE 7', '!IE' => true),));
  drupal_add_css($theme_path . '/plugins/semanticOutliner/outliner.css', array('group' => CSS_THEME, 'media' => 'screen, handheld, projection, tv', 'browsers' => array('IE' => 'gte IE 7', '!IE' => true),));
  // Theme style (colors, padding, font, etc)
  drupal_add_css($theme_path . '/css/five.css', array('group' => CSS_THEME, 'media' => 'screen, handheld, projection, tv', 'browsers' => array('IE' => true, '!IE' => true),));
  // IE7+
  drupal_add_css($theme_path . '/css/ie/ie.css', array('group' => CSS_THEME, 'media' => 'screen, handheld, projection, tv', 'browsers' => array('IE' => 'IE', '!IE' => FALSE), 'preprocess' => FALSE,));
  drupal_add_css($theme_path . '/css/ie/ie-7.css', array('group' => CSS_THEME, 'media' => 'screen, handheld, projection, tv', 'browsers' => array('IE' => 'IE 7', '!IE' => FALSE), 'preprocess' => FALSE,));
  drupal_add_css($theme_path . '/css/ie/ie-8.css', array('group' => CSS_THEME, 'media' => 'screen, handheld, projection, tv', 'browsers' => array('IE' => 'IE 8', '!IE' => FALSE), 'preprocess' => FALSE,));
  drupal_add_css($theme_path . '/css/ie/ie-lte-8.css', array('group' => CSS_THEME, 'media' => 'screen, handheld, projection, tv', 'browsers' => array('IE' => 'lte IE 8', '!IE' => FALSE), 'preprocess' => FALSE,));
  drupal_add_css($theme_path . '/css/ie/ie-9.css', array('group' => CSS_THEME, 'media' => 'screen, handheld, projection, tv', 'browsers' => array('IE' => 'IE 9', '!IE' => FALSE), 'preprocess' => FALSE,));
  // Add JavaScript
  // html5.js is required for IE to understand the new elements like article
  // @see http://remysharp.com/2009/01/07/html5-enabling-script/
  drupal_add_js($theme_path . '/js/html5.js', array('every_page' => TRUE, 'scope' => 'footer', 'group' => JS_THEME,));
  drupal_add_js($theme_path . '/js/basic.js', array('every_page' => TRUE, 'scope' => 'footer', 'group' => JS_THEME,));
  // jQuery
  drupal_add_library('system','ui.dialog');
  // Other plugins
  drupal_add_js($theme_path . '/plugins/mediaqueries/jquery.mediaqueries.js', array('every_page' => TRUE, 'scope' => 'footer', 'group' => JS_THEME,));
  drupal_add_js($theme_path . '/plugins/semanticOutliner/jquery.semanticOutliner.js', array('every_page' => TRUE, 'scope' => 'footer', 'group' => JS_THEME,));
  drupal_add_js($theme_path . '/plugins/selectivizr/selectivizr.js', array('every_page' => TRUE, 'scope' => 'footer', 'group' => JS_THEME,));
  // Modernizr tests an agent's capabilites and adds classes to the html tag representing them
  drupal_add_js($theme_path . '/plugins/Modernizr/modernizr.js', array('every_page' => TRUE, 'scope' => 'footer', 'group' => JS_THEME,));
  //kpr($vars);
}

/**
 * Implements hook_html_head_alter().
 */
function sprouts_html_head_alter(&$head_elements) {
  // If the theme's info file contains the custom theme setting
  // default_favicon_path, change the favicon <link> tag to reflect that path.
  if (($default_favicon_path = theme_get_setting('default_favicon_path')) && theme_get_setting('default_favicon')) {
    $favicon_url = file_create_url(path_to_theme() . '/' . $default_favicon_path);
  }
  else {
    if (module_exists('gardens_misc')) {
      $favicon_url = file_create_url(drupal_get_path('module', 'gardens_misc') . '/images/gardens.ico');
    }
  }
  if (!empty($favicon_url)) {
    $favicon_mimetype = file_get_mimetype($favicon_url);
    foreach ($head_elements as &$element) {
      if (isset($element['#attributes']['rel']) && $element['#attributes']['rel'] == 'shortcut icon') {
	$element['#attributes']['href'] = $favicon_url;
	$element['#attributes']['type'] = $favicon_mimetype;
      }
    }
  }
}

/**
* Implements hook_preprocess_page().
*/

function sprouts_preprocess_page(&$variables) {
  $is_front = $variables['is_front'];
  // Adjust the html element that wraps the site name. h1 on front page, p on other pages
  $variables['wrapper_site_name_prefix'] = ($is_front ? '<h1' : '<p');
  $variables['wrapper_site_name_prefix'] .= ' id="site-name"';
  $variables['wrapper_site_name_prefix'] .= ' class="site-name'.($is_front ? ' site-name-front' : '').'"';
  $variables['wrapper_site_name_prefix'] .= '>';
  $variables['wrapper_site_name_suffix'] = ($is_front ? '</h1>' : '</p>');
  // If the theme's info file contains the custom theme setting
  // default_logo_path, set the $logo variable to that path.
  $default_logo_path = theme_get_setting('default_logo_path');
  if (!empty($default_logo_path) && theme_get_setting('default_logo')) {
    $variables['logo'] = file_create_url(path_to_theme() . '/' . $default_logo_path);
  }
  else {
    $variables['logo'] = null;
  }
  
  //Arrange the elements of the main content area (content and sidebars) based on the layout class
  $layoutClass = _sprouts_get_layout();
  $layout = substr(strrchr($layoutClass, '-'), 1); //Get the last bit of the layout class, the 'abc' string
  
  $contentPos = strpos($layout, 'c');
  $sidebarsLeft = substr($layout,0,$contentPos);
  $sidebarsRight = strrev(substr($layout,($contentPos+1))); // Reverse the string so that the floats are correct.
  
  $sidebarsHidden = ''; // Create a string of sidebars that are hidden to render and then display:none
  if(stripos($layout, 'a') === false) { $sidebarsHidden .= 'a'; }
  if(stripos($layout, 'b') === false) { $sidebarsHidden .= 'b'; }
  
  $variables['sidebars']['left'] = str_split($sidebarsLeft);
  $variables['sidebars']['right'] = str_split($sidebarsRight);
  $variables['sidebars']['hidden'] = str_split($sidebarsHidden);
}

/**
 * Implement hook_preprocess_block().
 */
function sprouts_preprocess_block(&$vars) {
  $vars['content_attributes_array']['class'][] = 'content';
}

/**
 * Retrieves the value associated with the specified key from the current theme.
 * If the key is not found, the specified default value will be returned instead.
 *
 * @param <string> $key
 *   The name of the key.
 * @param <mixed> $default
 *   The default value, returned if the property key is not found in the current
 *   theme.
 * @return <mixed>
 *   The value associated with the specified key, or the default value.
 */
function _sprouts_variable_get($key, $default) {
  global $theme;
  $themes_info =& drupal_static(__FUNCTION__);
  if (!isset($themes_info[$theme])) {
    $themes_info = system_get_info('theme');
  }

  $value = $themes_info[$theme];
  foreach (explode('/', $key) as $part) {
    if (!isset($value[$part])) {
      return $default;
    }
    $value = $value[$part];
  }
  return $value;
}

/**
 * Returns the name of the layout class associated with the current path.  The
 * layout name is used as a body class, which causes the page to be styled
 * with the corresponding layout.  This function makes it possible to use
 * different layouts on various pages of a site.
 *
 * @return <string>
 *   The name of the layout associated with the current page.
 */
function _sprouts_get_layout() {
  $layout_patterns = _sprouts_variable_get('layout', array('<global>' => 'body-layout-fixed-abc'));
  $global_layout = $layout_patterns['<global>'];
  unset($layout_patterns['<global>']);

  $alias_path = drupal_get_path_alias($_GET['q']);
  $path = $_GET['q'];
  foreach ($layout_patterns as $pattern => $layout) {
    if (drupal_match_path($alias_path, $pattern) ||
        drupal_match_path($path, $pattern)) {
      return $layout;
    }
  }
  return $global_layout;
}

/**
 * Implements hook_node_view_alter().
 */
function sprouts_node_view_alter(&$build) {
  if (isset($build['links']) && isset($build['links']['comment']) &&
    isset($build['links']['comment']['#attributes']) &&
    isset($build['links']['comment']['#attributes']['class'])) {
    $classes = $build['links']['comment']['#attributes']['class'];
    array_push($classes, 'actions');
    $build['links']['comment']['#attributes']['class'] = $classes;
  }
}

/**
 * Implements hook_preprocess_forum_topic_list
 */

function sprouts_preprocess_forum_topic_list(&$vars) {
  // Recreate the topic list header
  $list = array(
    array('data' => t('Topic'), 'field' => 'f.title'),
    array('data' => t('Replies'), 'field' => 'f.comment_count'),
    array('data' => t('Created'), 'field' => 't.created'),
    array('data' => t('Last reply'), 'field' => 'f.last_comment_timestamp'),
  );
  
  $ts = tablesort_init($list);
  $header = '';
  foreach ($list as $cell) {
    $cell = tablesort_header($cell, $list, $ts);
    $header .= _theme_table_cell($cell, TRUE);
  }
  $vars['header'] = $header;
}

/*
 * Implements hook_preprocess_media_gallery_license().
 */
function sprouts_preprocess_media_gallery_license(&$vars) {
  
}

function _sprouts_clean_stylesheets() {
  $exclude = array(
    'modules/system/system.base.css',
    'modules/system/system.menus.css',
    'modules/system/system.theme.css',
    'modules/system/system.messages.css',
    'modules/aggregator/aggregator.css',
    'modules/book/book.css',
    'modules/comment/comment.css',
    'modules/field/theme/field.css',
    'modules/node/node.css',
    'modules/poll/poll.css',
    'modules/search/search.css',
    'modules/user/user.css',
    'modules/forum/forum.css',
    'modules/forum/forum.css',
    'modules/toolbar/toolbar.css',
  );
  // Get a reference to the static css variable
  $css = &drupal_static('drupal_add_css');
  // Loop through the stylesheets and knock out
  // any in the exclude list
  foreach ($css as $k => $path) {
    if (in_array($k, $exclude)) {
      unset($css[$k]);
    }
  }
}
