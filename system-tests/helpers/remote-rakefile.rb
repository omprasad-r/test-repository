require 'rubygems'
require 'pathname'
#will be uploaded. On the server we install the itlib gem, we can't use acquia_qa because of that so far
require 'itlib/svn'
require 'itlib/it'
require 'itlib/log'
require 'itlib/aws'
require 'itlib/configuration'
require 'itlib/tasks/tasks'
require 'fileutils'
require 'yaml'

include OS
include Acquia::Config

$it = IT.new

#Either we have set a custom yaml file or we assume it's test_set.yaml
#this should be our uploaded file in /root/test_set.yaml
custom_yaml_name = Pathname.new(ENV['CUSTOM_YAML_TEST_SET'].to_s).basename.to_s
if custom_yaml_name.empty?
  Log.logger.warn("No CUSTOM_YAML_TEST_SET variable found: #{ENV['CUSTOM_YAML_TEST_SET'].inspect}, using default 'test_set.yaml'.")
  yaml_file_name =  'test_set.yaml'
end

#check if that file actually exists and error out otherwise
if File.exist?(custom_yaml_name)
  yaml_file_name =  custom_yaml_name  
else
  Log.logger.debug("Yaml files: #{Dir["*.yaml"].inspect}")
  
  if Dir["*.yaml"].size == 1
    Log.logger.debug("Only 1 yaml file, assuming that's ours")
    yaml_file_name = Dir["*.yaml"].first
  else
    raise("File #{yaml_file_name.inspect} not found. Can't read settings.")
  end
  

end

$config = YAML.load_file(yaml_file_name)
Log.logger.info("Yaml config loaded: #{$config.inspect}")

require 'remote-rakefile.lib.rb'

task :install_themebuildertest_module do
  Log.logger.info("Installing themebuilder modules on installed sites.")
  #Grab list of subdomains (= sites) to create
  subdomain_list = $config['subdomains'] || $config['new_sites']
  subdomain_list.each do |current_site|
    Log.logger.info("Installing themebuilder module for #{current_site}.")
    run "cd /mnt/www/#{$config['sut_host']}/docroot;../drush/drush --yes -l http:\/\/#{current_site}.#{$config['sut_host']} pm-enable themebuilder_test"
  end
end

task :delete_gardener_url do
  Log.logger.info("Deleting gardener url on installed sites.")
  #Grab list of subdomains (= sites) to create. If 'subdomains' doesn't exist, take 'new_sites'
  subdomain_list = $config['subdomains'] || $config['new_sites']
  subdomain_list.each do |current_site|
    Log.logger.info("Deleting gardener URL for #{current_site}.")
    run "cd /mnt/www/#{$config['sut_host']}/docroot;../drush/drush --yes -l http:\/\/#{current_site}.#{$config['sut_host']} vdel acquia_gardens_gardener_url"
  end
end

task :copy_testing_php_shortcuts do
  Log.logger.info("Copying themse_modules_operations.php to installed sites.")
  source_dir = "/mnt/www/#{$config['sut_host']}/system-tests/helpers/"
  target_dir = "/mnt/www/#{$config['sut_host']}/docroot/"
  file_name = "testing_themefolder_related.php"
  if File.exist?("#{target_dir}#{file_name}")
	Log.logger.info("File #{target_dir}#{file_name} already exists, nothing to copy")	
  else
  	run "cp #{source_dir}#{file_name} #{target_dir}#{file_name}"
  end
end
