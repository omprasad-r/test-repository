## This file houses common tasks which are run by multiple tests.
#require 'bundler/setup'
require 'acquia_qa/it'
require 'acquia_qa/log'
require 'acquia_qa/os'
require 'acquia_qa/ssh'
require 'acquia_qa/rake'
require '../helpers/fields'
require '../helpers/gardener'
require '../helpers/gardens_automation'
require 'acquia_qa/configuration'
require 'acquia_qa/svn'
require '../helpers/site'
require 'relative'
require 'pathname'
require "tmpdir"
include Acquia::Config
include OS
include Acquia::SSH
include DrupalSite
include GardensAutomation

desc 'Generic setup tasks to get the master / webnode server in place.'
task :infrastructure_setup => [:del_fields, :co_fields, :launch_gardens_site, :set_dns, :configure_vhost, :add_remote_creds_to_configuration_file, :upload_remote_rakefile, :install_from_svn_release, :install_gardens]

desc 'stop master'
task :stop_master do
  Dir.chdir(GardensTestRun.instance.config.fields_root + '/master-instance/'){
    run2 'rake stop --trace'
  }
end
desc 'svn co fields'
task :co_fields do
  if (GardensTestRun.instance.fields.fields_stage =~/gsteamer-trunk/) # steamer-trunk should always use latest hosting
    GardensTestRun.instance.config.fields['url'] = 'https://svn.acquia.com/repos/engineering/fields/trunk'
  end
  puts "Checking out fields code from #{GardensTestRun.instance.config.fields['url']}"
  @os = Object.new
  @os.extend OS
  Dir.chdir('../'){
    fields_svn = SvnCommand.new('fields')
    fields_svn.url = GardensTestRun.instance.config.fields['url']
    fields_svn.credentials = GardensTestRun.instance.it.svn_credentials.get
    @os.attempt(fields_svn.checkout)
  }
end

# new launch procedure
# in master instance on fields code
# start_gardens_clone
# wait for puppet to finish on master
# continue_gardens_clone
# wait for puppet to finish on gardener
# finish_gardens_clone
# wait for something

desc 'Launch a Gardens Site'
task :launch_gardens_site do
  puts "Launching a gardens site"
  attempts = 0
  max_attempts = 15
  sleep_time = 60
  instance_size = 'm1.large'
  if (GardensTestRun.instance.fields.fields_stage =~/gsteamer-trunk/)
    instance_size = 'm1.small'
  end
  launch_tasks = [
    'start_hosting_master',
    'run_puppet_on_gardens_master'
  ]
  Dir.chdir(GardensTestRun.instance.config.fields_root + '/master-instance'){
    # run the launch "mutiple times" to get around insufficient capacity
    launch_tasks.each do |task|
      attempts = 0 # reset each time through
      rake_cmd = "rake #{task}"
      begin
        attempts += 1
        puts "Trying to run #{rake_cmd}: attempt #{attempts}"
        run(rake_cmd)
      rescue => run2_fail
        if( attempts <= max_attempts)
          # linear backoff on launching.
          pause_time = sleep_time * attempts
          puts "Attempt #{attempts.to_s} of #{max_attempts.to_s} failed trying again in #{pause_time.to_s} seconds. "
          if (task =~/start_hosting_master/)
            puts "master failed to launch.  cleaning up cruft"
            run2 'rake stop --trace'
          end
          sleep pause_time
          retry
        else
          raise run2_fail
        end
      end
    end
  }
  #Server.InsufficientInstanceCapacity: Insufficient capacity.
  #rake aborted!
end

# Adds a new vhost file to the site so the SUT is available.
# the new_sites variable is defined in test_set.yaml.  It is a list of subdomains
# which should have sites created for them.  The sut_domain is also in the config
# file.  If sut_domain = "gardenssite.com" and new_sites = ["test"], you'll get a
# site at test.gardenssite.com.
# @TODO: This should really be a remote_task
task :configure_vhost do
  vhosts_record = "NameVirtualHost *:80\n"
  subdomain_list = $config['subdomains'] || $config['new_sites']
  subdomain_list.each {|site|
    vhosts_record += <<VHOST
<VirtualHost *:80>
  <Directory \\"/mnt/www/#{GardensTestRun.instance.config.sut_domain}/docroot\\">
    Order Deny,Allow
    Allow from all
    Options FollowSymLinks ExecCGI
    AllowOverride All
    FCGIWrapper /usr/bin/php-cgi .php
  </Directory>

  ServerName #{site}.#{GardensTestRun.instance.config.sut_domain}
  DocumentRoot \\"/mnt/www/#{GardensTestRun.instance.config.sut_domain}/docroot\\"

  UseCanonicalName Off
  RewriteEngine On
  ErrorLog syslog
  FileETag none
  AddOutputFilterByType DEFLATE text/css application/javascript application/x-javascript text/html
  <IfModule mod_fcgid.c>
    AddHandler fcgid-script .fcgi
    IdleTimeout 300
    BusyTimeout 300
    ProcessLifeTime 7200
    IPCConnectTimeout 300
    IPCCommTimeout 7200
  </IfModule>
</VirtualHost>
VHOST
  }

  ssh(GardensTestRun.instance.config.machine.dns_name) {|remote|
    Log.logger.info remote.run("echo \"#{vhosts_record}\" > /etc/apache2/conf.d/testing-vhost.conf")
    Log.logger.info remote.run("/etc/init.d/apache2 restart")
  }
end

# This is not pretty, but existing code requries on these variables being set
# dynamically, and not stored in the base config file.  The author (Jacob) is
# not sure why, but here for the sake of consistency.  These vars are added
# a new config file is generated and it is uploaded to the remote server.
task :add_remote_creds_to_configuration_file do
  unless GardensTestRun.instance.config_file
    raise '$config_file is undefined, cannot upload to remove host'
  end

  config = $config
  config['sut_host'] = GardensTestRun.instance.config.sut_domain
  # @TODO: Put these settings in constants or config files
  
  if ENV['CUSTOM_YAML_TEST_SET'].to_s.empty?
    yaml_file_name =  'test_set.yaml'
    Log.logger.info("No CUSTOM_YAML_TEST_SET found, assuming 'test_set.yaml' is our filename on the remote host.")
  else
    yaml_file_name =  Pathname.new(ENV['CUSTOM_YAML_TEST_SET'].to_s).basename.to_s
    Log.logger.info("Found a CUSTOM_YAML_TEST_SET. We will push the config file to the remote host as: #{yaml_file_name}.")
  end
  
  #Write yaml config to temporary file
  #@TODO: should be using tmpfile and/or not doing this.
  temporary_file = "#{Dir.tmpdir}/gardens_automation.#{Time.now.to_i}.#{yaml_file_name}"
  File.open(temporary_file, 'w') {|f| YAML::dump(config, f) }
  # Upload our existing dumped config file + these additions to the remote server
  Log.logger.info("Uploading our current config to /root/#{yaml_file_name} on the remote machine")
  scp(GardensTestRun.instance.config.machine.dns_name) {|remote|
    remote.upload!(temporary_file, "/root/#{yaml_file_name}")
  }
  Log.logger.info("Uploading done.")
end

desc "Uploads the rake file needed to run tasks on the server"
task :upload_remote_rakefile do
  remote_rakefile_path = '/root/gardens-test.rake'
  unless GardensTestRun.instance.remote_rakefile
    Log.logger.info("GardensTestRun.instance.remote_rakefile is NOT defined, uploading base config stub")
    # If there is no remote rakefile to upload, still upload the base configuration library (I guess).
    # This whole remote rakefile scheme needs massive refactoring.
    contents = <<RF
require 'remote-rakefile.lib.rb'
RF
    ssh(GardensTestRun.instance.config.machine.dns_name) {|remote|
      Log.logger.info(remote.run("echo \"#{contents}\" > #{remote_rakefile_path}"))
    }
  end
  rake_file_lib_path = File.expand_path_relative_to_caller('remote-rakefile.lib.rb')
  rake_file_path = GardensTestRun.instance.remote_rakefile
  
  Log.logger.info("Trying to upload #{rake_file_lib_path.inspect} and #{rake_file_path.inspect} to #{GardensTestRun.instance.config.machine.dns_name}")
  scp(GardensTestRun.instance.config.machine.dns_name) {|remote|
    Log.logger.info("Uploading #{rake_file_lib_path} to /root")
    remote.upload!(rake_file_lib_path, '/root')
    #@TODO: should be a constant or config and not defined within this task, and within another task.
    Log.logger.info("Uploading #{rake_file_path} to #{remote_rakefile_path}")
    remote.upload!(rake_file_path, remote_rakefile_path)
  }
end

desc 'delete old fields directory'
task :del_fields do
  puts "deleting the stale fields checkout"
  @os = Object.new
  @os.extend OS
  Dir.chdir('../'){
    @os.attempt('rm -rf ' + 'fields')
  }
end

task :teardown => [:stop_master]

remote_task :install_from_svn_release
remote_task :install_gardens

desc "Sets the DNS name to be the aws external name"
task :set_dns do
  
  subdomain_list = $config['subdomains'] || $config['new_sites']
  subdomain_list.each{|site|
    GardensTestRun.instance.set_dns(site+'.'+GardensTestRun.instance.config.sut_domain)
  }
end
