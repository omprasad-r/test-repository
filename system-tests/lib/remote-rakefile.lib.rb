require 'rubygems'
require 'fileutils'
require 'yaml'
require 'itlib/svn'
require 'itlib/it'
require 'itlib/log'
require 'itlib/tasks/tasks'
require 'itlib/aws'
require 'itlib/configuration'
include OS
include Acquia::Config

$it = IT.new

desc 'INSTALL Drupal - complete set of tasks'
task :install_drupal => [:install_from_svn_release, :install_gardens] do

end
desc 'Minimum install of gardens'
task :min_install => [:install_from_svn_release, :install_gardens]

desc "Sets the DNS name to be the aws external name"
task :set_dns do
  subdomain_list = $config['subdomains'] || $config['new_sites']
  subdomain_list.each{|site|
    set_dns(site+'.'+$config['sut_host'])
  }
end

desc 'Install into mount from the svn release'
task :install_from_svn_release do
  #"https://svn.acquia.com/repos/engineering/gardens/trunk"
  svn_path = $config['gardens']['url']
  Log.logger.info("Installing gardens from SVN path: #{svn_path.inspect}.")
  site_base_directory = "/mnt/www/#{$config['sut_host']}"
  Log.logger.info("Our current site base directory is: #{site_base_directory.inspect}.")
  
  if File.exist?(site_base_directory) #shouldn't be there... just in case.
    Log.logger.warn("Site base directory was already present! Cleaning up!")
    FileUtils.rm_f(site_base_directory, :force => true) 
  end

  Log.logger.info("Doing an SVN checkout to: #{site_base_directory.inspect}")
  #  FileUtils.mkdir_p "/tmp/svn_checkout"
  svn_co(svn_path, site_base_directory)
  Log.logger.info("SVN checkout done.")
  
  # centos has min regexp avail w/o enabling shopt so we assume ends w/digit to distinguish from -update   <-- Marc: what?
  Log.logger.info("Enabling 'RewriteBase' in #{site_base_directory}/docroot/.htaccess")
  run "sed -i -e 's,# RewriteBase /$, RewriteBase /,' #{site_base_directory}/docroot/.htaccess"

  installation_pids = []
  subdomain_list = $config['subdomains'] || $config['new_sites']
  subdomain_list.each {|site|
    if (site =~/d7/)
      site_dir = "default"
    else
      site_dir = "#{site}.#{$config['sut_host']}"
    end
    Log.logger.info("[#{site}] Installing for #{site}. site_dir is #{site_dir.inspect}")
    #Username in MySQL: max 16 chars!
    #create_db('localhost', 'root', '', db_user + site, db_passwd, site)
    #--> create_db(mysql_host, mysql_admin, mysql_admin_pw, mysql_user, mysql_user_pw, mysql_user_db)
    
    Log.logger.debug("[#{site}] Creating MySQL database + user for #{site}. site_dir is #{site_dir.inspect}")
    db_user = "#{$config['user_accounts']['database']['user']}#{site}".gsub("-","")[0..15]
    db_pass = $config['user_accounts']['database']['password']
    create_db('localhost', 'root', '', mysql_user = db_user, mysql_pass = db_pass, mysql_user_db = site.gsub("-",""))
      
    directories_to_create = []
    directories_to_create << "#{site_base_directory}/docroot/sites/#{site_dir}/files"
    directories_to_create << "#{site_base_directory}/docroot/sites/#{site_dir}/themes/mythemes"
    directories_to_create << "#{site_base_directory}/docroot/sites/#{site_dir}/private/files"
    directories_to_create << "#{site_base_directory}/docroot/sites/#{site_dir}/private/temp"
      
    Log.logger.info("[#{site}] Creating the site's directories: #{directories_to_create.inspect}")
    FileUtils.mkdir_p(directories_to_create)   
    Log.logger.info("[#{site}] Directories created sucessfully: #{Dir[site_base_directory + '/docroot/sites/' + site_dir +  '/*'].inspect}")
      
    Log.logger.info("[#{site}] Copying settings over")
    original_settings_file = "#{site_base_directory}/docroot/sites/default/default.settings.php"
    if File.exist?(original_settings_file)
      FileUtils.cp(original_settings_file, "#{site_base_directory}/docroot/sites/#{site_dir}/settings.php", :preserve => true)
    else
      Log.logger.warn("[#{site}] Didn't find settings file: #{original_settings_file.inspect}.")
      Log.logger.warn("[#{site}] Content of sites dir: #{Dir[site_base_directory + '/docroot/sites/*'].inspect}")
      Log.logger.warn("[#{site}] Content of default dir: #{Dir[site_base_directory + '/docroot/sites/default/*'].inspect}")
      raise "Couldn't copy default settings.php."
    end
    Log.logger.info("[#{site}] Settings sucessfully copied.")
      
    files_to_chown = []
    files_to_chown << "#{site_base_directory}/docroot/sites/#{site_dir}/settings.php"
    files_to_chown << "#{site_base_directory}/docroot/sites/#{site_dir}/files"
    files_to_chown << "#{site_base_directory}/docroot/sites/#{site_dir}/private/temp"
    files_to_chown << "#{site_base_directory}/docroot/sites/#{site_dir}/private/files"
    files_to_chown << "#{site_base_directory}/docroot/sites/all/themes"
    files_to_chown << "#{site_base_directory}/docroot/sites/#{site_dir}/themes"
    #chown to user www-data
    Log.logger.info("[#{site}] Setting www-data as owner")
    FileUtils.chown_R('www-data', nil, files_to_chown)  
    Log.logger.info("[#{site}] Permissions sucessfully set")
  } #end of the new sites iterator
end

desc 'do the gardens install ignoring d7 sites'
task :install_gardens do
  
  installation_pids = []
  base_path = "/mnt/www/#{$config['sut_host']}"  
  
  subdomain_list = $config['subdomains'] || $config['new_sites']
  subdomain_list.each{|site|
    if site =~/d7/
      Log.logger.info "[#{site}] Seems to be a D7 site, skipping)"
      next
    end
    site_dir = "#{site}.#{$config['sut_host']}"
    Log.logger.info "[#{site}] Starting installation) | base path: #{base_path.inspect} | site_dir: #{site_dir.inspect}"
    db_user = "#{$config['user_accounts']['database']['user']}#{site}".gsub("-","")[0..15]
    db_pass = $config['user_accounts']['database']['password']
    drupal_user = $config['user_accounts']['qatestuser']['user']
    drupal_pass = $config['user_accounts']['qatestuser']['password']
    install_cmd = "cd #{base_path}; " +
      "php -d memory_limit=128M " +
      "#{base_path}/install_gardens.php " +
      "acquia_gardens_local_user_accounts=1 " +
      "database=\"#{site.gsub("-","")}\" username=\"#{db_user}\" password=\"#{db_pass}\" " +
      " site_name=\"#{site_dir}\" name=\"#{drupal_user}\" pass=\"#{drupal_pass}\"" +
      " site_template=\"campaign\" url=\"http:\/\/#{site_dir}\" 2>&1 > /tmp/#{site}.install.log"
    Log.logger.debug("[#{site}] executing command: #{install_cmd}")
    #ok, this is ugly, but since ruby 1.8 is using green threads and can't deal with system calls properly
    #This is the way to speed it up I guess
    #We spin up a new thread for every site and fork the VM.
    installation_pids << Process.fork {
      begin
        Log.logger.info(run install_cmd)
      ensure
        log_path = "/tmp/#{site}.install.log"
        if File.exist?(log_path)
          install_log = IO.read(log_path)
          Log.logger.debug("[#{site}] Install log: #{install_log.inspect}")
        else
          Log.logger.warn("[#{site}] Install log: NO INSTALL LOG FOUND at #{log_path.inspect}")
        end
        files_to_chown = []
        files_to_chown << "#{base_path}/docroot/sites/#{site_dir}/themes"
        files_to_chown << "#{base_path}/docroot/sites/#{site_dir}/files"
        Log.logger.info("[#{site}] Setting www-data as owner")
        FileUtils.chown_R('www-data', nil, files_to_chown)  
        Log.logger.info("[#{site}] Creating and setting /mnt/tmp to 1777")
        unless File.exist?('/mnt/tmp')
          FileUtils.mkdir_p "/mnt/tmp"
        end
        FileUtils.chmod(1777, "/mnt/tmp")
      end
    } #fork
  } #new sites tierator
  #wait for the forks to finish
  results = Process.waitall #a wait a day keeps the zombies away
  Log.logger.info("All Installation forks came back. #{results.inspect}")
end

def it
  $it
end

def cvs_co(_server, _repo, _module, _localdir)
  old_dir = Dir.getwd()
  full_path = Pathname.new(_localdir)
  parent = full_path.parent
  sitename = full_path.basename
  server_type = "pserver"
  user = "anonymous"
  password = "anonymous"
  action = 'checkout'
  server_string = ":#{server_type}:#{user}:#{password}@#{_server}:#{_repo}"
  cmd  = "cvs -z6 -d#{server_string} #{action} -d #{sitename}/docroot #{_module}"
  Dir.chdir(parent)
  begin
    run cmd
  ensure
    Dir.chdir(old_dir)
  end
end

def svn_co(_branch, _localdir)
  svn_branch = _branch
  svn = SvnCommand.new
  svn.credentials = it.svn_credentials.get
  svn.url = svn_branch
  svn.localdir = _localdir
  SvnOperation.new(svn).force_get()
end

def svn_switch(_branch, _localdir)
  svn_branch = _branch
  svn = SvnCommand.new
  svn.credentials = it.svn_credentials.get
  svn.url = svn_branch
  svn.localdir = _localdir
  SvnOperation.new(svn).get()
end



# copied from IT lib/rake/fields.rb
class IT
  def system_details
    SystemDetails.new
  end
  def lib_svn
    svn = infrastructure_svn
    svn.url += '/lib/lib'
    svn.localdir = '/usr/lib/ruby/site_ruby/1.8'
    return svn
  end
end


module RakeConfig
  #  DB_VOLUME_ID = 'vol-da2bceb3'
  #  FILES_VOLUME_ID = 'vol-ead83c83'
  #  DRUPAL_VOLUME_ID = 'vol-c103e7a8'

  def initialize_config
    @it = IT.new
    @aws = Aws.new()
    @aws.get_metadata
    @thishour = Time.now.strftime("%Y%m%d%H")
    @database = 'production'
  end

  def aws
    @aws
  end

  def it
    @it
  end

  # assume the domain is nnn.com  and fdqn is aaa.bbb.nnn.com
  def set_dns(fdqn, ttl=60)
    frags = fdqn.split('.')
    domainarr = []
    # com
    domainarr.unshift(frags.pop)
    # acquia
    domainarr.unshift(frags.pop)
    domain  = domainarr.join('.')
    subdomain = frags.join('.')
    puts "fdqn: #{fdqn}, domain: #{domain}, subdomain: #{subdomain}"
    dns_api = it.dns_provider
    dns_api.create_or_replace_a_record(domain, "#{subdomain}.#{domain}", it.system_details.external_ipaddress())
  end

  def create_db(mysql_host, mysql_admin, mysql_admin_pw, mysql_user, mysql_user_pw, mysql_user_db)
    mycmd = 'mysql -u ' + mysql_admin + ' mysql -e '
    users = ["\'#{mysql_user}\'@\'localhost\'", "\'#{mysql_user}\'@\'%\'"]
    users.each{|user|
      puts "Dropping user"
      qs = "\"GRANT USAGE ON *.* TO " + user + ";\""
      qs2 =  "\"DROP USER " + user + ";\""
      puts qs
      cmd = mycmd  + qs; puts cmd
      result = `#{cmd}`
      puts result
      puts "query:" + qs2
      cmd = mycmd  + qs2; puts cmd
      result = `#{cmd}`
      puts(result)
      puts "Creating user"
      qs = "\"CREATE USER " + user + " IDENTIFIED BY \'" + mysql_user_pw + "\';\""
      cmd = mycmd + qs; puts cmd
      result = `#{cmd}`
      puts result
      puts "granting privs user"
      qs = "\"GRANT ALL on  " + mysql_user_db + '.* TO ' + user + ";\""
      cmd  = mycmd + qs; puts cmd
      result = `#{cmd}`
      puts result
    }
    puts "dropping db"
    qs = "\"DROP DATABASE IF EXISTS " + mysql_user_db + "\""
    cmd = mycmd + qs; puts cmd
    result = `#{cmd}`
    puts "recreating db"
    qs = "\"CREATE DATABASE IF NOT EXISTS " + mysql_user_db +"\""
    cmd = mycmd + qs; puts cmd
    result = `#{cmd}`

  end
end

include RakeConfig
initialize_config
