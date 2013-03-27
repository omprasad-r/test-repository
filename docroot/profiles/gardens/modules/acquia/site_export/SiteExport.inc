<?php

/**
 * Creates a tar.gz file containing a site export.
 */
class SiteExport {
  private static $siteName = NULL;

  /**
   * Constructor for the SiteExport class.
   *
   * @param {Boolean} $excludeSiteFiles
   *   If FALSE the site-specific files will be excluded from the site
   *   export.  This may be necessary if the site is very large
   *   causing the archive to be too large to be downloaded without
   *   losing the connection.
   */
  function __construct($excludeSiteFiles = FALSE) {
    // Make sure the site name is configured.
    if (empty(self::$siteName)) {
      global $conf;
      self::$siteName = empty($conf['site_name']) ? 'site_export' : $conf['site_name'];
    }
    $this->excludeSiteFiles = $excludeSiteFiles;
  }

  /**
   * Generates a name for a file being exported.  Does not include an extension.
   *
   * @return {String}
   *   The file name.
   */
  public static function createFilename() {
    return preg_replace('/[^[:alnum:]]/', '_', self::$siteName);
  }

  /**
   * Export docroot and database as a single tar.gz file.
   *
   * The site-specific files will be replaced with a script that uses
   * curl to retrieve them if this object was instantiated with
   * $excludeSiteFiles = TRUE.
   *
   * @return string
   *   The URI of the exported file.
   */
  public function export() {
    // Library for generating tar commands.
    include_once dirname(__FILE__) . '/CliTar.inc';

    $this->verifyTarVersion();

    $tarfile_uri = $this->createRandomURI('tar');
    $tarfile = drupal_realpath($tarfile_uri);
    $working_dir = $this->createWorkingDirectory();

    $this->createTarArchive($tarfile);
    $this->addDrupalFiles($tarfile);
    $this->addDatabaseAndCredsFiles($tarfile, $working_dir);
    $this->addSitesDirAndSettings($tarfile);
    $this->addSiteSpecificFiles($tarfile);

    if ($this->excludeSiteFiles) {
      // Add a script that can be used to download the site-specific files.
      // The script consists of a script file and a file that contains
      // a list of URIs that reference each file from the production site.
      $script_file = $this->createFileDownloadScript($working_dir);
      $file_list = $this->generateSiteFileList($working_dir);
      $this->addFilesToArchive($tarfile, array($script_file, $file_list), $working_dir, 'docroot/', '/');

      // Also add a readme that explains how to get the site-specific files.
      $this->addReadmeToArchive($tarfile);
    }

    // Include files that need to have certain lines stripped out of them before
    // being included in the site export.
    $files_to_strip_lines_from = $this->getFilesToStrip();

    $edited_files = array();
    foreach ($files_to_strip_lines_from as $file) {
      $edited_files[] = $this->stripFile($file, $working_dir);
    }
    foreach (array('gardens.profile', 'gardens.install') as $filename) {
      $path = $working_dir . '/profiles/gardens/' . $filename;
      $edited_files[] = $this->replaceProfileContents($path);
    }
    $this->addFilesToArchive($tarfile, $edited_files, $working_dir, 'docroot/', '/');

    // Include a settings.php
    $settings_file = $this->modifySettingsFile($working_dir);
    $this->addFilesToArchive($tarfile, array($settings_file), dirname($settings_file), 'docroot/sites/default/', '/');

    // Delete the temporary working directory and all of its contents.
    exec('rm -rf ' . escapeshellarg($working_dir));

    return $this->compressArchive($tarfile, $tarfile_uri);
  }

  /**
   * Verifies the tar version will work with the command line options we will be using.
   * @throws Exception if the tar command will not be suitable.
   */
  private function verifyTarVersion() {
    $version_info = CliTar::getVersion();
    if ($version_info['type'] == 'bsdtar') {
      throw new Exception('BSD tar is not supported');
    }
    if (version_compare($version_info['version'], '1.17', 'lt')) {
      throw new Exception('Your tar utility is too old - get 1.17 or above.');
    }
  }

  /**
   * Causes the tar file to be created.
   *
   * This is done mostly to normalize the other parts of the code that
   * in general simply append to the existing tar file.  Without this,
   * one of the items that is added to the tar file will break that
   * pattern by actually having to create the tar file as well,
   * causing an order dependence that is not desirable.
   *
   * @param {String} $tarfile
   *   The tar file that should be created.
   */
  private function createTarArchive($tarfile) {
    if (file_exists($tarfile)) {
      @unlink($tarfile);
    }
    @touch($tarfile);
  }

  /**
   * Adds the Drupal files to the specified tar file.
   *
   * @param {String} $tarfile
   *   The tar file that the drupal files will be appended to.
   */
  private function addDrupalFiles($tarfile) {
    // All hidden modules are in modules/acquia.
    $exclude_patterns = $this->getDirectoriesToRemove();
    $exclude_patterns += $this->getFilesToStrip();
    $exclude_patterns += $this->getFilesToRemove();

    $tar_cmd = new CliTar($tarfile);
    $tar_cmd->compression_mode = '';
    $tar_cmd->setMode('r');

    foreach ($exclude_patterns as $pattern) {
      $tar_cmd->setOption('--exclude', $pattern, TRUE);
    }
    $tar_cmd->setOption('-C', dirname(DRUPAL_ROOT));
    $tar_cmd->setOption('--mode', 'u+rw,g+r,o+r');
    // Add the docroot
    $tar_cmd->add('docroot');
    $output = '';
    $result = NULL;
    exec((string)$tar_cmd, $output, $result);
    if ($result != 0 || !file_exists($tarfile)) {
      throw new Exception('Unknown Error exporting site: ' . var_export($output, 1));
    }
  }

  /**
   * Adds a scrubbed copy of the site database and user credential files to the specified tar file.
   *
   * These two files are generated together because they are
   * interdependent.  As we scrub we will assign new passwords to the
   * users, and those passwords are placed into the database and the
   * credentials file.
   *
   * @param {String} $tarfile
   *   The tar file to which these files should be appended.
   * @param {String} $working_dir
   *   The temporary working directory used when building the tar file.
   */
  private function addDatabaseAndCredsFiles($tarfile, $working_dir) {
    // Generate new passwords for user1 and all the site owner and
    // write them to a credentials file in the private files directory.
    $updates = array();
    $output = "user\tpassword\n";
    require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
    $statement['table'] = 'users';
    do {
      $user1 = 'user1_'. substr(uniqid(), 0, 6);
    } while(db_query('SELECT name FROM {users} WHERE name = :name', array(':name' => $user1))->fetchField());
    $newpass = user_password();
    $newhash = user_hash_password($newpass);
    $statement['data'] = array('pass' => $newhash, 'name' => $user1, 'mail' => $user1 . '@example.com');
    $statement['where'] = 'uid = 1';
    $output .= "$user1\t$newpass\n";
    $updates[] = $statement;
    if (module_exists('scarecrow')) {
      foreach (scarecrow_list_site_owner_uids() as $uid) {
	$account = db_query('SELECT name FROM {users} WHERE uid = :uid', array(':uid' => $uid))->fetchObject();
	$newpass = user_password();
	$newhash = user_hash_password($newpass);
	$statement['table'] = 'users';
	$statement['data'] = array('pass' => $newhash);
	$statement['where'] = 'uid = '. $uid;
	$output .= $account->name . "\t$newpass\n";
	$updates[] = $statement;
      }
    }

    $conf_path = conf_path();
    // Fix paths of all Theme Builder themes.
    $statement = array();
    $statement['table'] = 'system';
    $statement['func'] = array('filename' => "REPLACE(filename, '$conf_path', 'sites/default')");
    $statement['where'] = "filename LIKE '$conf_path%'";
    $updates[] = $statement;

    // Export the database
    $modules_to_remove = $this->getModulesToRemove();
    $export_file = $this->exportDB($working_dir, $modules_to_remove, $updates);
    $creds_file =  $working_dir . "/credentials-" . self::createFilename() . "-${user1}.txt";
    file_put_contents($creds_file, $output);

    $this->addFilesToArchive($tarfile, array($creds_file, $export_file), $working_dir, 'docroot/', '/');
  }

  /**
   * Adds a sites directory and default settings file to the specified archive.
   *
   * @param {String} $tarfile
   *   The path to the archive.
   */
  private function addSitesDirAndSettings($tarfile) {
    $tar_cmd = new CliTar($tarfile);
    $tar_cmd->compression_mode = '';
    $tar_cmd->setMode('r');
    $tar_cmd->setOption('-C', dirname(DRUPAL_ROOT));
    $tar_cmd->setOption('--no-recursion', '');
    $tar_cmd->setOption('--mode', 'u+rw,g+r,o+r');
    $tar_cmd->add('docroot/sites');
    $tar_cmd->add('docroot/sites/default/default.settings.php');
    $output = "";
    $result = null;
    exec((string)$tar_cmd, $output, $result);
  }


  /**
   * Returns a list of module names that should not be included in the export.
   *
   * NOTE: Be careful; apparently by design this only works for modules in
   * modules/acquia now (as well as for installation profiles).
   *
   * @return Array
   *   An array of module names to exclude.
   */
  private function getModulesToRemove() {
    return array(
      // Actual modules.
      'akamai',
      'antivirus',
      'audit_trail',
      'blog_template',
      'campaign_template',
      'gardens_client',
      'gardens_cron',
      'gardens_duplication',
      'gardens_help',
      'gardens_misc',
      'gardens_limits',
      'jsunit',
      'mailhandler',
      'mailhandler_singlemailbox',
      'product_template',
      'scarecrow',
      'site_export',
      'site_template',
      'site_theme_settings',
      'siteinfo',
      'themebuilder_log',
      'themebuilder_advanced',
      'themebuilder_brand',
      'themebuilder_bar',
      'themebuilder_blocks',
      'themebuilder_compiler',
      'themebuilder_development_ui',
      'themebuilder_layout',
      'themebuilder_marketplace',
      'themebuilder_monotype',
      'themebuilder_resources',
      'themebuilder_screenshot',
      'themebuilder_styleedit',
      'themebuilder_themes',
      'themebuilder_typekit',
      'themebuilder_test',
      'thumbgen',
    );
  }

  /**
   * Returns a list of files that should not be included in the export.
   */
  private function getFilesToRemove() {
    return array(
      '.git',
      'settings.php',
      'profiles/gardens/gardens.install',
      'profiles/gardens/gardens.profile',
    );
  }

  /**
   * Returns a list of directories (not associated with individual modules) that
   * should not be included in the export.
   *
   * @return {Array}
   *   The set of directories that should be excluded from the export.
   */
  private function getDirectoriesToRemove() {
    return array(
      // We want to exclude all of modules/acquia.
      'profiles/gardens/modules/acquia',
      // We store some data in the top-level files directory intended to be
      // shared between Gardens sites, but we don't want to include that on
      // export.
      'docroot/files',
      // We don't want to copy everyone's site.  We'll come back for sites/all
      // and sites/whatevermysiteis later.
      'sites',
      // Exclude the WMG theme.
      'themes/acquia/wmg',
    );
  }

  /**
   * Return the list of files that need to have certain lines stripped out of
   * them before being included in the site export.
   *
   * @return {Array}
   *   The list of filenames that need to be stripped.
   */
  private function getFilesToStrip() {
    $files = array(
      'index.php',
      '.htaccess',
      'profiles/gardens/modules/contrib/views_bulk_operations/actions/script.action.inc',
      'profiles/gardens/modules/gardens/gardens_statsd/gardens_statsd.install',
      'profiles/gardens/modules/gardens/gardens_statsd/gardens_statsd.module',
      'modules/aggregator/aggregator.pages.inc',
      'modules/block/block.module',
    );
    foreach (module_implements('gardens_site_export_strip') as $module) {
      $files = array_merge($files, module_invoke($module, 'gardens_site_export_strip'));
    }
    return $files;
  }

  /**
   * Causes the specified tar file to be compressed for download.
   *
   * @param {String} $tarfile
   *   The path to the tar file that should be compressed.
   * @param {String} $tarfile_uri
   *   The URI to the tar file.
   * @return {String}
   *   The URI of the compressed file.
   */
  private function compressArchive($tarfile, $tarfile_uri = NULL) {
    $gzip_cmd = 'gzip ' . $tarfile;
    $output = '';
    $return = NULL;
    exec($gzip_cmd, $output, $return);
    $tarfile_gz = "${tarfile}.gz";

    if ($return != 0 || !file_exists($tarfile_gz)) {
      throw new Exception('Unknown Error exporting site: ' . var_export($output, 1));
    }
    $result = $tarfile_uri;
    if (!empty($tarfile_uri)) {
      $result = $tarfile_uri . '.gz';
    }
    return $result;
  }

  /**
   * Clears data out of the database that we prefer not to share.
   *
   * The sanitization does not work with a compressed sql file.  Each
   * insert must be its own statement as well (use mysqldump's
   * --skip-extended-insert option).
   *
   * @param <String> &$contents
   *   The uncompressed sql contents from the sql dump.  This string
   *   will be modified as a result of the db scrub.
   * @param {Array} $modules_to_remove
   *   The set of modules that must be scrubbed.
   * @param {Array} $updates
   *   Database updates that will be appended to the resulting sql.
   */
  private function sanitizeDB(&$contents, $modules_to_remove = NULL, $updates = array()) {
    // NOTE: Always use pass by reference for the database contents to avoid multiple
    // copies in memory.
    $sqlEditor = new SqlEditor($contents);
    $sqlEditor
      ->dropTableData('sessions')
      ->dropTableData('registry_file')
      ->dropTableData('authmap')
      ->dropTableData('cache')
      ->dropTableData('cache_block')
      ->dropTableData('cache_bootstrap')
      ->dropTableData('cache_field')
      ->dropTableData('cache_file_styles')
      ->dropTableData('cache_filter')
      ->dropTableData('cache_form')
      ->dropTableData('cache_image')
      ->dropTableData('cache_media_xml')
      ->dropTableData('cache_menu')
      ->dropTableData('cache_page')
      ->dropTableData('cache_path')
      ->dropTableData('cache_styles')
      ->dropTableData('gardens_broadcasting_client_tasks')
      ->dropTableData('gardens_broadcasting_client_notifications')
      ->dropTableData('gardens_broadcasting_client_read_notifications')
      ->dropTableRow('system', 'profiles/gardens/gardens.profile', 0)
      ->dropTableRow('system', 'profiles/gardens_demo/gardens_demo.profile', 0)
      ->insertTableData('variable', "'menu_rebuild_needed', '" .serialize(TRUE) ."'")
      // Antivirus
      ->dropTableRow('system', 'sites/all/modules/antivirus/antivirus.module', 0)
      ->dropTableRow('variable', 'antivirus_settings_clamavdaemon', 0)
      ->dropTableRow('variable', 'antivirus_scanner_clamavdaemon_flags', 0)
      // Audit trail
      ->dropTableRow('variable', 'audit_trail_separator', 0)
      ->dropTableRow('variable', 'audit_trail_log_destination', 0)
      ->dropTableRow('variable', 'audit_trail_log_file', 0)
      ->dropTableRow('variable', 'audit_trail_form_changes', 0)
      ->dropTableRow('variable', 'audit_trail_form_changes_path', 0)
      ->dropTableRow('variable', 'audit_trail_form_submissions', 0)
      ->dropTableRow('variable', 'audit_trail_form_submissions_path', 0)
      ->dropTableRow('variable', 'audit_trail_form_management_link', 0)
      // Gardens Stats
      ->dropTableRow('variable', 'gardens_statsd_env_checked', 0)
      ->dropTableRow('variable', 'gardens_statsd_enabled', 0)
      ->dropTableRow('variable', 'gardens_statsd_prefix', 0)
      ->dropTableRow('variable', 'gardens_statsd_suffix', 0)
      ->dropTableRow('variable', 'gardens_statsd_host', 0)
      ->dropTableRow('variable', 'gardens_statsd_host_ip', 0)
      ->dropTableRow('variable', 'gardens_statsd_port', 0)
      ->dropTableRow('variable', 'gardens_statsd_stage', 0)
      // Mollom settings.
      ->dropTableRow('variable', 'mollom_public_key', 0)
      ->dropTableRow('variable', 'mollom_private_key', 0)
      // Themebuilder values.
      ->dropTableRow('variable', 'themebuilder_screenshot_access_key', 0)
      ->dropTableRow('variable', 'themebuilder_screenshot_private_key', 0)
      ->dropTableRow('variable', 'themebuilder_update_level', 0)
      // Gardens domain management.
      ->dropTableRow('variable', 'gardens_misc_canonical_domain', 0)
      ->dropTableRow('variable', 'gardens_misc_standard_domain', 0)
      // Mailhandler settings.
      ->dropTableRow('variable', 'mailhandler_singlemailbox_default_mailbox_name', 0)
      ->dropTableRow('variable', 'mailhandler_singlemailbox_addressing_scheme', 0)
      ->dropTableRow('variable', 'mailhandler_temporary_path', 0)
      // Misc settings.
      ->dropTableRow('variable', 'https', 0)
      ->dropTableRow('variable', 'file_temporary_path', 0)
      ->dropTableRow('variable', 'preprocess_css', 0)
      ->dropTableRow('variable', 'file_public_path', 0)
      // Embed.ly API key.
      ->dropTableRow('variable', 'oembedembedly_api_key', 0)
      // Akamai related settings.
      ->dropTableRow('variable', 'akamai_wsdl', 0)
      ->dropTableRow('variable', 'akamai_basepath', 0)
      ->dropTableRow('variable', 'akamai_username', 0)
      ->dropTableRow('variable', 'akamai_password', 0)
      ->dropTableRow('variable', 'akamai_domain', 0)
      ->dropTableRow('variable', 'akamai_action', 0)
      ->dropTableRow('variable', 'akamai_email', 0)
      // WMG-specific settings.
      ->dropTableRow('menu_links', 'admin/structure/block/list/wmg', 3)
      ->dropTableRow('menu_links', 'admin/structure/block/list/wmg/add', 3)
      ->dropTableRow('menu_router', 'admin/structure/block/demo/wmg', 0)
      ->dropTableRow('menu_router', 'admin/structure/block/list/wmg', 0)
      ->dropTableRow('menu_router', 'admin/structure/block/list/wmg/add', 0)
      ->dropTableRow('system', 'themes/acquia/wmg/wmg.info', 0)
      ->dropTableRow('xmlsitemap', 'admin/structure/block/list/wmg', 3)
      ->dropTableRow('xmlsitemap', 'admin/structure/block/list/wmg/add', 3)
      // Empty preprocessed CSS list.
      ->insertTableData('variable', "'preprocess_css', '" .serialize(0) ."'")
      // Reset file path.
      ->insertTableData('variable', "'file_public_path', '" . serialize('sites/default/files') . "'");

    $this->resetUserEmails($sqlEditor);

    if ($modules_to_remove !== NULL) {
      foreach ($modules_to_remove as $module_name) {
	$sqlEditor->removeModule($module_name);
      }
      // If we are deleting the current install profile, replace it with the
      // default profile.
      if (in_array(variable_get('install_profile', 'standard'), $modules_to_remove)) {
	$sqlEditor->dropTableRow('variable', 'install_profile', 0);
	$sqlEditor->insertTableData('variable', "'install_profile', '" . serialize('standard') . "'");
      }
      // Hack to get all caches flushed including file paths.
      $sqlEditor->insertTableData('variable', "'gardens_misc_flush_all_caches', '" . serialize(TRUE) ."'");
    }
    foreach ($updates as $statement) {
      $sqlEditor->updateTableData($statement);
    }
  }

  /**
   * Helper function for the sanitizeDB() method.
   *
   * Delete Gardens-specific versions of user notification emails ("Your account
   * has been approved!" etc.) saved by the Gardens installer.
   *
   * @param {SqlEditor} $sqlEditor
   *   the SqlEditor instance.
   */
  private function resetUserEmails($sqlEditor) {
    require_once DRUPAL_ROOT . '/' . drupal_get_path('module', 'gardens') . '/gardens.install';

    // Retrieve the user emails as the installation process originally set
    // them, and reset any that haven't changed since then. This will make them
    // automatically fall back on the Drupal core defaults (rather than the
    // Drupal Gardens-specific defaults) on the exported site.
    $original_settings = gardens_setup_user_mail_variables();
    $this->resetVariables($original_settings, $sqlEditor);
  }

  /**
   * Deletes variables set at install time.
   *
   * Make sure that we only tamper with these variables if they haven't been
   * edited by the site administrator. If the administrator hasn't touched the
   * variable, we delete it so that the Drupal default will take effect.
   *
   * @param Array $original_settings
   *   An associative array of variables set at install time, where the key is
   *   the variable name, and the value is the contents of the variable.
   * @param SqlEditor $sqlEditor
   *   The SqlEditor object being used to export the site.
   */
  private function resetVariables($original_settings, &$sqlEditor) {
    foreach (gardens_misc_get_unchanged_variables($original_settings) as $variable) {
      $sqlEditor->dropTableRow('variable', $variable, 0);
    }
  }

  /**
   * Creates a simple script that can be used to dowload all of the files in the .../files directory.
   *
   * This is important for large sites for which the download would be
   * interrupted.  On most browsers the download through our balancer will be
   * interrupted resulting in either no site export or a truncated one.  This
   * function generates a script that will allow the user to download their
   * files after their site has been exported, making the site export archive
   * significantly smaller on sites with a large files directory.
   *
   * @param {String} $dir
   *   The path to the directory in which the script should be written.
   */
  private function createFileDownloadScript($dir) {
    $name = 'get_files.sh';
    $filename = "${dir}/${name}";

    $script = <<<EOT
#!/bin/bash

# Note: This script must be executed from the docroot.
curl -R --create-dirs -K ./file_list.txt
EOT;
    file_put_contents($filename, $script);
    chmod($filename, 0755);
    return $filename;
}

  /**
   * Generates a file that contains a list of site-specific files to be downloaded.
   *
   * This file will have the format:
   *   url = "http://example.com/sites/example.com/files/file1.jpg"
   *   output = "sites/default/files/file1.jpg"
   *
   * The file is formatted to be used with the curl command to
   * efficiently download the site's files and place them in the
   * proper location.
   *
   * @param {String} $dir
   *   The location where the site file list should be written.
   * @return {String}
   *   The filename of the site file list.
   */
  private function generateSiteFileList($dir) {
    $filename = "${dir}/file_list.txt";

    $script = array();
    $lines = $this->getFileDownloadLines();
    file_put_contents($filename, implode("\n", $lines));
    return $filename;
  }

  /**
   * Exports the default sql database used in Drupal
   *
   * @global array $databases
   *
   * @param {String} $working_dir
   *   The working directory where the sql file can be written.
   * @param {Array} $modules_to_remove
   *   The set of modules that must be scrubbed.
   * @param {Array} $updates
   *   Database updates that will be appended to the resulting sql.
   * @return {string}
   *   The path of the file containing the db export
   */
  private function exportDB($working_dir, $modules_to_remove = NULL, $updates = array()) {
    global $databases;
    drupal_flush_all_caches();
    $db = $databases['default']['default'];
    $name = self::createFilename();
    $ext = "sql";

    $path = "${working_dir}/${name}.${ext}";

    /**
     * There is no need to export tables for modules we are removing.
     */
    $tables_to_ignore = '';
    $modules_to_remove = $this->getModulesToRemove();
    foreach ($modules_to_remove as $module) {
      $schema = drupal_get_schema_unprocessed($module);
      $tables = array_keys($schema);
      foreach ($tables as $table) {
	$tables_to_ignore .= '--ignore-table=' . escapeshellarg($db['database'] . '.' . $table) . ' ';
      }
    }

    // Be sure to use the --skip-extended-insert option to force individual
    // insert statements.  This is required by the SqlEditor, which edits the
    // database dump.
    $cmd = sprintf('mysqldump %s -u%s %s --skip-extended-insert --single-transaction -h%s %s',
      $tables_to_ignore,
      escapeshellarg($db['username']),
      empty($db['password']) ? '' : '-p' . escapeshellarg($db['password']),
      escapeshellarg($db['host']),
      escapeshellarg($db['database'])
    );

    $contents = array();
    exec($cmd, $contents);
    $contents = implode("\n", $contents) ."\n";


    $this->sanitizeDB($contents, $modules_to_remove, $updates);
    file_put_contents($path, $contents);
    return $path;
  }

  /**
   * Creates a random name for a temporary file.
   *
   * @param {String} $ext
   *   An extension for the file. Defaults to "tmp" (e.g. 1234.tmp).
   * @return {String}
   *   The URI to the temporary file.
   */
  private function createRandomURI($ext = 'tmp') {
    $temp_name = preg_replace('/([ ])/e', 'chr(rand(97,122))', '            ');
    return 'temporary://' . $temp_name . ($ext === '' ? '' : ".$ext");
  }

  /**
   * Creates a version of the provided file that strips out special lines in the
   * file that are marked as not being appropriate for site export, and puts the
   * new file in the provided directory.
   *
   * @param $original_file
   *   The path of the file to use (relative to DRUPAL_ROOT).
   * @param $destination_directory
   *   The directory in which the modified file will be placed.
   *
   * @return
   *   The path to the newly-created file.
   */
  private function stripFile($original_file, $destination_directory) {
    $source = DRUPAL_ROOT . '/' . $original_file;
    $path = $destination_directory . '/' . $original_file;
    $contents = file_get_contents($source);
    $new_contents = preg_replace('/##GardensExcludeFromExportStart.*?GardensExcludeFromExportEnd##/s', '', $contents);
    $directory = dirname($path);
    if (!is_dir($directory)) {
      mkdir($directory, 0755, TRUE);
    }
    file_put_contents($path, $new_contents);
    chmod($path, 0644);
    return $path;
  }

  /**
   * Creates a sanitized .install or .profile file at the specified path.
   *
   * @param $path
   *   The full path where the modified file is to be placed.
   *
   * @return
   *   The path to the newly-created file.
   */
  private function replaceProfileContents($path) {
    preg_match('/\.(install|profile)$/', $path, $matches);
    if (empty($matches)) {
      return;
    }
    switch ($matches[1]) {
      case 'install':
        $new_contents = <<<EOT
<?php

/**
 * Implement hook_install().
 *
 * Perform actions to set up the site for this profile.
 */
function gardens_install() {
  include_once DRUPAL_ROOT . "/profiles/standard/standard.install";
  standard_install();
}
EOT;
        break;
      case 'profile':
        $new_contents = <<<EOT
<?php

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Allows the profile to alter the site configuration form.
 */
function gardens_form_install_configure_form_alter(&\$form, \$form_state) {
  // Pre-populate the site name with the server name.
  \$form["site_information"]["site_name"]["#default_value"] = \$_SERVER["SERVER_NAME"];
}
EOT;
        break;
    }

    $directory = dirname($path);
    if (!is_dir($directory)) {
      mkdir($directory, 0755, TRUE);
    }
    file_put_contents($path, $new_contents);
    chmod($path, 0644);
    return $path;
  }

  /**
   * Creates a settings file in the specified directory.  This settings file
   * will have database connection settings that will likely cause a db
   * connection error which will hopefully prompt the user to change the
   * settings.
   *
   * @param {String} $dir
   *   The directory in which the settings.php should be written.
   * @return {String}
   *   The path to the modified settings file.
   */
  private function modifySettingsFile($dir) {
    $path = $dir . '/settings.php';
    $default = DRUPAL_ROOT . '/sites/default/default.settings.php';
    $contents = file_get_contents($default);
    $matches = array();
    $replace = <<<EOT
\$databases = array (
  'default' =>
  array (
    'default' =>
    array (
      'driver' => 'mysql',
      'database' => 'database',
      'username' => 'user',
      'password' => 'changeme',
      'host' => 'localhost',
      'port' => '',
    ),
  ),
);
EOT;
    $result = preg_replace('/^\$databases \= array\(\)\;$/ms', $replace, $contents);
    // Check matches, make sure I got before, line, after.
    file_put_contents($path, $result);
    chmod($path, 0600);
    return $path;
  }

  /**
   * Returns an array of lines for a text file that can be used to retrieve the files from the site on production.
   *
   * This function is used to generate a file that can be used to download
   * files for a site that uses a lot of disk space.
   *
   * @return {Array}
   *   An array of URLs that if used with wget would result in the export of the
   *   site-specific files.
   */
  private function getFileDownloadLines() {
    $lines = array();
    $conf_path = conf_path();
    $dir = DRUPAL_ROOT . "/${conf_path}/files";
    if (is_dir($dir)) {
      $this->getFileDownloadLinesRecursive($dir, $dir, $lines);
    }
    return $lines;
  }

  /**
   * Recurses into the specified directory, appending lines to the specified array to provide a mechanism for exporting files from website that uses a lot of disk space.
   *
   * @param {String} $dir
   *   The directory to inspect.  Any files within the specified directory will
   *   cause a line to be added to the $lines array.
   * @param {String} $dir_prefix
   *   The directory prefix, which is used to remove the uninteresting part of
   *   the path.  This is used to make the path in the corresponding command
   *   relative to the docroot of the exported site.
   * @param {Array} $lines
   *   An array to which a command string will be appended for every file in the
   *   files directory.
   */
  private function getFileDownloadLinesRecursive($dir, $dir_prefix, &$lines) {
    $handle = opendir($dir);
    while (FALSE !== ($file = readdir($handle))) {
      if ($file == '.' || $file == '..' || $file == '.htaccess') {
	continue;
      }
      $full_path = "${dir}/${file}";
      $relative_path = str_replace("${dir_prefix}/", '', $full_path);
      if ($this->omitFile($relative_path)) {
	continue;
      }
      if (is_dir($full_path)) {
	$this->getFileDownloadLinesRecursive($full_path, $dir_prefix, $lines);
      }
      else if (is_file($full_path)) {
	// This is a file.
	$this->addDownloadLine($relative_path, $lines);
      }
    }
    closedir($handle);
  }

  /**
   * Renders one or more lines in a file that contains URIs for each file within the site's files directory.
   *
   * This method renders the necessary text in the file for
   * downloading a particular file and placing it in the proper
   * location.
   *
   * @param {String} $file
   *   The file path relative to the conf_path/files directory.
   * @return {String}
   *   A string containing the URI for the specified file that can be
   *   used to download the file from the live site.
   */
  private function addDownloadLine($file, &$lines) {
    $url = url(conf_path() . "/files/$file", array('absolute' => TRUE));
    $lines[] = "url = \"${url}\"";
    $lines[] = "output = \"sites/default/files/${file}\"";
    $lines[] = ""; //Space between each entry
    return $url;
  }

  /**
   * Appends the files corresponding to the specified filenames to the specified tarfile.
   *
   * @param {String} $tarfile
   *   The path to the tarfile to which the files should be appended.
   * @param {Array} $filenames
   *   An array containing any number of paths to files that should be
   *   added to the archive.
   * @param {String} $from
   *   Indicates the prefix of the filename path that should be
   *   removed when adding the file into the archive.
   * @param {String} $to
   *   Indicates the prefix that should be added to the filename path
   *   (after the $from prefix is removed) so the file has the correct
   *   path within the archive.
   * @param {String} $dir
   *   The base directory where the files can be found.
   */
  private function addFilesToArchive($tarfile, $filenames, $from, $to, $dir = NULL) {
    if (empty($dir)) {
      $dir = dirname(DRUPAL_ROOT);
    }
    $tar_cmd = new CliTar($tarfile);
    $tar_cmd->compression_mode = '';
    $tar_cmd->setMode('r');
    $tar_cmd->setOption('-C', $dir);
    // Archive the content of symbolic links
    $tar_cmd->setOption('--dereference', '');
    for ($i = 0, $len = count($filenames); $i < $len; $i++) {
      $tar_cmd->add(trim($filenames[$i], '/'));
    }
    $tar_cmd->setOption('--transform', "s:" . trim($from, '/') . "/:${to}:", TRUE);
    $output = "";
    $return = null;
    $cmd = (string)$tar_cmd;
    exec((string)$tar_cmd, $output, $return);
    if ($return != 0 || !file_exists($tarfile)) {
      throw new Exception('Unknown Error exporting site: ' . var_export($output, 1));
    }
  }

  /**
   * Adds a readme file to the sites/default/files directory to let the user know how to populate their files directory.
   *
   * @param {String} $tarfile
   *   The filename of the tar file to append the readme file onto.
   */
  private function addReadmeToArchive($tarfile) {
    $module_path = drupal_get_path('module', 'site_export');
    $help_file = "README.txt";
    $this->addFilesToArchive($tarfile, array("docroot/${module_path}/${help_file}"), "docroot/${module_path}", 'docroot/sites/default/files/');
  }

  /**
   * Adds the site-specific files to the archive.
   *
   * @param {String} $tarfile
   *   The filename of the tar file to append the readme file onto.
   */
  private function addSiteSpecificFiles($tarfile) {
    $conf_path = conf_path();
    // Since we don't need to include all sites, just the one that belongs to the
    // person requesting the backup, we'll manually add sites/all and sites/example,
    $tar_cmd = new CliTar($tarfile);
    $tar_cmd->compression_mode = '';
    $tar_cmd->setMode('r');

    $tar_cmd->setOption('--mode', 'u+rw,g+r,o+r');
    $tar_cmd->setOption('-C', dirname(DRUPAL_ROOT));
    // Archive the content of symbolic links
    $tar_cmd->setOption('--dereference', '');
    $tar_cmd->add('docroot/sites/all');

    // Do not include site-specific modules in the export.
    $tar_cmd->setOption("--exclude", "${conf_path}/modules", TRUE);

    // Do not include the mailhandler temporary directory in the export.
    $tar_cmd->setOption("--exclude", "${conf_path}/files/gardens_temp", TRUE);

    $tar_cmd->add('docroot/' . $conf_path);

    $tar_cmd->setOption('--transform', 's:docroot/' . $conf_path . ':docroot/sites/default:', TRUE);

    // Only include the site files if the site is small enough that the download
    // won't be interrupted.
    if ($this->excludeSiteFiles) {
      $files_exclude = "docroot/${conf_path}/files/*";
      $tar_cmd->setOption("--exclude", $files_exclude, TRUE);
    }

    $output = "";
    $return = null;
    exec((string)$tar_cmd, $output, $return);
  }

  /**
   * Indicates whether the specified file should be omitted from the file that is used to retrieve site-specific files.
   *
   * @param {String} $fileneame
   *   The relative path to the file or directory to check.
   * @return {Boolean}
   *   TRUE if the specified file should be omitted; false otherwise.
   */
  private function omitFile($filename) {
    static $omit_files = array(
      'backup',
      'css',
      'ctools',
      'js',
      'styles',
      'xmlsitemap',
    );
    return in_array($filename, $omit_files);
  }

  /**
   * Creates a temporary working directory for assets that need to be generated during site export and then added to the tar file.
   *
   * @return {String}
   *   The path to the temporary working directory.
   */
  private function createWorkingDirectory() {
    $directory_uri = $this->createRandomURI('');
    $path = drupal_realpath($directory_uri);
    mkdir($path);
    return $path;
  }
}
