<?php
/**
 * @file
 * Drush commands to facilitate Gardens development.
 */

/**
 * Includes the Gardens Drush library file.
 */
require_once dirname(__FILE__) . '/gardens.drush.lib.inc';

/**
 * Defines Gardens' default Amazon S3 bucket name.
 */
define('GARDENS_DEFAULT_EXPORT_BUCKET_NAME', 'gardens-default.site-archives');

/**
 * Creates an exception class for throwing custom exceptions.
 */
class GardensDrushCmdExecutionException extends Exception{}

/**
 * Implements hook_drush_init().
 */
function gardens_drush_init() {
  gardens_drush_includes();
}

/**
 * Implements hook_drush_command().
 */
function gardens_drush_command() {
  $items = array();

  // Runs on a local Gardens site (e.g., 'gardens-trunk') to generate a new
  // local Gardens site (e.g., 'foo') with a copy of the database (scrubbed, of
  // course) and files from a remote site (e.g., 'foo.drupalgardens.com').
  $items['getsite'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_CONFIGURATION,
    'description' => "Gets a site from production, and installs it locally.",
    'arguments' => array(
      'sitename' => 'As in $sitename.drupalgardens.com',
      'tangle' => 'Like tangle001 or tangle002. You can find it at http://www.drupalgardens.com/content/$sitename.',
    ),
    'options' => array_map('_gardens_drush_option_description', gardens_drush_options('getsite')),
  );

  // Runs on the remote site to duplicate its db, scrub it, and dump it for
  // transfer back to the local machine that is running the 'getsite' command.
  $items['sql-dump-with-scrub'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_DATABASE,
    'arguments' => array(
      'database_server' => 'Database server to store scrubbed databases on (e.g., "fsdb-4"). This is optional and if not provided dbmaster-45 (the default for SMB Gardens) will be used.',
    ),
    'description' => "Duplicates a database, scrubs it, dumps it stdout.",
  );

  $items['gardens-sql-dump'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_DATABASE,
    'description' => "Dumps database from the slave server to stdout.",
  );

  $items['export'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'description' => "Exports a site and its database",
    'options' => array_map('_gardens_drush_option_description', gardens_drush_options('export')),
    'examples' => array(
      'drush export --memory-limit=1024M' => 'Export the site using a maximum of 1024 MB of RAM',
    ),
  );

  $items['export-s3'] = array(
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'description' => "Exports a site and its database, uploads the file to S3 and notifies the gardener",
    'options' => array_map('_gardens_drush_option_description', gardens_drush_options('export-s3')),
  );

  $items['gardens-pdf-regen-site'] = array(
    'description' => "Force regeneration of pdf versions of all pages on a site",
    'aliases' => array('gprs'),
    'options' => array_map('_gardens_drush_option_description', gardens_drush_options('gardens-pdf-regen-site')),
  );

  return $items;
}

/**
 * Returns a list of option descriptions and defaults.
 *
 * Descriptions are needed by hook_drush_command(). Defaults are needed by the
 * command callback. Combining the two in this central function improves code
 * maintainability.
 *
 * @see _gardens_drush_options_description()
 * @see _gardens_drush_options_defaults()
 */
function gardens_drush_options($command) {
  $options = array();
  switch ($command) {
    case 'getsite':
      $options = array(
        'no-files' => array('Do not rsync the files directory', FALSE),
        'theme-files-only' => array('Do not rsync the files directory, but do rsync the themes directory.'),
        'no-db' => array('Do not get the database', FALSE),
        'folder-template' => array('How the sites folder should look', '%sitename.%stage.dev'),
        'db-template' => array('How the created dbs should be named', 'gardens_%sitename'),
        'db-username' => array('Local database username', NULL),
        'db-password' => array('Local database password', NULL),
        'db-host' => array('Local database host', NULL),
        'db-port' => array('Local database port', NULL),
        'stage' => array('Hosting stage to get the site from; usually one of the production stages', 'gardens'),
        'managed-server' => array('Managed server associated with the tangle to run commands on (e.g., "managed-11"); usually determined automatically but it can be overridden if necessary', NULL),
        'database-server' => array('Database server to store scrubbed databases on (e.g., "fsdb-4"); usually determined automatically but it can be overridden if necessary', NULL),
        'remote-domain' => array('The domain name of the site; usually determined automatically but it can be overridden if necessary', NULL),
        'no-preserve-permissions' => array('Disallows preserving permission option for rsync. This switch might be useful to avoid permission errors on certain systems (eg. OSX Lion)', FALSE),
        'remote-username' => array('Username of the remote server. Defaults to the tangle name.', NULL),
      );
      break;

    case 'export-s3':
      $options = array(
        's3key' => array('Mandatory if --backup is provided.  The key to use for connecting to S3.', NULL),
        's3secret' => array('Mandatory if --backup is provided.  The secret key to use for connecting to S3.', NULL),
        'memory-limit' => array('The maximum memory to use.', NULL),
      );
    break;

    case 'export':
      $options = array(
        'memory-limit' => array('The maximum memory to use.', NULL),
        'exclude-files' => array('Excludes files from the archive - they will be available after running a script.  Do not use this option if the site is being destroyed after the site export!', NULL),
      );
    break;

    case 'gardens-pdf-regen-site':
      $options = array(
        'uid' => array('UID of the gardener user requesting PDF regeneration.', NULL),
      );
    break;
  }
  return $options;
}

/**
 * Helper function to return the description of an option.
 */
function _gardens_drush_option_description($option_value) {
  // Sometimes $option_value only contains one item.
  $description = array_shift($option_value);
  $default = $option_value ? array_shift($option_value) : NULL;

  if (is_bool($default)) {
    $default = $default ? "TRUE" : "FALSE";
  }
  if (!empty($default)) {
    $description .= ' (default: ' . $default . ')';
  }

  return $description;
}

/**
 * Returns the default value of an option.
 */
function _gardens_drush_option_default($option_value) {
  return $option_value[1];
}

/**
 * Implements drush_COMMAND_init().
 */
function drush_getsite_init() {
  // Set default options for the getsite command.
  $defaults = array_map('_gardens_drush_option_default', gardens_drush_options('getsite'));
  drush_set_context('default', $defaults);
}

/**
 * Drush command callback for fetching a Gardens site.
 */
function drush_gardens_getsite($sitename = '', $tangle = '') {
  global $databases;

  $version = DRUSH_VERSION;
  if (version_compare($version, '4.0', '<')) {
    drush_log("Error: You must run this command with at least Drush 4 (your current version is $version", 'error');
    return;
  }

  if (!$sitename) {
    drush_log('Error: You must provide a sitename. See drush help getsite for usage.', 'error');
    return;
  }
  if (!$tangle) {
    drush_log('Error: You must provide a tangle. See drush help getsite for usage.', 'error');
    return;
  }

  // Get the hostname, docroot, and site folder name of the remote site.
  $stage = drush_get_option('stage');

  $server = drush_get_option('managed-server');
  if (!$server) {
    $server = _gardens_drush_get_server_by_tangle($tangle, $stage);
  }

  // Format the managed server's FQDN.
  $hostname = "$server.$stage.hosting.acquia.com";

  // The remote-domain options makes it possible to clone sites with custom
  // domains such as www.example.com, which do not have a domain name in the
  // format example.drupalgardens.com.
  $remote_domain = drush_get_option('remote-domain');

  $domain = $remote_domain ? $remote_domain :
    $sitename . '.' . _gardens_drush_get_root_domain_name_by_stage($stage);

  $remote_docroot = "/mnt/www/html/$tangle/docroot";
  $remote_site_folder = "$remote_docroot/sites/$domain";

  // Get the database name and site folder name to use for the local copy.
  $tokens = array(
    '%sitename' => $sitename,
    '%stage' => $stage,
  );

  $database_name = str_replace(array_keys($tokens), array_values($tokens), drush_get_option('db-template'));

  $site_folder = DRUPAL_ROOT . '/sites/' . str_replace(array_keys($tokens), array_values($tokens), drush_get_option('folder-template'));

  // Create the local site folder.
  _gardens_drush_create_site_folder($site_folder);

  // Store the credentials we will use for connecting to the local database.
  $username = drush_get_option('db-username', $databases['default']['default']['username']);
  $password = drush_get_option('db-password', $databases['default']['default']['password']);
  $host = drush_get_option('db-host');
  $port = drush_get_option('db-port');
  $database = $databases['default']['default'];
  $database['database'] = $database_name;

  // Set the local database driver to mysql. This is required by settings.php.
  $database['driver'] = 'mysql';

  // Only override each entry in the $database array if an override was
  // provided.
  $overridable_properties = array('username', 'password', 'host', 'port');
  foreach ($overridable_properties as $variable) {
    // Check for the variables $username, $password, $host, and $port, and set
    // them in the database array if any of them were provided.
    if ($$variable) {
      $database[$variable] = $$variable;
    }
  }

  try {
    $remote_username = drush_get_option('remote-username');
    if (!$remote_username) {
      $remote_username = $tangle;
    }

    // Unless the no-db option was passed, retrieve a database dump from the
    // remote site.
    if (!drush_get_option('no-db')) {
      $connection_string = _gardens_drush_get_local_db_connect_string($username, $password, $host, $port);

      // Create the local databse.
      _gardens_drush_create_db($database_name, $connection_string);

      // Create the SSH connection string.
      $ssh = "ssh -o StrictHostKeyChecking=no -C $remote_username@$hostname";

      // Get a temp file location for the dump file.
      $system_temp_dir = sys_get_temp_dir();

      // Calculate the database server from which to dump the database.
      $database_server = drush_get_option('database-server');
      if (!$database_server) {
        $database_server = _gardens_drush_get_database_server_by_stage($stage);
      }

      // Create the database dump command string.
      $drush = "/mnt/www/html/$tangle/drush/drush --include=../hosting-drush";
      $db_command = "$drush --uri=$domain gardens-sql-dump";

      // Set the SQL dump filepath.
      $temp_dump_file = "$system_temp_dir/gardens_$sitename.sql";

      // Create the remote SSH command string.
      $ssh_command = "$ssh \"cd $remote_docroot && sudo $db_command\" > $temp_dump_file";

      // Invoke the database dump command remotely via SSH.
      gardens_drush_exec($ssh_command);

      if (filesize($temp_dump_file) == 0) {
        // If there was an error running the dump, abort. Unfortunately, Drush
        // doesn't support status codes, so if the command bails somehow, we
        // don't have a good way to detect it. This is the best we can do at the
        // moment.
        throw new Exception('Unable to continue, database dump failed. Run with -sv to debug the command');
      }
      drush_log("Got database and dumped to $temp_dump_file", 'success');

      // Import the database.
      $import_command = sprintf('mysql %s %s < %s', $connection_string, $database_name, $temp_dump_file);
      gardens_drush_exec($import_command);

      // Scrub the database.
      $import_command = sprintf('mysql %s %s < %s', $connection_string, $database_name, dirname(dirname(__FILE__)) . '/gsite-scrub.sql');
      gardens_drush_exec($import_command);
      $import_command = sprintf('mysql %s %s < %s', $connection_string, $database_name, dirname(dirname(__FILE__)) . '/gsite-scrub-local-dev.sql');
      gardens_drush_exec($import_command);
    }

    // Build the settings.php file.
    $variables = array(
      'update_free_access' => TRUE,
      'conf' => array(
        'acquia_gardens_local_user_accounts' => TRUE,
      ),
    );
    $settings = _gardens_drush_build_settings_php($database, $variables);
    file_put_contents($site_folder . '/settings.php', $settings);

    // Unless the no-files or theme-files-only options are passed, fetch the
    // files directory from the remote site.
    if (!drush_get_option('no-files') && !drush_get_option('theme-files-only')) {
      _gardens_drush_rsync_exec($site_folder, $remote_site_folder, $remote_username, $hostname);
      drush_log('Downloaded files', 'success');
    }
    // If the theme-files-only options is passed, fetch the theme files from the
    // remote site.
    elseif (drush_get_option('theme-files-only')) {
      _gardens_drush_create_site_folder($site_folder . '/files');
      $folder = "$site_folder/themes";
      _gardens_drush_rsync_exec($folder, $remote_site_folder, $remote_username, $hostname);
      drush_log('Downloaded theme files', 'success');
    }
  }

  // Catch GardensDrushCmdExecutionException if needed.
  catch (Exception $e) {
    drush_log($e->getMessage(), 'error');
    exit;
  }

  drush_log("Successfully retreived $sitename. All passwords set to 'admin', user 1 name is also set to admin");
}

/**
 * Executes an rsync command depending on the desired location (files or theme).
 *
 * @param String $local_folder
 *  The path of the local folder to sync.
 * @param String $remote_folder
 *  The path of the remote folder whose contents you wish to sync.
 * @param String $user
 *  The remote username.
 * @param String $host
 *  The remote hostname.
 */
function _gardens_drush_rsync_exec($local_folder, $remote_folder, $user, $host) {
  $p = drush_get_option('no-preserve-permissions') ? '' : 'p';
  // rsync -a is exactly the same as rsync -rlptgoD, where -p is
  // preserve permissions
  $cmd = "rsync -rl{$p}tgoDz -e \"ssh -o StrictHostKeyChecking=no\" --exclude=settings.php --exclude=private --exclude=files/styles " . escapeshellarg("{$user}@{$host}:$remote_folder/") . ' ' . escapeshellarg("$local_folder");
  gardens_drush_exec($cmd);
}

/**
 * Returns a connection string for connecting to local databases.
 */
function _gardens_drush_get_local_db_connect_string($username, $password = NULL, $host = 'localhost', $port = NULL) {
  $username = "-u{$username}";
  $password = !empty($password) ? "-p$password" : '';
  $host = !empty($host) ? "-h$host" : '';
  $host .= !empty($port) ? ":$port" : '';
  return "$username $password $host";
}

/**
 * Creates a local database using a provided connection string.
 *
 * @throws GardensDrushCmdExecutionException
 *
 * @param string $database_name
 *  The name of the database to be created
 */
function _gardens_drush_create_db($database_name, $connection_string) {
  $cmd = "mysql {$connection_string} -e 'DROP DATABASE IF EXISTS {$database_name};'";
  gardens_drush_exec($cmd);

  $cmd = "mysql {$connection_string} -e 'CREATE DATABASE {$database_name};'";
  gardens_drush_exec($cmd);
}

/**
 * Generates a settings file with the passed variables and database credentials.
 *
 * @param array $database
 *  A database definition as used in settings.php.
 *
 * @param array $variables
 *  Key/value pairs which should be written as $key = $value.
 */
function _gardens_drush_build_settings_php($database, $variables = array()) {
  require_once DRUPAL_ROOT . '/includes/install.inc';

  $settings['databases'] = array(
    'value' => array(
      'default' => array(
        'default' => $database,
      ),
    ),
    'required' => TRUE,
  );

  foreach ($variables as $key => $value) {
    $settings[$key] = array(
      'value' => $value,
      'required' => TRUE,
    );
  }

  return drupal_generate_settings_file_content('sites/default/default.settings.php', $settings);
}

function _gardens_drush_create_site_folder($site_folder) {
  if (file_exists($site_folder) && is_dir($site_folder)) {
    drush_log("Site folder $site_folder exists, not creating.", 'success');
  }
  else {
    drush_log("Site folder $site_folder does not exist, creating.", 'success');
    drush_op('mkdir', $site_folder);
  }
}

/**
 * Returns the server from hardcoded lists corresponding tangles to servers.
 *
 * @param string $tangle
 *  The name of the tangle to get the server for.
 * @param string $stage
 *  The name of the stage in which the tangle lives.
 *
 * @return string
 *  The server corresponding to that tangle.
 */
function _gardens_drush_get_server_by_tangle($tangle, $stage) {
  $map = array(
    'tangle001' => 'managed-47',
    'tangle002' => 'managed-52',
    'tangle003' => 'managed-120',
    'tangle004' => 'managed-89',
    'tangle005' => 'managed-140',
    'tangle006' => 'managed-159',
    'tangle007' => 'managed-182',
    'tangle008' => 'managed-203',
  );
  // Enterprise Gardens stages use a different map.
  switch ($stage) {
    case 'wmg-egardens':
    case 'fpmg-egardens':
    case 'utest':
      $map['tangle001'] = 'managed-11';
      $map['tangle002'] = 'managed-46';
      $map['tangle003'] = 'managed-78';
      break;

    case 'enterprise-g1':
      $map['tangle001'] = 'managed-16';
      break;

    case 'umg-egardens':
      $map['tangle001'] = 'managed-17';
      break;
  }
  return $map[$tangle];
}

/**
 * Returns the root domain from hardcoded lists corresponding stages to domains.
 *
 * @param string $stage
 *  The name of the stage to get the domain for.
 *
 * @return string
 *  The domain corresponding to that stage.
 */
function _gardens_drush_get_root_domain_name_by_stage($stage) {
  // Production stages have special domains.
  $map = array(
    'gardens' => 'drupalgardens.com',
    'wmg-egardens' => 'wmg-gardens.com',
    'fpmg-egardens' => 'fpmg-drupalgardens.com',
    'enterprise-g1' => 'pfizer.edrupalgardens.com',
    'umg-egardens' => 'umg.edrupalgardens.com',
  );
  // Otherwise, fall back on the standard Hosting domain.
  $domain = isset($map[$stage]) ? $map[$stage] : "$stage.acquia-sites.com";
  return $domain;
}

/**
 * Returns a stage specific database server to use for creating scrubbed DBs.
 *
 * Return the stage's gardener db server, since it will hopefully have less
 * traffic and less sensitive to database dump.
 *
 * @param string $stage
 *   The name of the stage to get the database server for.
 *
 * @return string
 *   The database server corresponding to that stage.
 */
function _gardens_drush_get_database_server_by_stage($stage) {
  // Hardcode the database servers for production stages.
  $map = array(
    'gardens' => 'dbmaster-45',
    'wmg-egardens' => 'dbmaster-4',
    'fpmg-egardens' => 'dbmaster-4',
    'enterprise-g1' => 'dbmaster-6',
    'umg-egardens' => 'ded-14',
    'utest' => 'dbmaster-4',
  );
  // Otherwise, fall back on the standard one used e.g. in gsteamer.
  $db_server = isset($map[$stage]) ? $map[$stage] : 'dbmaster-45';
  return $db_server;
}

/**
 * Gets a site's database dump from the slave database.
 */
function drush_gardens_gardens_sql_dump() {
  $result = NULL;
  // Currently the $databases variable in the settings file only points to the
  // site's master database. However there is an other variable which contains
  // information about both the master and the slave servers.
  if (!empty($GLOBALS['conf']['acquia_hosting_site_info']['db']['db_url_ha']) && is_array($GLOBALS['conf']['acquia_hosting_site_info']['db']['db_url_ha'])) {
    // The master and slave DB information is stored in D6 style DB url. Make
    // sure we use the last one which should be the slave.
    $slave_db_spec = drush_convert_db_from_db_url(end($GLOBALS['conf']['acquia_hosting_site_info']['db']['db_url_ha']));
    $sql_dump_info = drush_sql_dump($slave_db_spec);
    // The actual database dump command is in the first key.
    if (!empty($sql_dump_info[0])) {
      $result = drush_op_system($sql_dump_info[0]);
    }
  }
  if (!isset($result)) {
    drush_log('Could not generate a database dump.', 'error');
  }
  return $result;
}

/**
 * Drush callback. Runs on the hosting server.
 */
function drush_gardens_sql_dump_with_scrub($database_server = NULL) {
  $host = $database_server ? $database_server : 'dbmaster-45';

  try {
    $database_name = uniqid('tmp_dev_');
    _gardens_drush_create_scrub_db($database_name, $host);
    $connect = _gardens_drush_get_scrub_db_connect_string($host);
    $dump_cmd_build = drush_sql_dump();
    $dump_cmd = $dump_cmd_build[0];

    $cmd = "$dump_cmd | mysql {$connect} {$database_name}";
    gardens_drush_exec($cmd);

    // The scrub files are one directory above this file.
    $scrub_file = dirname(dirname(__FILE__)) . '/gsite-scrub.sql';
    $local_dev_scrub_file = dirname(dirname(__FILE__)) . '/gsite-scrub-local-dev.sql';

    // Run the scrub scripts.
    $cmd = "mysql {$connect} {$database_name} < $scrub_file";
    gardens_drush_exec($cmd);
    $cmd = "mysql {$connect} {$database_name} < $local_dev_scrub_file";
    gardens_drush_exec($cmd);

    $cmd = "mysqldump {$connect} --single-transaction {$database_name}";
    drush_log($cmd, 'notice');
    if (!drush_get_context('DRUSH_SIMULATE')) {
      $return = drush_op_system($cmd);
      _gardens_drush_remove_scrub_db($database_name, $host);
      return $return;
    }
  } catch (Exception $e) { // Could catch GardensDrushCmdExecutionException if needed.
    _gardens_drush_remove_scrub_db($database_name, $host);
    drush_log($e->getMessage(), 'error');
    exit;
  }
}

function _gardens_drush_remove_scrub_db($database_name, $host) {
  $connect = _gardens_drush_get_scrub_db_connect_string($host);
  // DB_name
  $cmd = "mysql {$connect} -e 'drop database {$database_name}'";
  gardens_drush_exec($cmd);
}

function _gardens_drush_create_scrub_db($database_name, $host) {
  $connect = _gardens_drush_get_scrub_db_connect_string($host);
  // DB_name
  $cmd = "mysql {$connect} -e 'create database {$database_name}'";
  gardens_drush_exec($cmd);
}

/**
 * Retrieves credentials from the filesystem.
 *
 * @see _acquia_gardens_xmlrpc_creds()
 *
 * @param String $host
 *  The hostname to use alongside the stored credentials.
 */
function _gardens_drush_get_scrub_db_creds($host) {
  if (!empty($_ENV['AH_SITE_GROUP']) && !empty($_ENV['AH_SITE_ENVIRONMENT'])) {
    $site_dir = sprintf('%s.%s', $_ENV['AH_SITE_GROUP'], $_ENV['AH_SITE_ENVIRONMENT']);
  }
  else {
    $site_dir = basename(dirname(dirname(__FILE__)));
  }
  $cred_location = sprintf('/mnt/gfs/%s/nobackup/gardens_xmlrpc_creds.ini', $site_dir);
  $gardens_xmlrpc_creds = parse_ini_file($cred_location, TRUE);
  $creds = $gardens_xmlrpc_creds['gardens_temporary'];

  return array(
    'user' => $creds['username'],
    'pass' => $creds['password'],
    'host' => $host,
  );
}

function _gardens_drush_get_scrub_db_connect_string($host) {
  $creds = _gardens_drush_get_scrub_db_creds($host);
  return sprintf('-u%s -p%s -h%s', escapeshellarg($creds['user']), escapeshellarg($creds['pass']), escapeshellarg($creds['host']));
}

/**
 * Implementation of hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function gardens_drush_help($section) {
  switch ($section) {
    case 'drush:getsite':
      return dt("Get a Drupal Gardens site and install it locally. All passwords are set to 'admin' and the admin account name is 'admin'.

You can specify database parameters to the script for a user able to create databases locally.  If you do not provide them it will default to the credentials found in your current local settings.php file.

NOTE: This command requires Drush 4 or higher.");
    case 'drush:sql-dump-with-scrub':
      return dt("Clones, scrubs and dumps a DB.  Run on gardens web-nodes.");
    case 'drush:export':
      return dt("Exports a site and its database files");
  }
}

/**
 * Runs a command via exec(); if the status code is non-zero throws exception.
 *
 * @throws GardensDrushCmdExecutionException
 * @param string $cmd
 *  The command to run
 *
 * @return Array
 *  An array containing the output and status code.
 */
function gardens_drush_exec($cmd) {
  $stdout = NULL;
  $status = NULL;

  // drush_op() would check Drush context for us, but it can't be used with
  // exec(), because call_user_func_array() doesn't handle references. So we
  // have to call exec() directly.
  if (drush_get_context('DRUSH_VERBOSE') || drush_get_context('DRUSH_SIMULATE')) {
    drush_print("Calling exec(". $cmd .')');
  }
  if (drush_get_context('DRUSH_SIMULATE')) {
    return TRUE;
  }

  exec($cmd, $stdout, $status);
  if ($status != 0) {
    $msg = "Command failed to exit successfully.\nCommand:\n{$cmd}\n({$status})\nOutput:\n$stdout";
    throw new GardensDrushCmdExecutionException($msg);
  }

  return array(
    'status' => $status,
    'stdout' => $stdout,
  );
}

/**
 * Drush command to export a gardens site.
 *
 * @return string
 *  Path to export tarball.
 */
function drush_gardens_export() {
  $options = drush_get_context('options') + array_map('_gardens_drush_option_default', gardens_drush_options('export'));
  $memory_limit = drush_get_option('memory-limit', $options['memory-limit']);
  if ($memory_limit !== NULL) {
    if (!preg_match('/^[0-9]+M$/', $memory_limit)) {
      drush_log(sprintf('memory-limit %s not recognized.  Should be of the form "128M".', $memory_limit), 'failed');
      return;
    }
    // Set the memory limit.
    ini_set('memory_limit', $memory_limit);
  }

  $export_files = (NULL == drush_get_option('exclude-files', $options['exclude-files']));

  $uri = site_export_export_both(!$export_files);
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
function drush_gardens_export_s3() {
  $options = drush_get_context('options') + array_map('_gardens_drush_option_default', gardens_drush_options('export-s3'));
  $s3key = drush_get_option('s3key', $options['s3key']);
  $s3secret = drush_get_option('s3secret', $options['s3secret']);
  // load from netrc.
  if (empty($s3key) || empty($s3secret)) {
    if ($creds = drush_gardens_s3_get_creds()) {
      $s3key = $creds['login'];
      $s3secret = $creds['password'];
    }
    else {
      drush_set_error('s3_keys_missing', 'To backup, you must provide --s3key and --s3secret');
      return;
    }
  }

  // Set the memory limit.
  $memory_limit = drush_get_option('memory-limit', $options['memory-limit']);
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
  $authenticated_url = drush_gardens_send_to_s3($s3key, $s3secret, $path, $resource_name, $bucket);
  if (!$authenticated_url) {
    drush_set_error('archive_failed', 'Unable to archive site to s3. Reporting failure to the gardener.');
    gardens_drush_call_gardener('site.archive.failed', array($nid));
    return;
  }
  // Notify the gardener that we we have exported this site and moved it to S3.
  drush_log("Query Auth URL:" . $authenticated_url, 'success');
  $result = gardens_drush_call_gardener('site.archived', array($nid, $authenticated_url));
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
function drush_gardens_s3_get_creds() {
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
function drush_gardens_send_to_s3($key, $secret, $path, $resource_name = NULL, $bucket = GARDENS_DEFAULT_EXPORT_BUCKET_NAME) {
  require_once dirname(__FILE__) . '/../library/S3.php';
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

/**
 * Drush command to regenerate PDF representations of all pages on a site.
 */
function drush_gardens_pdf_regen_site() {
  $uid = (int) drush_get_option('uid');
  $gardener_url = scarecrow_get_gardener_url();
  // Attempting to get the local uid
  $localuid = db_query('SELECT uid FROM {authmap} WHERE module = :module AND authname = :authname', array(
    ':module' => 'openid',
    ':authname' =>  "{$gardener_url}/user/{$uid}/identity",
  ))->fetchField();

  if (function_exists('gardens_pdf_rendition_sitemap_queue')) {
    $account = $localuid ? user_load($localuid) : NULL;
    gardens_pdf_rendition_sitemap_queue($account);
    drush_print(dt('Enqueued regeneration of the entire site\'s PDF versions.'));
  }
  else {
    drush_set_error('DRUSH_MODULE_ERROR', dt('Module gardens_pdf_rendition_sitemap is not enabled.'));
  }
}

