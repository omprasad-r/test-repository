#!/usr/bin/ruby 

### A stupid script to back up the config.xml files from Jenkins to SVN...
### Shoule be runfrom the back-up directory for this script to work (currently /vol/ebs1/jenkins/jobs)


## Ways this script is stupid: 
###### 1) Might fail if you don't execute it from the desired back up directory (cd <backup> && ruby $WORKSPACE/<script>) (FIXED)
###### 2) Run with tomcat6 creds, so limited privileges when running on the machine 
###### 3) Not parametrized, so it is instance specific (easily changed, though...)
###### 4) Doesn't bother to check if a job is version-controlled - blindly adds the top-level directory and subsequently the config.xml
###### 5) Doesn't bother to check if a commit failed

## Why stupidity is ok:
###### 1) Functional backup with little overhead
###### 2) Should produce no "malicious side-effects"
###### 3) We can extend this so that the script is "smart(er)"
###### 4) "Brute-force backup mechanism"

job_dir  = "/vol/ebs1/jenkins/jobs"
filters  = ["commons", "gardens", "gsteamer", "network", "crawler", "acquia_qa", 'frontpage', 'insight', 'webdriver', 'cloud']   # Ditto...
creds    = "/usr/share/tomcat6/ec2/gardens-dev/netrc"
user     = `whoami`
mach     = "svn.acquia.com.client"
svn_user = ""
svn_pass = ""

# Check that we are the right user...
unless user.include?("tomcat6")
  $stdout.print"We aren't operating as tomcat6...won't be able to open tomcat6's netrc file\n"
  $stdout.flush
end

# Check that we can find our netrc creds...
unless File.exists?(creds)
  $stdout.print"Couldn't find the netrc file at the given locations #{creds}\n"
  $stdout.flush
  exit 1
end

# Verify we have privilege to access these creds...
unless File.owned?(creds)
  $stdout.print"Can't access tomcat6's netrc file because we don't own it...exiting.\n"
  $stdout.flush
  exit 2
end

# Grab our creds...
begin
  File.open(creds,"r") { |file|
    file.each{|line| 
      line.chomp!
      next unless line.include?(mach)
      svn_user  = file.readline.chomp!.split(' ').last
      svn_pass  = file.readline.chomp!.split(' ').last
    }
  }
rescue e
  $stdout.print"Error while opening netrc: #{e.message}#{e.backtrace}\n"
  $stdout.flush
  exit 3
end

# Make sure we got valid credentials...
unless (svn_user.length > 0) && (svn_pass.length > 0)
  $stdout.print"Couldn't locate the desired credentials in our netrc file...exiting\n"
  $stdout.flush
  exit 4
end

# Make sure we are backing up from a valid directory...
unless File.directory?(job_dir)
  $stdout.print"Couldn't find target directory...exiting.\n"
  $stdout.flush
  exit 5
end

# Make sure we have criteria to filter with...
if filters.empty?
  $stdout.print"No criteria to filter on...exiting.\n"
  $stdout.flush
  exit 6
end

# Move to the root backup directory
begin
  Dir.chdir(job_dir)
rescue
  $stdout.print"Caught an error while trying to change to the back-up directory...exiting.\n"
  $stdout.flush
  exit 7
end

# Find the jobs we are interested in...
relevant_jobs = Dir["*"].reject{|dir| filters.select{|filter| dir.include?(filter) }.empty?}

# Exit if there aren't any...
if relevant_jobs.empty?
  $stdout.print"Couldn't find any jobs to back-up...exiting.\n"
  $stdout.flush
  exit 8
end

# Sanity check!
unless Dir.getwd == job_dir
  $stdout.print"Somehow, we aren't in the back-up directory...exiting.\n"
  $stdout.flush
  exit 9
end

# Backup each directory!
relevant_jobs.each{ |job|
  unless File.exists?("#{job}/config.xml")
    $stdout.print "Failed to find a config.xml file for job #{job}....skipping to next job\n"
    $stdout.flush
    next
  end
  cmds = [ 
		   "svn add --depth=empty #{job}", 
           "svn add #{job}/config.xml" ,
		   "svn ci -m \"[ JENKINS ] CONFIG.XML AUTO BACKUP\" --username #{svn_user} --password #{svn_pass} #{job}" 
  ]
  cmds.each{ |cmd| res = `#{cmd}` ; $stdout.print"#{res.inspect}\n" ; $stdout.flush }
}

exit 0
