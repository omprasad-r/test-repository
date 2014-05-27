<?php

/**
 * @file
 * Post-install hook to allow the use of a custom profile.
 *
 * The default behavior is to use a profile called "gardens". This post-install
 * hook allows a profile matching the name of the AcsfSite->client_name variable
 * to be used instead.
 */

if (function_exists('acsf_register_autoloader')) {
  acsf_register_autoloader();
  $site = Acquia\Acsf\AcsfSite::load();
}
else {
  $site = AcsfSite::load();
}

$profile_name = $site->client_name;
if ($profile_name && file_exists(DRUPAL_ROOT . "/profiles/{$profile_name}/{$profile_name}.info")) {
  // Save the value for future use.
  $previous_profile = variable_get('install_profile', 'gardens');
  // First set the profile to make sure the profile "module" is found when we...
  variable_set('install_profile', $profile_name);
  // ... rebuild module data (making sure it's not cached).
  system_list_reset();
  drupal_flush_all_caches();
  // If the profile "module" is not found, reset to the previous profile.
  if (db_query("SELECT 1 FROM {system} WHERE type = 'module' AND name = :name", array(':name' => $profile_name))->fetchField()) {
    // Pass TRUE to module_enable() so that dependent modules are also enabled.
    // Normally, for performance, we would list dependencies here explicitly and
    // pass FALSE, but in this case, we've already incurred the penalty of
    // rebuilding the module data, so can allow module_enable() to benefit from
    // that.
    module_enable(array($profile_name), TRUE);
  }
  else {
    // If the profile "module" is not found, reset to the previous profile. This
    // should not happen.
    variable_set('install_profile', $previous_profile);
    system_list_reset();
    system_rebuild_module_data();
  }
}

