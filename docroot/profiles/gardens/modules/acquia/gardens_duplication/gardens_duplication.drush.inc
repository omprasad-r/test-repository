<?php

/**
 * @file
 * Provides drush commands and related code for the site duplication scrub.
 */

/**
 * Alters drush command help information.
 *
 * @param array $command
 *   An array of data representing the currently invoked command.
 */
function gardens_duplication_drush_help_alter(&$command) {
  if ($command['command'] == 'acsf-duplication-scrub-batch') {
    $options = array(
      'batch-webform' => dt('The number of webform submissions to delete each iteration (defaults to --batch).'),
      'batch-votingapi' => dt('The number of votes to delete each iteration (defaults to --batch).'),
      'retain-janrain-settings' => dt('Retain Janrain settings (defaults to --exact-copy).'),
      'retain-webform-submissions' => dt('Retain Webform submissions (defaults to --exact-copy).'),
      'retain-voting-api-data' => dt('Retain Voting API data (defaults to --exact-copy).'),
      'retain-acquia-lift-config' => dt('Retain Acquia Lift campaign configuration (defaults to --exact-copy).'),
      'mollom-public-key' => dt('The Mollom public key to set.'),
      'mollom-private-key' => dt('The Mollom private key to set.'),
    );
    $command['options'] = array_merge($command['options'], $options);
  }
}

