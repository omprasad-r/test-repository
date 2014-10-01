require 'json'
require "acquia_qa/log"

class SeleniumInfo
  attr_accessor :host, :port, :browser, :timeout
  
  def initialize(options = {})
    Log.logger.debug("Selenium Options: #{options.inspect}")
    #always respect selenium server env before anything else
    @host = ENV['SELENIUM_SERVER'] || options[:host]
    @port = options[:port] || 4444
    @browser = ENV['SELENIUM_BROWSER'] || options[:browser]
    @timeout = options[:timeout] || 60
  end
end

class WebDriverInfo 
  attr_accessor :browser, :timeout, :capabilities

  def initialize(options = {})
    Log.logger.debug("WebDriver Options: #{options.inspect}")
    ######################################################
    ####### CURRENT WEBDRIVER OPTIONS
    ####### :firefox  => firefox
    ####### :chrome   => google chrome (requires a download)
    ####### :ie       => internet explorer
    ####### :htmlunit => html_unit (headless...needs a remote server to run)
    @browser = ENV['SELENIUM_DRIVER'] || options[:browser] || :firefox
    @timeout = options[:timeout] || 3
    @capabilities = false ## TODO: enable modifications of capabilities
  end
end

class SauceWebDriverInfo < WebDriverInfo
  attr_accessor :host, :port, :version , :platform, :username, :testname, :access_key, :setting_group

  ## TODO: consider adding more fields to this class for the Selenium Configuration
  
  def initialize(options = {})
    @host = options[:host] 
    @port = options[:port]
    #Add extra options like tags, build numbers, ... from the extra hash to the browser field
    #This will allow saucelabs to give us more information in the webinterface
    #symbolize hash
    new_options = options.inject({}){|memo,(k,v)| memo[k.to_sym] = v; memo}
    #merge extras with browser hash and delete extras
    if new_options.key?(:extra)
    #  new_options[:browser] = new_options[:browser].merge(new_options[:extra]).to_json
      @version = new_options[:extra][:version]
      @platform =  new_options[:extra][:platform]
      @setting_group = new_options[:extra][:setting_group]
      @testname = new_options[:extra][:testname]
      @username = new_options[:extra][:username]
      @access_key = new_options[:extra][:access_key]
      new_options.delete(:extra)
    else
      Log.logger.info("Didn't find the :extra field for Sauce WebDriver info...There's a high probability that this test WILL NOT run...")
    end  
    
    super(new_options)
    @capabilities = true
  end
end

class SauceSeleniumInfo < SeleniumInfo
  def initialize(options = {})
    #Add extra options like tags, build numbers, ... from the extra hash to the browser field
    #This will allow saucelabs to give us more information in the webinterface
    
    #symbolize hash
    new_options = options.inject({}){|memo,(k,v)| memo[k.to_sym] = v; memo}
    #merge extras with browser hash and delete extras
    if new_options.key?(:extra)
      new_options[:browser] = new_options[:browser].merge(new_options[:extra]).to_json
      new_options.delete(:extra)
    end
    
    super(new_options)
  end
end
