require "rubygems"
require 'bundler/setup'
require "test/unit"
require "net/netrc"
require "selenium/client"
require "selenium"
require "test/unit"
require "acquia_qa/credentials"
require "acquia_qa/log"
require "acquia_qa/site"
require "acquia_qa/ssh"
require "acquia_qa/user_info"
require "acquia_qa/selenium_info"
require "acquia_qa/node/node_builder"
require "acquia_qa/theme_builder"
require 'acquia_qa/gardens_automation'


class Test10Performance < Test::Unit::TestCase
  include GardensAutomation::GardensSmokeTestCase
  
  attr_reader :loginmgr

  def setup
    $SUT_URL = @site_name = "http://perftest.gsteamer.acquia-sites.com"
    setup_base 
    @loginmgr = LoginManagerGM.new()
    @test_iterations = 11    # This is test count, we can change it to any number as per test requirement in the format: n+1
    @final_perf_results = Hash.new
  end
  
  def teardown
    @browser.close_current_browser_session
  end
  
  # This test calculate the time taken to perform various tasks on site. It contains two types of Run for test.
  # One is the Cold Run, which is the initial run without any cache and other is the warm run, which will be the
  # aggregate of other 10 runs, so that we have the actual results. This test starts with gsteamer site to check
  # whether that is running accurately, then switch to perftest site. It calculates the time taken for homepage load,
  # login overlay, logged in state, open configuration overlay, open account settings page, load wysiwyg editor,
  # start themebuilder, exit themebuilder and publish theme. This runs for both gsteamer site and prod site.
  # This test also enters their results in to perfresults created especially for logging of these test results. 
  def test_01_home_page_load_performance
    Log.logger.info("Starting test_01_home_page_load_performance test for : #{@site_name}")
    Log.logger.info("Loading Home Page for ITERATION NUMBER: 0")
    pageload_start_time = Time.now
    #@browser.remote_control_command('open', [@site_name, 'true'])
    pageload_end_time = Time.now
    action_time = self.calculate_time(pageload_end_time, pageload_start_time, action_type = "Home Page Load")
    @browser.close_current_browser_session
    @browser = browser(@site_name, @sel_info)
    @browser.open('/')
    @test_iterations.times do |count|
      @final_perf_results[count] = Hash.new() unless (@final_perf_results[count])
      if (count > 0)   # Home Page Load for Cold Run is already recorded, therefore skipping it for iteration 0
        Log.logger.info("Loading Home Page for ITERATION NUMBER: #{count}")
        @browser.open('/user')
        action_time = record_performance("Home Page Load") {@browser.remote_control_command('open', [@site_name, 'true'])}
      end
      @final_perf_results[count]["home_page_load"] = action_time
    end
    @browser.close_current_browser_session
    self.log_test_results(@final_perf_results, @site_name, site_action = "Home Page Load")
  end
    
  def test_02_open_login_overlay_performance
    Log.logger.info("Starting test_02_open_login_overlay_performance test for : #{@site_name}")
    @test_iterations.times do |count|
      @browser.open('/')
      @final_perf_results[count] = Hash.new() unless (@final_perf_results[count])
      Log.logger.info("Opening 'Log in or Register' overlay to login for ITERATION NUMBER: #{count}.")
      if (!@browser.element?("//a[text()='Log in or register']"))
        @browser.open('/user')
      end
      action_time = record_performance("Login Overlay") {self.open_login_overlay}
      self.close_overlay
      @final_perf_results[count]["login_overlay"] = action_time
    end
    @browser.close_current_browser_session
    self.log_test_results(@final_perf_results, @site_name, site_action = "Login Overlay")
  end
  
  def test_03_login_performance
    Log.logger.info("Starting test_03_login_performance test for : #{@site_name}")
    @test_iterations.times do |count|
      @final_perf_results[count] = Hash.new() unless (@final_perf_results[count])
      if (!@browser.element?("//a[text()='Log in or register']"))
        @browser.open('/user')
      end
      self.open_login_overlay
      self.enter_login_info(@site_name, @test_config.performance_test_user, @test_config.performance_test_password)
      Log.logger.info("Logging into site for ITERATION NUMBER.....: #{count}")
      action_time = record_performance("Logged IN") {self.login}
      @final_perf_results[count]["logged_in"] = action_time
      @browser.open('/user/logout')  # User logouts using logoutd7 method rather than general logout on both sites.
    end
    @browser.close_current_browser_session
    self.log_test_results(@final_perf_results, @site_name, site_action = "Logged IN")
  end
  
  def test_04_configuration_overlay_performance
    Log.logger.info("Starting test_04_configuration_overlay_performance test for : #{@site_name}")
    self.login_as_admin(@site_name)
    @test_iterations.times do |count|
      @final_perf_results[count] = Hash.new() unless (@final_perf_results[count])
      Log.logger.info("Clicking 'Configuration' link to open Account settings for ITERATION NUMBER: #{count}")      
      @browser.wait_for_element('toolbar-link-admin-config', :timeout_in_seconds => 30)
      action_time = record_performance("Configuration Overlay") {self.open_config_overlay}
      @browser.click('overlay-close')
      @final_perf_results[count]["configuration_overlay"] = action_time
    end
    @browser.close_current_browser_session
    self.log_test_results(@final_perf_results, @site_name, site_action = "Configuration Overlay")
  end
  
  def test_05_account_settings_overlay_performance
    Log.logger.info("Starting test_05_account_settings_overlay_performance test for : #{@site_name}")    
    self.login_as_admin(@site_name)
    @test_iterations.times do |count|
      @final_perf_results[count] = Hash.new() unless (@final_perf_results[count])
      @browser.open('/')
      Log.logger.info("Clicking 'Configuration' link to open Account settings for ITERATION NUMBER: #{count}.")      
      @browser.wait_for_element('toolbar-link-admin-config', :timeout_in_seconds => 30)
      self.open_config_overlay
      action_time = record_performance("Account Settings Overlay") {self.open_account_settings_overlay}
      @browser.select_frame "relative=up"
      @final_perf_results[count]["account_settings_overlay"] = action_time
    end
    @browser.close_current_browser_session
    self.log_test_results(@final_perf_results, @site_name, site_action = "Account Settings Overlay")
  end
  
  
  def test_06_wysiwyg_editor_performance
    Log.logger.info("Starting test_06_wysiwyg_editor_performance test for : #{@site_name}")
    self.login_as_admin(@site_name)
    @test_iterations.times do |count|
      @final_perf_results[count] = Hash.new() unless (@final_perf_results[count])
      #Editing blog
      @browser.open('/')
      Log.logger.info("Opening Blog page to check page load for WYSIWYG Editor for ITERATION NUMBER: #{count}.")      
      @browser.open('/content/boston-skyline-0')
      @browser.wait_for_element("//a[text() = 'Edit']", :timeout_in_seconds => 30)
      action_time = record_performance("WYSIWYG Editor") {self.load_wysiwyg_editor}
      @final_perf_results[count]["wysiwyg_editor"] = action_time
    end
    @browser.close_current_browser_session
    self.log_test_results(@final_perf_results, @site_name, site_action = "WYSIWYG Editor")
  end
  
  
  def test_07_start_themebuilder_performance
    Log.logger.info("Starting test_07_start_themebuilder_performance test for : #{@site_name}")
    self.login_as_admin(@site_name)
    themer = ThemeBuilder.new(@browser)          
    @test_iterations.times do |count|
      @final_perf_results[count] = Hash.new() unless (@final_perf_results[count])
      # Start Themebuilder
      begin
        Log.logger.info("Calculating the Theme Builder open time for ITERATION NUMBER: #{count}.")      
        action_time = record_performance("Start Themebuilder") {themer.start_theme_builder}
        @final_perf_results[count]["start_themebuilder"] = action_time
        Log.logger.info("Exiting Theme Builder to start it again.")      
        themer.exit_theme_builder
        assert(true) #test passed
      rescue Exception => message
        assert(false, "Basic start theme builder test failed. #{message}")
      ensure
        Log.logger.info("Exiting Theme Builder to start it again.")      
        themer.exit_theme_builder
      end
    end
    @browser.close_current_browser_session
    self.log_test_results(@final_perf_results, @site_name, site_action = "Start Themebuilder")
    
  end

  
  def test_08_publish_theme_performance
    Log.logger.info("Starting test_08_publish_theme_performance test for : #{@site_name}")
    self.login_as_admin(@site_name)
    themer = ThemeBuilder.new(@browser)          
    @test_iterations.times do |count|
      @final_perf_results[count] = Hash.new() unless (@final_perf_results[count])
      # Publish Theme
      begin
        Log.logger.info("Calculating the Theme Builder publish theme time for ITERATION NUMBER: #{count}.")      
        themer.start_theme_builder
        themer.open_theme('Gardens', 'campaign')
        themer.switch_tab('Styles')
        themer.click_in_tab('Font')
        themer.select_element('css=h1#site-name')
        font_size = rand(15) + 10
        Log.logger.info("Changing font size to #{font_size}")
        themer.type_text('font-size', font_size)
        action_time = record_performance("Publish Theme") {themer.publish_theme('test0')}
        @final_perf_results[count]["publish_theme"] = action_time
        themer.switch_tab('Themes')
        assert(true) #test passed
      rescue Exception => message
        assert(false, "Basic theme publish test failed. #{message}")
      ensure
        Log.logger.info("Exiting Theme Builder (will start it again on next iteration).")      
        themer.exit_theme_builder
      end
    end
    @browser.close_current_browser_session
    self.log_test_results(@final_perf_results, @site_name, site_action = "Publish Theme")
  end
  
  def test_09_exit_themebuilder_performance
    Log.logger.info("Starting test_09_exit_themebuilder_performance test for : #{@site_name}")
    self.login_as_admin(@site_name)
    themer = ThemeBuilder.new(@browser)          
    @test_iterations.times do |count|
      @final_perf_results[count] = Hash.new() unless (@final_perf_results[count])
      begin
        themer.start_theme_builder
        Log.logger.info("Calculating the Theme Builder exit time for ITERATION NUMBER: #{count}.")      
        action_time = record_performance("Exit Themebuilder") {themer.exit_theme_builder}
        @final_perf_results[count]["exit_themebuilder"] = action_time
      rescue Exception => message
        assert(false, "Basic start themebuilder test failed. #{message}")
      else
        assert(true)
      ensure
        Log.logger.info("Exiting Theme Builder to start it again.")      
        themer.exit_theme_builder
      end
    end
    @browser.close_current_browser_session
    self.log_test_results(@final_perf_results, @site_name, site_action = "Exit Themebuilder")
  end
  
  # Method to close any open overlay
  def close_overlay
    Log.logger.info("Closing current overlay.")
    @browser.select_frame @loginmgr.overlay_frame
    @browser.click('overlay-close')
    @browser.wait_for_no_element('overlay-close', :timeout_in_seconds => 60)
    @browser.select_frame "relative=up"
  end
  
  # Method to login as admin to the site
  def login_as_admin(sitename)
    Log.logger.info("Opening 'Log in or Register' overlay to login into site: #{sitename}")
    if (!@browser.element?("//a[text()='Log in or register']"))
      @browser.open('/user')
    end
    self.open_login_overlay
    Log.logger.info("Entering Logging In info for performance test user (#{@test_config.performance_test_user.inspect}).")
    self.enter_login_info(@site_name, @test_config.performance_test_user, @test_config.performance_test_password)
    Log.logger.info("Logging In as a performance test user (#{@test_config.performance_test_user.inspect}).")
    self.login
    @browser.open('/')
  end
  
  # Method to log test results to the results site on prod site
  def log_test_results(final_perf_results, sitename, site_action)
    results_site = "http://perfresults.drupalgardens.com"
    Log.logger.info("Entering performance test reults into the perfresults site: #{results_site}.")
    @browser = browser(results_site, @sel_info)
    @browser.open('/user/logout')  # User logouts using logoutd7 method rather than general logout on both sites.
    @browser.remote_control_command('open', [results_site, 'true'])
    self.login_as_admin(results_site)
    Log.logger.info("Final Performance Results = #{final_perf_results.sort.inspect}")
    run_types = ["Warm Run", "Cold Run"] # Cold Run is the initial run of test, having no cache and warm run are all other successive runs
    run_types.each do |run_type|
      sorted_result = self.sort_perf_test_results(final_perf_results, site_action, run_type)
      perf_result = Hash.new
      perf_result[site_action] = Hash.new() unless (perf_result[site_action])
      perf_result[site_action][:run_type] = run_type
      perf_result[site_action][:site_url] = sitename
      perf_result[site_action][:site_action] = site_action
      perf_result[site_action][:avg_time] = self.calculate_mean(sorted_result)
      perf_result[site_action][:min_time] = sorted_result.min
      perf_result[site_action][:max_time] = sorted_result.max
      perf_result[site_action][:sd] = self.calculate_std_deviation(sorted_result)
      self.enter_perf_test_results(perf_result, site_action)
    end
  end
  
  # Record performance statistics
  def record_performance(action_type)
    Log.logger.info("Recording performance time for #{action_type}.")
    start_time = Time.now
    yield if block_given?
    end_time = Time.now
    action_time = self.calculate_time(end_time, start_time, action_type)
    return action_time
  end
  
  # To open configuration overlay
  def open_config_overlay
    @browser.click('toolbar-link-admin-config')
    @browser.wait_for_element("//iframe[contains(@class, 'overlay-element overlay-active')]", :timeout_in_seconds => 30)
    @browser.select_frame "//iframe[contains(@class, 'overlay-element overlay-active')]"
    @browser.wait_for_element("//a[text() = 'Account settings']") # Just to make sure that its fully loaded.
  end
  
  # Open account settings overlay
  def open_account_settings_overlay 
    @browser.click("//a[text() = 'Account settings']")
    @browser.select_frame "relative=up"
    @browser.wait_for_element("//iframe[contains(@class, 'overlay-element overlay-active')]", :timeout_in_seconds => 30)
    @browser.select_frame "//iframe[contains(@class, 'overlay-element overlay-active')]"
    #@browser.wait_for_element("//fieldset[contains(@id, 'edit-anonymous-settings')]", :timeout_in_seconds => 60)  # Just to make sure account settings page is fully loaded.
  end
  
  # Load wysiwyg editor 
  def load_wysiwyg_editor
    @browser.click("//a[text() = 'Edit']")
    @browser.wait_for_element("//iframe[contains(@class, 'overlay-element overlay-active')]", :timeout_in_seconds => 30)
    @browser.select_frame "//iframe[contains(@class, 'overlay-element overlay-active')]"
    @browser.wait_for_element('edit-title', :timeout_in_seconds => 30)
  end
  
  # This method takes as arguments an array(hash) of results to be entered and the site action for which results are tracked.
  def enter_perf_test_results(test_results, site_action)
    date = Time.now.to_i
    title = "perfresult#{date}"
    site_url = test_results[site_action][:site_url]
    run_type = test_results[site_action][:run_type]
    avg_time = test_results[site_action][:avg_time]
    min_time = test_results[site_action][:min_time]
    max_time = test_results[site_action][:max_time]
    std_dev = test_results[site_action][:sd]
    #site_action = test_results[site_action][:site_action]  We are using that from argument value.
    @browser.open('/node/add/performance-test-result')
    @browser.wait_for_element('edit-title', :timeout_in_seconds => 30)
    @browser.type('edit-title', title)
    @browser.type('edit-field-testdate-und-0-value', date)
    @browser.type('edit-field-site-type-und-0-value', site_url)
    @browser.click("//label[contains(text(), '#{site_action}')]/preceding-sibling::input")
    @browser.click("//label[contains(text(), '#{run_type}')]/preceding-sibling::input")
    @browser.type('edit-field-avg-time-und-0-value', avg_time)
    @browser.type('edit-field-min-und-0-value', min_time)
    @browser.type('edit-field-max-und-0-value', max_time)
    @browser.type('edit-field-sd-und-0-value', std_dev)
    @browser.click('edit-submit', :wait_for => :page)
  end
  
  # This method calculates the time taken to perform the action, taking as arguments type of action, end time of that action
  # and start time of that action[all in Unix Epoch time]
  def calculate_time(end_time, start_time, action_type)
    action_time = end_time - start_time
    self.verify_performance_goal(action_time, action_type)
    return action_time
  end
  
  # Method to calculate standard deviation of numbers supplied as argument in the form of array.
  def calculate_std_deviation(vals)
    m = self.calculate_mean(vals)
    if vals.length <= 1
      std_dev = 0
    else
      variance = vals.inject(0) { |variance, x| variance += (x - m) ** 2 }
      std_dev = Math.sqrt(variance/(vals.size-1))
    end
    return std_dev
  end
  
  # Method to calculate mean of numbers supplied as argument in the form of array.
  def calculate_mean(vals)
    if (vals.size == 0)
      mean = 0
    else
      mean = vals.inject(0) { |sum, x| sum += x } / vals.size.to_f
    end
    return mean
  end
  
  # Method to sort the perf test results from the hash array of results between cold run and all others i.e. warm runs
  # Takes hash array of results, site action type[string b/w list of actions] and run type[string b/w Cold and Warm] as argument.
  def sort_perf_test_results(final_perf_results, site_action, run_type = "Cold Run")
    sorted_result = []
    h_keys = final_perf_results.keys
    hash_keys = []
    if (run_type == "Cold Run")
      hash_keys.push(h_keys[0])
    else
      h_keys.slice!(0)
      hash_keys = h_keys
    end
    s_action = self.translation_hash(site_action)
    hash_keys.each do |k|
      val = final_perf_results[k][s_action]
      sorted_result.push(val)
    end
    Log.logger.info("Sorted Results for #{site_action} = #{sorted_result.inspect}")
    return sorted_result
  end
  
  #Translate site action to something useful in hash map
  def translation_hash(s_action)
    site_action = s_action.downcase
    site_action.gsub!(/ /, '_')
    return site_action
  end
  
  # I just added this method to log the messages whether we are reaching erformance goals or not.
  # We can always skip it any time or change the goal time as needed.
  def verify_performance_goal(action_time, action_type)
    default_time =  2.0
    goal_times = {}  # Right now performance goal time is 2 for every action_type except below mentioned three in if statement. Change argument as per requirement.
    site_action = self.translation_hash(action_type)
    if (site_action == "exit_themebuilder" || site_action == "start_themebuilder" || site_action == "publish_theme")
      goal_times[site_action] = 8
    else
      goal_times[site_action] = default_time
    end
    goal_time = goal_times[site_action]
    Log.logger.info("Total action time taken for #{action_type} is #{action_time}. We set a maximum of #{goal_time} for this action.")
  end
  
  # This method is also defined in gardens.rb using it here without sleep and dividing it here to three methods rather than one original
  # NOTE: Be careful to use all these methoda in succession without any other click. Because we are not releasing iframe.
  # Method to open the login overlay only. 
  def open_login_overlay
    @browser.wait_for_element(@loginmgr.login_reg_link)
    @browser.click(@loginmgr.login_reg_link)
    @browser.wait_for_element(@loginmgr.overlay_frame, :timeout_in_seconds => 60)
    @browser.wait_for_element('overlay-close', :timeout_in_seconds => 60)
  end

  # Method to enter login information in the overlay and only.  
  def enter_login_info(current_page, login_info, password_info)
    @browser.select_frame @loginmgr.overlay_frame
    @browser.wait_for_element(@loginmgr.login_txtbox, :timeout_in_seconds => 60)
    @browser.type(@loginmgr.login_txtbox, login_info)
    @browser.type(@loginmgr.password_txtbox, password_info)
  end
  
  # Method to just click the login button and wait for page load.
  def login
    @browser.click(@loginmgr.login_btn, :wait_for => :page)
    @browser.select_frame "relative=top"
  end
  
  class LoginManagerGM
    attr_reader :login_reg_link, :overlay_frame
    attr_reader :password_txtbox, :login_txtbox, :login_btn
    def initialize()
      @login_reg_link = 'link=Log in or register'
      @overlay_frame = '//iframe[contains(@class, "overlay-active")]'
      @login_txtbox = '//input[@id="edit-name"]'
      @password_txtbox = '//input[@id="edit-pass"]'
      @login_btn = '//input[@id="edit-submit"]'
    end
  end
  
end
