require "rubygems"
require "selenium/webdriver"
require "selenium_info.rb"
require 'theme_builder.rb'
require "acquia_qa/log"


class FontManagement
	
  def initialize(_browser,_url=nil)
    @browser = _browser
    @sut_url = _url || $config['sut_url']
  end
  
  #disables the typekit feature of the font-management module
  def disable_typekit
    Log.logger.info("Disabling typekit in the font management.")
    @browser.get("#{@sut_url}/admin/config/user-interface/font-management") 
    typekit_btn = @browser.find_element(:id => "edit-font-management-typekit-enable")
    if typekit_btn.selected?
      typekit_btn.click
      @browser.find_element(:id => "edit-submit").click
    else
      Log.logger.info("Typekit wasn't enabled.")
    end
  end
  
  #enables the typekit feature of the font-management module, needs the API key (usually in the tests test_set yaml file)
  def enable_typekit(key)
    Log.logger.info("Disabling typekit in the font management.")
    @browser.get("#{@sut_url}/admin/config/user-interface/font-management") 
    typekit_btn = @browser.find_element(:id => "edit-font-management-typekit-enable")
    changed = false
    if typekit_btn.selected?
      Log.logger.info("Typekit was already enabled.")
    else
      typekit_btn.click
      @browser.find_element(:id => "edit-submit").click
      changed = true
    end
    
    current_key = @browser.find_element(:id => 'edit-font-management-typekit-key').attribute("value")
    correct_current_key = (current_key == key)
    if correct_current_key
      Log.logger.info("Typekit key is correct.")
    else
      Log.logger.info("Changing typekit key to #{key.inspect} (was: #{current_key.inspect}).")
      temp = @browser.find_element(:id => "edit-font-management-typekit-key")
      temp.clear
      temp.send_keys(key)
    end
    
    if (!correct_current_key or changed)
      Log.logger.info("Saving configuration.")
      @browser.find_element(:id => "edit-submit").click
    else
      Log.logger.info("No changes made, everything was set up properly.")
    end
  end
  
  
  #turns on the font management module
  def turn_on_fontmanagement
    mods = ModuleManager.new(@browser)
    Log.logger.info("Making sure 'font management' module is enabled.")
    mods.enable_disable_modules(["Font management"], "on")
  end
  
  #turns on the font management module AND enables typekit with the proper API key
  def turn_on_typekit(key)
    Log.logger.info("Ensuring Typekits is On")
    self.turn_on_fontmanagement
    self.enable_typekit(key)
    Log.logger.info("Done with the typekit setup.")
  end  
end
