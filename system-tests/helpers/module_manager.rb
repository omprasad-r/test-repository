require "rubygems"
require "acquia_qa/log"
require 'acquia_qa/ssh'

class ModuleManager
  include Acquia::SSH

  attr_reader :export_path, :modmgr

  def initialize(_browser,_url=nil)
    @browser = _browser
    @export_path = '/tmp'
    @modmgr = ModuleManagerGM.new()
    @sut_url = _url || $config['sut_url']
  end
  
  def open_modules_tab(force = false, open_in_iframe = false)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    if open_in_iframe
      Log.logger.info("Opening Modules Tab in iframe.")
      wait.until { @browser.find_element(:id => "toolbar-link-admin-modules") }.click
      iframe_path = "//iframe[contains(@title, 'Modules')]"      
      begin
        frame = wait.until { @browser.find_element(:xpath => iframe_path) }
        @browser.switch_to.frame(frame)
      rescue Selenium::WebDriver::Error => e
        Log.logger.info("Timed out while waiting for modules i-frame. Maybe we opened in a fullscreen frame. (#{e.message.inspect})")
      end
    else
      Log.logger.info("Opening Modules Tab (not in an iframe).")
      @browser.get("#{@sut_url}/admin/modules/")
    end
    wait_for_modules_tab #makes sure Modules Tab is loaded
  end
  
  def close_modules_tab
    Log.logger.debug("Closing Modules Tab")
    if not @browser.find_elements(:id => "overlay-close").empty?
      @browser.find_element(:id => "overlay-close").click
    else
      Log.logger.info("Didn't close overlay, apparently the modules tab wasn't openend in an overlay iframe.")
    end
    @browser.switch_to.default_content
  end
  
  # waits for two default elemets on the Modules Tab to be loaded
  def wait_for_modules_tab(browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    wait.until { !browser.find_elements(:xpath => '//input[@value="Save configuration"]').empty? }
  end
  # Lists the modules and their current values on the Modules overlay
  # This method takes as argument the array of module types for which user wants the list of modules.
  # Module type whether it is core, media, other etc type and value of module.
  
  def list_modules_only(mod_type = nil)
    if mod_type.nil?
      result = list_modules().keys
    else
      result = list_modules(mod_type).keys
    end
    result
  end
  
  def list_modules(mod_type = ["core", "other"])
    self.open_modules_tab(false, false)
    mod_list_value = Hash.new
    mod_type.each do |type|
      Log.logger.info("Working on module group: #{type}")
      i = 1
      xpath = "//fieldset[@id='edit-modules-#{type}']//strong"
      n = Integer(@browser.find_elements(:xpath => xpath).size)
      Log.logger.info("Found #{n} items in the group #{type}")
      while i < (n+1)
        module_name = @browser.find_element(:xpath => "//fieldset[@id='edit-modules-#{type.downcase}']//tr[#{i}]//strong").text
        mod_enabled = @browser.find_element(:xpath => @modmgr.get_module_checkbox_xpath(module_name)).selected?
        mod_enabling_id = @browser.find_element(:xpath => "//strong[text() = '#{module_name}']/../../preceding-sibling::*[1]//input").attribute("id")
        mod_list_value[module_name] = Hash.new() unless (mod_list_value[module_name])
        mod_list_value[module_name][:enabled] = mod_enabled
        mod_list_value[module_name][:checkboxID] = mod_enabling_id
        i += 1
      end
    end
    self.close_modules_tab
    return mod_list_value
  end
  
  # To get the list of modules_types
  def list_modules_types
    self.open_modules_tab(false, false)
    module_types = []
    i = 1
    n = Integer(@browser.find_elements(:xpath => "//fieldset[contains(@id, 'edit-modules-')]").size)
    while i < (n + 1)
      mod_type = @browser.find_element(:xpath => "//form[@id='system-modules']/div/fieldset[#{i}]/legend/span/a").text
      mod_type["HIDE\n"] = ""  # Text returns the Hide(space) in the val, so that needs to be removed to compare.
      module_types = module_types.push(mod_type.capitalize)
      i += 1
    end
    return module_types
  end
  
  # enabling/disabling of various specified modules
  # This method takes as argument the module name which needs to be enabled or disabled,
  # Module type whether it is core, media, other etc type and value of module. (off for disabled and on for enabled)
  # User can enable or disable multiple modules at once. But the modules to be enabled should be run in a different loop than disabled ones.
  # Multiple Enable Example: enable_disable_modules(module_names = ["AddThis", "Blog"], module_value = "on")
  def enable_disable_modules(module_names, module_value = "on")
    wait = Selenium::WebDriver::Wait.new(:timeout => 5)
    open_modules_tab
    Log.logger.info("Figuring out the current status of these modules: #{module_names.inspect}")
    module_names.each do |mod|
      Log.logger.info("Working on module: #{mod}")
      enabled = self.read_module_value(mod)
      Log.logger.info("Module is #{enabled}, it should be #{module_value}.")
      if(enabled == "off")
        if(module_value == "on")
          Log.logger.info("Enabling module: #{mod}.")
          wait.until { @browser.find_element(:xpath => @modmgr.get_module_checkbox_xpath(mod)) }.click
          Log.logger.info("Submitting module form.")
          wait.until { @browser.find_element(:xpath => @modmgr.save_config_btn) }.click
          Log.logger.info("Waiting for status message")
          wait.until { @browser.find_element(:xpath => "//div[@class ='messages status']") }
          Log.logger.info("The module #{mod} is enabled now.")
        else
          Log.logger.info("No changes needs to be made on the module #{mod}. Its already disabled.")
        end
      else
        if(module_value == "off")
          Log.logger.info("Disabling module: #{mod}.")
          wait.until { @browser.find_element(:xpath => @modmgr.get_module_checkbox_xpath(mod)) }.click
          Log.logger.info("Submitting module form.")
          wait.until { @browser.find_element(:xpath => @modmgr.save_config_btn) }.click
          Log.logger.info("Waiting for status message")
          begin
            wait.until { @browser.find_element(:xpath => "//div[@class ='messages status']") }
          rescue Selenium::WebDriver::Error => error
            Log.logger.info("There was an error waiting for the status message: #{error}... Most likely, this is because it was a module that could not be disabled")
          else
            Log.logger.info("The module #{mod} is disabled now.")
          end
        else
          Log.logger.info("No changes needs to be made on the module #{mod}. Its already enabled.")
        end
      end
    end
    close_modules_tab
  end
  
  # Reads the values of modules list provided as argument and returns the hash map for that.
  
  def read_all_modules_values(modules_list)
    self.open_modules_tab
    mod_list_values = Hash.new
    modules_list.each do |mod|
      module_value = self.read_module_value(mod)
      #mod_list_values[mod] = Hash.new() unless mod_list_values.key?(mod)
      mod_list_values[mod] = module_value
    end
    return mod_list_values
  end
  
  # Reads the value of single module provided as argument and returns the value for that.
  
  def read_module_value(mod_name)
    wait = Selenium::WebDriver::Wait.new(:timeout => 5)
    Log.logger.info("Reading module value for #{mod_name}.")
    JQuery.wait_for_events_to_finish(@browser)
    mod = wait.until { @browser.find_element(:xpath => @modmgr.get_module_checkbox_xpath(mod_name)) }
    mod_value = mod.selected?
    Log.logger.info("#{mod_name} module is checked: #{mod_value}.")
    if (mod_value == false)
      return "off"
    else
      return "on"
    end
  end

  def get_modules_values(ft_names)
    current_modules_values = Hash.new
    n = 0
    i = ft_names.length
    while n < i
      case ft_names[n]
      when 'Comments'
        current_val = self.read_module_value("Comment")
      when 'Follow us'
        current_val = self.read_module_value("Follow")
      when 'Stay notified'
        current_val = self.read_module_value("Comment Notify")
      when 'Get feedback'
        current_val = self.read_module_value("Gardens feedback")
      when 'Contact us'
        current_val = self.read_module_value("Contact")
      when 'Share this'
        current_val = self.read_module_value("AddThis")
      when 'Twitter feed'
        current_val = self.read_module_value("Aggregator")
      when 'Mailing list'
        current_val = self.read_module_value("Mailing List")
      when 'Rotating banner'
        current_val = self.read_module_value("Rotating banner")
      when 'Blog'
        current_val = self.read_module_value("Flexible blogs")
      when 'Forums'
        current_val = self.read_module_value("Forum")
      when 'Forms'
        current_val = self.read_module_value("Webforms")
        # Here else scenario begins for the switch statement.
      else
        current_val = nil
      end
      current_modules_values[ft_names[n]] = Hash.new() unless (current_modules_values[ft_names[n]])
      current_modules_values[ft_names[n]] = current_val
      n += 1
    end
    @browser.get(@sut_url)
    return current_modules_values
  end
  
  # It is called from verify_features method which calls this method twice. Once for list where features are off and next time passes on the list where features are on.
  # Verifies only the following types of features : Product feature blocks, Product overview, About, Blog, News, FAQ, Media gallery, Testimonials, Customers & Forums.
  def check_feature_element_presence(feature)
    case feature
    when 'Product feature blocks'
      val = @browser.find_elements(:xpath => @modmgr.prod_feat_block_1).size > 0
    when 'Product overview'
      val = @browser.find_elements(:xpath => @modmgr.prod_overview_link).size > 0
    when 'About'
      val = @browser.find_elements(:xpath => @modmgr.about_us_link).size > 0
    when 'Customers'
      val = @browser.find_elements(:xpath => @modmgr.customers_block).size > 0
    when 'Webforms'
      @browser.get("#{@sut_url}/node/add/")
      val = @browser.find_elements(:xpath => @modmgr.webform_link).size > 0
      # sleep 2 ##TODO
    else
      feat = feature.downcase
      if (feat == "media gallery")
        feat = "galleries"
      end
      feature_link_path = homepage_features_link_selector(feat)
      val = @browser.find_elements(:xpath => feature_link_path).size > 0
    end
    @browser.get(@sut_url)
    return val
  end

  def homepage_features_link_selector(feat)
    return "//a[contains(@href, '/#{feat}')]"
  end

  # GUI Map
  
  class ModuleManagerGM
    attr_reader :module_list_link, :module_uninstall_link
    attr_reader :save_config_btn, :webform_link
    
    attr_reader :prod_feat_block_1
    attr_reader :prod_overview_link, :about_us_link, :customers_block
    
    def initialize()
      @module_list_link = '//a[@href = "/admin/modules"]'
      @module_uninstall_link = '//a[@href = "/admin/modules/uninstall"]'
      @save_config_btn = '//input[@id="edit-submit"]'
      @prod_feat_block_1 = '//*[contains(text(), "Product feature 1")]'
      @prod_overview_link = '//a[@href="/content/product-description"]'
      @about_us_link = '//a[@href="/content/about-us"]'
      @customers_block = '//*[text() = "Customers"]'
      @webform_link = '//a[@href="/node/add/webform"]'
    end
    
    def get_module_checkbox_xpath(module_name)
      "//strong[text() = '#{module_name}']/../../../td[@class='checkbox']/div/input[@type='checkbox']"
    end
  end
end
