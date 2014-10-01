class GardenerTasksWebDriver
  require 'acquia_qa/it'
  require 'acquia_qa/ssh'
  require 'acquia_qa/log'
  require 'acquia_qa/credentials'
  require 'acquia_qa/fields'
  require 'acquia_qa/gardener'
  require 'acquia_qa/configuration'
  require 'acquia_qa/site'
  require 'acquia_qa/gardens_automation'
  require "net/http"
  
  include Acquia::Config
  include Acquia::SSH
  include GardensAutomation
  include WebDriverTestCase

  def initialize
    #nothing so far
  end
  
  def create_sites(sites_to_create = [])
    if sites_to_create.empty?
      Log.logger.info("No sites to create were specified.")
      return false
    end
    setup #setting up selenium and configuration, defined in the GardensAutomation module inside of the gardens_automation.rb file
    Log.logger.info("I am going to create the following sites on gsteamer: #{sites_to_create.inspect} | The gardener is at #{GardensTestRun.instance.gardener.gardener_url.inspect} according to GardensTestRun.instance.gardener.gardener_url")
      
    if GardensTestRun.instance.gardener.gardener_url.include?("http://")
      url = GardensTestRun.instance.gardener.gardener_url
    else
      url = "https://" + GardensTestRun.instance.gardener.gardener_fqhn
    end
      
    #kick out all sites that exist already
    sites_to_create.reject! { |site_name| 
      full_site_domain = GardensTestRun.instance.gardener.gardener_fqhn.gsub("gardener", site_name)
      current_status = 0
      begin
        current_status = Net::HTTP.new(full_site_domain, 80).request_head('/').code
      rescue StandardError => e
        Log.logger.info("Error while trying to access #{full_site_domain}: #{e.message}")
      end
        
      if current_status.to_i == 200
        Log.logger.info("#{full_site_domain} already seems to exist, deleting from list")
        exists_already = true
      else
        Log.logger.info("#{full_site_domain} doesn't seems to exist so far (code: #{current_status}), continuing with creation.")
        exists_already = false
      end
      #tell the block weather to delete the site from the array or not
      exists_already
    }
      
    if sites_to_create.empty?
      Log.logger.info("After checking the sites, there don't seem to be any more left to create.")
      return false
    end
      
    Log.logger.info("URL in the create_testing_sites task is: #{url.inspect}")
    Log.logger.info("Making sure qatest-user exists.")
    create_qatest_user #defined in the GardenerSmokeTestCase inside of the gardens_automation.rb file
    #Closing the admin browser session since it was only needed to create the qauser
    @browser.quit rescue nil
    Log.logger.info("qatest-user created")
    qa_user_name = $config['user_accounts']['qatestuser']['user']
    qa_user_pass = $config['user_accounts']['qatestuser']['password']
    site_creation_forks = {}
      
    #create packages of 4 domains each
    parallel_forks = 4
    site_packages = sites_to_create.each_slice(parallel_forks).map
    site_packages.each_with_index do |site_package, package_index|
      Log.logger.info("[Package #{package_index+1}/#{site_packages.size}] | Launching #{parallel_forks} new site creation forks.")
      site_package.each_with_index do |site_name, index|
        current_fork_pid = Process.fork {
          full_site_domain = GardensTestRun.instance.gardener.gardener_fqhn.gsub("gardener", site_name)
          Log.logger.info("Creating a new browser session pointing at #{@sut_url}")          
          Site.open_and_close_browser(@sut_url, @sel_info) do |temp_browser|
            Log.logger.info("[Package #{package_index+1}/#{site_package.size}] | [#{index+1}/#{site_package.size}] creating a new site (#{full_site_domain}) using: make_gardens_site_as_user")
            begin
              newsite = GardensTestRun.instance.gardener.make_gardens_site_as_user(temp_browser, qa_user_name, qa_user_pass, site_name, 'Campaign template', 600)
              puts "[#{index + 1}/#{site_package.size}] new site is: #{site_name}"
              unless (newsite)
                raise "[Package #{package_index+1}/#{site_package.size}] | [#{index}/#{site_package.size}] site creation did not successfully complete: #{newsite}"
              end
            rescue InvalidSiteNameError => e
              Log.logger.warn("We got an 'invalid site name' error on #{site_name}. (site seems to exist in the gardener nodelist but didn't return HTTP 200 when we checked). We'll check internally what's up with that site.")
              newsite = GardensTestRun.instance.gardener.wait_for_site_creation(temp_browser, qa_user_name, qa_user_pass, site_name, timeout=600)
            end
          end
        }
        site_creation_forks[current_fork_pid] = site_name
      end
      Log.logger.info("[Package #{package_index+1}/#{site_packages.size}] | Waiting for site creation forks to come back.")
      results = Process.waitall
      Log.logger.info("[Package #{package_index+1}/#{site_packages.size}] | Forks all came back: #{results.inspect}")
        
      #check if everything that came back worked, if not: error out
      results.each do |status_pair|
        current_pid = status_pair[1].pid
        exit_status = status_pair[1].exitstatus
        raise "The site creation process for #{site_creation_forks[current_pid]} failed" if exit_status != 0
      end 
    end
    Log.logger.info("Seems like all of the sites got created.")
    
    
    #Check again
    successful_creations = sites_to_create.select { |site_name| 
      full_site_domain = GardensTestRun.instance.gardener.gardener_fqhn.gsub("gardener", site_name)
      current_status = 0
      begin
        current_status = Net::HTTP.new(full_site_domain, 80).request_head('/').code
      rescue StandardError => e
        Log.logger.info("Error while trying to access #{full_site_domain}: #{e.message}")
      end
        
      if current_status.to_i == 200
        Log.logger.info("#{full_site_domain} got created, yay!")
        exists_already = true
      else
        Log.logger.info("#{full_site_domain} doesn't seems to exist (code: #{current_status}). I guess we're going to fail :(")
        exists_already = false
      end
      #tell the block weather to delete the site from the array or not
      exists_already
    }
    
    failed_sites = (sites_to_create - successful_creations)
    if failed_sites.empty?
      Log.logger.info("All sites responded with HTTP 200, yay!")
    else
      raise "The following sites didn't get created: #{failed_sites.inspect}"
    end
    return true
  end
  
  
  #This method injects our testing shortcuts into the managed servers. Without those, we won't be able to run some of our tests
  def inject_php_files
    
    raise "We only inject php to gsteamer for now!" if GardensTestRun.instance.gardener.stage != 'gsteamer'
    Log.logger.info("Injecting testing php files into stage: #{GardensTestRun.instance.gardener.stage}")
    
    #TODO: These are hardcoded! Change that!
    managed_servers = ['managed-51.gsteamer.hosting.acquia.com', 'managed-47.gsteamer.hosting.acquia.com']
    
    managed_servers.each do |host|
      Log.logger.info("Inbjecting PHP file into host: #{host.inspect}.")
      scp(host) {|remote|
        remote.upload!("../helpers/testing_themefolder_related.php", "/mnt/www/html/tangle001/docroot/testing_themefolder_related.php")
      }
    end
  
  end
end
