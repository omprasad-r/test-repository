#! /usr/bin/env ruby

# This script adds symlinks from the up tangle's site directory
# (eg /mnt/gfs/tangle001.update/sites/g/files/g12345) to the production site
# directory so that the site continues to function during updates when the site
# is on the up tangle. Freshly created sites on 2.01 will already have this link
# but older sites do not.
#
# Run this script from the site root ./tools directory.  Run with -h for
# options and help.
#
# On the current setup, the gardens_sites_dir argument is intended to be the
# place where you'd find the most sites directories that are not symlinks. After
# 2.01, this should be /mnt/gfs/<site>.<live-env>/sites/g/files. (Typically, the
# 'gardens-sites' directory in gluster now contains symlinks and will be missing
# entries for the newest sites).
#
# You can dry-run the script by not specifying --run.  This is useful in
# combination with verbose -v. When you want to create symlinks, add the --run
# option.
#
# Example usage:
#
# ./fix_up_symlinks.rb --sitegroup=tangle001 --env=prod-gsteamer --up_env=test-gsteamer --gardens_sites_dir=/mnt/gfs/tangle001.prod-gsteamer/sites/g/files --run
#

require 'optparse'

def parse_options
  options = {}

  optparse = OptionParser.new do |opts|
    opts.banner = "Usage: #{__FILE__} --sitegroup=<sitegroup> --env=<env> --up_env=<env> --prefix=<site-prefix> --run\n" +
    "You must SSH to a webnode of the tangle to execute this command.\n" +
    "This command will not do anything unless the --run option is set."

    opts.on('-s', '--sitegroup=[sitegroup]', 'The name of the sitegroup in which to perform the command') do |sg|
      options[:sitegroup] = sg
    end

    opts.on('-d', '--gardens_sites_dir=[dir]', 'The name of the directory to use for finding site directories. ' +
      'As this is only used as a source of site directories, either /mnt/gfs/*/gardens-sites or /mnt/gfs/*/sites/g/files ' +
      'are good options.' ) do |d|
      options[:dir] = d
    end

    opts.on('-e', '--env=[env]', 'The name of the environment in which to perform the command') do |e|
      options[:env] = e
    end

    opts.on('-u', '--up_env=[env]', 'The name of the environment in which to perform the command') do |e|
      options[:up_env] = e
    end

    opts.on('-r', '--run', 'Execute the commands as opposed to just print them out') do |r|
      options[:run] = r
    end

    opts.on('-v', '--verbose', 'Print verbose output (even if individual sites do not log errors)') do |v|
      options[:verbose] = v
    end

    opts.on('-p', '--prefix=[<prefix>]', '(Optional - default "g") The site prefix for sanity checking site dirs (eg for SMB ' +
      'gardens and older installations, this is always "g"). Note that it is assumed that this script will not run in ' +
      '*staging* environments, where there are suffixes on some DB roles: to fix staging, just re-stage.') do |p|
      options[:prefix] = p
    end

    # This displays the help screen, all programs are
    # assumed to have this option.
    opts.on('-h', '--help', 'Display this screen' ) do
      puts opts
      exit!
    end
  end
  optparse.parse!

  # Site prefix defaults to 'g', which is normal for SMB and some older enterprise installs.
  options[:prefix] ||= 'g'

  # Validation.
  if !options[:sitegroup] || !options[:env] || !options[:dir] || !options[:up_env]
    STDERR.puts "A required option (sitegroup, env, up_env, dir) has not been supplied."
    exit -1
  end
  if options[:prefix].match('[^a-z]')
    STDERR.puts "prefix can only contain lowercase letters"
    exit -1
  end

  return options
end

options = parse_options

run = options[:run] ? '--run=1' : ''

result = []
Dir.foreach(options[:dir]) do |filename|
  # sanity check: this should be prefix12345 (eg g12345)
  next unless filename.match("^#{options[:prefix]}[0-9]+$")

  gardens_site_id = filename
  cmd = "sudo -u #{options[:sitegroup]} drush5 --root=../docroot --include=. fix-up-symlinks #{options[:sitegroup]} #{options[:env]} #{gardens_site_id} #{options[:up_env]} #{run}"
  puts cmd
  output = `#{cmd}`

  if options[:verbose]
    puts output
    puts ''
  end
  unless $?.exitstatus == 0
    result.push(gardens_site_id)
  end
end

puts "The following nodes failed to set their symlinks:"
puts result.join("\n")
