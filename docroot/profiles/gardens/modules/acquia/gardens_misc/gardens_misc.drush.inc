<?php

/**
 * Implements hook_drush_command().
 */
function gardens_misc_drush_command() {
  $items = array(
    'mollom-keys-get' => array(
      'description' => 'Get from the Gardener and save mollom keys if they are currently missing',
      'aliases' => array('mk-get'),
    ),
    'mollom-keys-replace' => array(
      'description' => 'Get from the Gardener and save mollom keys even if they already exist',
      'aliases' => array('mk-repl'),
    ),
    'gather-image-dimensions' => array(
      'description' => dt('Walk through image files, get their dimensions (width, height), and insert the data into the database.'),
      'arguments' => array(
        'limit' => dt('The number of image files to be processed at once.'),
      ),
      'aliases' => array('get-img-dim'),
    ),
  );
  return $items;
}

/**
 * Implements of hook_drush_help().
 */
function gardens_misc_drush_help($section) {
  switch ($section) {
    case 'drush:gather-image-dimensions':
      return dt('Walk through image files, get their dimensions (width, height), and insert the data into the database.');
  }
}

/**
 * Implements hook_drush_sql_sync_sanitize().
 */
function gardens_misc_drush_sql_sync_sanitize() {
  $static_scrub = file_get_contents(DRUPAL_ROOT . '/../gsite-scrub.sql');
  drush_sql_register_post_sync_op('staging-static-scrub',
    dt('Scrub Gardens site database'),
    $static_scrub);
}

/**
 * Retrieves information about the gardener.
 *
 * @return array
 *   An array of gardener information.
 */
function drush_gardens_misc_get_gardener_info() {
  if (!class_exists('\Acquia\Acsf\AcsfConfigDefault')) {
    // Since there might not be a bootstrap, we need to find our config objects.
    require_once DRUPAL_ROOT . '/profiles/gardens/modules/contrib/acsf/src/Acquia/Acsf/AcsfConfig.php';
    require_once DRUPAL_ROOT . '/profiles/gardens/modules/contrib/acsf/src/Acquia/Acsf/AcsfConfigDefault.php';
  }

  $config = new \Acquia\Acsf\AcsfConfigDefault();
  return array(
    'url' => $config->getUrl(),
    'username' => $config->getUsername(),
    'password' => $config->getPassword(),
    'url_suffix' => $config->getUrlSuffix(),
    'source_url' => $config->getSourceUrl(),
  );
}

/**
 * drush command callback. Get from the Gardener and save mollom keys if they are currently missing.
 */
function drush_gardens_misc_mollom_keys_get() {
  $public = variable_get('mollom_public_key');
  $private = variable_get('mollom_private_key');
  if (!$public || !$private) {
    drush_gardens_misc_mollom_keys_replace();
  }
  else {
    drush_log("Mollom keys already existed - not replaced", 'warning');
  }
}

/**
 * drush command callback. Get from the Gardener and save mollom keys even if they already exist.
 */
function drush_gardens_misc_mollom_keys_replace() {
  $site = acsf_get_acsf_site();
  $nid = $site->site_id;
  try {
    $message = new \Acquia\Acsf\AcsfMessageRest('GET', 'site-api/v1/mollom/keys/' . $nid);
    $message->send();
    $response = $message->getResponseBody();
    $mollom_keys = $response['keys'];
  }
  catch (Exception $e) {
    watchdog('gardens_misc', $e->getMessage(), array(), WATCHDOG_ERROR);
  }

  if ($mollom_keys) {
    variable_set('mollom_public_key', $mollom_keys['public_key']);
    variable_set('mollom_private_key', $mollom_keys['private_key']);
    // Reset mollom status to force a key check.
    if (function_exists('_mollom_status')) {
      _mollom_status(TRUE);
    }
    drush_log("Set mollom keys", 'success');
  }
  else {
    drush_log("Failed to set mollom keys", 'error');
  }
}

/**
 * Command callback. Gather image dimensions and insert them into the database.
 */
function drush_gardens_misc_gather_image_dimensions($limit = NULL) {
  if (is_numeric($limit)) {
    // Total files processed.
    $processed = 0;
    // Process records in chunks to cut down resource usage and prevent memory
    // or time limit errors.
    $position = 0;

    do {
      $sql = 'SELECT f.fid FROM {file_managed} f WHERE NOT EXISTS (SELECT * FROM {image_dimensions} i WHERE i.fid = f.fid) ORDER BY f.fid ASC';
      $files = db_query_range($sql, ($position++ * $limit), $limit)->fetchAll();
      if (empty($files)) {
        // Output an error if there are no more items to process.
        drush_set_error(NULL, dt('There are no more files to process.'));
        break;
      }
      $processed += _gardens_misc_get_image_dimensions($files);
    } while ($processed < $limit);

    drush_log("Done gathering image dimensions. Image files processed: $processed.", 'ok');
  }
  else {
    drush_set_error(NULL, dt('"@limit" is not a valid number.', array('@limit' => $limit)));
  }
}

/**
 * Helper function.
 */
function _gardens_misc_get_image_dimensions($files = array()) {
  // Files processed.
  $processed = 0;

  foreach ($files as $key) {
    if ($file = file_load($key->fid)) {
      // Prevent PHP notices when trying to read empty files.
      // @see http://drupal.org/node/681042
      if (!$file->filesize) {
        continue;
      }

      // Do not bother proceeding if this file does not have an image
      // mime type.
      if (strpos($file->filemime, 'image/') !== 0) {
        continue;
      }

      // Get details about the image.
      $image_info = image_get_info($file->uri);
      if ($image_info) {
        // Store dimensions.
        db_merge('image_dimensions')
          ->key(array('fid' => $file->fid))
          ->fields(array(
            'width' => $image_info['width'],
            'height' => $image_info['height'],
          ))
          ->execute();

        $processed++;
      }
    }
  }

  return $processed;
}
