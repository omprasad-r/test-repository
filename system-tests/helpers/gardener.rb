$LOAD_PATH << File.dirname(__FILE__)
require 'rubygems'
require 'net/ssh'
require 'xmlrpc/client'
require 'json'
require 'acquia_qa/it'
require 'acquia_qa/os'
require 'acquia_qa/ssh'
require 'acquia_qa/configuration'
require 'site.rb'
require 'fields.rb'
require 'acquia_qa/util'
require 'gardener/exception'

class Gardener
  include NetworkSite, Acquia::SSH, Acquia::Config, OS, Acquia::TestingUtil
  @env_set = false

  def initialize(_stage=nil)
    if (_stage)
      stage(_stage)
    end
    @it = IT.new
  end

  def set_env(_conf_file)
    read_inputs(_conf_file)
    home_config = ENV['HOME'] + '/' + File.basename(_conf_file)
    if File.exists?(home_config)
      read_inputs(home_config)
    end
    self.set_environment_variables
  end

  def set_environment_variables
    $basethemesinfo = basethemes
    ENV['FIELDS_STAGE'] = stage
    Log.logger.debug('stage: ' + ENV['FIELDS_STAGE'])
    @env_set = true
  end

  attr_accessor :browser

  # This is getting rather absurd, 
  # but we need to specify the scheme somehow to switch to https
  # @TODO AN-22694: Cleanup all this legacy stuff and implement it via config files or
  # at least most consistent variables.
  def gardener_url
    (ENV['SUT_SCHEME'] || 'https') + "://" + gardener_fqhn()
  end

  def gardener_fqhn
    fqhn = nil
    if ENV['SUT_URL']
      fqhn = ENV['SUT_URL'].to_s.gsub("http://", "").chomp("/")
    else
      fqhn = "gardener.#{self.stage}.acquia-sites.com"
    end
    fqhn
  end

  def get_gardens_domain
    ENV['GARDENER_SITES_DOMAIN'] || "#{self.stage}.acquia-sites.com"
  end

  def sites_domain
    "acquia-sites.com"
  end

  def gardener_preserved_url(site_url)
    ENV['SUT_URL'] || "#{site_url}.#{self.stage}.acquia-sites.com"
  end

  #return gardens stage
  # env FIELDS_STAGE will be set so that read_inputs will not set FIELDS_STAGE and overwrite
  # the work
  def stage(_stage=nil)
    unless (_stage)
      _stage  = ENV['FIELDS_STAGE'] || "#{ENV['USER']}-forgot-fields-stage"
    end
    ENV['FIELDS_STAGE'] = _stage
    _stage
  end

  # returns "the" XMLRPC client; this *used* to be a singleton,
  # but that caused issues w/ SSL, so the singleton pattern was removed.
  def gardener_xmlrpc
    Log.logger.info("Starting gardener XML RPC call")
    #unless (@env_set)
    #  raise "You have not set the gardener env properly"
    #end
    return XMLRPC::Client.new2(self.gardener_api_url)
  end

  def gardener_api_url
    Log.logger.info("Starting gardener api url...")
    # This can be set locally for debugging.
    if @gardener_api_url
      return @gardener_api_url
    end
    #if (@env_set)
    auth = @it.gardenerrpc_credentials.get
    api_url = "https://#{auth.login}:#{auth.password}@" + self.gardener_fqhn() + '/xmlrpc.php?format=none'
    return api_url
    #else
    #  raise "You have not set the fields env properly"
    #end
  end

  def get_tangle_sites(tangle)
    Log.logger.info("Starting XML RPC call")
    val = gardener_xmlrpc
    json_data = val.call('acquia.gardens.get.active.domains.by.site', tangle)
    Log.logger.info("Returning the list of sites in the JSON format.....")
    return json_data
  end

  def get_site_node(site_prefix)
    return get_node_by_domain(site_prefix + '.' + get_gardens_domain)
  end
  
  # @return the node object created for the site
  def make_gardens_site_as_user(browser, user, password, site_prefix, template = 'Campaign template', timeout=300)
    Log.logger.info("Trying to create a new site called #{site_prefix}")
    Log.logger.info("Calling the enter_new_sitename method")
    self.enter_new_sitename(browser, user, password, site_prefix)
    sleep 1 # To let the automatic validation work
    if (!browser.find_elements(:xpath => "//div[@id='edit-site-prefix-hint']").empty? || !browser.find_elements(:xpath => "//p[contains(@title, 'URL taken')]").empty?)
      Log.logger.info("Site name selection failed.")
      return nil
    else
      wait = Selenium::WebDriver::Wait.new(:timeout => 30)
      Log.logger.info("Waiting for the site to be available")
      wait.until { browser.find_element(:xpath => "//p[contains(@title, 'Available')]") }
      Log.logger.info("Waiting for the submit button to be enabled")
      btn = wait.until { browser.find_element(:xpath => "//button[@id='edit-submit']") }
      wait.until { btn.enabled? }
      Log.logger.info("Clicking on the submit button and waiting for new page to load")
      btn.click
      if browser.find_element(:xpath => "//body").text.include?("Validation error")
        raise "We ended up with a validation Error. Does the site maybe exist already?"
      end
      Log.logger.info("Page loaded, going on to template selection")
      # @TODO: This is actually a URL, not a domain
      domain = self.finish_site_creation(browser, user, password, site_prefix, template, timeout, template_selected = false)
      if domain.nil?
        Log.logger.warn("Creation of site '#{site_prefix}' seems to have failed!")
      else
        Log.logger.info("Creation of '#{domain}' seems to have succeeded")
      end 
      return get_node_by_domain(site_prefix + '.' + get_gardens_domain)
    end
  end

  def enter_new_sitename(browser, user, password, site_prefix)
    Log.logger.info("Beginning to make new gardens site")
    browser.get(@sut_url)
    Log.logger.info("logging in as: #{user}")
    login(user, password, browser)
    # Click "Create Site"
    Log.logger.info("Switching back to main site!")
    browser.get(@sut_url)
    #TODO: sometimes we're logged out after switching pages... a refresh solves this. Maybe a bug?
    error_count = 0
    while !browser.find_elements(:xpath => "//body[contains(@class, 'not-logged-in')]").empty?
      if (error_count+=1) > 5
        raise 'site creation did not successfully complete (we seem to have been logged out)'
      end
      Log.logger.info("We're not logged in, refreshing!")    
      browser.navigate.refresh
      sleep 1 #TODO
    end
    Log.logger.info("We are logged in!")    

    Log.logger.info("Waiting and clicking on the create site link")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    wait.until { browser.find_element(:xpath => "//a[@href='/create-site']") }.click
    #TODO: sometimes we're logged out after switching pages... a refresh solves this. Maybe a bug?
    error_count = 0
    while !browser.find_elements(:xpath => "//body[contains(@class, 'not-logged-in')]").empty?
      error_count+=1
      if ((error_count > 10) and (error_count < 15))
        Log.logger.warn("Still logged out, trying to log in again... bug?")
        login(user, password, browser)
      elsif error_count >= 15
        raise 'site creation did not successfully complete'
      else
        Log.logger.info("We're not logged in, refreshing!")    
        browser.navigate.refresh
        sleep 1 #TODO
      end
    end

    # Add the site "name" to the site
    Log.logger.info("waiting for the edit-site-prefix to be able to add the site name")
    temp = wait.until { browser.find_element(:xpath => "//input[@id='edit-site-prefix']") }
    temp.clear
    temp.send_keys(site_prefix)
    Log.logger.info("Waiting for validation to do its work")
    sleep 0.5
    wait.until { browser.find_element(:xpath => "//p[@id='site-prefix-status' and contains(@class, 'valid')]") } # wait for the site name validator to show up
    if browser.find_elements(:xpath => "//p[@id='site-prefix-status' and contains(@class, 'invalid')]").size > 0
      Log.logger.warn("Tried to set an invalid site name: #{site_prefix.inspect}")
      raise InvalidSiteNameError
    end
  end

  def select_template_and_features(browser, user, password, site_prefix, template = 'Campaign template', timeout=300, select_features = 'all')
    self.enter_new_sitename(browser, user, password, site_prefix)
    if (!browser.find_elements(:xpath => "//div[@id='edit-site-prefix-hint']").empty? ||
        !browser.find_elements(:xpath => "//p[contains(@title, 'URL taken')]").empty?)
      Log.logger.info("Site name selection failed.")
      return nil
    end
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    wait.until { browser.find_element(:xpath => "//p[contains(@title, 'Available')]") }
    browser.find_element(:xpath => "//button[@id='edit-submit']").click
    self.template_selection(template, browser)
    if(select_features != 'random')
      wait.until { browser.find_element(:xpath => "//a[contains(@class,'select-#{select_features}')]") }.click
      features_list_values = self.make_list_of_features_and_values(random=false)
    else
      features_list_values = self.make_list_of_features_and_values(random=true)
    end
    return features_list_values
  end

  def make_gardens_site(browser, user, password, mail, site_prefix, regcode, template = 'Campaign template', timeout=300)
    Log.logger.info("Logging out and making new user and site with a registration code")
    browser.get(@sut_url)
    logout
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    wait.until { browser.find_element(:xpath => "//a[@href='/create-site']") }.click
    temp = wait.until { browser.find_element(:xpath => "//input[@id='edit-site-prefix']") }
    temp.clear
    temp.send_keys(site_prefix)
    temp = wait.until { browser.find_element(:xpath => "//input[@id='edit-mail']") }
    temp.clear
    temp.send_keys(mail)
    temp = wait.until { browser.find_element(:xpath => "//input[@id='edit-name']") }
    temp.clear
    temp.send_keys(user)
    temp = wait.until { browser.find_element(:xpath => "//input[@id='edit-pass']") }
    temp.clear
    temp.send_keys(password)
    temp = wait.until { browser.find_element(:xpath => "//input[@id='edit-regcode-code']") }
    temp.clear
    temp.send_keys(regcode)
    wait.until { browser.find_element(:xpath => "//button[@id='edit-submit']") }.click
    return self.finish_site_creation(browser, user, password, site_prefix, template, timeout)
  end

  def make_gardens_site_with_current_user(site_prefix, template = 'Campaign template', browser = @browser)
    Log.logger.info("Creating site #{site_prefix} with the user currently signed in.")
    browser.get("#{@sut_url}/create-site")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    temp = wait.until { browser.find_element(:id => "edit-site-prefix") }
    temp.clear
    temp.send_keys(site_prefix)
    sleep 1
    browser.find_element(:id => "edit-submit").click
    return finish_site_creation(browser, nil, nil, site_prefix, template)
  end

  ##TODO...
  def create_user_with_reg_code(user_name,e_mail,password, roles = ['gardens preview', 'acquia engineering'])
    reg_code = self.get_reg_code(true)
  end

  # the 'acquia tester (QA)' role is allowed to create unlimited sites without
  # hitting the flood controls.
  def create_user_as_admin(user_name,e_mail,password, roles = ['gardens preview', 'acquia engineering'], browser = @browser)
    Log.logger.info("Logging into #{browser.browser_url} as admin and creating new user #{user_name}")
    browser.get(@sut_url)
    login_as_admin(browser)
    browser.get("#{@sut_url}/users/#{user_name}")

    if (browser.find_element(:xpath => "//body").text.include?("404 error"))
      browser.get("#{@sut_url}/admin/user/user/create")
      temp = browser.find_element(:id => "edit-name")
      temp.clear
      temp.send_keys(user_name)
      temp = browser.find_element(:id => "edit-mail")
      temp.clear
      temp.send_keys(e_mail)
      temp = browser.find_element(:id => "edit-pass-pass1")
      temp.clear
      temp.send_keys(password)
      temp = browser.find_element(:id => "edit-pass-pass2")
      temp.clear
      temp.send_keys(password)
      # to get the role checkbox "//div[contains(@id,'edit-roles')]//label[contains(text(), 'administrator')]//input"
      unless roles.nil?
        roles.each do |role|
          rol_sel = "//div[contains(@id,'edit-roles')]//label[contains(text(), '#{role}')]//input"
          browser.find_element(:xpath => rol_sel).click if !(browser.find_elements(:xpath => rol_sel).empty?)
        end
      end
      agree_with_tos = 'edit-I-agree'
      browser.find_element(:id => agree_with_tos).click if !(browser.find_elements(:id => agree_with_tos).empty?)
      browser.find_element(:xpath => "//input[@id='edit-submit' and @name='op' and @value='Create new account']").click
    else
      Log.logger.debug("User " + user_name + " appears to already exist");
    end
  end

  def delete_user(user_name, browser = @browser)
    Log.logger.info("Deleting user #{user_name}")
    browser.get("#{@sut_url}/user/#{user_name}")
    if (! browser.find_element(:xpath => "//body").text.include?("Page not found"))
      browser.find_element(:xpath => "//a[contains(@href, 'user') and contains(@href, 'edit')]/span[text()='Edit']").click
      browser.find_element(:id => "edit-delete").click
      browser.find_element(:id => "edit-submit").click
    else
      Log.logger.debug(user_name + " not found not deleting user")
    end
  end

  # screen scrapes for reg codes
  # in_use = true, return a reg code that is in use or nil if none in use
  # in_use = false, return the next available reg code
  # this must run as admin
  def get_reg_code(in_use = false, browser = @browser)
    Log.logger.info("Starting Regcode processing")
    browser.get(@sut_url)
    login_as_admin(browser)
    browser.get("#{@sut_url}/admin/user/regcodes/list")
    flag = false
    browser.find_element(:id => 'edit-category').find_elements(:xpath => '//option').each { |e| next unless e.text.include?("all"); flag = true ; e.click; break; }
    Log.logger.info("Select FAILED") unless flag
    flag = false
    browser.find_element(:id => 'edit-is-active').find_elements(:xpath => '//option').each { |e| next unless e.text == '1'; flag = true; e.click; break; }
    Log.logger.info("Select FAILED") unless flag
    flag = false
    browser.find_element(:id => 'edit-is-sent').find_elements(:xpath =>  '//option').each { |e| next unless e.text == '0'; flag = true; e.click; break; }
    Log.logger.info("Select FAILED") unless flag
    browser.find_element(:id => 'edit-submit').click
    body = browser.find_element(:xpath => "//body").text
    codes = body.split(/--/)
    avail_regcodes = Array.new
    used_regcodes = Array.new
    codes.each do |code|
      pat = /Private Beta(.+)(Yes|No)/
      regmatch = pat.match(code)
      regcode = regmatch.to_a
      if (regcode)
        if (regcode[2] == 'Yes')
          avail_regcodes.push(regcode[1])
        end
        if (regcode[2] == 'No')
          used_regcodes.push(regcode[1])
        end
      end
    end
    Log.logger.info("End of regcode processing")
    if (in_use)
      return used_regcodes[rand[used_regcodes.size]]
    else
      return avail_regcodes[0]
    end
    return nil
  end

  # remove the "old" branch assume it is seqrch-qa
  def remove_active_branch(project = 'gardener', branch = 'branches/gsteamer-nightly')
    if (branch =~/live|trunk/i &&
          branch !=/tags/i)
      raise("Do not try and remove #{branch}")
    end
    svn_uri = project + '/' + branch
    if (repo_exists?(svn_uri))
      Log.logger.info("Attempting to remove #{svn_uri}")
      svn = SvnCommand.new
      svn.credentials = @it.svn_credentials.get
      svn.url = @it.engineering_svn_url + '/' + svn_uri
      svn.ci_comment = 'Removing for QA Automated Test'
      run(svn.remove)
    end
  end

  def copy_branch_to_active(project = 'gardener', trunk_branch = 'trunk', live_branch = 'branches/gsteamer-nightly')
    if (live_branch =~/live|trunk/i &&
          branch !=/tags/i)
      raise("Do not try and copy onto #{live_branch}")
    end
    live_uri = project + '/' + live_branch
    trunk_uri = project + '/' + trunk_branch
    Log.logger.info("Copying #{trunk_uri} to #{live_uri}")
    svn = SvnCommand.new
    svn.credentials = @it.svn_credentials.get
    svn.url = @it.engineering_svn_url + '/' + trunk_uri
    svn.localdir = @it.engineering_svn_url + '/' + live_uri
    svn.ci_comment = "Copying to #{live_uri} for QA Automated Test"
    run(svn.copy)    
  end

  def repo_exists?(_branch)
    rexists = true
    svn = SvnCommand.new
    svn.credentials = @it.svn_credentials.get
    svn.url = @it.engineering_svn_url + '/' + _branch
    result = run(svn.info + ' 2>&1') # need to capture STDERR
    if (result =~/Not a valid url/i)
      rexists = false
    end
    return rexists
  end

  # Asks the gardener whether a particular Gardens site has been successfully created, continues polling for {retry_time} seconds or when site is complete. Returns site state
  def wait_for_site_creation(browser, user, password, site_prefix, timeout=600)
    Log.logger.info("Checking if site with prefix #{site_prefix} has been created successfully yet using '/gardens-site-install-status-report'.")
    Log.logger.info("Logging in as admin")
    browser.get(@sut_url)
    self.login_as_admin(browser)
    
    Log.logger.info("Going to site status page")
    site_status_url = "/gardens-site-install-status-report?field_install_status_value_many_to_one=All&field_site_id_value=&title_op=contains&title=#{site_prefix}&uid_op=in&uid=&rid_op=or&rid=All"
    #Our initial status so we have something to compare against
    site_status = "Incomplete"
    polling_start_time = Time.now
    polling_end_time = polling_start_time + timeout
    # Loop until the site has been completed or our polling times out
    while (site_status != "Completed" && polling_end_time > Time.now) do
      begin
        browser.get(@sut_url + site_status_url)
        old_site_status = site_status
        site_status = browser.find_element(:xpath => "//td[contains(@class, 'views-field-field-install-status-value')]").text
        if old_site_status != site_status
          Log.logger.info("Site #{site_prefix}: status changed: #{old_site_status} -> #{site_status}")
        end
      rescue StandardError => e
        puts "Error while polling for site completion: #{e.message}"
        #TODO: can we replace this by logged_in?
        logged_in = browser.find_elements(:xpath => "//body[contains(@class, ' logged-in')]").size > 0
        if logged_in
          Log.logger.info("We're still logged in. good!")
        else
          Log.logger.warn("We seem to have been logged out for some weird reason... Logging in again!")
          self.login_as_admin(browser)
        end
      ensure
        sleep 5
      end
    end
    Log.logger.info("Site #{site_prefix} status: #{site_status}")
    if site_status == "Completed"
      new_gardens_site_url = browser.find_element(:xpath => "//td[@class='views-field views-field-field-domain-value']/a").text
      Log.logger.info("Site #{site_prefix} finally created after a while at #{new_gardens_site_url}")
      logout(browser)
      newurl = self.check_newsite_url(new_gardens_site_url)
      return newurl
    else
      logout(browser)
      raise "Waiting for site creation of #{site_prefix.inspect} didn't succeed. Last status before hitting the (#{timeout}s) timeout: #{site_status.inspect}"
    end

  end

  # Waits until the site creation progress bar goes away
  def wait_for_progress_bar(browser, site_prefix, timeout = 300)
    begin
      #<div id="updateprogress" class="progress"><div class="bar"><div class="filled" style="width: 6%; "></div></div><div class="percentage">6%</div><div class="message">Please wait.<br>&nbsp;</div></div>
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      Log.logger.info("Waiting for progress bar on #{site_prefix}.")
      wait.until { browser.find_element(:xpath => '//div[@id="updateprogress"]') }
      Log.logger.info("Progress bar detected for #{site_prefix}.")
      Log.logger.info("Waiting for '%' status indicator")
      wait.until { browser.find_element(:xpath => '//div[@id="updateprogress"]//div[@class="percentage"]') }
      Log.logger.info("Found the '%' status indicator for #{site_prefix}.")
      begin
        counter = 0
        while (!browser.find_elements(:xpath => '//div[@id="updateprogress"]').empty? && (counter+=1) < 40 ) 
          percentage = "n/a"
          begin
            percentage = browser.find_element(:xpath => '//div[@id="updateprogress"]//div[@class="percentage"]').text
          rescue Exception => e
            Log.logger.info("Exception while getting updateprogress percentage: #{e.message}")
          end
          Log.logger.info("Status bar for #{site_prefix} at: #{percentage.inspect}")
          if percentage == "n/a"
            if browser.find_element(:xpath => "//body").text.include?("An error was encountered while creating the site")
              raise "We got the 'An Error was ancountered' message while trying to create #{site_prefix.inspect}"
            end
          end
          sleep 5
        end
      rescue Exception => e
        Log.logger.info("Error while checking percentage for #{site_prefix}: #{e}")
      end
      Log.logger.info("Waiting for progress bar for #{site_prefix} to disappear")
      begin
        wait.until { browser.find_elements(:xpath => '//div[@id="updateprogress"]').empty? }
      rescue Exception => e
        Log.logger.warn("Waiting for the updateprogress bar to disappear didn't seem to go well: #{e.message}")
      end
      Log.logger.info("Progress bar for #{site_prefix} went away")
      Log.logger.info("Waiting until we're redirected to our new site on #{site_prefix}")
    rescue Exception => message
      raise("Something went wrong during site creation of #{site_prefix}: " + message + "\n" + message.backtrace.join("\n"))
    end
  end

  # Common method between creating site as a user or creating site from a registration code
  def finish_site_creation(browser, user, password, site_prefix, template, timeout = 300, template_selected = false)
    Log.logger.info("Finishing the site creation for #{site_prefix}.")
    # If selecting features then template will be selected first and then its features. And finally it comes for finish site creation from testcase.
    if (template_selected == false)
      self.template_selection(template, browser)
    else
      Log.logger.debug("Template for #{site_prefix} already selected")
    end
    # Submit our template choice, start website creation process
    create_site_btn = 'edit-submit'
    Log.logger.info("Waiting for the create site button for #{site_prefix}")
    wait = Selenium::WebDriver::Wait.new(:timeout => 30)
    btn = wait.until { browser.find_element(:xpath => create_site_btn) }
    Log.logger.info("Clicking the submit button for #{site_prefix}: #{create_site_btn}")
    btn.click
    if browser.find_element(:xpath => "//body").text.include?("We were unable to create the site for #{site_prefix}")
      browser.quit
      raise("Site creation for #{site_prefix} failed, got the 'We were unable to create the site' message :(")
    end

    # Cue Jeopardy music while waiting for loading bar
    Log.logger.info("Waiting for progress bar for #{site_prefix} to go away")
    begin
      self.wait_for_progress_bar(browser, site_prefix, timeout)
    rescue Exception => e
      #This can happen if we immediately run into the "site creation busy" screen.
      Log.logger.warn("Waiting for progress bar failed: #{e.message}.")
    end
      
    retries = 0
    #We will loop and either fill this with the URL of the site or it will stay nil and the creation process failed
    return_value = nil

    #We will loop and check for a number of things to figure out
    #if we've already been redirected to our shiny new page
    #We could do it in javascript, but this way we get better logs and it won't time out on us that easily
    loop do
      sleep 2.5 #So we leave some time in between checks
      begin
        retries+=1
        if retries > 60
          Log.logger.error("We didn't get our site (#{site_prefix}) within our timeout limits :(")
          #No point in continuing really
          break
        end
        #gather our current environments data
        begin
          current_location = browser.current_url
        rescue Exception => e
          Log.logger.warn("Got an exception while checking for current location: #{e.message}")
          current_location = ""
          sleep 5
        end
          
        #when the browser is in the middle of a redirect, this sometimes is a bit wonky
        begin
          current_body = browser.get_body_text()
        rescue Exception => e
          Log.logger.info("Problem while getting html body for #{site_prefix}from #{current_location.inspect}: #{e.message.inspect}, ignoring")
          current_body = ""
        end

        #start checking
        on_sites_url = current_location.include?("http://#{site_prefix}")
        openid_in_url = current_location.include?("openid")
        #space in front of 'front' because the error page has 'not-front'
        final_site_present = browser.find_elements(:xpath => "//body[contains(@class, ' front')]").size > 0
        Log.logger.info("Current URL: #{current_location} | on our site's domain -> #{on_sites_url} | openid in url -> #{openid_in_url} | final_site_present -> #{final_site_present}")

        #Error Message:
        #http://gardener.gsteamer.acquia-sites.com/site-not-found?site=derp.gsteamer.acquia-sites.com
        #Check back soon, derp.gsteamer.acquia-sites.com is undergoing maintenance
        #The site you requested derp.gsteamer.acquia-sites.com is undergoing brief maintenance.
        site_not_found_page = current_location.include?("/site-not-found")
        if site_not_found_page
          #We don't get redirected from here, so we have to click the link
          Log.logger.info("We are on the 'site not found' page for #{site_prefix}! Clicking on the 'try again' link.")
          browser.find_element(:xpath => "//a[text()='try again']").click
          next
        end

        #http://gardener.gsteamer.acquia-sites.com/site-creation-busy
        #Oops, this is taking longer than usual
        site_creation_busy = current_location.include?("/site-creation-busy")
        if site_creation_busy
          Log.logger.info("Temporary failure detected for #{site_prefix} (site creation busy, 'oooops'), waiting for site creation using special URL")
          #finish the loop
          break
        end

        errors_present = current_body.include?("choose another name") or current_body.include?("encountered error") or 
		       	 (browser.find_elements(:xpath =>"//div[@class='messages error']").size > 0)

        if errors_present
          if browser.find_elements(:xpath => "//div[@class='messages error']").size > 0
            error_text = browser.find_element(:xpath => "//div[@class='messages error']").text
            Log.logger.info("Exiting for #{site_prefix} because of error message: #{error_text.inspect}")
          else
            Log.logger.info("Exiting for #{site_prefix} because of error message")
          end
          #and we're out...
          break
        end


        if (on_sites_url && !openid_in_url && final_site_present && !errors_present)
          Log.logger.info("Yay, we seem to be on our site for #{site_prefix}!")
          #http://xqsqavrk.gsteamer.acquia-sites.com/node
          return_value = current_location.gsub("/node", "")
          break
        end
      rescue Exception => e
        Log.logger.info("Error while checking for site. Retrying. #{site_prefix}: #{e.message.inspect}\n#{e.backtrace.inspect}")
      end
    end #loop
  
    #Check if we failed or not. If we couldn't detect our site or landed somewhere weird, check internally for the site
    if return_value.nil?
      Log.logger.info("not on the right site for #{site_prefix} yet -> running wait_for_site_creation()")
      return self.wait_for_site_creation(browser, user, password, site_prefix, timeout = 600)
    else
      Log.logger.info("New site created for #{site_prefix}! Returning #{return_value.inspect}")
      return return_value
    end
  end

  def template_selection(template, browser = nil)
    browser = @browser unless browser
    #Select a template
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Selecting #{template}")
    wait.until { browser.find_element(:xpath => "//div[@class='site-template']") }.click
    browser.find_element(:xpath => "//div[@class='site-template' and h2='#{template}']").click
    Log.logger.info("Selected #{template}")
    sleep 2
  end

  # It makes a list of all the features present for the particular template and their values selected by user.
  # It also calls Change_feature_value method if we are selecting features randomly. Otherwise it wouldn't.

  def make_list_of_features_and_values(random = false)
    Log.logger.info("Reading the current features displayed and their respective values")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    features_list_values = Hash.new
    features_types = ['Features', 'Pages and blocks']
    features_types.each {|ftype|
      feature_types_path = feature_types_path_selector(ftype)
      count = @browser.find_elements(:xpath => feature_types_path).size
      while i < (count + 1)
        feature_name_path = feature_name_path_selector(ftype, i)
        if (@browser.find_elements("//legend[contains(text(), '#{ftype}')]/following-sibling::*[#{i}][@style = 'display: none;']").size > 0)
          ft_name = @browser.find_element(:xpath => feature_name_path).text
          Log.logger.info("This feature '#{ft_name}' is hidden for template")
        else
          feature = wait.until { @browser.find_element(:xpath => feature_name_path) }.text
          feature_value_path = feature_value_path_selector(feature)
          ft_value = wait.until { @browser.find_element(:xpath => feature_value_path) }.text
          if (random == true)
            new_value = self.change_feature_value(feature, ft_value)
            ft_value = new_value
          end
        end
        i += 1
        if (ftype == features_types[0])
          features_list_values[feature] = Hash.new() unless (features_list_values[feature])
        end
        features_list_values[feature] = ft_value
      end
    }
    #if (random)
    # NOTE - TODO: # This if statement is commented due to the bugs AN-21165 and AN-21166
    sorted_features_values = self.sort_features_dependency(features_list_values)
    features_list_values = sorted_features_values
    #end
    return features_list_values
  end

  # This method changes the features values and verifies that feaure value is changed.

  def change_feature_value(feature, value)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    ft_new_value = self.generate_feature_value_randomly
    if (ft_new_value == value)
      Log.logger.debug("#{feature} already turned #{value}.")
    else
      Log.logger.debug("Changing the #{feature} value to #{ft_new_value}. This feature is currently #{value}.")
      feature_value_path = feature_value_path_selector(feature)
      wait.until { @browser.find_element(:xpath => feature_value_path) }.click
      feature_value_path = feature_value_path_selector(feature)
      ft_value = wait.until { @browser.find_element(:xpath => feature_value_path) }.text
      if(ft_new_value != ft_value)
        Log.logger.debug("#{feature} value failed to change from #{value} to #{ft_new_value}.")
      end
    end
    return ft_new_value
  end

  # Sorts out dependencies of the various modules on each other.

  def sort_features_dependency(features_list_values)
    Log.logger.info("Sorting features values on their dependencies.")
    if (features_list_values['Stay notified'] == "on" || features_list_values['Forums'] == "on")
      features_list_values['Comments'] = "on"
    end
    if (features_list_values['Get feedback'] == "on")
      features_list_values['Contact us'] = "on"
    end
    # TODO: # These two lines were added due to the bugs AN-21165 and AN-21166
    features_list_values['Comments'] = "on"
    features_list_values['Rotating banner'] = "on"
    # Once the issues will be resolved, these above mentioned dependencies will be erased.
    return features_list_values
  end

  # It generates the random number between 0 and 1. And sets the feature value to on and off accordingly.

  def generate_feature_value_randomly
    b = rand(2)
    if (b == 0)
      return "on"
    else
      return "off"
    end
  end

  def feature_name_path_selector(ftype, i)
    return "//legend[contains(text(), '#{ftype}')]/following-sibling::*[#{i}]//span[contains(@class, 'label-text')]"
  end

  def feature_value_path_selector(feature)
    return "//span[contains(text(), '#{feature}')]/following-sibling::*/div[contains(@class, 'value')]"
  end

  def feature_types_path_selector(ftype)
    return "//legend[contains(text(), '#{ftype}')]/following-sibling::*"
  end

  # Method to add "http://" to the url we get by logging in as admin
  def check_newsite_url(newsite)
    Log.logger.info("Adding 'http://' to the #{newsite} if not present.")
    check_url = newsite["http://"]
    if (check_url == "http://")
      return newsite
    else
      newurl = "http://" + newsite
      return newurl
    end
  end

  def login_as_admin(browser = @browser)
    login($config['user_accounts']['gardener_admin']['user'], $config['user_accounts']['gardener_admin']['password'], browser)
  end
  
  def login_as_config_user(user = 'qatestuser', browser = @browser)
    Log.logger.warn("Please remove this call to 'login_as_config_user()', this should be replaced by a regular login() call")
    Log.logger.info("Logging in as #{user}")
    login($config['user_accounts']['qatestuser']['user'], $config['user_accounts']['qatestuser']['password'], browser)
  end

  def get_node_by_domain(domain)
    Log.logger.info("Getting node for #{domain}")
    return gardener_xmlrpc.call('acquia.gardens.get_node_by_domain', domain)
  end

  def get_user_by_name(name)
    Log.logger.info("Getting user for login: #{name}")
    return gardener_xmlrpc.call('acquia.gardens.get_user_by_name', name)
  end

  def get_node_by_nid(nid)
    Log.logger.info("Getting node by nid #{nid}")
    return gardener_xmlrpc.call('acquia.gardens.get_node_by_nid', nid)
  end

  # Method to click on More link in my sites page and then to click duplicate site and enter the name of new site being duplicated.
  # Takes as argument current sitename and new site name after duplication. Returns error message if any, otherwise returns nil.

  def duplicate_site(sitename, dup_sitename, gardens_domain = self.get_gardens_domain, browser = @browser)
    Log.logger.info("Beginning to make duplicate gardens site of site #{sitename}")
    wait = Selenium::WebDriver::Wait.new(:timeout => 60)
    siteurl = sitename + '.' + gardens_domain
    wait.until { browser.find_element(:xpath => self.more_link(siteurl)) }.click
    wait.until { browser.find_element(:xpath => self.duplicate_site_link(siteurl)) }.click
    temp = wait.until { browser.find_element(:xpath => "//input[@id='edit-site-prefix']") }
    temp.clear
    temp.send_keys(dup_sitename)
    browser.find_element(:xpath => "//button[@id='edit-submit']").click
    if(browser.find_elements(:xpath => "//div[contains(@class, 'messages error')]").size > 0)
      error_msg = browser.find_element(:xpath => "//div[contains(@class, 'messages error')]").text
    else
      error_msg = nil
    end
    return error_msg
  end
  
  # Changes the subscription plan on any site, considering that site is recently created.
  #NOTE TODO: Checks on the first page of this url '/admin/content/node/overview', if the site is present or not. Wouldn't check for further pages.
  
  def change_subscription_plan(sitename, plan_name, user, password, browser = @browser)
    Log.logger.info("Changing site's '#{sitename}' subscription plan to '#{plan_name}'.")
    wait = Selenium::WebDriver::Wait.new(:timeout => 60)
    browser.get(@sut_url)
    login_as_admin(browser)
    browser.get("#{@sut_url}/admin/content/node/overview")
    wait.until { browser.find_element(:xpath => "//a[text() = '#{sitename}']/../following-sibling::*/a[text()='edit']") }.click
    flag = false
    wait.until { browser.find_element(:xpath => "//select[@id='edit-field-subscription-product-nid-nid']") }.find_elements(:xpath => '//option').each { |e|
      next unless e.text == plan_name ; flag = true ; e.click ; break ; 
    }
    Log.logger.info("SELECT FAILED") unless flag
    if (browser.find_elements(:id => 'edit-field-db-cluster-id-0-value').size > 0)  # This is the bug, needs to be removed from gardener. DB Cluster ID.
      temp = browser.find_element(:id => 'edit-field-db-cluster-id-0-value')
      temp.clear
      temp.send_keys("123")
    end
    wait.until { browser.find_element(:id => 'edit-submit') }.click
    Log.logger.info("Changed the site #{sitename} subscription plan to #{plan_name}.")
    wait.until { browser.find_element(:xpath => "//div[contains(@class, 'messages status')]") }
    login(user, password, browser)
    browser.get(@sut_url)
  end

  # A link to More link for any sitename
  def more_link(sitename)
    return "//a[text()='#{sitename}']/../following-sibling::td//a[text()='More']"
  end

  # A link to Duplicate Site link for any sitename
  def duplicate_site_link(sitename)
    return "//a[text()='#{sitename}']/../following-sibling::td//a[text()='Duplicate site']"
  end

  def enable_mollom_testing_mode(browser=@browser)
    testing_mode_checkbox = "//input[@id = 'edit-mollom-testing-mode']"
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    login($config['user_accounts']['gardener_admin']['user'], $config['user_accounts']['gardener_admin']['password'], browser)
    Log.logger.info("Setting Gardener Mollom module to testing mode")
    browser.get("#{@sut_url}/admin/settings/mollom/settings")
    chk_box = wait.until { browser.find_element(:xpath => testing_mode_checkbox) }
    if chk_box.selected?
      Log.logger.debug("Mollom testing mode already selected!") 
    else 
      chk_box.click
    end
    Log.logger.info("Mollom testing mode enabled!")
    browser.find_element(:xpath => "//input[@id='edit-submit']").click
    logout(browser)
  end
  
  def disable_mollom_testing_mode(browser=@browser)
    testing_mode_checkbox = "//input[@id = 'edit-mollom-testing-mode']"
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    login($config['user_accounts']['gardener_admin']['user'], $config['user_accounts']['gardener_admin']['password'], browser)
    Log.logger.info("Unsetting Gardener Mollom module from testing mode")
    browser.get("#{@sut_url}/admin/settings/mollom/settings")
    chk_box = wait.until { browser.find_element(:xpath => testing_mode_checkbox) }
    if not chk_box.selected?
      Log.logger.debug("Mollom testing mode already deselected!") 
    else
      chk_box.click
    end
    browser.find_element(:xpath => "//input[@id='edit-submit']").click
    Log.logger.info("Mollom testing mode disabled!")
    logout(browser)
  end

  def sign_up_newuser(browser,user_cred, captcha_answer = "correct", site_name = generate_random_string(12))
    browser.get("#{@sut_url}/create-site")
    self.input_ready?(browser)
    temp = browser.find_element(:id => 'edit-site-prefix')
    temp.clear
    temp.send_keys(site_name)
    temp = browser.find_element(:id => 'edit-name')
    temp.clear
    temp.send_keys(user_cred['login'])
    temp = browser.find_element(:id => 'edit-pass')
    temp.clear
    temp.send_keys(user_cred['password'])
    temp = browser.find_element(:id => 'edit-mail')
    temp.clear
    temp.send_keys(user_cred['email'])
    temp = browser.find_element(:id => 'edit-mollom-captcha')
    temp.clear
    temp.send_keys(captcha_answer)
  end
  
  def input_ready?(browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    elements = [ "//input[@id = 'edit-site-prefix']", "//input[@id = 'edit-name']", "//input[@id = 'edit-pass']", "//input[@id = 'edit-mail']","//input[@id = 'edit-mollom-captcha']" ]
    wait.until { elements.reject { |elem| !browser.find_elements(:xpath => elem).empty? }.empty? }
    Log.logger.info("Input fields are ready")
  end

  def generate_random_string(length=8)
    string = ""
    chars = ("a".."z").to_a
    length.times do
      string << chars[rand(chars.length-1)]
    end
    string
  end
end

# Used when running tests locally, does not perform actions which require a functional hosting environment.
class LocalGardener < Gardener
  # This is sorta a lie, since we create it as an admin, but it makes the implementation less cluttered.
  def make_gardens_site_as_user(browser, user, password, site_prefix, template = 'Campaign template', timeout=300)
    browser = @browser unless browser
    Log.logger.info("Logging in as admin and creating new site #{site_prefix}")
    browser.get(@sut_url)
    login_as_admin(browser)
    Log.logger.info("Beginning to make new gardens site")
    browser.get("#{@sut_url}/node/add/site")
    wait = Selenium::WebDriver::Wait.new(:timeout => 30)
    # Put the author as the user we are creating it for
    temp = wait.until { browser.find_element(:xpath => "//input[@id='edit-name']") }
    temp.clear
    temp.send_keys(user)
    # Add the site "name" to the site
    temp = browser.find_element(:xpath => "//input[@id='edit-field-site-id-0-value']")
    temp.clear
    temp.send_keys(site_prefix)

    # Add other fields we need
    temp = browser.find_element(:xpath => "//input[@id='edit-title']")
    temp.clear
    temp.send_keys(site_prefix)
    temp = browser.find_element(:xpath => "//input[@id='edit-field-site-id-0-value']")
    temp.clear
    temp.send_keys(site_prefix)
    temp = browser.find_element(:xpath => "//input[@id='edit-field-url-0-value']")
    temp.clear
    temp.send_keys('https://' + site_prefix + '.' + get_gardens_domain)
    flag = false
    browser.find_element(:id => 'edit-field-install-status-value').find_elements(:xpath => '//option').each {|e| 
      next unless e.text.downcase.include?('completed'); flag = true ; e.click; break ; }
    Log.logger.info("Select FAILED") unless flag
    flag = false
    browser.find_element(:id => 'edit-field-template-nid-nid').find_elements(:xpath => '//option').each {|e| 
      next unless e.text == template; flag = true ; e.click; break ; }
    Log.logger.info("Select FAILED for #{template}") unless flag
    flag = false
    temp = browser.find_element(:xpath => "//input[@id='edit-field-domain-0-value']")
    temp.clear
    temp.send_keys(site_prefix + '.' + get_gardens_domain)
    browser.find_element(:id => 'edit-field-subscription-product-nid-nid').find_elements(:xpath => '//option').each {|e| 
      next unless e.text.downcase.include?('starter'); flag = true ; e.click; break ; }
    Log.logger.info("Select FAILED") unless flag
    flag = false
    temp = browser.find_element(:xpath => "//input[@id='edit-field-subscription-plan-0-value']")
    temp.clear
    temp.send_keys('DGFree-0')
    # The answer to the life, universe and everything.
    temp = browser.find_element(:xpath => "//input[@id='edit-field-db-cluster-id-0-value']")
    temp.clear
    temp.send_keys("42")
    browser.find_element(:xpath => "//input[@id='edit-submit' and @value='Save']").click

    unless (browser.find_element(:xpath => "//body").text.match(/Site #{site_prefix} has been created/i))
      raise "Local site creation failed"
    end
    return get_node_by_domain(site_prefix + '.' + get_gardens_domain)
  end
end
