<?php

/**
 * Defines Gardens' default Amazon S3 bucket name.
 */
define('GARDENS_DEFAULT_EXPORT_BUCKET_NAME', 'gardens-default.site-archives');

/**
 * Implements hook_drush_command()
 */
function site_export_drush_command() {
  return array(
    'export' => array(
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
      'description' => "Exports a site and its database",
      'options' => array(
        'memory-limit' => 'The maximum memory to use.',
        'exclude-files' => 'Excludes files from the archive - they will be available after running a script.  Do not use this option if the site is being destroyed after the site export!',
      ),
      'examples' => array(
        'drush export --memory-limit=1024M' => 'Export the site using a maximum of 1024 MB of RAM',
      ),
    ),
    'export-s3' => array(
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
      'description' => "Exports a site and its database, uploads the file to S3 and notifies the gardener",
      'options' => array(
        's3key' => 'Mandatory if --backup is provided.  The key to use for connecting to S3.',
        's3secret' => 'Mandatory if --backup is provided.  The secret key to use for connecting to S3.',
        'memory-limit' => 'The maximum memory to use.',
      ),
    ),
  );
}

/**
 * Exports a Site Factory Site to a tar archive.
 *
 * @return string
 *  Path to export tar file.
 */
function drush_site_export_export() {
  $memory_limit = drush_get_option('memory-limit', NULL);
  if ($memory_limit !== NULL) {
    if (!preg_match('/^[0-9]+M$/', $memory_limit)) {
      drush_log(sprintf('memory-limit %s not recognized.  Should be of the form "128M".', $memory_limit), 'failed');
      return;
    }
    // Set the memory limit.
    ini_set('memory_limit', $memory_limit);
  }

  $exclude_files = drush_get_option('exclude-files');
  $uri = site_export_export_both($exclude_files );
  $path = drupal_realpath($uri);
  drush_log("Path to backup:" . $path, 'success');
  drush_print_pipe($path);
}

/**
 * Drush command to export a gardens site and backup to s3
 *
 * @return string
 *  S3 URL with token for unauthenticated access.
 */
function drush_site_export_export_s3() {
  $s3key = drush_get_option('s3key', NULL);
  $s3secret = drush_get_option('s3secret', NULL);
  // load from netrc.
  if (empty($s3key) || empty($s3secret)) {
    if ($creds = drush_site_export_s3_get_creds()) {
      $s3key = $creds['login'];
      $s3secret = $creds['password'];
    }
    else {
      drush_set_error('s3_keys_missing', 'To backup, you must provide --s3key and --s3secret');
      return;
    }
  }

  // Set the memory limit.
  $memory_limit = drush_get_option('memory-limit');
  if ($memory_limit !== NULL) {
    if (!preg_match('/^[0-9]+M$/', $memory_limit)) {
      drush_log(sprintf('memory-limit %s not recognized.  Should be of the form "128M".', $memory_limit), 'failed');
      return;
    }
    // Set the memory limit.
    ini_set('memory_limit', $memory_limit);
  }

  $nid = gardens_client_get_site_identifier();
  $uri = site_export_export_both();
  $path = drupal_realpath($uri);
  $resource_name = 'g' . $nid . '.tar.gz';

  if (defined('AH_HOSTNAME_STAGE')) {
    $bucket = AH_HOSTNAME_STAGE . '.site-archives';
  }
  else {
    $bucket = GARDENS_DEFAULT_EXPORT_BUCKET_NAME;
  }
  // This URL is publicly accessible.
  include_once(sprintf('%s/../library/acquia_gardens_xmlrpc.inc', DRUPAL_ROOT));
  $authenticated_url = drush_site_export_send_to_s3($s3key, $s3secret, $path, $resource_name, $bucket);
  if (!$authenticated_url) {
    drush_set_error('archive_failed', 'Unable to archive site to s3. Reporting failure to the gardener.');
    acquia_gardens_call('site.archive.failed', array($nid));
    return;
  }
  // Notify the gardener that we we have exported this site and moved it to S3.
  drush_log("Query Auth URL:" . $authenticated_url, 'success');
  $result = acquia_gardens_call('site.archived', array($nid, $authenticated_url));
  if ($result != 1) {
    // XML-RPC represents TRUE as 1, so check if the site returned that status.
    drush_set_error('archive_failed', "Unable to report successful s3 upload to the gardener. Check gardens_stats errors for site $nid on the gardener.");
    return;
  }

  drush_print_pipe($authenticated_url);
}

/**
 * Loads the netrc file and returns the aws creds.
 *
 * @return array
 *  A typical netrc array of login and password.
 */
function drush_site_export_s3_get_creds() {
  require_once '/usr/ah/lib/netrc.php';
  $netrc = netrc_read();
  if (empty($netrc['aws.gardens.archive.client'])) {
    return false;
  }
  $creds = $netrc['aws.gardens.archive.client'];
  return $creds;
}

/**
 * @param string $key
 *  AWS access key
 * @param string $secret
 *  AWS secret key
 * @param string $path
 *  The path of the file to upload
 * @param string $resource_name
 *  The name of the object to create in S3
 * @param string $bucket
 *  Bucket to upload to, defaults to  GARDENS_DEFAULT_EXPORT_BUCKET_NAME.
 *
 * @return mixed
 *  The authenticated url for accessing this content, or FALSE on failure.
 */
function drush_site_export_send_to_s3($key, $secret, $path, $resource_name = NULL, $bucket = GARDENS_DEFAULT_EXPORT_BUCKET_NAME) {
  require_once sprintf('%s/../library/S3.php', DRUPAL_ROOT);
  $resource_name = $resource_name ? $resource_name : basename($path);
  $s3 = new S3($key, $secret);

  if (!$s3->getBucket($bucket)) {
    if (!$s3->putBucket($bucket)) {
      drush_set_error('cannot_create_bucket', 'Unable to create site export bucket');
      return FALSE;
    }
  }
  $object = $s3->putObjectFile($path, $bucket, $resource_name);
  if ($object) {
    // URL will be valid for 365 days.
    return $s3->getAuthenticatedURL($bucket, $resource_name, 3600 * 24 * 365, false, false);
  }
  else {
    // The upload attempt failed.
    return FALSE;
  }

}


