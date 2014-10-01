$LOAD_PATH << File.dirname(__FILE__)
require "rubygems"
require "acquia_qa/log"
require "selenium/webdriver"
require 'tempfile'
require 'jquery.rb'

class WebformManager
  attr_reader :export_path, :webformmgr

  def initialize(_browser,_url=nil)
    @browser = _browser
    @webformmgr = WebformManagerGM.new()
    @sut_url = _url || $config['sut_url']
  end

  # Returns the url of the page where user can create new webform.
  def create_webform_url
    '/node/add/webform'
  end

  # Returns the url of the page where user can perform webform settings.
  def webform_settings_url
    '/admin/config/content/webform'
  end

  # Creates the new webform, all the operations done in go from adding title, adding fields, saving webform and returning confirmation message

  def create_webform(title, fields, browser = @browser)
    browser.navigate.to($config['sut_url'] + create_webform_url)
    Log.logger.debug("Entering new webform title '#{title}'")
    fields.each{|fieldname|
      self.add_webform_component(fieldname)
    }
    self.add_webform_title(title)
    message = self.save_webform(browser)
    return message
  end

  # Inserts the webform title

  def add_webform_title(title, browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 30)
    Log.logger.info("Adding webform title '#{title}'")
    elem = wait.until { browser.find_element(:xpath => @webformmgr.webform_title) }
    elem.clear
    elem.send_keys(title)
    Log.logger.info("Finished adding title '#{title}'")
  end

  # Inserts webform component

  def add_webform_component(fieldname, browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.debug("Adding new webform component: #{fieldname}")
    before_count = count_added_webform_components()
    Log.logger.info("Current amount of form-builder-element divs: #{before_count}")
    link_to_click = @webformmgr.fieldset_link(fieldname)
    Log.logger.info("Waiting for and clicking on: #{link_to_click.inspect}")
    temp = wait.until { browser.find_element(:xpath => link_to_click) }
    JQuery.wait_for_events_to_finish(browser)
    unless temp.displayed?
      wait.until { browser.find_element(:xpath => "//li[contains(@class,'-tab-button first')]/a[@href='#']") }.click
      JQuery.wait_for_events_to_finish(browser)
      Log.logger.info("After clicking 'Add Field' button, is the field displayed? #{temp.displayed?}")
    end
    wait.until { browser.find_element(:xpath => link_to_click) }.click
    JQuery.wait_for_events_to_finish(browser)
    if(fieldname == "Formatted content")
      wait.until { browser.find_element(:xpath => @webformmgr.formatted_content_fieldset) }
    elsif(fieldname == "Page break" || fieldname == "Hidden field")
      wait.until { browser.find_element(:xpath => "//div[contains(text(), '#{fieldname}')]") }
    elsif(fieldname == "Fieldset")
      wait.until { browser.find_element(:xpath => "//span[contains(text(), '#{fieldname}')]") }
    else
      wait.until { browser.find_element(:xpath => link_to_click) }
    end
    JQuery.wait_for_events_to_finish(browser)
    expected_count = (before_count + 1)
    current_count = count_added_webform_components()
    sucessfully_added = (current_count == expected_count)
    raise "Webform Element #{fieldname} not sucessfully added." if not sucessfully_added
    Log.logger.info("Added new webform component '#{fieldname}'")
  end

  # Clicks on save button and saves the webform and return confirmation message

  def save_webform(browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    JQuery.wait_for_events_to_finish(browser)
    wait.until { browser.find_element(:xpath => @webformmgr.save_webform_btn) }.click
    Log.logger.info("Saving.....")
    JQuery.wait_for_events_to_finish(@browser)
    message = wait.until { browser.find_element(:xpath => @webformmgr.status_message) }.text
    return message
  end

  # Counts the number of components in the webform after saving, excluding Page break and Hidden fields
  def count_added_webform_components(browser = @browser)
    JQuery.wait_for_events_to_finish(browser)
    Integer(browser.find_elements(:xpath => "//div[contains(@id, 'form-builder-element-new_')]").size)
  end

  def count_webform_components(fields, browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    JQuery.wait_for_events_to_finish(browser)
    Log.logger.info("Counting the number of webform components after saving webform")
    page_breaks = fields.select{|item| item == "Page break"}.size
    fields.delete("Page break")
    fields.delete("Hidden field")
    expected_fields = fields.size
    Log.logger.info("Expecting #{expected_fields} webform components in total.")
    actual_fields = browser.find_elements(:xpath => @webformmgr.webform_component).size
    puts "Page 1: #{actual_fields}"
    page_breaks.times do |i|
      Log.logger.info("Clicking on 'next page'.")
      wait.until { browser.find_element(:xpath => @webformmgr.next_page) }.click
      actual_fields += browser.find_elements(:xpath => @webformmgr.webform_component).size
      puts "Page #{1+i}: #{actual_fields}"
    end
    wait.until { browser.find_element(:xpath => @webformmgr.view_tab) }.click
    JQuery.wait_for_events_to_finish(browser)
    puts "returning: actual => #{actual_fields}"
    puts "returning: expected => #{expected_fields}"
    {:expected => expected_fields, :found => actual_fields}
  end

  # Edits the properties settings for various fields such as: Text Field, Multi-line Text Field, E-mail
  # Default value will always be nil for these fields : Radio buttons, Checkbox, Drop-down list, File Upload
  # Default value and new fieldname will be used for these fields: Hidden field
  # All values would be nil except new_fieldname for these fields: Page break, Fieldset
  # It takes 5 arguments including current fieldname and edits any of these new_fieldname, label_pos, def_value, help_text
  # If any of these arguments will be nil, it wouldn't edit their respective value

  def edit_field_properties_settings(fieldname, new_fieldname = nil, label_pos = nil, def_value = nil, help_text = nil)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Clicking on #{fieldname.inspect} element (should open 'Field Settings' tab)")
    wait.until { @browser.find_element(:xpath => @webformmgr.fieldset_label(fieldname)) }.click
    Log.logger.info("Waiting for 'Field Settings' tab to open")
    wait.until { @browser.find_element(:xpath => "//div[@id='edit-ui-wrapper']//li[contains(@class,'horizontal-tab-button last selected')]") }
    if(new_fieldname != nil)
      Log.logger.info("Changing #{fieldname} to '#{new_fieldname}'")
      temp = wait.until { @browser.find_element(:xpath => @webformmgr.labeltitle(fieldname)) }
      JQuery.wait_for_events_to_finish(@browser)
      temp.clear
      temp.send_keys(new_fieldname)
      wait.until { @browser.find_element(:xpath => @webformmgr.fieldset_label(new_fieldname)) }
      JQuery.wait_for_events_to_finish(@browser)
    end
    if(label_pos != nil)
      Log.logger.info("Changing #{fieldname} label position to '#{label_pos}'")
      flag = false
      @browser.find_element(:xpath => @webformmgr.label_display).find_elements(:xpath => "//option").each {|e|
        next unless e.text.downcase.include?(label_pos.downcase) ; flag = true ; e.click ; break ;
      }
      raise "Selected label #{label_pos.inspect} isn't included in the possible options" unless flag
    end
    if(def_value != nil)
      Log.logger.info("Adding default value '#{def_value}' to webform component '#{fieldname}'")
      @browser.find_element(:xpath => @webformmgr.default_value_chkbox).click
      temp = wait.until { @browser.find_element(:xpath => @webformmgr.default_value) }
      temp.clear
      temp.send_keys(def_value)
      wait.until { @browser.find_element(:xpath => "//input[@value='#{def_value}']") }
    end
    if(help_text != nil)
      Log.logger.info("Adding Help Text '#{help_text}' to webform component '#{fieldname}'")
      @browser.find_element(:xpath => @webformmgr.desc_chkbox).click
      temp = wait.until { @browser.find_element(:xpath => @webformmgr.desc_textarea) }
      temp.clear
      temp.send_keys(help_text)
      wait.until { @browser.find_element(:xpath => "//div[text()='#{help_text}']") }
    end
    return new_fieldname
  end

  # Edit the properties of the formatted content - It doesn't expect any fancy HTML. Expects plain text.

  def edit_formatted_content_properties_settings(fieldname, new_content = nil, text_format = "Plain text")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Editing Properties settings of webform component #{fieldname} of type Formatted Content.")
    wait.until { @browser.find_element(:xpath => @webformmgr.formatted_content_fieldset) }.click
    JQuery.wait_for_events_to_finish(@browser)
    wait.until { @browser.find_element(:xpath => @webformmgr.properties_link) }.click
    JQuery.wait_for_events_to_finish(@browser)
    flag = false
    wait.until { @browser.find_element(:xpath => @webformmgr.markup_text_format) }.find_elements(:xpath => "//option").each {|e|
      next unless e.text.downcase.include?(text_format.downcase) ; flag = true ; e.click ; break ;
    }
    Log.logger.info("Didn't click #{text_format}") unless flag
    JQuery.wait_for_events_to_finish(@browser)
    Log.logger.info("Changing #{fieldname} to '#{new_content}'")
    text_area = wait.until { @browser.find_element(:xpath => @webformmgr.markup_textarea) }
    wait.until { text_area.displayed? }
    text_area.clear
    text_area.send_keys(new_content)
    JQuery.wait_for_events_to_finish(@browser)
    new_content = "Empty markup field" unless new_content
    begin
      JQuery.wait_for_events_to_finish(@browser)
      elems = wait.until { @browser.find_element(:xpath => "//*[contains(text(),'#{new_content}')]") }   # To accomodate all types of tags like strong, p, h1 etc
      Log.logger.info("Found our new element.")
    rescue Selenium::WebDriver::Error::TimeOutError => e
      raise "Timeout while waiting for formatted content with the text #{new_content.inspect} to show up in our webform: #{e}"
    end
    JQuery.wait_for_events_to_finish(@browser)
    return new_content
  end

  # Edit the properties of the Page break, Hidden field, Fieldset

  def edit_other_field_properties(fieldname, new_fieldname)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Editing Properties settings of webform component #{fieldname}.")
    if (@browser.find_elements(:xpath => @webformmgr.pagebreak_hidden_fieldset(fieldname)).size > 0)
      element_id = @webformmgr.pagebreak_hidden_fieldset(fieldname)
      final_element = @webformmgr.pagebreak_hidden_fieldset(new_fieldname)
    else
      element_id = @webformmgr.new_fieldset(fieldname)
      final_element = @webformmgr.new_fieldset(new_fieldname)
    end
    wait.until { @browser.find_element(:xpath => element_id) }.click
    JQuery.wait_for_events_to_finish(@browser)
    wait.until { @browser.find_element(:xpath => @webformmgr.properties_link) }.click
    Log.logger.info("Changing #{fieldname} to '#{new_fieldname}'")
    JQuery.wait_for_events_to_finish(@browser)
    temp = wait.until { @browser.find_element(:xpath => @webformmgr.labeltitle(fieldname)) }
    temp.clear
    temp.send_keys(new_fieldname)
    wait.until { @browser.find_element(:xpath => final_element) }
    return new_fieldname
  end

  # Edits the Display settings for various fields such as: Text Field, E-mail, Multiline text field
  # Edits prefix, suffix, size for Test field
  # Edits only size of File Upload
  # Edits rows and cols for Multiline text field
  # Collapsed and collapsible values are used for Fieldset

  def edit_field_display_settings(fieldname, prefix_text = nil, suffix_text = nil, size_text = nil, rows_length = nil, cols_width = nil, collapsed = true, collapsible = true)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Waiting for label with name #{fieldname.inspect}.")
    wait.until { @browser.find_element(:xpath => @webformmgr.fieldset_label(fieldname)) }.click
    self.expand_field_to_edit(@webformmgr.display_link, @webformmgr.expanded_display)
    if(@browser.find_elements(:xpath => @webformmgr.collapsible_chkbox).size > 0)
      self.edit_fieldset_display_settings(fieldname, collapsed, collapsible)
    else
      if(prefix_text != nil)
    #    self.click_checkbox(@webformmgr.prefix_chkbox)
    Log.logger.info("Changing Prefix Text to '#{prefix_text}'.")
    self.open_text_field(@webformmgr.edit_prefix)
    self.type_text(@webformmgr.edit_prefix, prefix_text)
    JQuery.wait_for_events_to_finish(@browser)
    Log.logger.info("Waiting for a span that contains our prefix test ('#{prefix_text}').")
    wait.until { @browser.find_element(:xpath => "//span[contains(@class, 'field-prefix') and text()='#{prefix_text}']") }
  end
  if(suffix_text != nil)
    Log.logger.info("Changing Suffix Text to '#{suffix_text}'.")
        #self.click_checkbox(@webformmgr.suffix_chkbox)
        self.open_text_field(@webformmgr.edit_suffix)
        JQuery.wait_for_events_to_finish(@browser)
        self.type_text(@webformmgr.edit_suffix, suffix_text)
        Log.logger.info("Waiting for a span that contains our suffixtest ('#{suffix_text}').")
        wait.until { @browser.find_element(:xpath => "//span[contains(@class, 'field-suffix') and text()='#{suffix_text}']") }
      end
      if(size_text != nil)
        Log.logger.info("Changing Size to '#{size_text}'.")
        #self.click_checkbox(@webformmgr.size_chkbox)
        self.open_text_field(@webformmgr.edit_size)
        JQuery.wait_for_events_to_finish(@browser)
        self.type_text(@webformmgr.edit_size, size_text)
        Log.logger.info("Waiting for input that is our new Size ('#{size_text}').")
        wait.until { @browser.find_element(:xpath => "//input[contains(@id, 'edit-new-') and @size='#{size_text}']") }
      end
      if(rows_length != nil)   # Must be positive integer value
        Log.logger.info("Changing Rows Length to '#{rows_length}'.")
        #self.click_checkbox(@webformmgr.rows_chkbox)
        self.type_text(@webformmgr.edit_rows, rows_length)
        JQuery.wait_for_events_to_finish(@browser)
        Log.logger.info("Waiting for textarea that is our new length ('#{rows_length}').")
        wait.until { @browser.find_element(:xpath => "//textarea[@rows='#{rows_length}']") }
      end
      if(cols_width != nil)   # Must be positive integer value
        Log.logger.info("Changing Columns Width to '#{cols_width}'.")
        #self.click_checkbox(@webformmgr.cols_chkbox)
        self.type_text(@webformmgr.edit_cols, cols_width)
        JQuery.wait_for_events_to_finish(@browser)
        Log.logger.info("Waiting for textarea that is our new width ('#{cols_width}').")
        wait.until { @browser.find_element(:xpath => "//textarea[@cols='#{cols_width}']") }
      end
    end
  end

  def check_checkbox(element_id)
    Log.logger.info("Waiting for presence of checkbox: #{element_id.inspect}.")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    box = wait.until { browser.find_element(:xpath => element_id) }
    if box.selected?
      Log.logger.debug("Already checked")
    else
      Log.logger.info("Checking checkbox: #{element_id.inspect}.")
      box.click
    end
  end

  def open_text_field(element_id)
    wait = Selenium::WebDriver::Wait.new(:timeout => 10)
    if element_id == @webformmgr.edit_prefix
      wait.until { @browser.find_element(:xpath => "//li[contains(@class,'-tab-button last')]/a") }.click
      JQuery.wait_for_events_to_finish(@browser)
      wait.until { @browser.find_element(:xpath => "//a[@href='#' and contains(text(),'Display')]") }.click
      JQuery.wait_for_events_to_finish(@browser)
      wait.until { @browser.find_element(:xpath => "//input[@id='field_prefix-checkbox']") }.click
      JQuery.wait_for_events_to_finish(@browser)
    elsif element_id == @webformmgr.edit_suffix
      wait.until { @browser.find_element(:xpath => "//li[contains(@class,'-tab-button last')]/a") }.click
      JQuery.wait_for_events_to_finish(@browser)
      wait.until { @browser.find_element(:xpath => "//a[@href='#' and contains(text(),'Display')]") }.click
      JQuery.wait_for_events_to_finish(@browser)
      wait.until { @browser.find_element(:xpath => "//input[@id='field_suffix-checkbox']") }.click
      JQuery.wait_for_events_to_finish(@browser)
    elsif element_id == @webformmgr.edit_size
      wait.until { @browser.find_element(:xpath => "//li[contains(@class,'-tab-button last')]/a") }.click
      JQuery.wait_for_events_to_finish(@browser)
      wait.until { @browser.find_element(:xpath => "//a[@href='#' and contains(text(),'Display')]") }.click
      JQuery.wait_for_events_to_finish(@browser)
      wait.until { @browser.find_element(:xpath => "//input[@id='size-checkbox']") }.click
      JQuery.wait_for_events_to_finish(@browser)
    end
  end

  def type_text(element_id, text)
    Log.logger.info("Waiting for presence of: #{element_id.inspect}.")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    temp = wait.until { @browser.find_element(:xpath => element_id) }
    unless temp.displayed?
      wait.until { @browser.find_element(:xpath => "//li[contains(@class,'-tab-button last')]/a") }.click
      JQuery.wait_for_events_to_finish(@browser)
      Log.logger.info("After clicking the tab, is the element displayed? #{temp.displayed?}")
    end
    wait.until { temp.displayed? }
    temp.clear
    temp.send_keys(text)
    JQuery.wait_for_events_to_finish(@browser)
  end

  def expand_field_to_edit(element_id, expanded_element_id)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Waiting for presence of: #{element_id.inspect}.")
    wait.until { @browser.find_element(:xpath => element_id) }.click
    #why do we need a sleep here? please add more documentation :)
sleep 2
current_clicks = 0
    #Added a maximum for the number of clicks
    #TODO: What would be a good number for this? Could it be possible that the element is still there just not visible?
    while (!@browser.find_elements(:xpath => expanded_element_id).empty? && current_clicks < 10)
      @browser.find_element(:xpath => element_id).click
      current_clicks += 1
      sleep 3
    end
  end

  # Edits the fieldset display settings and checks or unchecks the collapsed and collapsible feature

  def edit_fieldset_display_settings(fieldname, collapsed = true, collapsible = true)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    elem = wait.until { @browser.find_element(:xpath => @webformmgr.collapsible_chkbox) }
    collapsible_check = elem.selected?
    if (collapsible_check)
      if (collapsible)
        Log.logger.info("Collapsible feature already enabled")
      else
        Log.logger.info("Enabling Collapsible feature")
        elem.click
      end
    else
      if (collapsible)
        Log.logger.info("Disabling Collapsible feature")
        elem.click
      else
        Log.logger.info("Collapsible feature already disabled")
      end
    end

    collapsed_check = @browser.find_element(:xpath => @webformmgr.collapsed_chkbox)
    if (collapsed_check.selected?)
      if (collapsed)
        Log.logger.info("Collapsed feature already enabled")
      else
        Log.logger.info("Enabling Collapsed feature")
        collapsed_check.click
      end
    else
      if (collapsed)
        Log.logger.info("Disabling Collapsed feature")
        collapsed_check.click
      else
        Log.logger.info("Collapsed feature already disabled")
      end
    end
  end

  # Changes the text of the Submit button to new name

  def change_submit_button(fieldname, button_name)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    wait.until { @browser.find_element(:xpath => @webformmgr.fieldset_label(fieldname)) }.click
    wait.until { @browser.find_element(:xpath => @webformmgr.properties_link) }.click
    temp = wait.until { @browser.find_element(:xpath => "//input[@id='edit-submit-button']") }
    temp.clear
    temp.send_keys(button_name)
    wait.until { @browser.find_element(:xpath => @webformmgr.fieldset_label(button_name)) }
    return button_name
  end

  # Edits the Options settings for radio buttons, drop-down list and checkboxes
  # Like adding new option, removing option, changing the option name, or changing default value.

  def edit_options_settings(fieldname, remove_options, change_options, new_options, default_options)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    field_label = wait.until { @browser.find_element(:xpath => @webformmgr.fieldset_label(fieldname)) }
    wait.until { field_label.displayed? }
    field_label.click
    JQuery.wait_for_events_to_finish(@browser)
    self.remove_item(remove_options)
    self.change_item(change_options)
    self.add_new_item(new_options)
    self.change_item(change_options)
    self.edit_default_options(fieldname, default_options)
  end

  # Removes any option from list of options

  def remove_item(remove_options)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    JQuery.wait_for_events_to_finish(@browser)

    regular_options = "//a[@href='#' and contains(text(),'Options')]"
    expanded_options = "//a[@href='#' and @aria-expanded='true' and contains(text(),'Options')]"
    unexpanded_options = "//a[@href='#' and @aria-expanded='false' and contains(text(),'Options')]"

    Log.logger.info("Waiting for the unexpanded 'Options' tab.")
    unexpanded_opts = wait.until { @browser.find_element(:xpath => unexpanded_options) }
    Log.logger.info("Clicking on the unexpanded 'Options' tab.")
    unexpanded_opts.click
    JQuery.wait_for_events_to_finish(@browser)
    Log.logger.info("Waiting for the expanded 'Options' tab.")
    expanded_opts = wait.until { @browser.find_element(:xpath => expanded_options) }

    JQuery.wait_for_events_to_finish(@browser)
    remove_options.each {|remove_option|
      temp = wait.until { @browser.find_element(:xpath =>
       "//input[contains(@class, 'form-radio option-default') and @value = '#{remove_option}']/../../td[3]/a[contains(@class, 'remove')]") }
      puts "Found element to remove"
      wait.until {temp.displayed?}
      puts "Element is visible now, clicking"
      temp.click
      JQuery.wait_for_events_to_finish(@browser)
      wait.until { @browser.find_elements(:xpath => "//input[contains(@class, 'form-radio option-default') and @value = '#{remove_option}']").empty? }
    }
  end

  # Changes any option name to something new from list of options

  def change_item(change_options)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    change_options.sort
    options = change_options.keys
    values = change_options.values
    count = options.length
    count.times {|i|
      Log.logger.info("Webform is changing option #{options[i]} to value #{values[i]}.")
      temp = wait.until { @browser.find_element(:xpath =>
       "//input[contains(@class, 'form-radio option-default') and @value = '#{options[i]}']/../../td[2]/input[2]") }
      temp.clear
      temp.send_keys(values[i])
      sleep 2
    }
  end

  # Adds any new option into list of options

  def add_new_item(new_options)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    new_options.each do |new_option|
      wait.until { @browser.find_element(:xpath => @webformmgr.add_option) }.click
      wait.until { @browser.find_element(:xpath => "//input[contains(@class, 'form-radio option-default') and @value = '']") }
      radio_input = @browser.find_element(:xpath => "//input[contains(@class, 'form-radio option-default') and @value = '']/../../td[2]/input[2]")
      radio_input.clear
      radio_input.send_keys(new_option)
      JQuery.wait_for_events_to_finish(@browser)
      wait.until { @browser.find_element(:xpath => "//input[contains(@class, 'form-radio option-default') and @value = '#{new_option}']") }
    end
  end

  # Edits the default selected option for radio, check and drop-down options.

  def edit_default_options(fieldname, default_options)
    default_options.each do |default_option|
      options_type = @browser.find_element(:xpath => "//input[contains(@class, 'form-radio option-default')]").attribute("type")

      if options_type == "checkbox"
        options = @browser.find_elements(:xpath => "//input[contains(@class, 'form-radio option-default')]").size
        options.times do |i|       # Uncheck all options
          i += 1
          checked = @browser.find_element(:xpath => "//tbody/tr[#{i}]//input[contains(@class, 'form-radio option-default')]").selected?
          if checked
            @browser.find_element(:xpath => "//input[contains(@class, 'form-radio option-default') and @value = '#{default_option}']").click
            JQuery.wait_for_events_to_finish(@browser)
          end
        end
      end

      element = (@browser.find_elements(:xpath => "//input[contains(@class,'form-text option-value')]").select do |option_textbox|
        option_textbox.attribute('value') == default_option
      end).first.find_element(:xpath => "../..//input[contains(@class, 'form-radio option-default')]")

      if !element.selected?
        element.click
        JQuery.wait_for_events_to_finish(@browser)
        wait = Selenium::WebDriver::Wait.new(:timeout => 15)

        wait.until do
          if fieldname == "Drop-down list"
            @browser.find_element(:xpath => "//option[contains(text(), '#{default_option}') and @selected]")
          else
            @browser.find_element(:xpath => "//label[contains(text(), '#{default_option}')]/../input[contains(@checked, 'checked') and @type = '#{options_type}']")
          end
        end
      end
    end
  end

  # It works for text field, file upload, multi line text field, radio, checkbox and drop-down list
  # Specially to specify the file size and file types allowed for File Upload component
  # All = includes all the file types
  # All archives = bz2, gz, rar, sit, tar, zip
  # All media =  avi, mov, mp3, ogg, wav
  # All web images = gif, jpg, png
  # All desktop images = bmp, eps, tif, pict, psd
  # All documents = txt, rtf, html, odf, pdf, doc, docx, ppt, pptx, xls, xlsx, xml
  # File uploader should choose one of these file options

  def edit_field_validation_settings(fieldname, required = true, file_size = nil, media_types_allowed = [])
    if required
      Log.logger.info("Setting fieldname #{fieldname.inspect} to be required.")
    else
      Log.logger.info("Setting fieldname #{fieldname.inspect} to be NOT required.")
    end
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)

    #Open the validation settings for this field
    flag = self.open_field_validation_settings(fieldname)
    if flag == false
      Log.logger.info("Couldn't open field settings for #{fieldname}. Browser couldn't find the element...")
      raise "QA ERROR: Could not find fieldset label for #{fieldname}"
    end
    Log.logger.info("Waiting for the required checkbox (again).")
    box = wait.until { @browser.find_element(:xpath => @webformmgr.required_chkbox) }
    Log.logger.info("Found it (again).")
    check = box.selected?
    Log.logger.info("It is checked: #{check}")
    if(required and check)
      Log.logger.info("Field #{fieldname} is already Required")
    elsif(required and !check)
      Log.logger.info("#{fieldname} is required but not checked --> Checking box")
      #we need click because check won't activate javascript
      box.click
      Log.logger.info("Checked it, waiting for * to show up next to #{fieldname} label (#{@webformmgr.required_field_label(fieldname)})")
      wait.until { @browser.find_element(:xpath => @webformmgr.required_field_label(fieldname)) }
    elsif(!required and check)
      Log.logger.info("#{fieldname} is not required but checked --> Unchecking box")
      #we need click because check won't activate javascript
      box.click
      Log.logger.info("Checked it, waiting for * to disappear next to #{fieldname} label (#{@webformmgr.required_field_label(fieldname)})")
      wait.until { @browser.find_elements(:xpath => @webformmgr.required_field_label(fieldname)).empty? }
    else
      Log.logger.info("Field Already not Required")
    end
    Log.logger.info("Finished our 'required' checkbox tasks")
    if (file_size != nil)
      if(file_size < 1000)
        file_size = 1000
      elsif(file_size > 20000)
        file_size = 20000
      else
        Log.logger.info("File size entered in within the limits of 1MB - 20MB.")
      end
      temp = wait.until { @browser.find_element(:xpath => @webformmgr.file_upload_size) }
      temp.clear
      temp.send_keys(file_size)
      sleep 2
    end
    if (media_types_allowed != [])
      media_box = wait.until { @browser.find_element(:xpath => @webformmgr.media_types_checkbox("All")) }
      checked = media_box.selected?
      if (checked)
        2.times {
          media_box.click # why do we do this two times??!
        }
      else
        media_box.click
      end
      @browser.navigate.refresh
      self.open_field_validation_settings(fieldname)
      media_types_allowed.each {|type|
        wait.until { @browser.find_element(:xpath => @webformmgr.media_types_checkbox(type)) }.click
        @browser.navigate.refresh
        self.open_field_validation_settings(fieldname)
        wait.until { @browser.find_element(:xpath => "//label[text() = '#{type} ']/preceding-sibling::input[@checked = 'checked']") }
      }
    end
  end

  # Opens the Validation settings for any field, which has validation settings

  def open_field_validation_settings(fieldname)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    if @browser.find_elements(:xpath => @webformmgr.fieldset_label(fieldname)).size > 0
      Log.logger.info("Opening field validation settings for #{fieldname.inspect}.")
      wait.until { @browser.find_element(:xpath => @webformmgr.fieldset_label(fieldname)) }.click
      wait.until { @browser.find_element(:xpath => @webformmgr.expanded_properties) && @browser.find_element(:xpath => @webformmgr.validation_link) }
      sleep 2 #I guess that's how long it needs to add the javascript function
      Log.logger.info("Clicking on Validation Link")
      @browser.find_element(:xpath => @webformmgr.validation_link).click
      begin
        wait.until { @browser.find_element(:xpath => @webformmgr.expanded_validation) }
        validation_expanded = true
      rescue
        validation_expanded = false
      end
      if (!validation_expanded)
        Log.logger.info("Didn't find an expanded validation link. Clicking on validation link to expand the field")
        wait.until { @browser.find_element(:xpath => @webformmgr.validation_link) }.click
        wait.until { @browser.find_element(:xpath => @webformmgr.expanded_validation) }
      end
      Log.logger.info("Waiting for the 'required' Checkbox")
      wait.until { @browser.find_element(:xpath => @webformmgr.required_chkbox) }
      Log.logger.info("Found the 'required' Checkbox")
      return true
    else
      Log.logger.info("Warning: could not locate Fieldset Label #{fieldname.inspect}")
      return false
    end
  end

  # Deletes the webform component from the webform

  def delete_webform_component(fieldname)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Deleting webform component named '#{fieldname}'")
    wait.until { @browser.find_element(:xpath => @webformmgr.fieldset_label(fieldname)) }.click
    wait.until { @browser.find_element(:xpath => @webformmgr.fieldset_label(fieldname)) }.click
    wait.until { @browser.find_element(:xpath => @webformmgr.active_delete_link) }.click
    wait.until { @browser.find_elements(:xpath => @webformmgr.active_delete_link).empty? }
    Log.logger.debug("Deleted webform component named '#{fieldname}'")
  end

  # Fill in the values to the webform fields except File uploader.

  def edit_webform(fieldname_values)
    Log.logger.info("Found #{fieldname_values.size} fields to fill out")

    fieldname_values.each_pair do |key, value|
      element_id = @browser.find_element(:xpath => @webformmgr.fieldset_label(key)).attribute("for")
      class_name = @browser.find_element(:xpath => "//*[@id='#{element_id}']").attribute("class")
      Log.logger.info("Setting #{class_name.inspect} in the webform")
      case class_name
      when 'form-text'
        self.edit_text_field(element_id, fieldname_values[key])
      when 'form-textarea'
        self.edit_text_field(element_id, fieldname_values[key])
      when 'email form-text'
        self.edit_text_field(element_id, fieldname_values[key])
      when 'form-select'
        self.edit_dropdown_field(element_id, fieldname_values[key])
      when 'form-checkboxes'
        self.edit_checkbox_field(element_id, fieldname_values[key])
      when 'form-radios'
        self.edit_radio_field(element_id, fieldname_values[key])
      else
        Log.logger.info("This #{class_name} is not valid class, therefore can't be edited.")
      end
    end
  end

  # Open webform results

  def open_webform_results
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    #This is configurable
    if @browser.find_elements(:xpath => @webformmgr.back_to_webform_link).size > 0
      Log.logger.info("Clicking 'back to webform' link.")
      wait.until { @browser.find_element(:xpath => @webformmgr.back_to_webform_link) }.click
    end
    if @browser.find_elements(:xpath => @webformmgr.previous_submissions_link).size > 0
      Log.logger.info("Clicking 'previous submissions' link.")
      @browser.find_element(:xpath => @webformmgr.previous_submissions_link).click
      self.view_webform_submission
      frame = wait.until { @browser.find_element(:xpath => @webformmgr.overlay_frame) }
      @browser.switch_to.frame(frame)
    else
      Log.logger.info("'Previous submissions' link not found, clicking on results link.")
      wait.until { @browser.find_element(:xpath => @webformmgr.results_link) }.click
      frame = wait.until { @browser.find_element(:xpath => @webformmgr.overlay_frame) }
      @browser.switch_to.frame(frame)
      self.view_webform_submission
    end
    Log.logger.info("Webform results page opened.")
  end

  # View the most recent webform submisiion

  def view_webform_submission
    Log.logger.info("Viewing webform submission.")
    self.sort_content(ascending = true)
    sleep 2
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    wait.until { @browser.find_element(:xpath => @webformmgr.view_first_webform_result) }.click
  end

  # Sorts the webform results by ascending or descending order

  def sort_content(ascending = true)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Sorting Webform Results")
    wait.until { @browser.find_element(:xpath => @webformmgr.sort_asc) }.click
    elem = wait.until { @browser.find_element(:xpath => "//a[contains(@title, 'sort by Submitted')]/img") }
    current_sorting = elem.attribute("title")
    if (current_sorting == 'sort descending' and ascending)
      @browser.find_element(:xpath => @webformmgr.sort_asc).click
    elsif (current_sorting == 'sort descending' and !ascending)
      Log.logger.info("Already sorted by Descending")
    elsif (current_sorting == 'sort ascending' and ascending)
      Log.logger.info("Already sorted by Ascending")
    else
      @browser.find_element(:xpath => @webformmgr.sort_asc).click
    end
  end

  # Extracts the results of the webform submission made by the user

  def get_webform_results(fieldname)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    wait.until { @browser.find_element(:xpath => @webformmgr.submissions_info_header) }
    multiple_checkbox = @browser.find_elements(:xpath => "//label[text()='#{fieldname}: ']/..//li").size > 0
    if(multiple_checkbox)
      Log.logger.info("We have a multi checkbox!")
      count = Integer(@browser.find_elements(:xpath => "//label[text()='#{fieldname}: ']/..//li").size)
      value = []
      count.times {|i|
        i += 1
        val = @browser.find_element(:xpath => "//label[text()='#{fieldname}: ']/..//li[#{i}]").text
        value = value.push(val)
      }
      final_value = "#{fieldname}: #{value}"
    else
      final_value = @browser.find_element(:xpath => "//div/label[contains(text(), '#{fieldname}')]/parent::div").text
    end
    Log.logger.info("Final value: #{final_value}")
    return final_value
  end


  # Fill in the values to the webform fields except File uploader.

  def edit_webform2(fieldnames)
    fieldnames.each {|fieldname|
      element_id = @browser.find_element(:xpath => "#{@webformmgr.fieldset_label(fieldname)}").attribute("for")
      class_name = @browser.find_element(:xpath => "//*[@id='#{element_id}']").attribute("class")
      case class_name
      when 'form-text'
        self.edit_text_field(element_id, val = "test")
      when 'form-textarea'
        self.edit_text_field(element_id, val = "testing text")
      when 'email form-text'
        self.edit_text_field(element_id, val = "test@test.co.in")
      when 'form-select'
        self.edit_dropdown_field(element_id, val = "test")
      when 'form-checkboxes'
        self.edit_checkbox_field(element_id, val = ["test", "one"])
      when 'form-radios'
        self.edit_radio_field(element_id, val = "test")
      else
        Log.logger.info("This #{class_name} is not valid class, therefore can't be edited.")
      end
    }
  end

  # Edits the field value when component is one of these types: text field, multiline text field, email

  def edit_text_field(id, val)
    Log.logger.info("Editing #{val} in the Textfield.")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    temp = wait.until { @browser.find_element(:xpath => "//*[@id = '#{id}']") }
    temp.clear
    temp.send_keys(val)   # By using * we can edit three fields at one go.
  end

  # Edits the field value when component is one of these types: radio button

  def edit_radio_field(id, val)
    Log.logger.info("Selecting #{val} option from Radio field.")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    elem = wait.until { @browser.find_element(:xpath => @webformmgr.chkbox_radio_option(id, val)) }
    option_id = elem.attribute("for")
    @browser.find_element(:xpath => @webformmgr.option_id(option_id)).click
  end

  # Edits the field value when component is one of these types: checkbox

  def edit_checkbox_field(id, vals)
    Log.logger.info("Checking #{vals} in the checkbox")
    count = Integer(@browser.find_elements(:xpath => "//div[@id = '#{id}']/div").size)
    #    self.click_all_checkboxes(count, id)
    vals.each {|val|
      count.times {|i|
        i += 1
        if(@browser.find_elements(:xpath => "//div[@id = '#{id}']/div[#{i}]/label[contains(text(), '#{val}')]").size > 0)
          option_id = @browser.find_element(:xpath => "#{@webformmgr.chkbox_radio_option(id, val)}").attribute("for")
          check = @browser.find_element(:xpath => @webformmgr.option_id(option_id)).selected?   # Just to make sure that option is not checked
          if(!check)        # If checked, it wouldn't click.
            @browser.find_element(:xpath => @webformmgr.option_id(option_id)).click
          end
        end
      }
    }
  end

  # Uncheck all the options under checkbox. Takes as argument id of checkbox and number of options.

  def uncheck_all_checkboxes(count, id)
    Log.logger.info("Unchecking all '#{count}' options in the checkbox")
    count.times {|i|
      i += 1
      check = @browser.find_element(:xpath => @webformmgr.clickbox_id(id, i)).selected?
      if(check == true)
        @browser.find_element(:xpath => @webformmgr.clickbox_id(id, i)).click
      end
    }
  end

  # Edits the field value when component is one of these types: Drop-down list

  def edit_dropdown_field(id, val)
    Log.logger.info("Selecting the drop-down list value to #{val}")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    select_elem = wait.until { @browser.find_element(:xpath => "//select[@id = '#{id}']") }
    flag = false
    select_elem.find_elements(:xpath => "//option").each {|e|
      next unless e.text.downcase.include?(val.downcase) ; flag = true ; e.click ; break ;
    }
    Log.logger.info("Failed to edit dropdown field for #{val}") unless flag
  end


  def enable_disable_webform_content(content_types, enable = true)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    @browser.get("#{@sut_url}/admin/config/content/webform")
    content_types.each {|type|
      Log.logger.debug("Working on enabling webforms for: #{type}.")
      box = wait.until { @browser.find_element(:xpath => @webformmgr.content_chkbox(type)) }
      checked = box.selected?
      if (checked and enable)
        Log.logger.info("Webforms already enabled for #{type} content type.")
      elsif (checked and !enable)
        box.click
        Log.logger.info("Disabled webforms for #{type} content type.")
      elsif (!checked and enable)
        box.click
        Log.logger.info("Enabled webforms for #{type} content type.")
      else
        Log.logger.info("Webforms already disabled for #{type} content type.")
      end
    }
    Log.logger.debug("Waiting for save button")
    wait.until { @browser.find_element(:xpath => @webformmgr.save_webform_btn) }.click
    JQuery.wait_for_events_to_finish(@browser)
    Log.logger.debug("Waiting for status message")
    message = wait.until { @browser.find_element(:xpath => @webformmgr.status_message) }.text
    Log.logger.info("Got message: #{message}")
    return message
  end

  def enable_disable_webform_component(component_names, enable = true)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    @browser.get("#{@sut_url}/admin/config/content/webform")
    component_names.each {|cname|
      box = wait.until { @browser.find_element(:xpath => @webformmgr.component_chkbox(cname)) }
      checked = box.selected?
      if (checked and enable)
        Log.logger.info("#{cname} Webform component already enabled.")
      elsif (checked and !enable)
        box.click
        Log.logger.info("Disabled webform component #{cname}.")
      elsif (!checked and enable)
        box.click
        Log.logger.info("Enabled webform component #{cname}.")
      else
        Log.logger.info("#{cname} Webform component already disabled.")
      end
    }
    Log.logger.debug("Waiting for save button")
    wait.until { @browser.find_element(:xpath => @webformmgr.save_webform_btn) }.click
    JQuery.wait_for_events_to_finish(@browser)
    Log.logger.debug("Waiting for status message")
    message = wait.until { @browser.find_element(:xpath => @webformmgr.status_message) }.text
    return message
  end

  # component_names = ["email", "file", "fieldset", "hidden", "markup", "pagebreak", "select", "textarea", "textfield"]
  # Allowed content types are: article, page, poll, media-gallery, book, blog, forum, webform

  def add_content_with_webform(content_type, title, comp_names)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    case content_type
    when 'forum'
      self.add_content(content_type, title, comp_names)
      Log.logger.info("Selecting the proper forum type from the dropdown ('This is a forum')")
      flag = false
      wait.until { @browser.find_element(:xpath => @webformmgr.forum_type) }.find_elements(:xpath => "//option").each {|e|
        next unless e.text.include?('This is a forum') ; flag = true; e.click ; break ;
      }
      Log.logger.info("Didn't add content!!!") unless flag
      message = self.save_webform
    when 'poll'
      self.add_content(content_type, title, comp_names)
      temp = wait.until { @browser.find_element(:xpath => @webformmgr.poll_choice_one) }
      temp.clear
      temp.send_keys("Yes")
      temp = wait.until { @browser.find_element(:xpath => @webformmgr.poll_choice_two) }
      temp.clear
      temp.send_keys("No")
      message = self.save_webform
    else
      self.add_content(content_type, title, comp_names)
      message = self.save_webform
    end
    return message
  end

  def add_content(content_type, title, fieldnames)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    @browser.get("#{@sut_url}/node/add/#{content_type}")
    Log.logger.debug("Adding content type '#{content_type}' titled '#{title}' with webform.")
    temp = wait.until { @browser.find_element(:xpath => @webformmgr.content_title) }
    temp.clear
    temp.send_keys(title)
    fieldnames.each {|fieldname|
      self.add_webform_component(fieldname)
    }
  end

  # Arrange webform components by names, where they can be identified in webform.

  def arrange_webform_component_names(comp_names)
    fieldnames = []
    comp_names.each {|cname|
      case cname
      when 'email'
        fieldnames = fieldnames.push("E-mail")
      when 'file'
        fieldnames = fieldnames.push("File upload")
      when 'fieldset'
        fieldnames = fieldnames.push("Fieldset")
      when 'hidden'
        fieldnames = fieldnames.push("Hidden field")
      when 'markup'
        fieldnames = fieldnames.push("Formatted content")
      when 'pagebreak'
        fieldnames = fieldnames.push("Page break")
      when 'select'
        fieldnames = fieldnames.push("Radio buttons")
        fieldnames = fieldnames.push("Check boxes")
        fieldnames = fieldnames.push("Drop-down list")
      when 'textarea'
        fieldnames = fieldnames.push("Multi-line text field")
      when 'textfield'
        fieldnames = fieldnames.push("Text field")
      else
        Log.logger.info("'#{cname}' is not a valid option of webform component.")
      end
    }
    return fieldnames
  end

  # Change the advanced settings of the webforms. Takes as argument format of results download,
  # enabling status of cookies and text separator type(if download format will be Delimited text).
  # download_format should be excel/delimited, enable_cookies should be true or false
  # text_separator should be one of following: Comma (,),

  def change_advanced_settings(download_format, enable_cookies, text_separator = nil)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    advanced_collapsed = @browser.find_elements(:xpath => @webformmgr.advanced_settings_collapsed).size > 0
    if (advanced_collapsed)
      wait.until { @browser.find_element(:xpath => @webformmgr.webform_advanced_link) }.click
      wait.until { @browser.find_element(:xpath => @webformmgr.advanced_settings_exapnded) }
    end
    if (download_format == "excel")
      Log.logger.info("Changing download file format to Microsoft Excel.")
      wait.until { @browser.find_element(:xpath => @webformmgr.excel_format_button) }.click
    else
      Log.logger.info("Changing download file format to Delimited text.")
      wait.until { @browser.find_element(:xpath => @webformmgr.delimited_text_format_button) }.click
      Log.logger.info("Changing download file format text separator to #{text_separator}.")
      flag = false
      wait.until { @browser.find_element(:xpath => @webformmgr.delimited_text_separator) }.find_elements(:xpath => "//option") {|e|
        next unless e.text.downcase.include?(text_separator.downcase) ; flag = true ;  e.click ; break ;
      }
      Log.logger.info("didn't click on #{text_separator}") unless flag
    end
    element = @browser.find_element(:xpath => @webformmgr.cookies_chkbox)
    allow_cookies = element.selected?
    if (allow_cookies and enable_cookies)
      Log.logger.info("Cookies are already being tracked against webform submissions.")
    elsif (allow_cookies and !enable_cookies)
      Log.logger.info("Disabling tracking of Cookies for webform submissions.")
      element.click
    elsif (!allow_cookies and enable_cookies)
      Log.logger.info("Enabling tracking of Cookies for webform submissions.")
      element.click
    else
      Log.logger.info("Tracking of Cookies is already disabled for webform submissions.")
    end
    message = self.save_webform
    return message
  end

  # Get the current values of webform advanced settings

  def get_advanced_settings
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    advanced_collapsed = @browser.find_elements(:xpath => @webformmgr.advanced_settings_collapsed).size > 0
    if (advanced_collapsed)
      wait.until { @browser.find_element(:xpath => @webformmgr.webform_advanced_link) }.click
      wait.until { @browser.find_element(:xpath => @webformmgr.advanced_settings_exapnded) }
    end
    element = wait.until { @browser.find_element(:xpath => @webformmgr.excel_format_button) }
    checked = element.selected?
    if (checked)
      download_format = element.attribute("value")
      text_separator = nil
    else
      download_format = @browser.find_element(:xpath => @webformmgr.delimited_text_format_button).attribute("value")
      ##########
      ## @browser.get_selected_label
      flag = false
      text_separator = @browser.find_element(:xpath => @webformmgr.delimited_text_separator).find_elements(:xpath => "//option").each {|e|
        next unless e.selected? ; flag = true ; return e.text ; }
        Log.logger.info("Didn't find selected label!") unless flag
    end
    cookies_allowed = @browser.find_element(:xpath => @webformmgr.cookies_chkbox).selected?
    return download_format, cookies_allowed, text_separator
  end

  # Delete webform

  def delete_webform
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Deleting webform")
    wait.until { @browser.find_element(:xpath => @webformmgr.edit_webform_link) }.click
    frame = wait.until { @browser.find_element(:xpath => @webformmgr.overlay_frame) }
    @browser.switch_to.frame(frame)
    wait.until { @browser.find_element(:xpath => @webformmgr.delete_btn) }.click
    wait.until { @browser.find_element(:xpath => @webformmgr.cancel_link) }
    @browser.find_element(:xpath => @webformmgr.confirm_delete).click
    @browser.switch_to.default_content
    JQuery.wait_for_events_to_finish(@browser)
    message = wait.until { @browser.find_element(:xpath => @webformmgr.status_message) }.text
    Log.logger.debug("Deleted webform")
    return message
  end

  # Change Form Submission Access Settings
  #Show standard confirmation page
  #Redirect to a different page
  #Stay on the same page

  def customize_confirmation_message_page(mesaage_customization, page, redirect_url, show_msg, text_format, custom_message)
    Log.logger.info("Changing webform submission settings - customizing confirmation message and redirect page.")
    self.enable_disable_checkbox(@webformmgr.customized_message_chkbox, mesaage_customization, "Customize confirmation")
    if (mesaage_customization)
      self.customize_submission_settings(page, redirect_url, show_msg, text_format, custom_message)
    else
      Log.logger.info("Cannot customize confirmation message after webform submission, as checkbox to edit that is disabled.")
    end
    Log.logger.debug("Changed webform submission settings - customizing confirmation message and redirect page.")
  end

  def customize_submission_settings(page_after_submission, redirect_url, show_msg, text_format, custom_message)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Changing webform submission settings, customizing confirmation message.")
    flag = false
    wait.until { @browser.find_element(:xpath => @webformmgr.redirect_page) }.find_elements(:xpath => "//option").each {|e|
      next unless e.text.downcase.include?(page_after_submission.downcase) ; flag = true; e.click ; break ;
    }
    Log.logger.info("Didin't find #{page_after_submission}") unless flag
    if (page_after_submission == "Redirect to a different page")
      temp = wait.until { @browser.find_element(:xpath => @webformmgr.redirect_url_textbox) }
      temp.clear
      temp.send_keys(redirect_url)
      self.change_complex_confirmation_message(show_msg, text_format, custom_message, "Show a confirmation message")
    elsif (page_after_submission == "Stay on the same page")
      self.change_complex_confirmation_message(show_msg, text_format, custom_message, "Show a confirmation message")
    else
      self.change_simplified_confirmation_message(text_format, custom_message)
    end
  end

  def change_complex_confirmation_message(show_msg, text_format, custom_message, checkbox_name)
    self.enable_disable_checkbox(@webformmgr.confirmation_msg_chkbox, show_msg, checkbox_name)
    if (show_msg)
      Log.logger.info("Changing Customized confirmation message after webform submission.")
      flag = false
      @browser.find_element(:xpath => @webformmgr.text_format).find_elements(:xpath => "//option").each {|e|
        next unless e.text.downcase.include?(text_format.downcase) ; flag = true ; e.click ; break ;
      }
      Log.logger.info("Didn't change settings for #{text_format}") unless flag
      temp = @browser.find_element(:xpath => @webformmgr.customized_message_textbox)
      temp.clear
      temp.send_keys(custom_message)
      Log.logger.info("Changed Customized confirmation message after webform submission.")
    else
      Log.logger.info("Customization of message after submission not possible, as checkbox to edit that is disabled.")
    end
  end

  def change_simplified_confirmation_message(text_format, custom_message)
    Log.logger.info("Changing Customized confirmation message after webform submission.")
    flag = false
    @browser.find_element(:xpath => @webformmgr.simplified_text_format).find_elements(:xpath => "//option").each {|e|
      next unless e.text.downcase.include?(text_format.downcase) ; flag = true ; e.click ; break ;
    }
    temp = @browser.find_element(:xpath => @webformmgr.simplified_message_textbox)
    temp.clear
    temp.send_keys(custom_message)
    Log.logger.info("Changed Customized confirmation message after webform submission.")
  end

  def limit_submissions(limit_submissions, limit_text, limit_timeframe)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Changing webform submission settings - limiting number of submissions.")
    self.enable_disable_checkbox(@webformmgr.submission_limit_chkbox, limit_submissions, "Limit submissions")
    if (limit_submissions)
      temp = wait.until { @browser.find_element(:xpath => @webformmgr.submission_limit_textbox) }
      temp.clear
      temp.send_keys(limit_text)
      flag = false
      @browser.find_element(:xpath => @webformmgr.submission_limit_timeframe).find_elements(:xpath => "//option").each {|e|
        next unless e.text.downcase.include?(limit_timeframe.downcase) ; flag = true ; e.click ; break ;
      }
      Log.logger.info("Failed to find #{limit_timeframe}") unless flag
    else
      Log.logger.info("Cannot limit webform submissions, as checkbox to edit that is disabled.")
    end
    Log.logger.debug("Changed webform submission settings - limiting number of submissions.")
  end

  def change_mollom_settings(mollom)
    Log.logger.info("Changing webform submission settings - Mollom Settings.")
    self.enable_disable_checkbox(@webformmgr.mollom_chkbox, mollom, "Enable spam protection")
    Log.logger.debug("Changed webform submission settings - Mollom Settings.")
  end


  # Change Form Submission Access Settings

  def change_form_submission_access_settings(anonymous, authenticated, admin, site_owner, blogger, editor)
    Log.logger.info("Changing webform submission access settings")
    self.expand_field_to_edit(@webformmgr.form_submission_access_settings, @webformmgr.expanded_form_submission_access_settings)
    self.enable_disable_checkbox(@webformmgr.anonymous_user_chkbox, anonymous, "Anonymous user")
    self.enable_disable_checkbox(@webformmgr.authenticated_user_chkbox, authenticated, "Authenticated user")
    self.enable_disable_checkbox(@webformmgr.administrator_chkbox, admin, "Administrator")
    self.enable_disable_checkbox(@webformmgr.site_owner_chkbox, site_owner, "Site owner")
    Log.logger.debug("Changed webform submission access settings")
  end

  # Change Form Advanced Settings

  def change_form_advanced_settings(block, teaser, prev_submissions)
    Log.logger.info("Changing webform advanced settings")
    self.expand_field_to_edit(@webformmgr.form_advanced_settings, @webformmgr.expanded_form_advanced_settings)
    self.enable_disable_checkbox(@webformmgr.block_chkbox, block, "Create a block")
    self.enable_disable_checkbox(@webformmgr.teaser_chkbox, teaser, "Show complete form in teaser")
    self.enable_disable_checkbox(@webformmgr.prev_submissions_chkbox, prev_submissions, "Display a link to previous submissions Block")
    Log.logger.debug("Changed webform advanced settings")
  end

  # Method to check and uncheck various checkboxes. Takes as argument path for element and its enabling or disabling status in true/false.

  def enable_disable_checkbox(element_id, enable_val, checkbox)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Checking checbox status of #{checkbox}")
    elem = wait.until { @browser.find_element(:xpath => element_id) }
    enabled = elem.selected?
    if (enabled and enable_val)
      Log.logger.info("#{checkbox} checkbox already enabled")
    elsif (enabled and !enable_val)
      if (element_id == @webformmgr.mollom_chkbox)
        self.expand_field_to_edit(@webformmgr.form_submission_access_settings, @webformmgr.expanded_form_submission_access_settings)
        self.enable_disable_checkbox(@webformmgr.anonymous_user_chkbox, false, "Anonymous user")
        wait.until { @browser.find_elements(:xpath => @webformmgr.verify_spam_protection).empty? }
      end
      elem.click
      Log.logger.info("#{checkbox} checkbox disabled")
    elsif (!enabled and enable_val)
      if (element_id == @webformmgr.anonymous_user_chkbox)
        anonymous_disabled = @browser.find_elements(:xpath => @webformmgr.disabled_anonymous_user_chkbox).size > 0
        if (anonymous_disabled)
          self.enable_disable_checkbox(@webformmgr.mollom_chkbox, true, "Enable mollom protection")
          wait.until { @browser.find_elements(:xpath => @webformmgr.disabled_anonymous_user_chkbox).empty? }
        end
      end
      elem.click
      Log.logger.info("#{checkbox} checkbox enabled")
    else
      Log.logger.info("#{checkbox} checkbox already disabled")
    end
  end

  def publish_content_on_front_page
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    wait.until { @browser.find_element(:xpath => '//strong[text()="Promotion settings"]') }.click
    wait.until { @browser.find_element(:xpath => @webformmgr.promote_content_chkbox) }
    #    self.enable_disable_checkbox(@webformmgr.publish_content_chkbox, true, "Publish Content")
    self.enable_disable_checkbox(@webformmgr.promote_content_chkbox, true, "Promote Content to Front page")
    #    @browser.click(@webformmgr.form_settings_link)
    #    @browser.wait_for_element(@webformmgr.submission_limit_chkbox)
    @browser.find_element(:xpath => "//input[@id = 'edit-submit']").click
  end

  def get_element_id(webform_title, fieldname)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    @browser.get("#{@sut_url}/content/#{webform_title}")
    Log.logger.info("Opening edit webform overlay")
    wait.until { @browser.find_element(:xpath => @webformmgr.edit_webform_link) }.click
    frame = wait.until { @browser.find_element(:xpath => @webformmgr.overlay_frame) }
    @browser.switch_to.frame(frame)
    element_id = @browser.find_element(:xpath => "#{@webformmgr.fieldset_label(fieldname)}/..").attribute("id")
    @browser.switch_to.default_content
    return element_id
  end

  def get_submission_access_settings(webform_title, role)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    @browser.get("#{@sut_url}/content/#{webform_title}")
    Log.logger.info("Opening edit webform overlay")
    wait.until { @browser.find_element(:xpath => @webformmgr.edit_webform_link) }.click
    frame = wait.until { @browser.find_element(:xpath => @webformmgr.overlay_frame) }
    @browser.switch_to.frame(frame)
    Log.logger.info("Getting webform submission access settings")
    self.expand_field_to_edit(@webformmgr.form_submission_access_settings, @webformmgr.expanded_form_submission_access_settings)
    element_id = @browser.find_element(:xpath => "#{@webformmgr.fieldset_label(fieldname)}/..").attribute("id")
    @browser.switch_to.default_content
    return element_id
  end

  # GUI MAP

  class WebformManagerGM
    attr_reader :content_title, :webform_title, :save_webform_btn
    attr_reader :status_message
    attr_reader :formatted_content_fieldset
    attr_reader :multiline_rows_chkbox, :multiline_cols_chkbox, :multiline_edit_rows, :multiline_edit_cols
    attr_reader :label_display, :default_value_chkbox, :default_value, :desc_chkbox, :desc_textarea
    attr_reader :properties_link, :display_link, :validation_link, :submit_field, :prefix_chkbox
    attr_reader :suffix_chkbox, :size_chkbox, :edit_prefix, :edit_suffix, :edit_size
    attr_reader :required_chkbox
    attr_reader :webform_component, :next_page
    attr_reader :view_tab, :sort_asc, :overlay_frame
    attr_reader :delete_webform_component
    attr_reader :markup_textarea, :markup_text_format
    attr_reader :collapsed_chkbox, :collapsible_chkbox
    attr_reader :edit_rows, :edit_cols, :rows_chkbox, :cols_chkbox
    attr_reader :options_link, :add_option
    attr_reader :expanded_display, :expanded_validation, :expanded_properties
    attr_reader :file_upload_size
    attr_reader :active_delete_link
    attr_reader :results_link, :previous_submissions_link, :view_first_webform_result, :submissions_info_header
    attr_reader :back_to_webform_link
    attr_reader :poll_choice_one, :poll_choice_two, :forum_type
    attr_reader :webform_advanced_link, :advanced_settings_collapsed, :advanced_settings_exapnded
    attr_reader :excel_format_button, :delimited_text_format_button
    attr_reader :cookies_chkbox, :delimited_text_separator
    attr_reader :edit_webform_link, :delete_btn, :confirm_delete, :cancel_link
    attr_reader :customized_message_chkbox, :redirect_page, :simplified_message_textbox
    attr_reader :simplified_text_format, :submission_limit_chkbox, :submission_limit_textbox
    attr_reader :submission_limit_timeframe, :form_submission_settings, :form_submission_access_settings
    attr_reader :form_advanced_settings, :anonymous_user_chkbox, :authenticated_user_chkbox, :administrator_chkbox, :site_owner_chkbox
    attr_reader :blogger_chkbox, :editor_chkbox
    attr_reader :block_chkbox, :teaser_chkbox, :prev_submissions_chkbox
    attr_reader :expanded_form_advanced_settings, :expanded_form_submission_access_settings
    attr_reader :redirect_url_textbox, :verify_spam_protection, :confirmation_msg_chkbox, :mollom_chkbox
    attr_reader :disabled_anonymous_user_chkbox, :text_format, :customized_message_textbox
    attr_reader :publishing_options_link, :form_settings_link, :publish_content_chkbox, :promote_content_chkbox

    def fieldset_link(fieldname)
      return "//a[contains(text(), '#{fieldname}')]"
    end
    def fieldset_label(labelname)
      return "//label[contains(text(), '#{labelname}')]"
    end
    def new_fieldset(name)
      return "//span[contains(text(), '#{name}')]"
    end
    def pagebreak_hidden_fieldset(fieldname)
      return "//div[contains(text(), '#{fieldname} -')]"
    end
    def labeltitle(fieldname)
      return "//input[(@id = 'edit-title') and (@value = '#{fieldname}')]"
    end
    def required_field_label(fieldname)
      return "//label[contains(text(), '#{fieldname}')]/span[contains(text(), '*')]"
    end
    def media_types_checkbox(type)   # Takes as argument name of the file upload type allowed for File Uploader
      return "//label[text() = '#{type} ']/preceding-sibling::input[1]"
    end
    def checkbox_id(id, i)   # Takes as argument id of Checkbox and option number(i) of checkbox
      return "//div[@id = '#{id}']/div[#{i}]/input"
    end
    def chkbox_radio_option(id, val)    # Takes as argument id of Checkbox or radio and name(val) of option(as label) checkbox/radio
      return "//div[@id = '#{id}']//label[contains(text(), '#{val}')]"
    end
    def option_id(id)
      return "//input[@id = '#{id}']"
    end
    # Allowed content types are: article, page, poll, media-gallery, book, blog, forum, webform
    def content_chkbox(content_type)
      return "//input[@id='edit-node-types-#{content_type}']"
    end
    # Allowed content types are: email, file, fieldset, hidden, markup, pagebreak, select, textarea, textfield
    def component_chkbox(component_name)
      return "//input[@id='edit-components-#{component_name}']"
    end

    def user_role_chkbox(role)
      return "//label[contains(text(), '#{role}')]/preceding-sibling::input"
    end

    def initialize()
      @content_title = '//input[@id = "edit-title"]'
      @webform_title = '//input[@id = "edit-title"]'
      @save_webform_btn = '//input[@id = "edit-submit"]'
      @status_message = '//div[contains(@class, "messages status")]'
      @formatted_content_fieldset = '//strong[contains(text(), "New HTML Markup")]'
      @multiline_rows_chkbox = '//input[@id = "rows-checkbox"]'
      @multiline_cols_chkbox = '//input[@id = "cols-checkbox"]'
      @multiline_edit_rows = '//input[@id = "edit-rows"]'
      @multiline_edit_cols = '//input[@id = "edit-cols"]'
      @label_display = '//select[@id = "edit-title-display"]'
      @default_value_chkbox = '//input[@id = "default_value-checkbox"]'
      @default_value = '//input[@id="edit-default-value"]'
      @desc_chkbox = '//input[@id = "description-checkbox"]'
      @desc_textarea = '//textarea[@id = "edit-description"]'
      @properties_link = '//a[text() = "Properties"]'
      @display_link = '//a[text() = "Display"]'
      @validation_link = '//a[text() = "Validation"]'
      @options_link = '//a[text() = "Options"]'
      @expanded_display = '//a[text() = "Display" and @aria-expanded = "true"]'
      @expanded_properties = '//a[text() = "Properties" and @aria-expanded = "true"]'
      @expanded_validation = '//a[text() = "Validation" and @aria-expanded = "true"]'
      @submit_field = '//a[text() = "Submit"]'
      @prefix_chkbox = '//input[contains(@id, "field_prefix-checkbox")]'
      @suffix_chkbox = '//input[@id = "field_suffix-checkbox"]'
      @size_chkbox = '//input[@id = "size-checkbox"]'
      @edit_prefix = '//input[@id = "edit-field-prefix"]'
      @edit_suffix = '//input[@id="edit-field-suffix"]'
      @edit_size = '//input[@id = "edit-size"]'
      @required_chkbox = '//input[@id = "edit-required"]'
      @view_tab = '//a[text() = "View"]'
      @webform_component = '//*[contains(@id, "webform-component-new-")]'   # * is used to accomodate fieldset
      @next_page = '//input[@id = "edit-next"]'
      @sort_asc = '//table[contains(@class, "sticky-enabled")]//th[2]/a[@title="sort by Submitted"]'
      @overlay_frame = '//iframe[contains(@class, "overlay-element overlay-active")]'
      @delete_webform_component = '//a[text() = "Delete"]'
      @markup_text_format = '//select[@id = "edit-markup-format"]'
      @markup_textarea = '//textarea[@id = "edit-markup-value"]'
      @collapsible_chkbox = '//input[@id = "edit-collapsible"]'
      @collapsed_chkbox = '//input[@id = "edit-collapsed"]'
      @rows_chkbox = '//input[@id = "rows-checkbox"]'
      @cols_chkbox = '//input[@id = "cols-checkbox"]'
      @edit_rows = '//input[@id = "edit-rows"]'
      @edit_cols = '//input[@id = "edit-cols"]'
      @add_option = '//a[text() = "Add item"]'
      @file_upload_size = '//input[@id = "edit-webform-file-filtering-size"]'
      @active_delete_link = '//div[contains(@class, "form-builder-wrapper form-builder-active")]//a[text() = "Delete"]'
      @previous_submissions_link = '//a[text()="View your previous submissions"]'
      @results_link = '//a[text()="Results"]'
      @view_first_webform_result = '//table[contains(@class, "sticky-enabled")]/tbody/tr[1]/td[5]/a[text() = "View"]'   # tr[1] represents first result
      @submissions_info_header = '//h5[text() = "Submission information"]'
      @back_to_webform_link = '//a[text()="Go back to the form"]'
      @poll_choice_one = '//input[@id="edit-choice-new1-chtext"]'
      @poll_choice_two = '//input[@id="edit-choice-new0-chtext"]'
      @forum_type = '//select[@id="edit-taxonomy-forums-und"]'
      @webform_advanced_link = '//a[text()="Advanced"]'
      @advanced_settings_collapsed = '//a[text()="Advanced"]/span[text()="Show"]'
      @advanced_settings_exapnded = '//a[text()="Advanced"]/span[text()="Hide"]'
      @excel_format_button = '//input[@id="edit-webform-export-format-excel"]'
      @delimited_text_format_button = '//input[@id="edit-webform-export-format-delimited"]'
      @cookies_chkbox = '//input[@id="edit-webform-use-cookies"]'
      @delimited_text_separator = '//select[@id="edit-webform-csv-delimiter"]'
      @edit_webform_link = '//a[text()="Edit"]'
      @delete_btn = '//input[@id="edit-delete"]'
      @confirm_delete = '//input[@id="edit-submit"]'
      @cancel_link = '//a[@id="edit-cancel"]'
      @customized_message_chkbox = '//input[@id="edit-confirm-redirect-toggle"]'
      @redirect_page = '//select[@id="edit-redirect"]'
      @simplified_message_textbox = '//textarea[@id="edit-confirmation-page-content-value"]'
      @simplified_text_format = '//select[@id="edit-confirmation-page-content-format"]'
      @text_format = '//select[@id="edit-confirmation-format"]'
      @customized_message_textbox = '//textarea[@id="edit-confirmation-value"]'
      @submission_limit_chkbox = '//input[@id="edit-submit-limit-toggle"]'
      @submission_limit_textbox = '//input[@id="edit-submit-limit--2"]'
      @submission_limit_timeframe = '//select[@id="edit-submit-interval"]'
      @form_submission_settings = '//fieldset[@id="edit-submission"]'
      @form_submission_access_settings = '//fieldset[@id="edit-role-control"]'
      @form_advanced_settings = '//fieldset[@id="edit-advanced"]'
      @anonymous_user_chkbox = '//label[contains(text(), "anonymous user")]/preceding-sibling::input'
      @authenticated_user_chkbox = '//label[contains(text(), "authenticated user")]/preceding-sibling::input'
      @blogger_chkbox = '//label[contains(text(), "blogger")]/preceding-sibling::input'
      @editor_chkbox = '//label[contains(text(), "editor")]/preceding-sibling::input'
      @administrator_chkbox = '//label[contains(text(), "administrator")]/preceding-sibling::input'
      @site_owner_chkbox = '//label[contains(text(), "site owner")]/preceding-sibling::input'
      @block_chkbox = '//input[@id="edit-block"]'
      @teaser_chkbox = '//input[@id="edit-teaser"]'
      @prev_submissions_chkbox = '//input[@id="edit-submit-notice"]'
      @expanded_form_advanced_settings = '//fieldset[@id="edit-advanced" and not(contains(@class, "collapsed"))]'
      @expanded_form_submission_access_settings = '//fieldset[@id="edit-role-control" and not(contains(@class, "collapsed"))]'
      @redirect_url_textbox = '//input[@id="edit-redirect-url"]'
      @verify_spam_protection = '//em[text()="anonymous user"]'
      @confirmation_msg_chkbox = '//input[@id="edit-confirmation-toggle"]'
      @mollom_chkbox = '//input[@id="edit-spam-protection"]'
      @disabled_anonymous_user_chkbox = '//div[contains(text(), "spam protection on")]'
      @publishing_options_link = '//strong[text()="Publishing options"]'
      @publish_content_chkbox = '//input[@id="edit-status"]'
      @promote_content_chkbox = '//input[@id="edit-promote"]'
      @form_settings_link = '//strong[text()="Form settings"]'

    end
  end

end
