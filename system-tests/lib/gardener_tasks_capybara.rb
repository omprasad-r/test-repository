$LOAD_PATH << File.dirname(__FILE__)
require 'rubygems'
require 'capybara/mechanize'
require '../helpers/gardens_automation'
require '../helpers/gardener'
require '../helpers/gardens_capybara'

class GardenerTasks
  include GardensAutomation::CapybaraTestCase

  def create_qatest_user()
   Log.logger.info("Making sure qatest-user exists.")
   #Create our qa test user. Gsteamer currently has a bug where this fails.
   first_try = true
   begin
    GardensManagerCapy.create_qatest_user(:log_in_as_admin => true)
    #Closing the admin browser session since it was only needed to create the qauser
    Log.logger.info("qatest-user created")
    Capybara.reset_sessions! # ES-187 | I think we should reset the session when switching from admin to qatestuser. Right?
   rescue StandardError => e
    if first_try
      first_try = false
      Log.logger.info("Received an error while trying to create qatest-user: '#{e.message}'. Retrying.")
      Capybara.visit("/")
      Capybara.reset_sessions!
      retry 
    else
      Log.logger.info("Received an error while retrying to create qatest-user: '#{e.message}'. Reraising exception.")
      raise e
    end
   end
  end

def create_sites(sites_to_create = [])
  input_unfiltered = sites_to_create

  duplicates = sites_to_create.inject(Hash.new(0)) {|h,v| h[v] += 1; h}.reject{|k,v| v==1}.keys
  unless duplicates.empty?
    puts "**************************************"
    puts "YOU HAVE PASSED IN DUPLICATE SITES!: #{duplicates.inspect} (This often happens when two different tests have the same class name due to too much copypasta.)"
    puts "**************************************"
    sites_to_create.uniq!
  end

  if sites_to_create.empty?
    Log.logger.info("No sites to create were specified.")
    return false
  end

  #This might help the initial login
  Capybara.default_wait_time = 30

  #setting up test configuration and capybara, defined in the CapybaraTestCase module inside of the gardens_automation.rb file
  setup_configuration
  setup_capybara

  if GardensAutomation::GardensTestRun.instance.gardener.gardener_url.include?("http")
    url = GardensAutomation::GardensTestRun.instance.gardener.gardener_url
  else
    url = "https://" + GardensAutomation::GardensTestRun.instance.gardener.gardener_fqhn
  end

  gardener_uri = URI.parse(url)

  Log.logger.info("I am going to create the following sites on gsteamer: #{sites_to_create.inspect} | The gardener is at #{url.inspect}.")

  #kick out all sites that exist already
  sites_to_create.reject! { |site_name| 
    full_site_domain = gardener_uri.host.gsub("gardener", site_name)
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
  Log.logger.info("Pointing capybara to: #{url.inspect}")
  Capybara.app_host = $config['sut_url'].to_s.chomp("/")

  create_qatest_user() if $config['create_qatestuser']

  qa_user_name = $config['user_accounts']['qatestuser']['user']
  qa_user_pass = $config['user_accounts']['qatestuser']['password']
  site_creation_forks = {}

  GardensManagerCapy.login(:username => qa_user_name, :password => qa_user_pass, :forced => true)
  sites_to_create.each do |site|
    Log.logger.info("Creating: #{site.inspect}")
    already_retried = false
    begin
      GardensManagerCapy.make_gardens_site_as_user(:username => qa_user_name, :password => qa_user_pass, :sitename => site, :template => "Campaign template", :do_not_log_in => true)
    rescue StandardError => e
      if already_retried
        puts Capybara.page.body  
        raise "Site creation for #{site.inspect} failed: #{e.message}\n#{e.backtrace}"
      else
        puts "Retrying"
        already_retried = true
        retry      
      end

    end
    Log.logger.info("Started site creation for #{site.inspect}.")
  end


  #Check if they actually got created

  checks_started = Time.now
  left_to_check = sites_to_create
  begin
    successful_creations = left_to_check.select { |site_name| 
      full_site_host = gardener_uri.host.gsub("gardener", site_name)
      current_status = 0
      begin
        current_status = Net::HTTP.new(full_site_host, 80).request_head('/').code
      rescue StandardError => e
        Log.logger.info("Error while trying to access #{full_site_host}: #{e.message}")
      end

      if current_status.to_i == 200
        Log.logger.info("#{full_site_host} got created, yay!")
        exists_already = true
      else
        Log.logger.info("#{full_site_host} doesn't seems to exist (code: #{current_status}) :(")
        exists_already = false
      end
      #tell the block weather to delete the site from the array or not
      exists_already
    }

    failed_sites = (left_to_check - successful_creations)
    if failed_sites.empty?
      Log.logger.info("All sites responded with HTTP 200, yay!")
    else
      left_to_check = failed_sites
      raise "The following #{failed_sites.size} sites didn't get created: #{failed_sites.inspect}"
    end
    return true
  rescue StandardError => e
    #we try again for 90 minutes (seriously...)
    if Time.now < (checks_started + 5400)
      Log.logger.info("The following #{failed_sites.size} sites didn't get created so far: #{failed_sites.inspect}, waiting 30 and trying again.")
      sleep 30
      retry
    else
      #reraise exception
      Log.logger.info("We tried for too long, we're giving up")
      raise e
    end
  end
end


#This method injects our testing shortcuts into the managed servers. Without those, we won't be able to run some of our tests
def inject_php_files

 # raise "We only inject php to gsteamer for now!" if GardensTestRun.instance.gardener.stage != 'gsteamer'
 Log.logger.info("Injecting testing php files into stage: #{GardensTestRun.instance.gardener.stage}")

 #TODO: These are hardcoded! Change that!
 managed_servers = ["managed-51.#{GardensTestRun.instance.gardener.stage}.hosting.acquia.com", "managed-47.#{GardensTestRun.instance.gardener.stage}.hosting.acquia.com"]

 failed_injects = 0
 managed_servers.each do |host|
  begin
    Log.logger.info("Inbjecting PHP file into host: #{host.inspect}.")
    scp(host) {|remote|
      remote.upload!("../helpers/testing_themefolder_related.php", "/mnt/www/html/tangle001/docroot/testing_themefolder_related.php")
    }

  rescue Exception => e
    failed_injects += 1
    puts "Error while injecting php file on #{host.inspect}: #{e.message}"
  end
end
raise "Couldn't inject the php files on ANY server." if failed_injects == managed_servers.size

end
end
