<?php

/**
 * Returns the value of an SVN info property; throws an exception on failure.
 *
 * @param $property_name
 *   The name of a property that is displayed when calling "svn info".
 * @param $svn_url
 *   (optional) The URL of the SVN repository to query. If not provided, the
 *   current working directory is used.
 *
 * @throws Exception
 *   If the provided property does not exist.
 *
 * @return
 *   The value of the SVN info property.
 */
function acquia_gardens_release_require_svn_info($property_name, $svn_url = NULL, $username = NULL, $password = NULL) {
  if (!extension_loaded('SimpleXML')) {
    throw new Exception('You need the SimpleXML extension enabled to run this script.');
  }
  // The URL might not exist, so redirect errors to /dev/null to suppress them.
  $svn_args = acquia_gardens_release_get_svn_args($username, $password);
  $command = "svn $svn_args info --xml $svn_url 2>/dev/null";

  $xml = simplexml_load_string(shell_exec($command));
  if (isset($xml->entry->{$property_name})) {
    return $xml->entry->{$property_name};
  }
  else {
    $message = isset($svn_url) ? "Could not find an SVN info entry for \"$property_name\" at $svn_url." : "Could not find an SVN info entry for \"$property_name\" in the current working directory.";
    throw new Exception($message);
  }
}

/**
 * Returns the value of an SVN info property, or FALSE on failure.
 *
 * @param $property_name
 *   The name of a property that is displayed when calling "svn info".
 * @param $svn_url
 *   (optional) The URL of the SVN repository to query. If not provided, the
 *   current working directory is used.
 *
 * @return
 *   The value of the SVN info property, or FALSE on failure.
 */
function acquia_gardens_release_get_svn_info($property_name, $svn_url = NULL, $user = NULL, $pass = NULL) {
  try {
    return acquia_gardens_release_require_svn_info($property_name, $svn_url, $user, $pass);
  }
  catch (Exception $e) {
    return FALSE;
  }
}

/**
 * Returns a full SVN URL inside the Gardens repository.
 */
function acquia_gardens_release_get_svn_url($relative_path_within_gardens_repo) {
  return "https://svn.acquia.com/repos/engineering/gardens/$relative_path_within_gardens_repo";
}

/**
 * Determines if an SVN URL corresponds to an actual repository.
 */
function acquia_gardens_release_svn_url_exists($url, $user = NULL, $pass = NULL) {
  $found_url = acquia_gardens_release_get_svn_info('url', $url, $user, $pass);
  return (bool) $found_url;
}

/**
 * Finds today's next available Gardens release tag and returns the tag name.
 */
function acquia_gardens_release_get_next_available_svn_tag_name($user = NULL, $pass = NULL) {
  // timezone where Acquia is located.
  // To avoid confusion, we get the dates for our release tags using the
  date_default_timezone_set('America/New_York');

  // This gets a tag name along the lines of "20100429.1".
  $release_number = 0;
  do {
    $tag_name = date('Ymd') . '.' . ++$release_number;
    $url = acquia_gardens_release_get_svn_url("tags/$tag_name");
  } while (acquia_gardens_release_svn_url_exists($url, $user, $pass));

  return $tag_name;
}

/**
 * Finds last tag on the branch
 */
function acquia_gardens_release_get_latest_svn_tag_name($user = NULL, $pass = NULL){
  $svn_url = acquia_gardens_release_get_svn_url("tags");
  if (!extension_loaded('SimpleXML')) {
    throw new Exception('You need the SimpleXML extension enabled to run this script.');
  }
  // The URL might not exist, so redirect errors to /dev/null to suppress them.
  $svn_args = acquia_gardens_release_get_svn_args($user, $pass);
  $command = "svn $svn_args ls --xml $svn_url 2>/dev/null";
  $latest_tag = '';
  $current_commit = 0;
  $last_commit = 0 ;
  $xml = simplexml_load_string(shell_exec($command));
  foreach ($xml->list as $list){
    foreach($list->entry as $entry){
      if ( (isset($entry->name)) && (isset($entry->commit)) ) {
        $current_commit = (integer) $entry->commit['revision'];
        if ($current_commit > $last_commit){
          $last_commit = $current_commit;
          $latest_tag = (string) $entry->name;
        }
      }
      else {
        $message = isset($svn_url) ? "Could not find a tag at $svn_url." : "Could not find a tag from the current working directory.";
        throw new Exception($message);
      }
    }
  }
  print "Found latest tag tags/$latest_tag\n";
  return $latest_tag;
}


/**
 * build the svn args check for proper args and die
 * violently if the args are wrong
 * @param $user
 *   The svn user that will be making the checkout
 * @param $pass
 *   The svn password
 */
function acquia_gardens_release_get_svn_args($user = NULL, $pass = NULL){
  $args = NULL;
  if (!$user && !$pass) {
    return $args;
  }
  if ( $user && $pass) {
    $args = " --no-auth-cache --username $user --password $pass ";
  }
  else {
    throw new Exception("Both username and password must be defined");
  }
  return $args;
}

/**
 * Returns the tangles that are used for releasing.
 */
function acquia_gardens_release_get_tangles(){
  $tangles = array('tangle001', 'tangle002', 'tangle004', 'tangle005', 'tangle006', 'tangle007', 'tangle008');
  return $tangles;
}

/**
 * builds the update svn command for pushing code to the tangles.
 * Assumes update-gardens is in your path.
 * Also assumes the primary tangle (the one being the canonical repo)
 * is the 1st in the list.
 * @param $relative_tag_path
 *   tag from which the code will be staged
 */
function acquia_gardens_release_get_update_svn_command($relative_tag_path, $dest_tag_path = NULL){
  $tangles = acquia_gardens_release_get_tangles();
  $command = 'update-gardens-svn.php';
  $tangle = reset($tangles);
  $command .= " --site $tangle";
  $command .= " --from-svn-path $relative_tag_path";
  if (isset($dest_tag_path)) {
    $command .= " --to-svn-tag $dest_tag_path";
  }
  return $command;
}

/**
 * Trigger an abort of the Gardens release preparation script.
 *
 * @param $message
 *   The error message to display before exiting.
 */
function acquia_gardens_release_abort($message) {
  print "***********************\n";
  print "GARDENS RELEASE ABORTED!\n\n";
  print "$message\n";
  print "***********************\n";
  exit(-1);
}

