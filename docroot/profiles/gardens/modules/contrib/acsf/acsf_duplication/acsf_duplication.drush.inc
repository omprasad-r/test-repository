<?php

/**
 * Implements hook_drush_command().
 */
function acsf_duplication_drush_command() {
  $items = array();

  $items['acsf-duplication-scrubbing'] = array(
    'description' => 'Scrubs the duplicated site.',
    'arguments' => array(
      'site_name' => dt('The new name of the duplicated site'),
      'standard_domain' => dt('The standard domain of the duplicated site.'),
    ),
  );

  return $items;
}

/**
 * Implements hook_drush_help().
 */
function acsf_duplication_drush_help($section) {
  switch ($section) {
    case 'drush:acsf-duplication-scrubbing':
      return dt('Scrubs the duplicated site.');
  }
}

/**
 * Scrubs the duplicated site.
 */
function drush_acsf_duplication_scrubbing($site_name, $standard_domain) {
  variable_del('acsf_site_duplication_complete');
  variable_set('site_name', $site_name);
  module_enable(array('acsf_duplication'), FALSE);
  $scrub_options = array();
  if (drush_get_option('exact_copy', FALSE)) {
    $scrub_options['retain_users_and_content'] = TRUE;
  }

  // Initiate an event so that modules can respond accordingly.
  $type = 'site_duplication_scrub';
  $context = array('site_name' => $site_name, 'standard_domain' => $standard_domain, 'scrub_options' => $scrub_options);
  $event = AcsfEvent::create($type, $context);
  $event->run();

  module_disable(array('acsf_duplication'), FALSE);
  variable_set('acsf_site_duplication_complete', 'complete');
}
