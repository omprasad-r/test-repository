require "rubygems"
require "selenium/webdriver"
require "test/unit"
require "module_manager.rb"

module Test000160createwebformHelper

  # To verify webform module is enabled or not for test_00_enable_webform_module
  def verify_webform_enabled(use_assertion = false)
    Log.logger.info("Verifying whether the link to add content of webform type is present or not")
    @browser.get(@sut_url + '/node/add')
    webform_visible = @browser.find_elements(:xpath => "//a[@href='/node/add/webform']").size > 0
    if use_assertion
      webform_visible.should be_true #, "Webform module is not enabled correctly, Please enable it again.")
    else
      Log.logger.info("Webform is visible now: #{webform_visible}")
    end
    webform_visible
  end

  # Random name generation for galleries name, url, description etc.
  def get_random_string(length=8)
    string = ""
    chars = ("a".."z").to_a
    length.times do
      string << chars[rand(chars.length-1)]
    end
    return string
  end

  # Verifies whether component is added into the webform fieldset or not.
  # This works only when the default names of webform components wouldn't be changed
  # NOTE TO DO: This didn't check for multiple page breaks
  def verify_component_after_saving(fieldname)
    if(fieldname == "Page break")
      element_id = "//input[@id='edit-next']"
      self.check_element_presence_nextpage(element_id, fieldname)
    elsif (fieldname == "Hidden field")
      element_id = "//input[@type = 'hidden' and @value = '' and contains(@name, 'submitted')]"
      self.check_element_presence_nextpage(element_id, fieldname)
    elsif (fieldname == "Formatted content")
      element_id = "//strong[contains(text(), 'New HTML Markup')]"
      self.check_element_presence_nextpage(element_id, fieldname)
    else
      element_id = "//*[contains(text(), '#{fieldname}')]"
      self.check_element_presence_nextpage(element_id, fieldname)
    end
  end

  # Verifies whether component is added into the webform fieldset or not.

  def verify_component_before_saving(fieldname)
    if(fieldname == "Formatted content")
      @browser.find_elements(:xpath => "//strong[contains(text(), 'New HTML Markup')]").should have_at_least(1).items #"#{fieldname} is not present in the Form Builder space area.")
    elsif(fieldname == "Page break" || fieldname == "Hidden field")
      @browser.find_elements(:xpath => "//div[contains(text(), '#{fieldname}')]").should have_at_least(1).items #"#{fieldname} is not present in the Form Builder space area.")
    elsif(fieldname == "Fieldset")
      @browser.find_elements(:xpath => "//span[contains(text(), '#{fieldname}')]").should have_at_least(1).items #"#{fieldname} is not present in the Form Builder space area.")
    else
      @browser.find_elements(:xpath => "//label[contains(text(), '#{fieldname}')]").should have_at_least(1).items #"#{fieldname} is not present in the Form Builder space area.")
    end
  end

  #Checks the presence of element on page, if not found check on next page.

  def check_element_presence_nextpage(element_id, fieldname)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    element_present = @browser.find_elements(:xpath => element_id).size > 0
    unless element_present
      while (@browser.find_elements(:xpath => "//input[@id='edit-next']").size > 0 and !element_present)
        @browser.find_element(:xpath => "//input[@id='edit-next']").click
        element_present = @browser.find_elements(:xpath => element_id).size > 0
      end
      link = wait.until { @browser.find_element(:xpath => "//a[text() = 'View']") }
      link.click
      ## TODO: check for element presence once more?
    end
     element_present.should be_true #, "#{fieldname} is not present in the Webform.")
  end

  # Verifies whether the options settings we made for radio buttons, checkboxes and drop-down list are correct or not
  def verify_options_settings(fieldname, remove_options, change_options, new_options, default_options)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    wait.until { @browser.find_element(:xpath => "//label[contains(text(), '#{fieldname}')]") }
    class_name = @browser.find_element(:xpath => "//label[contains(text(), '#{fieldname}')]/following-sibling::*[1]").attribute("class")
    if class_name == "form-select"
      removed_options, added_options = configure_options_test(remove_options, change_options, new_options)
      removed_options.each do |rem|
        @browser.find_elements(:xpath => "//option[text()='#{rem}']").should have(0).items
      end
      added_options.each do |new|
        @browser.find_elements(:xpath => "//option[text()='#{new}']").should have_at_least(1).items
      end
      Log.logger.info "default_options: #{default_options.inspect}"
      @browser.find_element(:xpath => "//option[text()='#{default_options[0]}']").should be_selected
    elsif class_name == "form-checkboxes" || class_name == "form-radios"
      removed_options, added_options = configure_options_test(remove_options, change_options, new_options)
      removed_options.each do |rem|
        @browser.find_elements(:xpath => "//label[contains(text(), '#{rem}')]").should have(0).items
      end
      added_options.each do |new|
        @browser.find_elements(:xpath => "//label[contains(text(), '#{new}')]").should have_at_least(1).items
      end
      default_options.each do |default|
        @browser.find_elements(:xpath => "//label[contains(text(), '#{default}')]/../input[@checked = 'checked']").should have_at_least(1).items
      end
    else
      Log.logger.info("Following class '#{class_name}' is not a part of test for Webform.")
    end
  end

  #Since not all templates have it
  def make_sure_webform_is_enabled
    unless self.verify_webform_enabled(use_assertion = false)
      self.enable_webform_module
      verify_webform_enabled(use_assertion = true)
    end
  end

  # Test to make sure that webform module is enabled, if disabled enables it.
  def enable_webform_module
    Log.logger.info("Starting test_00_enable_webform_module")
    login($config['user_accounts']['qatestuser']['user'], $config['user_accounts']['qatestuser']['password'])
    @browser.get(@sut_url + '/admin/modules')
    JQuery.wait_for_events_to_finish(@browser)
    modular = ModuleManager.new(@browser)
    mod_value = modular.read_module_value("Webforms")
    if (mod_value == "off")
      Log.logger.info("webforms module is not enabled, enabling it")
      modular.enable_disable_modules(module_names = ["Webforms"], module_value = "on")
    else
      Log.logger.info("Webforms module is already enabled")
    end
    Log.logger.info("Webform module is now enabled")
  end

  # Configures the elements to be either in the presence or removal list
  def configure_options_test(remove_options, change_options, new_options)
    removed_options = change_options.keys
    added_options = change_options.values
    removed_options = removed_options + remove_options
    added_options = added_options + new_options
    return removed_options, added_options
  end

  # Verify whether the field validation settings is changed to required or not.
  def verify_field_required(new_fieldname, should_be_required = true)
    Log.logger.info("Checking if #{new_fieldname} has a required box that says: #{should_be_required}")
    on_page_required = @browser.find_elements(:xpath => "//label[contains(text(), '#{new_fieldname}')]/span[text()='*']").size > 0
    on_page_required.should == should_be_required #, "Should #{new_fieldname} be required: #{should_be_required}. What we got: #{on_page_required}.")
  end

  # Verifies the changes in the field's display settings
  def verify_field_display_settings(prefix_text, suffix_text, size_text, def_value)
    @browser.find_elements(:xpath => "//span[contains(@class, 'field-prefix') and text()='#{prefix_text}']").should have_at_least(1).items #"Prefix text is not same as expected.")
    @browser.find_elements(:xpath => "//span[contains(@class, 'field-suffix') and text()='#{suffix_text}']").should have_at_least(1).items #"Suffix text is not same as expected.")
    sz = @browser.find_element(:xpath => "//input[@value='#{def_value}']").attribute("size")
    size = sz.to_i
    size_text.should == size #, "Field's size is not same as expected.")
  end

  # Verifies the changes in the field's properties settings
  def verify_field_properties_settings(new_fieldname, fieldname_display, def_value, desc_text)
    fieldname_labels = @browser.find_elements(:xpath => "//label[contains(text(), '#{new_fieldname}')]")
    if fieldname_display.downcase.include? "hidden"
      fieldname_labels.should be_empty
    else
      fieldname_labels.should have_at_least(1).items
    end
    @browser.find_elements(:xpath => "//input[@value='#{def_value}']").should have_at_least(1).items
    @browser.find_elements(:xpath => "//div[text()='#{desc_text}']").should have_at_least(1).items
  end

end
