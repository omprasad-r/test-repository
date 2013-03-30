#!/usr/bin/php -q
<?php
include_once('lib/release.inc');
date_default_timezone_set('EST');

/**
 * Returns a help message for this script.
 */
function prep_gardens_release_help_message() {
  $message = <<<EOF

Stages a Gardens branch for release.

TODO: THE DOCUMENTATION BELOW IS COMPLETELY INCORRECT!

This script is designed to run on a Gardens branch and create a release tag
from the tip of that branch that is ready to be deployed. In particular, it
does the following:

 1. Checks all Gardens themes in the branch to make sure they do not have
    improper fonts.
 2. Runs "SVN copy" to create a new tag (in the Acquia engineering repository)
    from the tip of the branch. This will be the next available tag, organized
    by date (for example, "tags/20100503.2" for the second release tag created
    on May 3, 2010).
 3. Prints out to the screen the Hosting commands that would need to be run to
    push the code live (but does not actually run them).

The following options are supported:

 --branch [branch]          svn branch from which to push. e.g. tags/20110101.1
                            if no branch is given takes the latest gardens tag.
 --to-svn-tag               svn tag to push to typically tags/20110101.1 (optional)
 --fields-path              path to fields root (where update-gardens-svn.php hides)
 --username [username]      svn username (optional)
 --password [password]      svn password (optional)
 --dry-run                  Prints all commands to the screen, but does not actually
                            create the new tag in the Acquia SVN repository.
 --help                     Prints this help text and exits.

EOF;

  return $message;
}

try {
  // Get optional command line parameters.
  $parameters = array(
    'fields-path' => NULL,
    'branch' => NULL, // Defaults to the branch where this script was run from.
    'to-svn-tag' => NULL,
    'dry-run' => FALSE,
    'username' => NULL,
    'password' => NULL,
    'help' => FALSE,
  );

  while ($arg = array_shift($_SERVER['argv'])) {
    if (preg_match('/--(\S+)/', $arg, $matches)) {
      if ($matches[1] == 'help') {
        // Print the help message and immediately quit.
        print prep_gardens_release_help_message();
        exit;
      }
      elseif ($matches[1] == 'dry-run') {
        $parameters['dry-run'] = TRUE;
      }
      elseif (array_key_exists($matches[1], $parameters)) {
        $parameters[$matches[1]] = array_shift($_SERVER['argv']);
      }
    }
  }
  // Create variables for each parameter, e.g. $username and $password.
  foreach ($parameters as $key => $value) {
    $variable_name = str_replace('-', '_', $key);
    $$variable_name = $value;
  }

  # Try and find the update-gardens-svn.php
  $update_svn_path = dirname(trim(shell_exec('which update-gardens-svn.php')));
  if(empty($update_svn_path) && !isset($fields_path)){
    throw new Exception("Could not find update-gardens-svn.php and fields_path is not set\n" .
    "Try \"--fields-path=<path to fields checkout>\"");
  }
  else {
    $update_svn_path = !empty($update_svn_path) ? $update_svn_path : $fields_path;
  }

  $initial_message = "Staging Gardens release";
  if ($dry_run) {
    $initial_message .= " (DRY RUN)";
  }
  print "\n$initial_message...\n\n";
  if (isset($branch)){
    $staging_tag = $branch;
  }
  else {
    $latest_tag = acquia_gardens_release_get_latest_svn_tag_name($username, $password);
    $staging_tag = "tags/" . $latest_tag;
  }
  print  "Staging tag $staging_tag \n";
  print  "Staging tag " . acquia_gardens_release_get_svn_url($staging_tag) ."\n";
  $update_command = $update_svn_path . '/' . acquia_gardens_release_get_update_svn_command($staging_tag, $to_svn_tag);
  print "Executing update command: $update_command\n";
  if($dry_run){
    print "If you want to execute the staging command do not use the --dry-run option\n";
  }
  else{
    print "Updating gardens with $staging_tag\n";
    $result = shell_exec($update_command);
    print "$result\n";
  }

}
catch (Exception $e) {
  acquia_gardens_release_abort($e->getMessage());
}

/**
 * Trigger an abort of the Gardens release preparation script.
 *
 * @param $message
 *   The error message to display before exiting.
 */
function stage_gardens_release_abort($message) {
  print "***********************\n";
  print "GARDENS RELEASE ABORTED!\n\n";
  print "$message\n";
  print "***********************\n";
  exit(-1);
}
