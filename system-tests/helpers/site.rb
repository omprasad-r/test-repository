$LOAD_PATH << File.dirname(__FILE__)
require 'rubygems'
require 'timeout'
require 'selenium/webdriver'
require 'selenium/client'
require 'acquia_qa/log'
require 'selenium_info.rb'


$selenium =  ENV['SELENIUM_SERVER'] || "localhost"
$webdriver = ENV['SELENIUM_DRIVER'] || "firefox"

module WebDriverSite

  def open_and_close_driver(site, sel_info = nil, &blk)
    WebDriverSite.open_and_close_driver(site, sel_info, &blk)
  end

  def self.open_and_close_driver(site, sel_info = nil, &blk)
    current_browser = nil
    thrown_exception = nil
    begin
      current_browser = driver(site, sel_info, true)
      yield(current_browser)
    rescue Exception => e
      thrown_exception = e
    ensure
      if current_browser
        begin
          current_browser.quit
        rescue Exception => e
          Log.logger.info("Got an exception while trying to close browser: #{e.message}\n#{e.backtrace}")
        end
      else
        Log.logger.warn("Trying to close browser but can't find it :(")
      end
      if thrown_exception
        puts "Got an exception, reraising it: #{thrown_exception.inspect}"
        raise thrown_exception 
      end
    end
  end
  
  def set_sut_url(site=nil)
    @sut_url = WebDriverSite.set_sut_url(site)
  end

  def self.set_sut_url(site)
    if site.nil?
      if $config['sut_url'].nil?
        raise "NO SUT_URL!"
      else
        site = $config['sut_url']
      end
    end
    site
  end

  #The force paramter will just return a new session, it will not care about running sessions, it will not close itself,
  #it will just return a new session. be aware!
  def driver(site, sel_info = nil, force = false, time = 60)
    WebDriverSite.driver(site,sel_info,force,time)
  end

  def self.construct_remote_url(options)
    case options.setting_group
    when "sauce"
      url = "http://#{options.username}:#{options.access_key}@#{options.host}:#{options.port}/wd/hub"
      return url
    when "sauceconnect"
      url = "http://#{options.host}:#{options.port}/wd/hub"
    else
      Log.logger.info("UNKNOWN SELENIUM SETTING GROUP! RAISING AN ERROR!")
      raise "CANNOT CONNECT TO UNKNOWN SAUCE CONFIG!"
    end
  end
  
  def self.driver(site, sel_info = nil, force = false, time = 10)
    Log.logger.debug("browser method in the acquia_qa got called, $browser defined: #{!$browser.nil?}, @browser defined: #{!@browser.nil?}")
    raise "Got nil as the site parameter when trying to open a browser. Check your inputs!" if site.nil?
    @sut_url = set_sut_url(site)
    @verification_errors = []
    selenium = $webdriver
    timeout = time 
    if (sel_info)
      Log.logger.debug("sel_inf: #{sel_info.inspect}")
      selenium = sel_info.browser
      timeout = sel_info.timeout
    end
    
    #This keeps track of the maximum session lifetime
    start_session_launch = Time.now
    profile = Selenium::WebDriver::Firefox::Profile.new
    profile.assume_untrusted_certificate_issuer = false 
    profile.native_events = false
    unless sel_info && sel_info.capabilities
      new_browser = Selenium::WebDriver.for(selenium.to_sym, :profile => profile)
      new_browser.manage.timeouts.implicit_wait = timeout
    else
      ### TODO : MAKE THIS SMOOTHER AND NICER
      Log.logger.info("Configuring Sauce test run...")
      caps = Selenium::WebDriver::Remote::Capabilities.firefox ## TODO: offer support for other browsers (chrome, ie, htmlunit)
      caps[:version] = "7." # sel_info.version #"7."
      caps[:platform] = :XP #sel_info.platform.to_sym #:XP
      caps[:testname] =  "Acquia_QA_SauceConnect" #sel_info.testname # "Acquia_QA_SauceConnect"
      caps[:username] = "acquia" # sel_info.username # "acquia"
      caps[:access_key] = "8a6674ed-b990-4ec4-92e1-5dba249b7a05" #sel_info.access_key # "8a6674ed-b990-4ec4-92e1-5dba249b7a05"
     # temp_options = SauceWebDriverInfo.new(:host => "localhost" , :port => "4445" ,:setting_group => "sauceconnect")
      #     server_url = WebDriverSite.construct_remote_url(temp_options)
      caps[:firefox_profile] = profile
      server_url = "http://localhost:4445/wd/hub"

      new_browser = Selenium::WebDriver.for(
                                           :remote,
                                           :url => server_url,
                                           :desired_capabilities => caps)
      new_browser.manage.timeouts.implicit_wait = timeout
    end
    retried_already = false
    begin
      #TODO: Maybe add this for firefox browsers?: ('commandLineFlags' => '--disable-web-security')
      Timeout::timeout(90) do
        new_browser.navigate.to site
      end
    rescue Exception => e
      if retried_already
        #could happen if an external JS/... doesn't load 100%
        raise "Wasn't able to open '/' on #{site.inspect} with our newly created browser session even after a ratry: #{e.message}."
      else
        sleep 10
        retried_already = true
        retry
      end
    end
    #return our current browser
    if force
      Log.logger.debug("Developers: This session was forced, this means that you have to clean it up manually")
    else
      #set the browser to be available via @browser so we don't have to pass it arround a lot for regular tests
      @browser = new_browser
    end
    new_browser
  end

  def close
    @browser.quit unless $browser
    if @verification_errors.length > 0
      raise "Verfications errors " + @verification_errors
    end
  end

  def self.logged_in?(browser = @browser)
    #as oposed to 'not-logged-in'
    browser.find_elements(:xpath => '//body[contains(@class, " logged-in")]').size > 0 
  end
  
  def logged_in?(browser = @browser)
    WebDriverSite.logged_in?(browser)
  end
  
  def login(user, password, browser = @browser)
    Log.logger.info("Trying to log in as user: #{user}.")
    where_we_came_from = browser.current_url.to_s
    if logged_in?(browser)
      # we'll first try to log out using the /user/logout method
      logoutd7(browser)
    end
    
    if logged_in?(browser)
      logout(browser)
    end
    
    #Try to detect weather or not the site is a gardens site that needs the gardener login
    Log.logger.info("Trying to detect if #{where_we_came_from} is a gardens server...")
    gardens_servers = "gsteamer"
    the_gardener_itself = "gardener."
    we_are_on_a_gardens_server = where_we_came_from.include?(gardens_servers)
    we_are_on_the_gardner = where_we_came_from.include?(the_gardener_itself)
    
    if (we_are_on_a_gardens_server and !we_are_on_the_gardner)
      Log.logger.debug("Detected a gardens site running on gsteamer, using the '/gardener/login' path")
      user_login_url = '/gardener/login'
    else
      Log.logger.debug("Detected a regular drupal site (gardens server: #{we_are_on_a_gardens_server.inspect} | gardener: #{we_are_on_the_gardner.inspect})")
      user_login_url = "/user"
    end
    
    browser.get($config['sut_url'] + user_login_url)
    JQuery.wait_for_events_to_finish(browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 10)
    pass_items = wait.until { browser.find_elements(:xpath => "//input[@id='edit-pass']") }

    if pass_items.size > 0
      Log.logger.debug("Found password field, we seem to be on the right page.")
    else
      Log.logger.warn("No password field found! Logging out again")
      logoutd7(browser)
      logout(browser)
      Log.logger.warn("Going to the login URL again")
      browser.get(@sut_url + user_login_url)
      JQuery.wait_for_events_to_finish(browser) 
    end
    Log.logger.debug("Entering username (#{user})")
    temp = wait.until { browser.find_element(:xpath => "//input[@id='edit-name']") }
    temp.clear
    temp.send_keys(user)
    Log.logger.debug("Entering password")
    temp = browser.find_element(:xpath => "//input[@id='edit-pass']")
    temp.clear
    temp.send_keys(password)
    Log.logger.debug("Submiting")
    browser.find_element(:id => "edit-submit").click # used to be click_at elem, '1,1'
    Log.logger.debug("Waiting for new page to load")
    #This one doesn't seem to work properly in IE9
    longwait = Selenium::WebDriver::Wait.new(:timeout => 20)
    begin
      #30 second timeout, especially when loading the page for the first time and no cache/... is there it takes a while
      longwait.until { browser.find_elements(:xpath => "//body[contains(@class, ' logged-in')]").size > 0 }
    rescue Exception => e
      Log.logger.info("Waiting for the new page timed out.")
    end
    login_successful = browser.find_elements(:xpath => "//body[contains(@class, ' logged-in')]").size > 0
    if login_successful
      Log.logger.debug("Login successful!")
    else
      raise("Login as user #{user.inspect} was NOT successful!")
    end
    unless where_we_came_from.include?('/logout') || where_we_came_from.include?('selenium-server/core/Blank.html') || where_we_came_from.include?('about:')
      Log.logger.debug("Going back to where we came from before the login (#{where_we_came_from.inspect})")
      browser.get(where_we_came_from)
    end
    login_successful
  end

  def logout(browser = @browser)
    browser.get("#{$config['sut_url']}/logout")
  end

  def logoutd7(browser = @browser)
    browser.get("#{$config['sut_url']}/user/logout")
  end
  
  def self.wait_for_site(site, options = {})
    time = options[:timeout] || 300
    timeout = Time.now + time
    print "Waiting for #{site}"
    while true
      print '.'
      if system("curl -qs -o /dev/null --head #{site}")
        puts 'found'
        return true
      end
      if (Time.now > timeout)
        return false
      end
      sleep 5
    end
  end
end

module Site

  # launch a browser session.  if sel_info is nil then use the defaults.
  # sel_info will need to have setters and getters that make sense as well
  # so that a default object give the same results.
  # TODO: move this whole pile to taek varagrs and 
  # if the 1st item is a hash just use the hash a'la
  #       def initialize(*args)
  #        if args[0].kind_of?(Hash)
  #          options = args[0]
  #          @host = options[:host]
  #          @port = options[:port].to_i
  #          @browser_string = options[:browser]
  #          @browser_url = options[:url]
  #          @default_timeout_in_seconds = (options[:timeout_in_seconds] || 300).to_i
  #          @default_javascript_framework = options[:javascript_framework] || :prototype
  #          @highlight_located_element_by_default = options[:highlight_located_element] || false
  #        else
  #          @host = args[0]
  #          @port = args[1].to_i
  #          @browser_string = args[2]
  #          @browser_url = args[3]
  #          @default_timeout_in_seconds = (args[4] || 300).to_i
  #          @default_javascript_framework = :prototype
  #          @highlight_located_element_by_default = false
  #        end
  #
  #        @extension_js = ""
  #        @session_id = nil
  #      end


  def open_and_close_browser(site, sel_info = nil, &blk)
    #we will just pass the block to the static method
    Site.open_and_close_browser(site, sel_info, &blk)
  end

  def self.open_and_close_browser(site, sel_info = nil, &blk)
    current_browser = nil
    thrown_exception = nil
    begin
      current_browser = browser(site, sel_info, true)
      yield(current_browser)
    rescue Exception => e
      thrown_exception = e
    ensure
      if current_browser
        begin
          current_browser.close_current_browser_session
        rescue Exception => e
          Log.logger.info("Got an exception while trying to close browser: #{e.message}\n#{e.backtrace}")
        end
      else
        Log.logger.warn("Trying to close browser but can't find it :(")
      end
      if thrown_exception
        puts "Got an exception, reraising it: #{thrown_exception.inspect}"
        raise thrown_exception 
      end
    end
  end

  #The force paramter will just return a new session, it will not care about running sessions, it will not close itself,
  #it will just return a new session. be aware!
  def browser(site, sel_info = nil, force = false, time = 60)
    @browser = Site.browser(site,sel_info,force,time)
  end
  
  def self.browser(site, sel_info = nil, force = false, time = 60)
    Log.logger.debug("browser method in the acquia_qa got called, $browser defined: #{!$browser.nil?}, @browser defined: #{!@browser.nil?}")
    raise "Got nil as the site parameter when trying to open a browser. Check your inputs!" if site.nil?
    @verification_errors = []
    selenium = $selenium
    port = 4444
    browser = "*firefox"
    timeout = time 
    if (sel_info)
      Log.logger.debug("sel_inf: #{sel_info.host}, #{sel_info.port}, #{sel_info.browser}, #{sel_info.timeout}")
      selenium = sel_info.host
      port = sel_info.port
      browser = sel_info.browser
      timeout = sel_info.timeout
    end
    
    #This keeps track of the maximum session lifetime
    start_session_launch = Time.now
    new_browser = Selenium::Client::Driver.new(selenium, port, browser, site, timeout)
 
       
    retried_already = false
    begin
      #TODO: Maybe add this for firefox browsers?: ('commandLineFlags' => '--disable-web-security')
      Timeout::timeout(90) do
        new_browser.start_new_browser_session
        #Visit main page so .getlocation doesn't go crazy
        new_browser.open("/")
      end
    rescue Exception => e
      if retried_already
        #could happen if an external JS/... doesn't load 100%
        raise "Wasn't able to open '/' on #{site.inspect} with our newly created browser session even after a ratry: #{e.message}."
      else
        sleep 10
        retried_already = true
        retry
      end
    end
    #return our current browser
    if force
      Log.logger.debug("Developers: This session was forced, this means that you have to clean it up manually")
    else
      #set the browser to be available via @browser so we don't have to pass it arround a lot for regular tests
      @browser = new_browser
    end
    return new_browser
  end

  def close
    @browser.stop unless $browser
    if @verification_errors.length > 0
      raise "Verfications errors " + @verification_errors
    end
  end

  def self.logged_in?(browser = @browser)
    #as oposed to 'not-logged-in'
    browser.element?('//body[contains(@class, " logged-in")]')   
  end
  
  def logged_in?(browser = @browser)
    Site.logged_in?(browser)
  end
  
  def login(user, password, browser = @browser)
    Log.logger.info("Trying to log in as user: #{user}.")
    
    where_we_came_from = browser.get_location
    
    if logged_in?(browser)
      # we'll first try to log out using the /user/logout method
      logoutd7(browser)
    end
    
    if logged_in?(browser)
      logout(browser)
    end
    
    #Try to detect weather or not the site is a gardens site that needs the gardener login
    gardens_servers = /\.gsteamer\.|\.gcurrent\./
    the_gardener_itself = "gardener."
    we_are_on_a_gardens_server = !where_we_came_from.match(gardens_servers).nil?
    we_are_on_the_gardner = where_we_came_from.include?(the_gardener_itself)
    
    if (we_are_on_a_gardens_server and !we_are_on_the_gardner)
      Log.logger.debug("Detected a gardens site running on gsteamer, using the '/gardener/login' path")
      user_login_url = '/gardener/login'
    else
      Log.logger.debug("Detected a regular drupal site (gardens server: #{we_are_on_a_gardens_server.inspect} | gardener: #{we_are_on_the_gardner.inspect})")
      user_login_url = "/user"
    end
    
    browser.open(user_login_url)

    if browser.element?("//input[@id='edit-pass']")
      Log.logger.debug("Found password field, we seem to be on the right page.")
    else
      Log.logger.warn("No password field found! Logging out again")
      logoutd7(browser)
      logout(browser)
      Log.logger.warn("Going to the login URL again")
      browser.open(user_login_url)
    end
    
    browser.wait_for_element("//input[@id='edit-pass']")
    Log.logger.debug("Entering username (#{user})")
    browser.type "//input[@id='edit-name']", user
    Log.logger.debug("Entering password")
    browser.type "//input[@id='edit-pass']", password
    Log.logger.debug("Submiting")
    #@browser.click "edit-submit", :wait_for => :page
    browser.click_at("edit-submit", "1,1")
    Log.logger.debug("Waiting for new page to load")
    #This one doesn't seem to work properly in IE9
    begin
      #30 second timeout, especially when loading the page for the first time and no cache/... is there it takes a while
      browser.wait_for_page_to_load "30000"
    rescue Exception => e
      Log.logger.info("Waiting for the new page timed out.")
    end
    login_successful = browser.element?("//body[contains(@class, ' logged-in')]")
    if login_successful
      Log.logger.debug("Login successful!")
    else
      raise("Login as user #{user.inspect} was NOT successful!")
    end
    unless where_we_came_from.include?('/logout') or where_we_came_from.include?('selenium-server/core/Blank.html')
      Log.logger.debug("Going back to where we came from before the login (#{where_we_came_from.inspect})")
      browser.open(where_we_came_from)
      browser.wait_for_page_to_load
    end
    login_successful
  end

  def logout(browser = @browser)
    browser.open("/logout")
  end

  def logoutd7(browser = @browser)
    browser.open("/user/logout")
  end
  
  def Site.wait_for_site(site, options = {})
    time = options[:timeout] || 300
    timeout = Time.now + time
    print "Waiting for #{site}"
    while true
      print '.'
      if system("curl -qs -o /dev/null --head #{site}")
        puts 'found'
        return true
      end
      if (Time.now > timeout)
        return false
      end
      sleep 5
    end
  end
end

module NetworkSite
  include Site
  
  def create_user(user_name,e_mail,password,full_name,current_site,title,org)
    @browser.open("users/" + user_name)
    if (@browser.text?("Page not found"))
      @browser.open("/admin/user/user/create")
      @browser.type("edit-name", user_name)
      @browser.type("edit-mail", e_mail)
      @browser.type("edit-pass-pass1", password)
      @browser.type("edit-pass-pass2", password)
      first_name, last_name =full_name.split(/ /,2)
      if(nil==last_name)
        last_name = "Lastname"
      end
      @browser.type("edit-profile-firstname", first_name)
      @browser.type("edit-profile-lastname", last_name)
      @browser.select("edit-profile-currentsite", 'label=' + current_site)
      @browser.type("edit-profile-title", title)
      @browser.type("edit-profile-organization", org)
      @browser.click("//input[@id='edit-submit' and @name='op' and @value='Create new account']",
        :wait_for => :page)
    else
      Log.logger.debug("User " + user_name + " appears to already exist");
    end
    
  end
 
  def create_network_user(user_name,e_mail,first_name, last_name, title, company, phone , job , country , state, industry)
    @browser.open("users/" + user_name)
    if (@browser.text?("Page not found"))
      @browser.open("/admin/user/user/create")
      @browser.type("edit-name", user_name)
      @browser.type("edit-mail", e_mail)
      @browser.type("edit-profile-firstname", first_name)
      @browser.type("edit-profile-lastname", last_name)
      @browser.type("edit-profile-title", title)
      @browser.type("edit-profile-organization", company)
      @browser.type("edit-profile-mobilephone", phone)
      @browser.select("edit-profile-job-function","value=" + job)
      @browser.select("edit-profile-country","value=" + country)      
      @browser.select("edit-profile-country-state","value=" + state) 
      @browser.select("edit-profile-industry","value=" + industry) 
      @browser.click("//input[@id='edit-submit' and @name='op' and @value='Create new account']",
        :wait_for => :page)
    else
      Log.logger.debug("User " + user_name + " appears to already exist");
    end
    
  end
  
  def delete_user(user_name)
    @browser.open("users/" + user_name)
    if (! @browser.text?("Page not found"))
      @browser.click("//a[contains(@href, 'user') and contains(@href, 'edit')]/span[text()='Edit']", :wait_for => :page)
      @browser.click("edit-delete", :wait_for => :page)
      @browser.click("edit-submit", :wait_for => :page)
    else
      Log.logger.debug(user_name + " not found not deleting user")
    end
  end

  def enable_fake_cc
    @browser.open("/admin/store/settings/payment/edit/gateways")
    @browser.uncheck("edit-uc-cybersource-soap-create-profile")
    @browser.click("//input[@id='edit-submit' and @name='op' and @value='Save configuration']",
      :wait_for => :page)
    if @browser.is_checked "edit-uc-cybersource-soap-create-profile"
      raise "Dang the box is still checked!"
    end
  end

  # this needs to be done as an API call and is infact broken at the moment
  def check_fake_cc
    return @browser.is_checked("edit-uc-cybersource-soap-create-profile")
  end
end

module DrupalSite
  include Site

  ADMIN_USER = 'admin'
  ADMIN_PASSWORD = 'ghetto#exits'
  
  def admin
    # Need to factor this out
    login(ADMIN_USER, ADMIN_PASSWORD)
  end

  def install_d7(db_user, db_password, db_name)
    @browser.open "install.php?profile=standard&locale=en"
    @browser.type "edit-mysql-database", db_name
    @browser.type "edit-mysql-username", db_user
    @browser.type "edit-mysql-password", db_password
    @browser.click "edit-save", :wait_for => :page
    
    install_admin_account
    home_page_works
  end
  
  def install(db_user, db_password, db_name)
    @browser.open "install.php?profile=acquia"
    @browser.type "edit-db-path", db_name
    @browser.type "edit-db-user", db_user
    @browser.type "edit-db-pass", db_password
    @browser.click "edit-save", :wait_for => :page
    
    install_admin_account
    home_page_works
  end

  def install_post_database
    @browser.open "install.php?profile=acquia"
    install_admin_account
    home_page_works
  end

  def install_admin_account
    @browser.wait_for_condition("selenium.isElementPresent(\"//h2[text()='Configure site']\")||selenium.isElementPresent(\"//h1[text()='Configure site']\")", 30000)
    @browser.type "edit-site-mail", "root@127.0.0.1"
    @browser.type "edit-account-mail", "root@127.0.0.1"
    @browser.type "edit-account-name", ADMIN_USER
    @browser.type "edit-account-pass-pass1", ADMIN_PASSWORD
    @browser.type "edit-account-pass-pass2", ADMIN_PASSWORD
    @browser.click "edit-submit", :wait_for => :page
  end

  def update(nest = 0)
    # hit update page
    @browser.open "update.php"

    # velocity site (site02) would fail w/wait_for_page_to_load, there must be some js on the page
    # that draws the continue button, failed at 30 secs, up-ing to 60
    @browser.wait_for_condition("selenium.isElementPresent(\"//input[@value='Continue']\")", 60000)

    # Backup warning
    @browser.click "xpath=//input[@value='Continue']"
    @browser.wait_for_page_to_load "30000"
    
    # update
    @browser.click "edit-submit"
    @browser.wait_for_page_to_load "30000"
    
    # wait for batch api to finish
    @browser.wait_for_condition("selenium.isElementPresent(\"//h2[text()='Drupal database update']\")", 30000)

    # assert no errors
    # Note, non-reentrant tests, failure will incorrectly succeed if run again
    # because failed updates are marked as complete by update.php
    if @browser.is_element_present("xpath=//*[@class='failure']")
      # ugh, apparently some modules fail the first time instructing the user
      # to run again.
      raise "Failures on update" if nest >= 1
      update(nest + 1)
    end

    home_page_works
  end

  def home_page_works
    # check for WSOD on home page
    @browser.open ""
    @browser.wait_for_page_to_load "30000"
    if ! @browser.is_element_present('xpath=//title')
      raise "Homepage did not render properly"
    end
  end

end
