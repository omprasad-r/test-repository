<?php

/**
 * @file
 * Simple API that gathers site and theme statistics.
 *
 * The intent of this API is to gather statistics on each gardens site
 * and push them to the gardener site so we can run reports or perform
 * queries to find out if and how the sites are being used.
 */

/**
 * Returns an array of site statistics.
 *
 * @return
 *  The associative array containing the site stats.
 */
function gardens_client_generate_site_stats() {
  global $databases;
  $start = microtime(TRUE);
  $stats = new stdClass();

  $stats->dbname = $databases['default']['default']['database'];
  $stats->dbmaster = gardens_client_get_db_master();
  $stats->tangle = themebuilder_compiler_get_tangle_name();
  _gardens_client_get_site_stats($stats);
  _gardens_client_get_theme_stats($stats);
  $stats->stats_cost = intval((microtime(TRUE) - $start) * 1000);
  $stats->notification_bits = gardens_limits_get_notification_bits();
  gardens_limits_clear_notification_bits();
  return $stats;
}

/**
 * Returns the dbmaster associated with this site.
 */
function gardens_client_get_db_master() {
  global $conf;
  $master = '';
  if (isset($conf['acquia_hosting_site_info']) && isset($conf['acquia_hosting_site_info']['db'])) {
    $db = $conf['acquia_hosting_site_info']['db'];
    if (isset($db['db_url_ha'])) {
      $master = reset(array_keys($db['db_url_ha']));
    }
  }
  return $master;
}

/**
 * Returns site statistics.
 *
 * @param $stats
 *   An object containing the statistics.
 * @return
 *   An associative array with key/value pairs that reveal the statistics.
 */
function _gardens_client_get_site_stats($stats) {
  // The site name
  $stats->site_name = variable_get('site_name', '');

  // # of active users
  $stats->users = intval(db_query('SELECT COUNT(uid) FROM {users} u WHERE u.uid > 1 AND u.status != 0')->fetchField());

  // Total users on the site (active or not)
  $stats->total_users = intval(db_query('SELECT COUNT(uid) FROM {users} u WHERE u.uid > 1')->fetchField());

  // # of nodes
  $stats->nodes = intval(db_query('SELECT COUNT(nid) FROM {node}')->fetchField());

  // date of most recent node created
  $stats->last_node = db_query('SELECT MAX(created) FROM {node}')->fetchField();


  // Date of most recent activity by a logged-in, non-support user.
  $stats->last_login = db_query('SELECT MAX(access) FROM {users} WHERE uid > 1')->fetchField();

  try {
    // # of comments.  Note that the comment module may be disabled.
    $stats->comments = intval(db_query('SELECT COUNT(nid) FROM {comment}')->fetchField());
    // date of most recent comment
    $stats->last_comment = db_query('SELECT MAX(created) FROM {comment}')->fetchField();
  }
  catch (PDOException $e) {
    $stats->comments = 0;
    $stats->last_comment = 0;
  }

  // We no longer measure disk space on the filesystem because it is too expensive.
  $stats->disk_space = 0;

  // disk space used measured in bytes
  $stats->managed_disk_space = intval(gardens_limits_storage_used());

  // Count of webforms with responses over the limit
  $stats->webforms_with_responses_over_limit = _gardens_client_webforms_over_limit();

  // Total count of webform responses across all forms
  $stats->webform_response_total = _gardens_client_webform_response_total();

  // Count of webform responses this month
  $stats->webform_responses_this_month = _gardens_client_webforms_responses_this_month();

  // Count of webforms on the site
  $stats->webform_count = _gardens_client_webform_count();
}

/**
 * Returns statistics about the themes associated with the site.
 *
 * @param $stats
 *   The object in which the statistics will be placed.
 */
function _gardens_client_get_theme_stats($stats) {
  // # of rows in themebuilder_css
  $stats->themebuilder_css_table_rows = intval(db_query('SELECT COUNT(*) FROM {themebuilder_css}')->fetchField());

  // # of custom themes
  $stats->custom_theme_count = intval(db_query('SELECT COUNT(DISTINCT theme_name) FROM {themebuilder_themes}')->fetchField());

  // Information about the currently published theme.
  $current_theme = variable_get('theme_default', '');
  _gardens_client_get_theme_summary($stats, $current_theme);
}

/**
 * Returns summary information for the specified theme.
 *
 * @param stats
 *   The object in which stats will be stored
 * @param $theme_name
 *   The name of the theme.
 * @return
 *   An array containing theme information.
 */
function _gardens_client_get_theme_summary($stats, $theme_name) {
  $path = DRUPAL_ROOT . '/' . drupal_get_path('theme', $theme_name);
  $path_info = pathinfo($path);
  $theme_name = $path_info['basename'];
  $info_file = file_get_contents("$path/$theme_name.info");
  $info = drupal_parse_info_format($info_file);

  $version = $info['version'];
  if (isset($info['base_theme'])) {
    $base_theme = $info['base_theme'];
  }
  else {
    $base_theme = _gardens_client_get_base_theme_name_from_screenshot($path, $theme_name);
  }
  $stats->base_theme = $base_theme;
  // Is the active theme a custom theme or a base theme.
  $stats->using_custom_theme = ($base_theme != $theme_name) ? 1 : 0;
  $stats->theme_version = $version;
  $stats->custom_css_lines = _gardens_client_line_count("$path/custom.css");
  $stats->advanced_css_lines = _gardens_client_line_count("$path/advanced.css");
}

/**
 * In the original release of Gardens, we did not have the base_theme property
 * in the .info file.  This is the only way we know the origin of the theme
 * after it has been saved.  This simple scheme relies on the fact that
 * originally we did not replace the thumbnail images for the theme
 * (screenshot_lg.png), so it is possible to look at the thumbnail for these
 * older themes and determine the base theme from there.  Eventually this will
 * be dead code because all of the themes should be converted over time.
 *
 * @param $theme
 *   Either the theme object or the theme name.
 */
function _gardens_client_get_base_theme_name_from_screenshot($path, $theme) {
  $map = array(
    '92bdf6d0f4d916981ffd03e7d819faa7' => 'broadway',
    '87ce6808dd821638aa4e10076290af15' => 'builderbase',
    'd71e2135c1f58c77bda84e318f0def7e' => 'campaign',
    '0f7cfc45ea93e61090c719fd498d07d1' => 'kenwood',
    '5d6e1abd629a5774d851d28c0fa3e19c' => 'sonoma',
    '1818a2557c35553c444d7c72c8ea740b' => 'sparks',
  );
  if (is_string($theme)) {
    // Passed in the theme name.
    $name = $theme;
  }
  else {
    $path = $theme->getPath();
    $name = $theme->getName();
  }
  if (empty($path)) {
    $path = drupal_get_path('theme', $name);
  }
  $screenshot_path = $path . '/screenshot_lg.png';
  $contents = file_get_contents($screenshot_path);
  $md5 = md5($contents);
  return $map[$md5];
}

/**
 * Counts lines in the file associated with the specified path.
 *
 * @param {String} $filepath
 *   The path to the file.
 * @return
 *   An integer that indicates the number of lines in the file.
 */
function _gardens_client_line_count($filepath) {
  $line_count = -1;
  $line = exec('/bin/grep -c "." ' . escapeshellarg($filepath), $output, $status);
  if ($status === 0) {
    $line_count = intval($line);
  }
  return $line_count;
}

/**
 * Counts the number of webforms that have more than 100 submissions.
 */
function _gardens_client_webforms_over_limit() {
  // @todo: Hard coding the limit isn't fun, but we want this for all forms?
  $limit = 100;
  $results = 0;

  if (db_table_exists('webform_submissions')) {
    $query = db_select('webform_submissions', 's');
    $query->addField('s', 'nid');
    $query->groupBy('nid');
    $query->having('count(s.sid) > :limit', array(':limit' => $limit));
    $results = $query->execute()->rowCount();
  }

  return $results;
}

/**
 * Counts the total number of webform submissions.
 */
function _gardens_client_webform_response_total() {
  $results = 0;

  if (db_table_exists('webform_submissions')) {
    $query = db_select('webform_submissions', 's');
    $query->addField('s', 'nid');
    $results = $query->execute()->rowCount();
  }

  return $results;
}

/**
 * Counts the total number of webform submissions since 12:00 AM on the 1st of
 * the current month.
 */
function _gardens_client_webforms_responses_this_month() {
  $today = getDate();
  $first_day = mktime(0,0,0,$today['mon'],1,$today['year']);
  $results = 0;

  if (db_table_exists('webform_submissions')) {
    $query = db_select('webform_submissions', 's');
    $query->addField('s', 'nid');
    $query->where('s.submitted > :first_day', array(':first_day' => $first_day));
    $results = $query->execute()->rowCount();
  }

  return $results;
}

/**
 * Counts the total number of webforms created across all content types.
 */
function _gardens_client_webform_count() {
  $results = 0;

  if (db_table_exists('webform')) {
    $query = db_select('webform', 's');
    $query->addField('s', 'nid');
    $results = $query->execute()->rowCount();
  }

  return $results;
}
