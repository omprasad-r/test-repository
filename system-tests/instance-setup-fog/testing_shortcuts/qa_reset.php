<?php
/**
 * @file
 * Provides a web UI for managing QA-related tasks.
 */

/**
 * Executes a shell command and prints the command and return values.
 */
class ShellOut {
  function __construct($command) {
    if (is_array($command)) {
      // Don't escape the first element, which will be the command string.
      $command_array = array(array_shift($command));

      foreach ($command as $argument) {
        $command_array[] = escapeshellarg($argument);
      }

      $this->command = implode(' ', $command_array);
    }
    else {
      $this->command = $command;
    }
  }

  function run() {
    print "Executing: $this->command\n";

    exec("$this->command 2>&1", $stdout, $ret_code);

    $out = implode("\n", $stdout);
    print "Command returned with $ret_code:\n$out\n";
    print "---DONE---\n";
  }
}

// Make sure the output of this script is not cached by Varnish.
header('Cache-Control: no-cache');

$operation = !empty($_GET['operation']) ? $_GET['operation'] : '';
if ($operation) {
  header('Content-Type: text/plain');
}

/**
 * Switches the mode of operation.
 */
switch ($operation) {
  case 'restore_snapshot':
    restore_snapshot($_GET['snapshot_name']);
    break;
  case 'create_snapshot':
    create_snapshot($_GET['snapshot_name']);
    break;
  case 'enable_module':
    enable_module($_GET['module_name']);
    break;
  case 'disable_module':
    disable_module($_GET['module_name']);
    break;
  case 'download_module':
    download_module($_GET['module_name']);
    break;
  case 'install_module':
    install_module($_GET['module_name']);
    break;
  case 'uninstall_module':
    uninstall_module($_GET['module_name']);
    break;
  case 'cleanup_installation':
    cleanup_installation();
    break;
  case 'create_user':
    create_user($_GET['username'], $_GET['password']);
    break;
  case 'add_user_role':
    add_user_role($_GET['username'], $_GET['role']);
    break;
  case 'remove_user_role':
    remove_user_role($_GET['username'], $_GET['role']);
    break;
  case 'role_add_permission':
    role_add_permission($_GET['role'], $_GET['permission']);
    break;
  case 'role_remove_permission':
    role_remove_permission($_GET['role'], $_GET['permission']);
    break;
  case 'list_directory_content':
    list_directory_content($_GET['directory_name']);
    break;
  case 'list_file_content':
    list_file_content($_GET['file_name']);
    break;
  case 'inject_file':
    $file_to = $_GET['file_to'];
    $file_from = $_GET['file_from'];
    inject_file($file_from, $file_to);
    break;
  default:
    print_web_ui();
}

/**
 * Returns the base URL of the request.
 */
function base_url() {
  $server_name = $_SERVER['SERVER_NAME'];
  return "http://$server_name/";
}

/**
 * Wraps ShellOut to actually execute the shell command.
 */
function exec_verbose($command) {
  $sh = new ShellOut($command);
  $out = $sh->run();
}

/**
 * Finds drush and composes a base command string.
 */
function drush_location_get() {
  // If mysqldump is not found, and it exists at the specified location, add it
  // as the last directory of PATH so not to override other system binaries.
  if (!exec('which mysqldump') && file_exists('/usr/local/bin/mysqldump')) {
    putenv('PATH=/usr/local/bin:' . getenv('PATH'));
  }

  // If drush is found in PATH, return the path.
  if (exec('which drush')) {
    $drush = exec('which drush');
  }
  // Otherwise, if gardens-drush exists, return the full path to that instead.
  elseif (file_exists('../gardens-drush')) {
    $drush = realpath('../gardens-drush');
  }

  if (file_exists('/home/ubuntu/.drush')) {
    $drush .= ' --include=/home/ubuntu/.drush';
  }

  $drush .= ' --uri=' . base_url();
  $drush .= ' --yes';

  return $drush;
}

/**
 * Creates a snapshot of the installation.
 */
function create_snapshot($snapshot_name) {
  $drush = drush_location_get();
  $pwd = getcwd();

  if (!isset($snapshot_name) || trim($snapshot_name) === '') {
    $snapshot_name = 'default';
  }

  print "Creating new snapshot $snapshot_name\n";

  // If docroot is not yet a git repo, make it so.
  if (!file_exists("$pwd/.git")) {
    exec_verbose('git init');
  }

  print "MySQL is about to take a dump\n";
  exec_verbose("$drush --create-db sql-dump > $pwd/mysql_dump.sql");

  exec_verbose('git add .');
  exec_verbose("git commit -m 'Snapshot: $snapshot_name'");

  print "Creating new tag $snapshot_name\n";
  exec_verbose("git tag $snapshot_name -f");
}

/**
 * Restores a previously-saved snapshot.
 */
function restore_snapshot($snapshot_name) {
  $drush = drush_location_get();

  if (empty($snapshot_name)) {
    $snapshot_name = 'default';
  }

  print "Restoring snapshot $snapshot_name\n";

  print "Cleaning out filesystem\n";
  exec_verbose('git clean -d -f');

  print "Restoring filesystem snapshot\n";
  exec_verbose("git reset --hard $snapshot_name");

  print "Dropping database\n";
  exec_verbose("$drush sql-drop");

  print "Reinjecting MySQL dump";
  exec_verbose("$drush sql-cli < mysql_dump.sql");
}

/**
 * Cleans up the filesystem by removing SCM meta directories.
 */
function cleanup_installation() {
  exec_verbose('find . -name .git -exec rm -rf {} \;');
  exec_verbose('find . -name .gitignore -exec rm -rf {} \;');
  exec_verbose('find . -name .svn -exec rm -rf {} \;');
}

/**
 * Prints the contents of a file.
 */
function list_file_content($filename) {
  print file_get_contents($filename);
}

/**
 * Lists the contents of a directory.
 */
function list_directory_content($directory) {
  if (is_dir($directory)) {
    $iterator = new DirectoryIterator($directory);

    // List the directory's contents.
    foreach ($iterator as $file) {
      print "$file\n";
    }
  }
  else {
    print "Error: $directory is not a directory";
  }
}

/**
 * Creates a new user.
 */
function create_user($username, $password) {
  $current_user_number = rand();
  $drush = drush_location_get();

  print "Adding new user\n";
  exec_verbose(array($drush, "--password=$password",
    "--mail=username@${current_user_number}.com", 'user-create', $username));
}

/**
 * Adds a role to a user.
 */
function add_user_role($username, $role) {
  $drush = drush_location_get();

  print "Adding new user role\n";
  exec_verbose(array($drush, 'user-add-role', $role, $username));
}

/**
 * Removes a role from a user.
 */
function remove_user_role($username, $role) {
  $drush = drush_location_get();

  print "Removing new user role\n";
  exec_verbose(array($drush, 'user-remove-role', $role, $username));
}

/**
 * Adds a permission to a role.
 */
function role_add_permission($role, $permission) {
  $drush = drush_location_get();

  print "Adding role permission\n";
  exec_verbose(array($drush, 'role-add-perm', $role, $permission));
}

/**
 * Removes a permission from a role.
 */
function role_remove_permission($role, $permission) {
  $drush = drush_location_get();

  print "Removing role permission\n";
  exec_verbose(array($drush, 'role-remove-perm', $role, $permission));
}

/**
 * Enables a module.
 */
function enable_module($module_name) {
  $drush = drush_location_get();

  exec_verbose(array($drush, 'pm-enable', $module_name));
  exec_verbose(array($drush, 'cache-clear', 'all'));
}

/**
 * Disables a module.
 */
function disable_module($module_name) {
  $drush = drush_location_get();

  exec_verbose(array($drush, 'pm-disable', $module_name));
  exec_verbose(array($drush, 'cache-clear', 'all'));
}

/**
 * Downloads a module.
 */
function download_module($module_name) {
  $drush = drush_location_get();
  $site_dir = $_SERVER['SERVER_NAME'];

  exec_verbose(array($drush, "--use-site-dir=$site_dir", 'pm-download', $module_name));
}

/**
 * Installs a module.
 */
function install_module($module_name) {
  $site_dir = $_SERVER['SERVER_NAME'];
  download_module($module_name);
  enable_module($module_name);
}

/**
 * Uninstalls a module.
 */
function uninstall_module($module_name) {
  $drush = drush_location_get();
  $site_dir = $_SERVER['SERVER_NAME'];
  disable_module($module_name);

  exec_verbose(array($drush, 'pm-uninstall', $module_name));
  exec_verbose("rm -rf /var/www/sites/$site_dir/modules/$module_name");
  exec_verbose("rm -rf /var/www/sites/all/modules/$module_name");
}

/**
 * Injects contents into a file.
 */
function inject_file($file_from, $file_to) {
  print "Uploading file $file_from to $file_to.\n";

  // Get the file as a stream.
  $contents_from = file_get_contents($file_from);
  if ($contents_from === FALSE) {
    print "Error: failed to open input file: $file_from";
  }
  else {
    // Write the stream back out to the file.
    if (file_put_contents($file_to, $contents_from) === FALSE) {
      print "Error: failed to put downloaded file here: $file_to";
    }
    else {
      print 'Injected file!';
    }
  }
}

/**
 * Displays the web UI.
 */
function print_web_ui() {
  $document = <<<DOC
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>QA reset</title>
<style>
body {
  font-family: Helvetica, Arial, sans-serif;
  margin: .5em 1em 2em;
  color: #555;
  background-color: #d1e6f8;
}
form {
  margin: .75em 0;
}
h1 {
  margin-bottom: 0;
  color: #0079a0;
  text-transform: uppercase;
}
h2 {
  margin: 1.5em 0 .5em;
}
h1 + h2 {
  margin-top: 1em;
}
input[type="text"] {
  padding: .5em;
  margin: 0 .5em 0 0;
}
input[type="submit"] {
  padding: .5em 1em;
  -webkit-appearance: none;
  -webkit-border-radius: 4px;
  -moz-border-radius: 4px;
  border-radius: 2px;
  border: 1px solid #2D5781;
  color: white;
  background: #518FC0;
  margin: 0;
  background: -webkit-gradient(
    linear,
    left top,
    left bottom,
    color-stop(0%, #a4cae7),
    color-stop(5%, #68a6d8),
    color-stop(100%, #3e7dac)
  );
}
.input-filesystem-path {
  width: 18em;
}
</style>
</head>
<body>
<h1>QA reset</h1>
<h2 id="snapshots">Manage snapshots</h2>
<form id="snapshot" method="GET">
  <input type="hidden" name="operation" value="create_snapshot" />
  <input type="text" name="snapshot_name" placeholder="name" />
  <input type="submit" value="Create" />
</form>
<form id="reset" method="GET">
  <input type="hidden" name="operation" value="restore_snapshot" />
  <input type="text" name="snapshot_name" placeholder="name" />
  <input type="submit" value="Restore" />
</form>
<form id="cleanup" method="GET">
  <input type="hidden" name="operation" value="cleanup_installation" />
  <input type="submit" value="Clean (removes .git/.gitignore/.svn)" />
</form>
<h2 id="users-roles">Manage users and roles</h2>
<form id="add_user" method="GET">
  <input type="text" name="username" placeholder="username" />
  <input type="text" name="password" placeholder="password" />
  <input type="hidden" name="operation" value="create_user" />
  <input type="submit" value="Add new user" />
</form>
<form id="add_user_role" method="GET">
  <input type="text" name="username" placeholder="username" />
  <input type="text" name="role" placeholder="role" />
  <input type="hidden" name="operation" value="add_user_role" />
  <input type="submit" value="Add role" />
</form>
<form id="remove_user_role" method="GET">
  <input type="text" name="username" placeholder="username" />
  <input type="text" name="role" placeholder="role" />
  <input type="hidden" name="operation" value="remove_user_role" />
  <input type="submit" value="Remove role" />
</form>
<form id="role_add_permission" method="GET">
  <input type="text" name="role" placeholder="role" />
  <input type="text" name="permission" placeholder="permission" />
  <input type="hidden" name="operation" value="role_add_permission" />
  <input type="submit" value="Add permission" />
</form>
<form id="role_remove_permission" method="GET">
  <input type="text" name="role" placeholder="role" />
  <input type="text" name="permission" placeholder="permission" />
  <input type="hidden" name="operation" value="role_remove_permission" />
  <input type="submit" value="Remove permission" />
</form>
<h2 id="modules">Manage modules</h2>
<form id="module_enable" method="GET">
  <input type="hidden" name="operation" value="enable_module" />
  <input type="text" name="module_name" placeholder="module_name" />
  <input type="submit" value="Enable Module" />
</form>
<form id="module_disable" method="GET">
  <input type="hidden" name="operation" value="disable_module" />
  <input type="text" name="module_name" placeholder="module_name" />
  <input type="submit" value="Disable Module" />
</form>
<form id="module_download" method="GET">
  <input type="hidden" name="operation" value="download_module" />
  <input type="text" name="module_name" placeholder="module_name" />
  <input type="submit" value="Download Module" />
</form>
<form id="module_install" method="GET">
  <input type="hidden" name="operation" value="install_module" />
  <input type="text" name="module_name" placeholder="module_name" />
  <input type="submit" value="Install Module" />
</form>
<form id="module_uninstall" method="GET">
  <input type="hidden" name="operation" value="uninstall_module" />
  <input type="text" name="module_name" placeholder="module_name" />
  <input type="submit" value="Uninstall Module" />
</form>
<h2 id="filesystem">Manage filesystem</h2>
<form id="list_directory_content" method="GET">
  <input type="hidden" name="operation" value="list_directory_content" />
  <input type="text" class="input-filesystem-path" name="directory_name" placeholder="directory path (relative to docroot)" />
  <input type="submit" value="List directory contents" />
</form>
<form id="list_file_content" method="GET">
  <input type="hidden" name="operation" value="list_file_content" />
  <input type="text" class="input-filesystem-path" name="file_name" placeholder="file path (relative to docroot)" />
  <input type="submit" value="Display file contents" />
</form>
</body>
</html>
DOC;
  print $document;
}

