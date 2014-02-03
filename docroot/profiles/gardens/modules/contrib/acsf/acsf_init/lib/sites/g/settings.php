<?php

if (file_exists('/var/www/site-php')) {
  // The DB role will be the same as the gardens site directory name
  $role = basename(conf_path());
  // This global is set in sites.php. It's used to reference the
  // live environment DB setting even when running on the update env.
  $site_settings = !empty($GLOBALS['gardens_site_settings']) ? $GLOBALS['gardens_site_settings'] : array('site' => '', 'env' => '');
  $site = $site_settings['site'];
  $env = $site_settings['env'];

  $settings_inc = "/var/www/site-php/{$site}.{$env}/D7-{$env}-{$role}-settings.inc";
  if (file_exists($settings_inc)) {
    include($settings_inc);
  }
  elseif (!isset($_SERVER['SERVER_SOFTWARE']) && (php_sapi_name() == 'cli' || (is_numeric($_SERVER['argc']) && $_SERVER['argc'] > 0))) {
    throw new Exception('No database connection file was found for DB {$role}.');
  }
  else {
    syslog(LOG_ERR, 'GardensError: AN-22471 - No database connection file was found for DB {$role}.');
    header($_SERVER['SERVER_PROTOCOL'] .' 503 Service unavailable');
    print 'The website encountered an unexpected error. Please try again later.';
    exit;
  }
  if (!class_exists('DrupalFakeCache')) {
    $conf['cache_backends'][] = 'includes/cache-install.inc';
  }
  // Rely on the external Varnish cache for page caching.
  $conf['cache_class_cache_page'] = 'DrupalFakeCache';
  $conf['cache'] = 1;
  $conf['page_cache_maximum_age'] = 300;
  // We can't use an external cache if we are trying to invoke these hooks.
  $conf['page_cache_invoke_hooks'] = FALSE;

  if (!empty($site_settings['flags']['memcache_enabled']) && !empty($site_settings['memcache_inc'])) {
    $conf['cache_backends'][] = $site_settings['memcache_inc'];
    $conf['cache_default_class'] = 'MemCacheDrupal';
    $conf['cache_class_cache_form'] = 'DrupalDatabaseCache';
  }

  // Until the site installation finishes, noone should be able to visit the
  // site, unless the site is being installed via install.php and the user has
  // the correct token to access it.
  if (!drupal_is_cli() && !empty($site_settings['flags']['access_restricted']['enabled'])) {
    $token_match = !empty($site_settings['flags']['access_restricted']['token']) && !empty($_GET['site_install_token']) && $_GET['site_install_token'] == $site_settings['flags']['access_restricted']['token'];
    $path_match = $_SERVER['SCRIPT_NAME'] == $GLOBALS['base_path'] . 'install.php';
    if (!$token_match || !$path_match) {
      header($_SERVER['SERVER_PROTOCOL'] .' 503 Service unavailable');
      if (!empty($site_settings['flags']['access_restricted']['reason'])) {
        print $site_settings['flags']['access_restricted']['reason'];
      }
      exit;
    }
  }

  if (!empty($site_settings['conf'])) {
    foreach ((array) $site_settings['conf'] as $key => $value) {
      $conf[$key] = $value;
    }
  }
}
