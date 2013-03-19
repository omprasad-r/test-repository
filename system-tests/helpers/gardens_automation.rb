$LOAD_PATH << File.dirname(__FILE__)
require 'acquia_qa/log'
Log.logger.debug("Disabling monkeypatches of HTTP lib as defined in fields.rb.")
$do_not_monkeypatch_http_lib = true

require 'acquia_qa/os'
require 'acquia_qa/ssh'
require "selenium/client"
require "selenium/webdriver"
require 'capybara/mechanize'
#require 'capybara/poltergeist'
require 'net/https' #a try to fix this behavior http://stufftohelpyouout.blogspot.com/2010/05/how-to-fix-nethttpbadresponse-wrong.html
require "selenium_info"
require 'fields.rb'
require 'gardener'
require 'gardens'
require 'acquia_qa/common_util'
require "site.rb"
require 'qa_backdoor.rb'
require 'relative'
require 'tempfile'
require "singleton"
require 'headless'

include Acquia::Config
include OS
include Acquia::SSH


module XvfbSetup
  #This method will launch a virtual framebuffer for the webbrowser
  def setup_xvfb_if_necessary
    if RbConfig::CONFIG['host_os'].include?("darwin")
      Log.logger.info("detected OSX, no need for headless gem")
    elsif RbConfig::CONFIG['host_os'].include?("linux")
      Log.logger.info("detected Linux")
      if ENV['DISPLAY']
        Log.logger.info("The DISPLAY env variable is set (#{ENV['DISPLAY'].inspect}), not creating a new framebuffer ")
      else
        Log.logger.info("Didn't detect a running display, activating headless gem")
        require "headless"
        xvfb_d = Process.pid + rand(9701)
        Log.logger.info("Setting up an XVFB display on display number #{xvfb_d}")
        @headless = Headless.new(:display => xvfb_d, :reuse => false, :destroy_at_exit => true)
        @headless.start
        Log.logger.info("xvfb started.")
      end
    else
      #Windows and Solaris?
      Log.logger.warn("no known OS detected... that is a bit weird")
    end
  end
end

# This module contains functions used by rake tasks which fire off tests.
module GardensAutomation

  # Defines a custom rake task type which will run on the system under test (SUT)
  def remote_task(*args)
    block = proc {|t|
      ssh_long_session(GardensTestRun.instance.config.machine.dns_name) {|s|
        s.run "#{Rake::Util.env_opts} rake --trace --rakefile gardens-test.rake #{t.name}"
      }
    }
    Rake::Task.define_task(*args, &block)
  end

  # Short hand to get the GardensTestRun instance.
  def acquia_test_run
    return GardensTestRun.instance
  end

  # Short hand to get the configuration singleton for this run.
  def acquia_test_config
    return GardensTestRun.instance.config
  end

  def get_svn_paths(url)
    root_url = 'https://svn.acquia.com/repos/engineering'
    repo_uri = url.gsub(root_url,'')
    repo_uri.gsub!(/^\/+/, '')
    repo_uri.gsub!(/\/+$/, '')
    (project, branch) = repo_uri.split('/',2)
    return project, branch
  end

  # Used to provide configuration variables to the rake tasks.  It is instantiated
  # by GardensTestRun.  Note the method_missing method at the bottom which acts
  # as a passthru to make all instance properties accessible.  This is not ideal
  # but is a good start at keeping the variable scope sane.
  class GardensTestRunConfig
    def machine
      GardensTestRun.instance.it.ec2_local.require_running_by_label(self.machine_label)
    end

    # Return TRUE if this appears to be a production stage. Otherwise it is
    # a development or cloned stage.
    def is_production
      self.stage == 'prod' || self.stage == 'gardens'
    end

    def stage
      stage = nil
      if ENV['FIELDS_STAGE']
        stage = ENV['FIELDS_STAGE']
      else
        raise "You didn't set a FIELDS_STAGE environment. This is needed so we know what to point the test at."
      end
      stage
    end

    def launching_user
      ENV['USER']
    end

    def external_hostname
      "master.e.#{self.stage}.f"
    end

    def internal_hostname
      "master.i.#{self.stage}.f"
    end

    def sut_domain
      "#{self.stage}.#{GardensTestRun.instance.fields.sites_domain}"
    end
    # use environment variables to override certain defaults
    def availability_zone
      ENV['FIELDS_AVAILABILITY_ZONE'] || @default_availability_zone
    end

    def machine_label
      self.external_hostname
    end

    def selenium_setting_group
      return ENV['SELENIUM_SETTING_GROUP'] || 'suace'
    end

    def selenium
      settings = {:host => nil, :port => nil, :timeout => nil, :browser_type => nil, :extra => nil}
      return settings.merge(self.selenium_config[self.selenium_setting_group])
    end

    def use_local_gardener
      result_local_gardener = false
      if ENV.key?('GARDENER_LOCAL')
        result_local_gardener = ENV['GARDENER_LOCAL']
      else
        if self.methods.include?('local_gardener')
          result_local_gardener = self.local_gardener
        end
      end
      result_local_gardener
    end
  end

  # Singleton which houses instances of the It and Fields classes as well as the
  # configuration for the test.
  class GardensTestRun
    attr_accessor :it, :fields, :gardener, :config, :config_file, :remote_rakefile, :run_identifier
    include Singleton

    def setup(config_file, remote_rakefile = nil)
      base_config_file = File.expand_path_relative_to_caller('./base_config.yaml')

      @remote_rakefile = remote_rakefile
      @config_file = config_file
      # The base config file which all tests include. This will have to be in the same directory as the file you're currently editing

      if File.exist?(base_config_file)
        base_config = YAML::load_file(base_config_file)
      else
        raise "Didn't find base config file: (#{base_config_file}) (exists: #{File.exist?(base_config_file)})"
      end

      if File.exist?(config_file)
        custom_config = YAML::load_file(config_file)
      end


      @config = GardensTestRunConfig.new
      @config.read_inputs(base_config_file, config_file)

      #our new way of managing configurations
      $config = self.get_combined_yaml(base_config_file, config_file)

      # JS: This isn't a great pattern IMO.  I indented this setup to be just for gardens, not for gardener.
      # If we want one that is gardener specific, wouldn't it be better to abstract that out?
      # The gardens tests shouldn't require the gardener and visa-versa, right?
      #@gardener.read_inputs(@@base_config_file, config_file)
      #@gardener.set_environment_variables
      #

      if $config['use_local_gardener']
        Log.logger.debug("Using LocalGardener")
        @gardener = LocalGardener.new
      else
        @gardener = Gardener.new
      end

      #grab selenium setting group from env if not in yaml file
      selenium_setting = ENV['selenium_setting_group'] || ENV['SELENIUM_SETTING_GROUP']
      $config['selenium_setting_group'] = $config['selenium_setting_group'] || selenium_setting
      if $config['selenium_setting_group'].nil?
        #our default
        puts "Didn't find selenium_setting_group in ENV or YAML. Assuming 'sauce'"
        $config['selenium_setting_group'] = 'sauce'
      end
      Log.logger.debug("Yaml configs loaded into $config.")
      #We don't really need this extra output
      #Log.logger.debug($config.inspect)

      @it = IT.new
      @fields = Fields.new
    end

    #This method will combine our base_config.yaml file with the project specifc yaml file and merge them.
    #It will return a hash
    def get_combined_yaml(base_config, test_specific_config)

      #load the base_config.yaml file.
      base_config = YAML::load_file(base_config)
      unless base_config.kind_of?(Hash)
        raise("Base config (#{base_config.inspect}) doesn't seem to be a proper yaml file with contents")
      end

      #only load the custom test yaml if we actually need one.
      if File.exist?(test_specific_config)
        overriden_config = YAML::load_file(test_specific_config)
        if overriden_config.kind_of?(Hash)
          base_config.merge!(overriden_config)
        else
          puts("Overridden config (#{test_specific_config.inspect}) doesn't seem to be a proper yaml file with contents")
        end
      end

      return base_config
    end

    # assume the domain is nnn.com  and fdqn is aaa.bbb.nnn.com
    def set_dns(fdqn, ttl=60)
      ssh(@config.machine.dns_name){|s|
        d = AwsData.new
        d.session = s

        frags = fdqn.split('.')
        domainarr = []
        # com
        domainarr.unshift(frags.pop)
        # acquia
        domainarr.unshift(frags.pop)
        domain  = domainarr.join('.')
        subdomain = frags.join('.')
        puts "fdqn: #{fdqn}, domain: #{domain}, subdomain: #{subdomain}"
        puts "external IP: #{d.metadata('public-ipv4')}"
        dns_api = it.dns_provider
        Log.logger.debug("setting zone #{domain} and fqhn #{subdomain}.#{domain} to ip #{d.metadata('public-ipv4')}")
        dns_api.create_or_replace_a_record(domain, "#{subdomain}.#{domain}", d.metadata('public-ipv4'), ttl)
      }
    end
  end

  module ConfigurationSetup

    def reset_installation
      if $config['sut_root']
        Log.logger.info "We're NOT calling our QA reset script because we have sut_root set in $config."
      else
        Log.logger.info("Resetting Gardens installation to default snapshot")
        backdoor = QaBackdoor.new($config['sut_url'], { :logger => Log.logger })
        backdoor.restore_snapshot
      end
    end


    def check_availibilty(url, expected_status)
      begin
        return (Excon.head(url).status == expected_status)
      rescue Exception => e
        Log.logger.info("Couldn't determine feature: #{e.message}")
        return false
      end
    end

    def setup_configuration
      # @TODO: paramaterize this.
      config_file = ENV['CUSTOM_YAML_TEST_SET'] || "test_set.yaml"
      config_file = File.expand_path(config_file)
      if File.exist?(config_file)
        Log.logger.debug("Using configuration file at #{config_file} (File exists: #{File.exist?(config_file)})")
      else
        Log.logger.warn("Could be an error: The config file #{config_file} doesn't exist!")
      end
      GardensTestRun.instance.setup(config_file) #creates $config
      @fields = GardensTestRun.instance.fields
      @test_config = GardensTestRun.instance.config

      #If we have a SUT_URL we'll just take that one without further modification
      #If we have a SUT_HOST, we'll take it and prepend the current tests name in a before() statement
      if ENV['SUT_URL'] || ENV['SUT_ROOT']
        if ENV['SUT_URL']
          Log.logger.debug("Found SUT_UTL environment variable: #{ENV['SUT_URL']}")
          $config['sut_url'] = @sut_url = "#{ENV['SUT_URL'].chomp('/')}"
        elsif ENV['SUT_ROOT']
          Log.logger.debug("Found SUT_ROOT environment variable found: #{ENV['SUT_ROOT']}")
          if self.class.respond_to?(:description)
            $test_number = ($test_number.to_i + 1)
            current_site_name = "qatestsite#{"%02d" % $test_number}"
            $config['sut_url'] = @sut_url = "http://#{current_site_name}.#{ENV['SUT_ROOT'].chomp('/')}"
          else
            raise "You set SUT_ROOT but we didn't seem to run within the context of an rspec context."
          end
        end
        #This one we set regardless
        $config['sut_host'] = @sut_host = URI.parse($config['sut_url']).host
      else
        raise "Didn't find a URL to point the test at. You need to set the ENV['SUT_URL'] or ENV['SUT_ROOT']."
      end

      if ENV['SELENIUM_SETTING_GROUP']
        $config['selenium_setting_group'] = ENV['SELENIUM_SETTING_GROUP']
      elsif ENV['selenium_setting_group']
        $config['selenium_setting_group'] = ENV['selenium_setting_group']
      end

      # Explicitly request headless, or xvfb-run. Know which you are using, too.
      if ENV['XVFB_RUN']
        $config['xvfb_run'] = ENV['XVFB_RUN']
      end

      if ENV['CREATE_QATESTUSER']
        $config['create_qatestuser'] = ENV['CREATE_QATESTUSER']
      end

      $site_capabilities = {}
      $site_capabilities[:fast_user_switching] = check_availibilty("#{$config['sut_url']}/devel/switch", 302)
      $site_capabilities[:backdoor] = check_availibilty("#{$config['sut_url']}/qa_reset.php", 200)

      Log.logger.info("Site capabilites: #{$site_capabilities.inspect}")

      Log.logger.info("$config['sut_url'] is #{$config['sut_url'].inspect}.")
      Log.logger.info("$config['sut_host'] is #{$config['sut_host'].inspect}.")
      Log.logger.info("$config['sut_root'] is #{$config['sut_root'].inspect}.")
    end
  end


  module GardensTestCaseBase
    include ConfigurationSetup, Site, Acquia::Config, Acquia::SSH #, EnvironmentVariables
    def self.included(base)
      #don't use the fields.rb monkeypatches
      $do_not_monkeypatch_http_lib = true
      #try to fix saucelabs errors
      #require 'acquia_qa/saucelabs_http_fix'
    end

    # Selenium 1.0 setup method
    def setup_selenium(extra_fields = {})
      Log.logger.debug("Setting up a Selenium session for #{self.class.to_s} => #{self.method_name if self.methods.include?('method_name')}(). Initial options: url: #{@sut_url} | params: #{extra_fields.inspect}")

      if (['sauce', 'sauceconnect'].include?($config['selenium_setting_group']))
        Log.logger.debug("We will be running this test on Saucelabs! (#{$config['selenium_setting_group'].inspect})")
        #the initial default if we don't have anything set so far
        extra_fields["tags"] = ["not-set"] unless extra_fields.key?("tags")

        #let's check for the sauce_tags item in the config (parsed from the test_set_*.yaml file)
        begin
          if Array($config['sauce_tags']).empty?
            Log.logger.warn("Empty sauce_tags param found in yaml config, using 'not-set' default.")
          else
            #set the tags we found
            extra_fields["tags"] = $config['sauce_tags']
          end
        rescue StandardError => e
          Log.logger.warn("Error while reading saucelabs tags: #{e.message}")
        end

        # Set testname explicitly.  If not called via Test::Unit name is not defined
        extra_fields['name'] = self.name if self.methods.include?('name')
        # Set the name to 'unknown' if we don't have a name by now (either via options or via the testunit method called name
        extra_fields['name'] = 'unknown' unless extra_fields.key?('name')

        if ENV['BUILD_TAG']
          extra_fields['build'] = ENV['BUILD_TAG']
        end
        Log.logger.info("Extra fields for selenium: #{extra_fields.inspect}")

        current_setting_group = $config['selenium_setting_group']
        Log.logger.debug("Using host/port data from the detected selenium_setting_group (#{current_setting_group.inspect})")
        host = $config['selenium_config'][current_setting_group]['host']
        port = $config['selenium_config'][current_setting_group]['port']
        browser_type = $config['selenium_config'][current_setting_group]['browser_type']
        timeout = $config['selenium_config'][current_setting_group]['timeout']

        Log.logger.debug("We will try to use a sauceconnect tunnel on port #{port}.") if current_setting_group == 'sauceconnect'
        @sel_info = SauceSeleniumInfo.new(:host => host, :port => port, :browser => browser_type, :timeout => timeout, :extra => extra_fields)
      else
        current_setting_group = $config['selenium_setting_group']
        host = $config['selenium_config'][current_setting_group]['host']
        port = $config['selenium_config'][current_setting_group]['port']
        browser_type = $config['selenium_config'][current_setting_group]['browser_type']
        timeout = $config['selenium_config'][current_setting_group]['timeout']
        @sel_info = SeleniumInfo.new(:host => host, :port => port, :browser => browser_type, :timeout => timeout)
      end

      #Also gets assigned in the browser() method, but sooner or later we might want to switch that over
      @browser = browser(@sut_url, @sel_info)

      if (['sauce', 'sauceconnect'].include?($config['selenium_setting_group']))
        #Saucelabs will give us a session ID
        session_id = @browser.get_eval('selenium.sessionId')
        if session_id.nil? or session_id.to_s.downcase == "null"
          #We should be running on saucelabs, but don't get a session ID from the browser
          raise("Browser didn't return a Saucelabs session ID.")
        else
          #needed for saucelabs jenkins plugin
          #private static final Pattern SESSION_ID_PATTERN = Pattern.compile("SauceOnDemandSessionID=([0-9a-fA-F]+) job-name=(.*)");
          #private static final Pattern OLD_SESSION_ID_PATTERN = Pattern.compile("SauceOnDemandSessionID=([0-9a-fA-F]+)");
          job_name = nil
          if extra_fields['name']
            job_name = extra_fields['name']
          elsif self.methods.include?('method_name')
            job_name = self.method_name
          else
            job_name = "undefined (bug?!)"
          end

          text_the_plugin_looks_for = "SauceOnDemandSessionID=#{session_id} job-name=#{job_name}"
          Log.logger.info("Something for the Jenkins Plugin: #{text_the_plugin_looks_for}")
          puts text_the_plugin_looks_for
        end
      end

      @browser.open("/")
    end

    def use_local_gardener
      return ENV['GARDENER_LOCAL'] || self.local_gardener
    end
  end

  module WebDriverTestCase
    include ConfigurationSetup, WebDriverSite, XvfbSetup

    def self.included(base)
      base.instance_eval do
        before(:all) do
          setup_configuration
          setup_xvfb_if_necessary
          #this will create a new @browser object that points to @sut_url
          setup_webdriver
          # possible workaround for 'outside the bounds'problem
          @browser.manage.window.resize_to(1280, 1024)
        end

        before(:each) do
          reset_installation
          # We are having instances of webdriver being stuck on 'about:blank'
          if @browser
            begin
              # This value should be defined and correct at this point
              Log.logger.info("Pointing driver at #{$config['sut_url']} ( current url: #{@browser.current_url} )")
              @browser.get($config['sut_url'])
            end
          else
            raise "No @browser webdriver object before test run..."
          end
        end

        after(:each) do
          Log.logger.info("Resetting browser session ( current url: #{@browser.current_url} )")
          if @browser

            # taking screenshots
            if example.exception
              Log.logger.info("Screenshot: Caught a failing test: #{example.exception}")
              test_name = example.metadata[:full_description].gsub(/['"]/, "").gsub(/\s/, "_").downcase
              current_filename_prefix = "test_fail_#{test_name}_#{Time.now.to_i}"

              Log.logger.info("Saving artifacts with prefix: #{current_filename_prefix}")
              @browser.save_screenshot("#{current_filename_prefix}.png")
              html = @browser.page_source
              File.open("#{current_filename_prefix}.html", "w") { |f| f.write html }
            end

            begin
              @browser.manage.delete_all_cookies
            rescue Selenium::WebDriver::Error::UnhandledError
              # delete_all_cookies fails when we've previously gone
              # to about:blank, so we rescue this error and do nothing
              # instead.
            end
            @browser.navigate.to('about:blank')
            Log.logger.info("After wiping the browser, we have current url: #{@browser.current_url}")
          else
            raise "No @browser webdriver object after test run..."
          end
        end

        after(:all) do
          begin
            if @browser
              @browser.quit
              @browser = nil
            end
          rescue StandardError, Selenium::WebDriver::Error, Errno::ECONNREFUSED => e
            # Browser must have already gone
            Log.logger.debug("Error while trying to teardown @browser: #{e.message}")
          end
        end


      end #instance_eval
    end #def

    def use_local_gardener
      return ENV['GARDENER_LOCAL'] || self.local_gardener
    end

    ##
    # Selenium 2.0 setup method
    def setup_webdriver(extra_fields = {})
      Log.logger.debug("Setting up a WebDriver session for #{self.class.to_s} => #{self.method_name if self.methods.include?('method_name')}(). Initial options: url: #{@sut_url} | params: #{extra_fields.inspect}")
      if (['sauce', 'sauceconnect'].include?($config['selenium_setting_group']))
        Log.logger.debug("We will be running this test on Saucelabs! (#{$config['selenium_setting_group'].inspect})")
        #the initial default if we don't have anything set so far
        current_setting_group = $config['selenium_setting_group']
        extra_fields["secure"] = ($config['selenium_setting_group'] == "sauceconnect")
        extra_fields["tags"] = ["not-set"] unless extra_fields.key?("tags")
        extra_fields["version"] = $config['webdriver_config'][current_setting_group]['browser_type']['browser-version']
        extra_fields["platform"] = $config['webdriver_config'][current_setting_group]['browser_type']['os']
        extra_fields["username"] = $config['webdriver_config'][current_setting_group]['browser_type']['username']
        extra_fields["setting_group"] = current_setting_group
        extra_fields["browser"] = $config['webdriver_config'][current_setting_group]['browser_type']['browser']
        extra_fields["access_key"] = $config['webdriver_config'][current_setting_group]['browser_type']['access-key']
        #let's check for the sauce_tags item in the config (parsed from the test_set_*.yaml file)
        begin
          if Array($config['sauce_tags']).empty?
            Log.logger.warn("Empty sauce_tags param found in yaml config, using 'not-set' default.")
          else
            #set the tags we found
            extra_fields["tags"] = $config['sauce_tags']
          end
        rescue StandardError => e
          Log.logger.warn("Error while reading saucelabs tags: #{e.message}")
        end

        # Set testname explicitly.  If not called via Test::Unit name is not defined
        extra_fields['testname'] = "Acquia_QA_SauceConnect_#{Time.now.to_i}"

        if ENV['BUILD_TAG']
          extra_fields['build'] = ENV['BUILD_TAG']
        end
        Log.logger.info("Extra fields for selenium: #{extra_fields.inspect}")

        Log.logger.debug("Using host/port data from the detected selenium_setting_group (#{current_setting_group.inspect})")
        host = $config['webdriver_config'][current_setting_group]['host']
        port = $config['webdriver_config'][current_setting_group]['port']
        timeout = $config['webdriver_config'][current_setting_group]['timeout']

        rigged_browser = "firefox"

        Log.logger.debug("We will try to use a sauceconnect tunnel on port #{port}.") if current_setting_group == 'sauceconnect'
        @sel_info = SauceWebDriverInfo.new(:browser => rigged_browser, :host => host, :port => port, :timeout => timeout, :extra => extra_fields)
      else
        current_setting_group = $config['selenium_setting_group']
        browser_type = "firefox" # $config['webdriver_config'][current_setting_group]['browser_type']
        timeout = "3" # $config['webdriver_config'][current_setting_group]['timeout']
        @sel_info = WebDriverInfo.new(:browser => browser_type, :timeout => timeout)
      end

      #Also gets assigned in the browser() method, but sooner or later we might want to switch that over
      @browser = driver(@sut_url, @sel_info)

      if (['sauce', 'sauceconnect'].include?($config['selenium_setting_group']))
        #Saucelabs will give us a session ID
        session_id = @browser.instance_variable_get("@bridge").instance_variable_get("@session_id")
        if session_id.nil? or session_id.to_s.downcase == "null"
          #We should be running on saucelabs, but don't get a session ID from the browser
          raise("Browser didn't return a Saucelabs session ID.")
        else
          #needed for saucelabs jenkins plugin
          #private static final Pattern SESSION_ID_PATTERN = Pattern.compile("SauceOnDemandSessionID=([0-9a-fA-F]+) job-name=(.*)");
          #private static final Pattern OLD_SESSION_ID_PATTERN = Pattern.compile("SauceOnDemandSessionID=([0-9a-fA-F]+)");
          job_name = nil
          if extra_fields['name']
            job_name = extra_fields['name']
          elsif self.methods.include?('method_name')
            job_name = self.method_name
          else
            job_name = "undefined (bug?!)"
          end

          text_the_plugin_looks_for = "SauceOnDemandSessionID=#{session_id} job-name=#{job_name}"
          Log.logger.info("Something for the Jenkins Plugin: #{text_the_plugin_looks_for}")
          puts text_the_plugin_looks_for
        end
      end
    end
  end #module

  module CapybaraTestCase
    include ConfigurationSetup, XvfbSetup
    require 'capybara'
    require 'capybara/dsl'

    #This will be called if somebody includes our module
    def self.included(base)

      #needs libxslt1-dev package for Nokogiri
      base.instance_eval do
        include Capybara::DSL
        #This will tell the base class to include the Capybara module so we can use visit()
        #instead of Capybara.visit() or something like that
        begin
          Log.logger.info("Adding rspec specific hooks for capybara setup")
          before(:all) do
            setup_configuration
            setup_capybara
          end

          before(:each) do
            reset_installation
            Log.logger.info("Resetting browser session")
            #Marc: This can crash if we switch browsers in between tests.
            #Sven: Added error logging
            begin
              Capybara.reset_sessions!
            rescue => err
              Log.logger.info("Reset error: " + err.inspect)
            end
          end
        rescue StandardError
          puts "Problem while adding rspec hooks, maybe we're not running inside an rspec task"
        end
      end
    end

    def setup_capybara
      #good default for now
      begin
        #
        require 'capybara/webkit'
        #Capybara.default_driver = :poltergeist
        Capybara.default_driver = :webkit
      rescue
        Capybara.default_driver = :selenium
      end
      Capybara.run_server = false
      #remove trailing slashes, capybara doesn't like them
      #this has to be set BEFORE requiring sauce/capybara
      Capybara.app_host = $config['sut_url'].chomp("/")
      #we can set the current driver in the test itself in case we don't want selenium
      Log.logger.info("Default target for capybara is: #{Capybara.app_host.inspect}.")

      if [:selenium, :sauce, :webkit].include?(Capybara.current_driver)
        if ($config['selenium_setting_group'] == 'sauce') or (Capybara.current_driver == :sauce)
          Log.logger.info("Setting up a connection to saucelabs")
          require 'sauce'
          require 'sauce/capybara'
          #TODO: add tags
          Sauce.config do |config|
            config.username = $config['selenium_config']['sauce']['browser_type']['username']
            config.access_key = $config['selenium_config']['sauce']['browser_type']['access-key']
            config.browser = $config['selenium_config']['sauce']['browser_type']['browser']
            config.os = $config['selenium_config']['sauce']['browser_type']['os']
            config.browser_version = $config['selenium_config']['sauce']['browser_type']['browser-verison']
            config.browser_url = $config['sut_url']
          end
          Capybara.default_driver = :sauce
        elsif Capybara.current_driver == :webkit
          setup_xvfb_if_necessary
          Capybara.register_driver :webkit do |app|
            browser = Capybara::Driver::Webkit::Browser.new(:ignore_ssl_errors => true)
            Capybara::Driver::Webkit.new(app, :browser => browser)
          end
        else
          #We're not on sauce, so we seem to want to use local selenium
          setup_xvfb_if_necessary
          if $config.key?('selenium_config')
            begin
              selected_browser = $config['selenium_config']['local']['browser_type']
            rescue NoMethodError => e
              #if there is no 'selenium_config' or no ['selenium_config']['local']
              Log.logger.warn("Didn't find selenium_config -> local -> browsertype property in config.")
            end
            capy_browser = :firefox #default
            case selected_browser
            when '*googlechrome'
              capy_browser = :chrome
            when '*firefox'
              capy_browser = :firefox
            when '*chrome'
              capy_browser = :firefox
            when '*iexplore'
              capy_browser = :ie
            when '*opera'
              capy_browser = :opera
            else
              Log.logger.warn("UNKNOWN BROWSER: #{selected_browser.inspect}.")
            end

            Log.logger.info("Switching Selenium browser to: #{capy_browser.inspect}")
            Capybara.register_driver :selenium do |app|
              #difference capybara 1.0 and 0.4
              begin
                Capybara::Selenium::Driver.new(app, :browser => capy_browser)
              rescue NameError => e
                Capybara::Driver::Selenium.new(app, :browser => capy_browser)
              end
            end
          else
            Log.logger.info('No selenium configuration in the test_set yaml file found')
          end
          Capybara.default_driver = :selenium
        end
      end
      Log.logger.info("Current Capybara driver is: #{Capybara.current_driver}")
    end

    def capybara_ignore_ssl
      case Capybara.current_driver
      when :selenium
        Capybara.register_driver :selenium do |app|
          profile = Selenium::WebDriver::Firefox::Profile.new
          profile.assume_untrusted_certificate_issuer = false
          profile.native_events = false
          Capybara::Selenium::Driver.new(app, :browser=> :firefox, :profile => profile)
        end
        Capybara.default_driver = :selenium
      when :webkit
        Capybara.register_driver :webkit do |app|
          browser = Capybara::Driver::Webkit::Browser.new(:ignore_ssl_errors => true)
          Capybara::Driver::Webkit.new(app, :browser => browser)
        end
        Capybara.default_driver = :webkit
      else
        Log.logger.info("No SSL configuration options currently available for driver: #{Capybara.current_driver}")
      end
    end
  end

  module GardensSmokeTestCase
    include GardensTestCaseBase
    def self.included(base)
      base.class_eval do
        alias_method :setup_base, :setup
        alias_method :teardown_base, :teardown
      end
    end

    def setup
      setup_configuration
      setup_selenium()
      subdomain_list = $config['subdomains'] || $config['new_sites']
    end

    def teardown
      unless @browser.nil?
        begin
          @browser.close_current_browser_session
        rescue StandardError, Timeout::Error => e
          Log.logger.info("Error while trying to teardown @browser: #{e.message}")
        end
      end
    end
  end

  module GardensHeadlessTestCase
    include GardensTestCaseBase
    def self.included(base)
      base.class_eval do
        alias_method :setup_base, :setup
      end
    end
    def setup
      setup_configuration
    end
  end

end
