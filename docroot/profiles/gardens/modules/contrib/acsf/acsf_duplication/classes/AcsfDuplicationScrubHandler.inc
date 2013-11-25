<?php

/**
 * @file
 * This class handles the scrubbing of core Drupal configuration.
 */

class AcsfDuplicationScrubHandler extends AcsfEventHandler {

  /**
   * Implements AcsfEventHandler::handle().
   */
  public function handle() {
    variable_set('acsf_duplicated_site', $this->event->context['site_name']);

    // Set install-related system variables.
    variable_set('install_time', time());
    variable_del('cron_last');
    variable_del('cron_semaphore');

    // Clear site-specific keys that will auto-initialize when first needed.
    // Initialize site-specific keys that do not auto-initialize.
    // Mollom keys are handled by install_gardens.inc instead of here, because
    // that's the code that has access to them.
    variable_del('drupal_private_key');
    variable_set('cron_key', drupal_hash_base64(drupal_random_bytes(55)));

    // The Google Analytics "Web Property Id" is not a private key, but it needs
    // to be unique per website, so we delete it here. A site administrator will
    // need to get a new Id from Google and enter it into the Google Analytics
    // module settings page.
    variable_del('googleanalytics_account');

    // Enable modules that are currently disabled, but that were once enabled, so
    // that their data cleanup hooks (e.g., hook_user_delete()) and functions
    // (e.g., search_reindex()) can be invoked.
    //
    // Note: These modules will all be uninstalled. Uninstalling them should really
    // take care of all the cleanup these modules should be doing. But enable them
    // here for good measure just incase there's some cleanup depending on these hooks.
    require_once DRUPAL_ROOT . '/includes/install.inc';
    $modules = system_rebuild_module_data();
    $enable_for_scrub = array();
    foreach ($modules as $module) {
      // Disabled modules with schema_version > -1 have not been uninstalled.
      if (empty($module->status) && $module->schema_version > SCHEMA_UNINSTALLED) {
        $enable_for_scrub[] = $module->name;
      }
    }
    // Get a list of disabled dependencies. These will get automatically enabled during
    // module_enable(), but we want to be able to disable and uninstall them explicitly later.
    foreach ($enable_for_scrub as $dependent) {
      foreach (array_keys($modules[$dependent]->requires) as $dependency) {
        // Use isset() to make sure the module is still in the filesystem before
        // trying to enable it. (Historically there have been modules in Gardens
        // which were disabled but then removed from the codebase without ever
        // uninstalling them, and we don't want to try to enable those now,
        // because it will fail.)
        if (isset($modules[$dependency]) && empty($modules[$dependency]->status)) {
          $enable_for_scrub[] = $dependency;
        }
      }
    }
    module_enable($enable_for_scrub);

    if (!$this->event->context['scrub_options']['retain_users_and_content']) {
      // Delete all users who aren't OpenID-linked site administrators. Content
      // modules are responsible for deleting the user's content in
      // hook_user_delete().
      $result = db_query('SELECT uid FROM {users} WHERE uid NOT IN (:uids)', array(':uids' => array(0, 1)));
      foreach ($result as $record) {
        $account = user_load($record->uid, TRUE);
        db_update('file_managed')
        ->fields(array(
          'uid' => 0,
        ))
        ->condition('uid', $account->uid)
        ->execute();
        // Now we can delete the user.
        user_delete($account->uid);
      }

      // Delete nodes and comments authored by anonymous users.
      $nids = db_query('SELECT nid FROM {node} WHERE uid=0')->fetchCol();
      node_delete_multiple($nids);
      if (module_exists('comment')) {
        $cids = db_query('SELECT cid FROM {comment} WHERE uid=0')->fetchCol();
        comment_delete_multiple($cids);
      }

      // Delete all remaining comments (i.e., ones authored by the administrators).
      // Yeah, this makes the above query for anonymous comments redundant, but the
      // separation allows for either one to be commented out.
      if (module_exists('comment')) {
        $cids = db_query('SELECT cid FROM {comment}')->fetchCol();
        comment_delete_multiple($cids);
      }
    }

    // Remove all temporary files. This code is copied from system_cron() so see
    // that function for more details. Some highlights:
    // - It's unclear if the status field can ever be anything other than 0
    //   (temporary) or 1 (permanenet), but system_cron() uses the bitwise &
    //   operator, so apparently, it thinks additional status bit fields are
    //   possible.
    // - It's unclear why <> is used instead of != for ("not equal").
    // - Separate placeholders are used instead of a single ":permanent" due to
    //   a bug in some PHP versions (see system_cron() for the d.o. issue link).
    $result = db_query('SELECT fid FROM {file_managed} WHERE status & :permanent1 <> :permanent2', array(
      ':permanent1' => FILE_STATUS_PERMANENT,
      ':permanent2' => FILE_STATUS_PERMANENT,
    ));
    foreach ($result as $record) {
      if ($file = file_load($record->fid)) {
        file_delete($file);
      }
    }

    // Invalidate search indexes. If the search module has never been enabled,
    // then it's not enabled now and this block is skipped.
    if (module_exists('search')) {
      // Call this function to ensure that necessary hooks get called.
      search_reindex();
      // search_reindex() called globally (with no parameters) invokes hooks, but
      // does not truncate these tables, so we do so here.
      foreach (array('search_dataset', 'search_index', 'search_node_links', 'search_total') as $table) {
        db_delete($table)->execute();
      }
      // What a shame that neither search.module nor node.module provide an API
      // function for this.
      variable_del('node_cron_last');
    }

    // Empty site traffic statistics.
    foreach (array('accesslog', 'node_counter') as $table) {
      if (db_table_exists($table)) {
        db_delete($table)->execute();
      }
    }

    // Empty batch, queue, and semaphore tables.
    foreach (array('batch', 'queue', 'semaphore') as $table) {
      db_delete($table)->execute();
    }

    // Empty sessions.
    db_delete('sessions')->execute();

    // Flush all caches, including in-progress multistep forms that are not
    // normally wiped during drupal_flush_all_caches().
    drupal_flush_all_caches();
    cache_clear_all('*', 'cache_form', TRUE);

    // Re-disable modules that were disabled prior to this function, and enabled
    // only for scrubbing.
    module_disable($enable_for_scrub);
    // Uninstall these modules. Drupal will drop their tables and any orphaned
    // data remaining in them.
    drupal_uninstall_modules($enable_for_scrub);

    // Also clear the theme field so all users will use the default theme.
    db_update('users')
      ->fields(array(
        'theme' => 0,
    ))
    ->execute();

    // Begin the site without any watchdog records.
    if (db_table_exists('watchdog')) {
      db_delete('watchdog')->execute();
    }
  }

}
