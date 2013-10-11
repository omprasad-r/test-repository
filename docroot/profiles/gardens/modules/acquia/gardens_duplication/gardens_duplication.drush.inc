<?php

/**
 * Implements hook_drush_command().
 */
function gardens_duplication_drush_command() {
  $items = array();

  $items['gardens-duplication-scrubbing'] = array(
    'description' => 'Scrubs the duplicated site.',
    'arguments' => array(
      'site_name' => dt('The new name of the duplicated site'),
      'standard_domain' => dt('The standard domain of the duplicated site.'),
    ),
    'options' => array(
      'mollom_public_key' => dt('The public mollom key for the site.'),
      'mollom_private_key' => dt('The private mollom key for the site.'),
      'exact_copy' => dt('A flag whether the duplication supposed to be an exact copy of the source site.'),
      'retain_janrain_settings' => dt('A flag whether the janrain settings should be preserved for the site.'),
    ),
  );

  return $items;
}

/**
 * Implements hook_drush_help().
 */
function gardens_duplication_drush_help($section) {
  switch ($section) {
    case 'drush:gardens-duplication-scrubbing':
      return dt('Scrubs the duplicated site.');
  }
}

/**
 * Scrubs the duplicated site.
 */
function drush_gardens_duplication_scrubbing($site_name, $standard_domain) {
  variable_del('gardens_site_duplication_complete');
  variable_set('site_name', $site_name);
  $mollom_public_key = drush_get_option('mollom_public_key', FALSE) ? drush_get_option('mollom_public_key', FALSE) : '';
  $mollom_private_key = drush_get_option('mollom_private_key', FALSE) ? drush_get_option('mollom_private_key', FALSE) : '';
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
  variable_set('gardens_misc_standard_domain', $standard_domain);
  variable_set('file_public_path', 'sites/g/files/' . variable_get('gardens_db_name', '') . '/f');
  module_enable(array('gardens_duplication'), FALSE);
  $scrub_options = array();
  $scrub_options['retain_janrain_settings'] = drush_get_option('exact_copy', FALSE) || drush_get_option('retain_janrain_settings', FALSE);
  if (drush_get_option('exact_copy', FALSE)) {
    $scrub_options['retain_users_and_content'] = TRUE;
    $scrub_options['retain_webform_submissions'] = TRUE;
    $scrub_options['retain_voting_api_data'] = TRUE;
  }
  gardens_duplication_scrub($scrub_options);
  module_disable(array('gardens_duplication'), FALSE);
  variable_set('gardens_site_duplication_complete', 'complete');
}
