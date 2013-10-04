#!/usr/bin/env ruby

# One time hotfix script for DG-9589 to undo the files symlink (sites.php)
# Half way through the sprint it was decided to revert back to using files only
# because symlinks offer poor performance on gluster.
# After some sanity check, this script will rm the 'files' symlink, mv the 'f'
# directory to 'files' and then create a 'f' symlink to 'files' just in case.

require 'optparse'

def colorize(text, color_code)
  "\e[#{color_code}m#{text}\e[0m"
end

def red(text); colorize(text, 31); end
def yellow(text); colorize(text, 33); end
def green(text); colorize(text, 32); end

def parse_options
  options = {}

  optparse = OptionParser.new do |opts|
    opts.banner = "Usage: #{__FILE__} --sitegroup=<sitegroup> --env=<env> --run\n" +
    "You must SSH to a webnode of the tangle to execute this command.\n" +
    "This command will not do anything unless the --run option is set."

    opts.on('-s', '--sitegroup=[sitegroup]', 'The name of the sitegroup in which to perform the command') do |sg|
      options[:sitegroup] = sg
    end

    opts.on('-e', '--env=[env]', 'The name of the environment in which to perform the command') do |e|
      options[:env] = e
    end

    opts.on('-r', '--run', 'Execute the commands as opposed to just print them out') do |r|
      options[:run] = r
    end

    # This displays the help screen, all programs are
    # assumed to have this option.
    opts.on('-h', '--help', 'Display this screen' ) do
      puts opts
      exit!
    end
  end
  optparse.parse!

  # Validation.
  if !options[:sitegroup] || !options[:env]
    STDERR.puts "Both sitegroup and env are required."
    exit -1
  end

  return options
end

options = parse_options

# The gfs location of the files is derived from the sitegroup and the env.
gfs_files_base = "/mnt/files/#{options[:sitegroup]}.#{options[:env]}/sites/g/files/"

puts "Base gfs directory: #{gfs_files_base}"

failed_fix = []
unsupported = []
Dir.foreach(gfs_files_base) do |filename|
  # sanity check: this should be az12345
  next unless filename.match('^[a-zA-Z]+[0-9]+$')

  gfs_files_g123 = "#{gfs_files_base}#{filename}"

  # Only proceed if we recognize a files symlink and an f directory.
  if File.directory?("#{gfs_files_g123}/f") && !File.symlink?("#{gfs_files_g123}/f") && File.directory?("#{gfs_files_g123}/files") && File.symlink?("#{gfs_files_g123}/files")
    puts "Processing #{gfs_files_g123}"
    cmd =  "rm #{gfs_files_g123}/files && "
    cmd += "mv #{gfs_files_g123}/f #{gfs_files_g123}/files && "
    cmd += "cd #{gfs_files_g123} && ln -s files f"
    # Command needs to run as tangle user.
    cmd = "sudo su -l #{options[:sitegroup]} -c '#{cmd}'"
    puts "Command: #{cmd}"
    if options[:run]
      output = `#{cmd}`
      if $?.exitstatus == 0
        puts "[" + green('DONE') + "] Fixed symlink for #{gfs_files_g123}"
      else
        puts "[" + red('FAILED') + "] command failed: " + output
        failed_fix.push(gfs_files_g123)
      end
    end
  else
    puts "[" + yellow('WARNING') + "] Sanity check failed for #{gfs_files_g123}"
    unsupported.push(gfs_files_g123)
  end
end

puts "\nThe following paths failed to fix their symlinks:"
puts failed_fix.join("\n")

puts "\nThe following paths were not processed because they didn't have the expected directory structure:"
puts unsupported.join("\n")
