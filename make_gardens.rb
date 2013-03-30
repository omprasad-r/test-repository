#!/usr/bin/env ruby

require 'optparse'

# Runs drush make with certain options set.
# The first parameter passed in specifies a particular project (module or
# library) to make, or "full" to build the entire codebase (i.e. core + contrib).
# The second parameter indicates whether the project being built is a library,
# so that the correct directory structure is used.
def execute_gardens_make(what, is_library = false)
  drush_exec = "./drush/drush"
  contrib_type = is_library ? "libraries" : "projects"
  cmd = drush_exec + " make --concurrency=1 --no-patch-txt --no-gitinfofile --working-copy"
  if what != 'full'
    if what != 'drupal'
      cmd = cmd + " --no-core"
    end
    cmd = cmd + " --#{contrib_type}=#{what}"
  end
  cmd = cmd + " gardens.make make-workspace/#{$subdir}"
  run_command(cmd)
  post_gardens_make(what, is_library)
end

# Perform any re-organization of the codebase that may be necessary in cases
# where we need to structure things in a way that drush make does not allow.
def post_gardens_make(what, is_library = false)
  # The location of the downloaded contrib projects
  contrib_location = "make-workspace/#{$subdir}/sites/all"

  # Perform whatever deletions are necessary for various contrib projects

  # Remove _samples dir from ckeditor
  if (what == 'full' or (what == "ckeditor" and is_library))
    run_command("rm -rf #{contrib_location}/libraries/ckeditor/_samples")
  end

  # Remove demos dir from getID3
  if (what == 'full' or (what == "getid3" and is_library))
    run_command("rm -rf #{contrib_location}/libraries/getid3/demos")
  end

  # Remove examples dir from plupload
  if (what == 'full' or (what == "plupload" and is_library))
    run_command("rm -rf #{contrib_location}/libraries/plupload/examples")
    run_command("rm -rf #{contrib_location}/libraries/plupload/docs")
  end

  # We only need the fancybox subdir from the package that gets downloaded
  if (what == 'full' or what == "fancybox")
    run_command("mv #{contrib_location}/libraries/fancybox #{contrib_location}/libraries/fancybox_tmp")
    run_command("mv #{contrib_location}/libraries/fancybox_tmp/fancybox #{contrib_location}/libraries/")
    run_command("rm -rf #{contrib_location}/libraries/fancybox_tmp")
  end

  # Rename contextual-flyout-links to use underscores
  if what == 'full' or what == "contextual-flyout-links"
    run_command("mv #{contrib_location}/modules/contextual-flyout-links #{contrib_location}/modules/contextual_flyout_links")
  end

  # Add the SPYC library required by the rest_server submodule of services.
  if what == 'full' or what == "services"
    run_command("curl http://spyc.googlecode.com/svn/trunk/spyc.php > #{contrib_location}/modules/services/servers/rest_server/lib/spyc.php")
  end

  # Add the jquery.timeago library required by the timeago module.
  # @todo Find out if it's possible to do this with drush make
  if what == 'full' or (what == "timeago" and !is_library)
    run_command("curl http://timeago.yarp.com/jquery.timeago.js > #{contrib_location}/modules/timeago/jquery.timeago.js")
  end

  # Grab the jQuery plugins required by rotating banner
  # @todo Find out if it's possible to do this with drush make
  # @todo Get rid of rotating banner :-/
  if what == 'full' or what == "rotating_banner"
    run_command("curl http://cloud.github.com/downloads/malsup/cycle/jquery.cycle.all.2.72.js >  #{contrib_location}/modules/rotating_banner/includes/jquery.cycle.js")
    run_command("curl http://gsgd.co.uk/sandbox/jquery/easing/jquery.easing.1.3.js >  #{contrib_location}/modules/rotating_banner/includes/jquery.easing.js")
  end

  if !is_library
    # Fix .info files for projects that were pulled from git
    cmd = "./tools/fixProjectInfoFiles -d \"make-workspace/#{$subdir}/\""
    if what != "full" and what != "drupal"
      cmd = cmd + " #{what}"
    end
    run_command(cmd)
  end
  return 0
end

def run_command(cmd)
  if $verbose
    STDOUT.puts  "Running '#{cmd}'"
  end
  process = IO.popen("#{cmd}")
  output = process.readlines.join("\n")
  process.close
  unless $? == 0
    STDERR.puts "There was an error running #{cmd}"
    exit -1
  end
  STDOUT.puts output
end

# Exclude all custom modules, profiles and themes from the rsync so
# that they don't get deleted. Also, make sure certain files and directories that
# we don't want in the repo don't get rsync'd over.
def rsync_exclude(src_path, to_build)
  list = "  --exclude=\"LICENSE.txt\" --exclude=\".cvsignore\" --exclude=\".git\""
  case src_path
  when "make-workspace/#{$subdir}"
    # We want to exclude every dir under "sites" if we are doing core update
    # otherwise do not exclude "sites/all".
    if to_build != "drupal"
      list += " --include=\"/sites/all/\""
    end
    list += " --exclude=\"/sites/*/\""
    # Exclude the acquia modules dir
    list += " --exclude=\"/modules/acquia/\""
    # Profiles to include (from core)
     profiles = [
      'minimal',
      'standard',
      'testing',
     ]
    profiles.each {|p| list += " --include=\"/profiles/#{p}/\""}
    # Exclude any custom profiles.
    list += "  --exclude=\"/profiles/*/\""
    # Modules to exclude
    modules = [
      "block_everything",
      "comment_on_anything",
      "coppa_lite",
      "feedback",
      "font_management",
      "gardens_features",
      "gardens_moderation",
      "gardens_statsd",
      "logintoboggan_email_login",
      "media_oembed_thumbnail_style",
      "pathauto_live_preview",
      "role_indicators",
      "seo_ui",
      "simpleviews",
      "sqbs",
      "styles",
      "tac_alt_ui",
      "tac_redirect_403"
    ]
    modules.each {|m| list += " --exclude=\"/sites/all/modules/#{m}/\""}
    # Themes to exclude
    themes = ["blossom"]
    themes.each {|t| list += " --exclude=\"/sites/all/themes/#{t}/\""}
    # Libraries to exclude
    libraries = ["responsivizer"]
    libraries.each {|l| list += " --exclude=\"/sites/all/libraries/#{l}/\""}
    # Miscellaneous stuff to exclude
    list += ' --exclude="/themes/acquia/"'
    # wvega timepicker, when part of the whole build
    list += ' --include="/sites/all/libraries/wvega\-timepicker/jquery.timepicker.*" --exclude=/sites/all/libraries/wvega\-timepicker/*'
  when /wvega\-timepicker$/
    list += ' --include="/jquery.timepicker.*" --exclude=*'
  end
  return list
end

# Make sure we are running from the directory where this file lives
if __FILE__ == $0
  # The directory to build the make file into
  $subdir = "gardens_build_" + Time.now.to_i.to_s
  # Some sensible default options
  is_library = false;
  copy = false
  file = nil
  test_mode = false;

  OptionParser.new do |opts|
    executable_name = File.basename($PROGRAM_NAME)
    opts.banner = "
Run drush make on the gardens.make file, either for an individial project, e.g.
media or ckeditor, or, if no project name is supplied, for the whole build.
Usage: #{executable_name} [options] [project_name]
    "
    opts.on('-l', "--libraries", "Indicates that the project we are building belongs in the libraries directory") do |v|
      is_library = v
    end
    opts.on('-v', "--verbose", "Verbose output") do |v|
      $verbose = v
    end
    opts.on('-e', "--execute-rsync", "Copy the result of the build into the doc root") do |v|
      copy = v
    end
    opts.on('-f', "--output-file FILE", "Specify a file into which to output the result of a diff on the codebase. Only works with the -e switch. If the file already exists it will be overwritten. Whitespace and eol character differences will be ignored.") do |v|
      file = v
    end
    opts.on('-t', "--test", "Test mode. Used in our testing environment to return an error if the diff between the make file and the codebase is non-empty.") do |v|
      test_mode = v
    end
  end.parse!

  # If no project has been specified, assume a full build
  if ARGV.size < 1
    to_build = 'full'
  elsif ARGV[0] =~ /^[a-zA-Z0-9\-_\.]+$/
    puts ARGV[0]
    to_build = ARGV[0]
  else
    STDERR.puts "Invalid project name specified"
    exit -1
  end

  # Run drush make
  cmd_output = execute_gardens_make(to_build, is_library)
  # Rsync the files if the -e switch was specified
  if copy
    if to_build == 'acquia'
      STDERR.puts "The -e flag is not valid for the 'acquia' project as it is an svn export"
      exit -1
    end
    rsync_src = "make-workspace/#{$subdir}"
    rsync_dest = "docroot"
    if (to_build != 'full' and to_build != 'drupal')
      type_dir = is_library ? "libraries" : "modules"
      rsync_src = rsync_src + "/sites/all/#{type_dir}/#{to_build}"
      rsync_dest = rsync_dest + "/sites/all/#{type_dir}/#{to_build}"
    end
    process = IO.popen("which rsync")
    rsync_command = process.readlines.last.chomp;
    process.close
    rsync_options = rsync_exclude(rsync_src, to_build)
    rsync_options = rsync_options + " -a --delete"
    run_command("#{rsync_command} #{rsync_options} #{rsync_src}/ #{rsync_dest}/")
    git_diff = "git diff -b -w --ignore-space-at-eol"
    if file
      run_command(git_diff + " > #{file}")
    end
    if test_mode
      # Run a diff but suppress output - we just want it to exit with 0 if there are
      # no differences. Otherwise, we want to find out which files have differences.
      process = IO.popen("#{git_diff} --quiet")
      process.close
      unless $? == 0
        STDERR.puts "Build broken. The following files do not match what's in the make file\n"
        files = IO.popen("#{git_diff} --name-only").readlines.map { |a| a.chomp }
        files.each do|f|
          STDERR.puts "#{f}\n"
        end
        exit -1
      end
    end
  end
  exit 0
end
