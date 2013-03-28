#!/usr/bin/env ruby

require 'optparse'
require 'ostruct'

=begin
 * Try to determine if a site is finished installing, using Drush.
 *
 * @param $site_domain
 *   The site domain, corresponding to the site's directory within sites/ in a
 *   Drupal multisite installation.
 * @return
 *   TRUE if the site is installed, FALSE if it isn't.
=end
def is_installed?(domain)
  return execute_drush_cmd('sql-query "SELECT value FROM variable WHERE name=\'install_task\'"', domain).include?('done');
end

def execute_drush_cmd(cmd, site_alias, nice = false)
  # A leading root: is the flag to run as root.
  if (cmd =~ /^root:/) then
    as_site_user = false
    cmd = cmd.split('root:').pop().strip()
  else
    as_site_user = true
  end
    
  
  # Capture stderr w/ stdout because drush doesn't yet report stderr properly
  cmd = "--yes @#{site_alias} #{cmd} 2>&1"

  # This assumes a Hosting environment (i.e., that this file is located in a
  # directory like /var/www/html/tangle001 where tangle001 is the site user we
  # want to run commands as).
  if as_site_user
    # Don't include the hosting drush commands.
    site_user = File.basename(File.dirname(File.expand_path(__FILE__)))
    cmd = 'sudo -u ' + site_user + " #{$drush} " + cmd
  else
    cmd = "#{$drush} -i #{$inc_dir} " + cmd
  end

  if nice
    cmd = 'nice ' + cmd
  end

  if $verbose
    puts cmd
  end
  unless $dry_run
    output = `#{cmd}`.strip()
    if $verbose
      puts output
    end
    return output
  end
end

puts $0

if __FILE__ == $0
  mydir = File.dirname(__FILE__)
  drushrc_dir = mydir + '/docroot/files/ms-drush'
  $inc_dir = mydir + '/hosting-drush'
  options = OpenStruct.new
  options.drush_cmds = []
  options.verbose = false
  options.log = nil
  options.domain = nil
  options.drush = "drush/drush.php --alias-path=#{drushrc_dir}"
  options.dry_run = false
  options.nice = false
  options.force = false

 
  OptionParser.new do |opts|
    opts.banner = "Usage: drush-ms.rb [options]"
    opts.on("-c", "--dc [[always:][root:]COMMAND]", "Drush command to run (required)") do |v|
      options.drush_cmds << v
    end
    opts.on("-v", "--[no-]verbose", "Run verbosely") do |v|
      options.verbose = v
    end
    opts.on("-l", "--log LOG", "Log file") do |v|
      options.log = v
    end
    opts.on("-d", "--site ALIAS", "Domain or gardens site identifier to run against.") do |v|
      options.site = v
    end
    opts.on('-e', "--drush PATHTODRUSH", "Path to the drush.php executable") do |v|
      options.drush = v
    end
    opts.on("-n", "--[no-]nice", 'Run "nice" ') do |v|
      options.nice = v
    end
    opts.on("-f", "--force", 'Run commands even for a site without a DB connection or that seem not to be installed') do |v|
      options.force = v
    end
    opts.on( '-h', '--help', 'Display this screen' ) do
      puts opts
      exit
    end
    opts.on("-t", "--[no-]dry-run", "Dry Run") do |v|
      options.dry_run = v
      options.verbose = true
    end
  end.parse!
  
  log = options.log
  domain = options.site
  $verbose = options.verbose
  $drush = options.drush
  $dry_run = options.dry_run
    
  if (domain.nil? or log.nil?)
    puts "--site ALIAS and --log LOG are required options" 
    exit 1;
  end

  drush_command_list = options.drush_cmds.join(', ')
  tstamp = Time.now.strftime('%Y%m%d_%H%M%S')
  log_header = "#{tstamp},#{domain},\"%s\",\"#{drush_command_list}\""
 
  summary_log = File.open(log + ".log", 'a')
  error_log = File.open(log + ".error.log", 'a')
  success_log = File.open(log + ".success.log", 'a')
  `export COLUMNS=$(tput cols)` if (ENV['COLUMNS'].nil?)
    

  if options.force || is_installed?(domain)
    # Execute each command in order, collecting all output, but marking the
    # overall result as an 'error' if any of the executed commands lead to an
    # error.
    output = ""
    result = ""
    log_file = ""
    options.drush_cmds.each { |command|
      # A leading always: is the flag to run a command after others have
      # failed.
      if (command =~ /^always:/) then
        run_always = true
        command = command.split('always:').pop().strip()
      else
        run_always = false
      end
      if (result != 'error' || run_always) then
        cmd_output = execute_drush_cmd(command, domain, options.nice)
        output << "\n" + cmd_output
        exit_status = $?
        if (exit_status == 0 && result != 'error')
          result = 'success';
          log_file = success_log;
        else
          result = 'error';
          log_file = error_log
        end
      else
        output << "\nThe command '#{command}' was skipped due to errors in previous commands."
      end
    }
  else
    output = "Not installed"
    result = 'not_ready';
    log_file = error_log;
  end

  # Format the header with the correct result
  log_header = format(log_header, result)

  # Write the full-form with the output
  log_message = "#{log_header} \n #{output}\n"
  log_file.puts(log_message)

  # Write to the summary log
  summary_log.puts(log_header)
end
