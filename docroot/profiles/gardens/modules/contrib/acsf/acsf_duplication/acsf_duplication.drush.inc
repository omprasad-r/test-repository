<?php

/**
 * Implements hook_drush_command().
 */
function acsf_duplication_drush_command() {
  $items = array();

  $items['acsf-duplication-scrub-batch'] = array(
    'description' => dt('Runs one iteration of the batch scrubbing process on the duplicated site.'),
    'arguments' => array(
      'site_name' => dt('The new name of the duplicated site'),
      'standard_domain' => dt('The standard domain of the duplicated site.'),
    ),
    'options' => array(
      'exact-copy' => dt('Indicates whether the duplicated site is required to be an exact copy of the source site (i.e. retain all content and users).'),
      'avoid-oom' => dt('The command should run just a single iteration of the batch process (the default is to loop until it runs out of memory). This is useful to keep the memory footprint low but you are expected to handle the looping externally.'),
      'batch-comment' => dt('The number of comments to delete each iteration (defaults to 1000).'),
      'batch-node' => dt('The number of nodes to delete each iteration (defaults to 1000).'),
      'batch-user' => dt('The number of users to delete each iteration (defaults to 1000).'),
    ),
  );
  $items['acsf-duplication-scrub-progress'] = array(
    'description' => dt('Returns status information about the progress of the batch scrubbing process.'),
    'options' => array(
      'format' => array(
        'description' => 'Format to output the object. Use "print_r" for print_r (default), "export" for var_export, and "json" for JSON.',
        'example-value' => 'json',
      ),
    ),
  );

  return $items;
}

/**
 * Implements hook_drush_help().
 */
function acsf_duplication_drush_help($section) {
  switch ($section) {
    case 'drush:acsf-duplication-scrub-batch':
      return dt('Runs one iteration of the batch scrubbing process on the duplicated site.');
    case 'drush:acsf-duplication-scrub-progress':
      return dt('Returns status information about the progress of the batch scrubbing process.');
  }
}

/**
 * Runs one iteration of the batch scrubbing process on the duplicated site.
 */
function drush_acsf_duplication_scrub_batch($site_name, $standard_domain) {
  if (empty($site_name)) {
    return drush_set_error(dt('You must provide the site name of the duplicated site as the first argument.'));
  }
  if (empty($standard_domain)) {
    return drush_set_error(dt('You must provide the standard domain of the duplicated site as the second argument.'));
  }

  if (!module_exists('acsf')) {
    return drush_set_error(dt('The ACSF module must be enabled.'));
  }

  // Enable the ACSF Duplication module and rebuild the ACSF Registry so that
  // the scrubbing classes are loaded and registered with AcsfEvent.
  module_enable(array('acsf_duplication'), FALSE);
  acsf_build_registry();

  $context = array(
    'site_name' => $site_name,
    'standard_domain' => $standard_domain,
    'scrub_options' => array(
      'retain_users' => drush_get_option('exact-copy'),
      'retain_content' => drush_get_option('exact-copy'),
      'avoid_oom' => drush_get_option('avoid-oom'),
      'comment_batch' => drush_get_option('batch-comment'),
      'node_batch' => drush_get_option('batch-node'),
      'user_batch' => drush_get_option('batch-user'),
    ),
  );
  // Load and execute the site duplication scrub event handlers.
  $event = AcsfEvent::create('site_duplication_scrub', $context);
  $event->run();

  // Return an error code if the process is incomplete.
  if (variable_get('acsf_duplication_scrub_status', NULL) !== 'complete') {
    return drush_set_error(dt('The scrubbing of this site is incomplete.'));
  }
}

/**
 * Outputs progress information on the site duplication scrub.
 */
function drush_acsf_duplication_scrub_progress() {
  if (!module_exists('acsf')) {
    return drush_set_error(dt('The ACSF module must be enabled.'));
  }

  $data = array(
    'comment_count' => AcsfDuplicationScrubCommentHandler::countRemaining(),
    'node_count' => AcsfDuplicationScrubNodeHandler::countRemaining(),
    'user_count' => AcsfDuplicationScrubUserHandler::countRemaining(),
  );
  $format = drush_get_option('format', 'print_r');
  drush_print(drush_format($data, NULL, $format));
}

