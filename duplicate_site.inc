<?php

/**
 * Configure a duplicated site.
 *
 * @param string $gardens_site_info
 *   A JSON-encoded array of site information.
 */
function acquia_gardens_configure_duplicated_site($gardens_site_info) {
  $gardens_site_info = json_decode($gardens_site_info);
  define('DRUPAL_ROOT', dirname(__FILE__) . '/docroot');
  require_once DRUPAL_ROOT . '/includes/bootstrap.inc';
  drupal_override_server_variables(array('url' => $gardens_site_info->url . '/index.php'));
  drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  variable_del('gardens_site_duplication_complete');
  variable_set('site_name', $gardens_site_info->site_name);
  $mollom_public_key = !empty($gardens_site_info->mollom->public_key) ? $gardens_site_info->mollom->public_key : '';
  $mollom_private_key = !empty($gardens_site_info->mollom->private_key) ? $gardens_site_info->mollom->private_key : '';
  gardens_misc_update_mollom_keys_if_necessary($mollom_public_key, $mollom_private_key);
  // Ensure Drupal filesystem related configuration variables are correct
  // for the new site. Consider the following variables:
  // - file_directory_path
  // - file_directory_temp
  // - file_public_path
  // - file_private_path
  // - file_temporary_path
  // Given the AH environment for Gardens, we want to leave the temp paths
  // alone, and we want to delete the other variables, to ensure they reset
  // to their defaults (because of scarecrow, these shouldn't exist in the
  // {variable} table anyway).
  foreach (array('file_directory_path', 'file_public_path', 'file_private_path') as $var) {
    variable_del($var);
  }
  // Save the site's original (standard *.drupalgardens.com) domain.
  $domains = $gardens_site_info->domains;
  $standard_domain = reset($domains);
  variable_set('gardens_misc_standard_domain', $standard_domain);
  // Normally we only have one domain during configure, so just set it.
  // @see: AN-19653  this is a quick fix for imagecache problems.
  $domain = end($domains);
  variable_set('file_public_path', "sites/g/files/{$gardens_site_info->site_id}/f");
  module_enable(array('gardens_duplication'), FALSE);
  $scrub_options = array();
  $scrub_options['retain_janrain_settings'] = $gardens_site_info->exact_copy || $gardens_site_info->retain_janrain_settings;
  if ($gardens_site_info->exact_copy) {
    $scrub_options['retain_users_and_content'] = TRUE;
    $scrub_options['retain_webform_submissions'] = TRUE;
    $scrub_options['retain_voting_api_data'] = TRUE;
  }
  gardens_duplication_scrub($scrub_options);
  module_disable(array('gardens_duplication'), FALSE);
  variable_set('gardens_site_duplication_complete', 'complete');
}
