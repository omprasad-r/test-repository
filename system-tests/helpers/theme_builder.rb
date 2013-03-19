# -*- coding: utf-8 -*-
$LOAD_PATH << File.dirname(__FILE__)
require "acquia_qa/log"
require 'acquia_qa/ssh'
require 'jquery.rb'
require "qa_backdoor.rb"
require 'selenium/webdriver'
require 'uri'
require 'open-uri'

class ThemeBuilder
  include Acquia::SSH

  attr_reader :export_path

  def self.ensure_open_and_close_tb(browser, options = {})
    themer = ThemeBuilder.new(browser)
    themer.start_theme_builder(true, browser)
    begin
      yield(themer)
    rescue Exception => e
      caught_exception = e
      Log.logger.warn("Caught an exception: #{e}\n#{e.backtrace.join("\n")}")
    ensure
      if themer.nil?
        Log.logger.warn("'themer' is nil, I'm not even trying to close tb in this situation...")
      else  
        if caught_exception
          Log.logger.info("Trying to at least close TB after the exception.")
        else
          Log.logger.info("Trying to close the Themebuilder.")
        end
        #if this crashes, I really don't care anymore
        begin
          themer.exit_theme_builder
        rescue Exception
          Log.logger.info("Ensuring the TB exit failed. Meh.")
        end
      end
      #don't lose that exception
      raise caught_exception if caught_exception
    end
  end

  def self.layouts
    [ 'abc', 'acb', 'cab', 'ac', 'bc', 'ca', 'cb', 'c' ]
  end

  def self.detect_layout(_page = nil, browser = @browser)
    #go to the requested page first
    raise "No browser object!" unless browser
    
    browser.get($config['sut_url'] + _page) unless _page.nil?
    
    Log.logger.info("Waiting for body element with an attribute 'class' that contains the word layout.")
    #"html front logged-in no-sidebars page-node webkit webkit5 mac themebuilder toolbar theme-markup-2 body-layout-fixed-ac"
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    begin
      wait.until { browser.find_element(:xpath => "//body[contains(@class, 'layout')]") }
    rescue
      raise "Waiting for 'layout' in body class failed! '#{@browser.find_element(:xpath => "//body").attribute("class") rescue nil}'"
    end
    html_body_class = browser.find_element(:xpath => "//body").attribute("class")
    match = html_body_class.to_s.match(/body-layout-fixed-([a-z]{1,3})/)
    if match
      layout = match[1]
    else
      match = html_body_class.to_s.match(/body-layout-([a-z]{1,3})/)
      raise "No layout class in body: #{html_body_class.inspect}" unless match
      layout = match[1]
    end
    #"abc"
    Log.logger.info("Detected Layout class: #{layout.inspect}.")      
    layout
  end
  
  
  def initialize(_browser,_url = nil)
    @browser = _browser
    @export_path = '/tmp'
    @layouts = ThemeBuilder.layouts

    @active_tab = nil # latest tab picked by the user
    @active_tab_xpath = "//div[@id='themebuilder-main']/ul/li[contains(@class, 'ui-tabs-selected')]"
    @sut_url = _url || $config['sut_url'] 
    @themes_tab = 
    {'tab_name' => 'Themes',#not an element locator, it simply gives a name to this hash
      'jQselector' => "div[id='themebuilder-themes']",#  jQuery selector
      'xpath' =>  "//div[@id='themebuilder-themes']",
      'tab_not_ready' => nil,
      'My Themes' => 'link=My themes',
      'Gardens' => 'link=Gardens*'

    }

    # http://stackoverflow.com/questions/485151/ruby-how-can-i-get-a-reference-to-a-method might be useful
    ###>> foo = Hash.new
    #=> {}
    #>> def bar(_arg)
    #>> puts _arg
    #>> end
    #>> bar 'nik'
    #nik
    #=> nil
    #>>  foo = {:baz => method(:bar)}
    #=> {:baz=>#<Method: Object#bar>}
    #>> foo[:baz].call('argh')
    #argh
    #=> nil
    #>> 

    @layout_tab = 
    {'tab_name' => 'Layout',
      'jQselector' => "div[id='themebuilder-layout']",
      'xpath' => "//div[@id='themebuilder-layout']",
      'tab_not_ready' => nil
    }

    @styles_tab = 
    {'tab_name' => 'Styles',
      'jQselector' => "div[id='themebuilder-style']",
      'xpath' => "//div[@id='themebuilder-style']",
      'tab_not_ready' => 'css=#themebuilder-wrapper .disable-controls-veil',
      #    'Apply to'=>"//div[@id='element-to-edit']/select",
      'Font' => "link=Font",
      'font-family' => "//select[@id='style-font-family']",
      'font_families' => {'web_safe' => ['Arial', 'Courier', 'Times', 'Palatino', 'Lucida Sans', 'Bradley Hand', 'Georgia', 'Monaco'],
        'font_face' =>['Aaargh', 'ChunkFive', 'GoodDog', 'Lane', 'FreeUniversal', 'CartoGothic', 'CartoGothic Bold', 'CartoGothic Italic']},
        'font-color' => "style-font-color",
        'font-size' =>'style-font-size',
        'font-style' =>'style-font-style',
        'font-weight' =>'style-font-weight',
        'text-decoration' =>'style-text-decoration',
        'text-transform' =>'style-font-transform',
        'Spacing' => 'link=Borders & Spacing',
        'BorderColor' => 'style-border-color',
        'margin' => "//*[text()='margin']",
        'margin-top' => 'tb-style-margin-top',
        'margin-bottom' => 'tb-style-margin-bottom',
        'margin-left' => 'tb-style-margin-left',
        'margin-right' => 'tb-style-margin-right',
        'border' => "//*[text()='border']",
        'border-top-width' => 'tb-style-border-top-width',
        'border-bottom-width' => 'tb-style-border-bottom-width',
        'border-left-width' => 'tb-style-border-left-width',
        'border-right-width' => 'tb-style-border-right-width',
        'padding' => "//*[text()='padding']",
        'padding-top' => 'tb-style-padding-top',
        'padding-bottom' => 'tb-style-padding-bottom',
        'padding-left' => 'tb-style-padding-left',
        'padding-right' => 'tb-style-padding-right',
        'next-DOM-nav' =>   "//div[@title='Select the next sibling element']",
        'prev-DOM-nav' =>   "//div[@title='Select the previous sibling element']",
        'parent-DOM-nav' => "//div[@title='Select the parent element']",
        'fchild-DOM-nav' => "//div[@title='Select the first child element']",
        'background' => "link=Background",
        'background-color' => "style-background-color",
        'redo-button' => "//div[@id='themebuilder-save']/a[contains(@class, 'themebuilder-redo-button')]",
        'undo-button' => "//div[@id='themebuilder-save']/a[contains(@class, 'themebuilder-undo-button')]",

      }

      @advanced_tab = 
      {'tab_name' => 'Advanced',
        'jQselector' => "div[id='themebuilder-advanced']",
        'xpath' => "//div[@id='themebuilder-advanced']",# is the contents of this tab, contents are not visible if the tab is not selected
        'tab_not_ready' => nil,
        'Update' => "//button[@class='themebuilder-advanced-update-button']",
        'CSS'=>"//a[@href='#themebuilder-advanced-css']",
        'CSS textarea' => "//textarea[@id='css_edit']",
        'CSS history' => "//a[@href='#themebuilder-advanced-history']",
        'CSS rule' => "//div[@id='css-history']//tr",
        'CSS hideall' => "#history-hide-all",
        'CSS showall' => "#history-show-all",
        'CSS deleteall' => "#history-delete-all-hidden"
      }

      @theme_editor = {'start_btn' => "//a[@id='toolbar-link-admin-appearance']",
        'exit_btn' => "themebuilder-exit-button",
        'undo_btn' => "link=Undo",#"//button[@onclick='ThemeBuilder.undo()']"
        'Themes' => {'locator' => 'link=Themes','GUI' => @themes_tab},# these refer to tab handles and not their contents, these handles are always visible
        'Layout' => {'locator' => 'link=Layout','GUI' => @layout_tab},
        'Styles' => {'locator' => 'link=Styles', 'GUI' => @styles_tab},
        'Advanced' => {'locator' => 'link=Advanced', 'GUI' => @advanced_tab},
        'save_as_btn' => "//div[@id='themebuilder-save']/button[contains(@class,'save-as')]",
        'save_btn' => "//div[@id='themebuilder-save']/button[contains(@class,'save') and not(@disabled)]",
        'disabled_save_btn' => "//div[@id='themebuilder-save']/button[contains(@class,'save') and @disabled]",
        'publish_btn' => "//div[@id='themebuilder-save']/button[contains(@class,'publish')]",
        'export_btn' => "//div[@id='themebuilder-save']/a[contains(@class,'export')]",
        'save_name_box' => "//form[@id='themebuilder-bar-save-form']/.//input[contains(@id,'edit-name')]",
        'publish_name_box' => "//form[@id='themebuilder-bar-publish-form']/.//input[contains(@id,'edit-name')]",
        'export_name_box' => "//form[@id='themebuilder-bar-export-form']/.//input[contains(@id,'edit-name')]",
        'sys_name_box' => 'edit-system-name',
        'export_sys_name_box' => 'edit-system-name-2',
        'ok_save_as' => "//button[contains(@type,'button')]", #TODO: FIX THIS BUTTON. WTF!
        'ok_export' => "//button[contains(@type,'button')]", #TODO: FIX THIS BUTTON. WTF!
        'tb_status' =>"themebuilder-status",
        'tb_veil' =>"//div[@id='themebuilder-veil']",
        'tb_theme_name' => "//div[@id='themebuilder-theme-name']/span[contains(@class,'theme-name')]",
        'tb_last_saved_text' => "//div[@id='themebuilder-theme-name']/span[@class='last-saved']",
        'tb_control_veil_id' => "themebuilder-control-veil",
        'tb_throbber_css' => "div.themebuilder-loader",
        'tb_live_status_msg' => "//div[@id='themebuilder-status']/span[contains(@class,'themebuilder-status-message') and contains(text(), 'is now live')]",
        'tb_published_status_msg' => "//div[@id='themebuilder-status']/span[contains(@class,'themebuilder-status-message') and contains(text(), 'was successfully published.')]",
        'tb_copied_saved_msg' => "//div[@id='themebuilder-status']/span[contains(@class,'themebuilder-status-message') and contains(text(), 'was successfully copied and saved.')]"
      }

      @tabs = {}
      @tabs["Layout"] = @layout_tab
      @tabs["Styles"] = @styles_tab
      @tabs["Advanced"] = @advanced_tab


    end

    def layouts
      ThemeBuilder.layouts
    end
    # _layout  acb, abc, ac,bc,c ...
    # _this_page_only boolean
    def select_layout(_layout, _this_page_only=false)
      wait = Selenium::WebDriver::Wait.new(:timeout => 10)
      Log.logger.debug("Selecting layout #{_layout}")
      self.switch_tab('Layout')
      Log.logger.info("Setting layout #{_layout}")
      layout = "fixed-#{_layout}"
      layout_path = layout_path_selector(layout)
      if _this_page_only
        apply_to = 'single'
      else
        apply_to ='all'
      end
      page_selector = this_page_selector(layout, apply_to)    
      Log.logger.info("Attempting to bring the layout into view.")
      bring_theme_into_view(layout_path, @browser)
      JQuery.wait_for_events_to_finish(@browser)
      Log.logger.info("Clicking on Layout option #{layout.inspect} (#{layout_path.inspect}).")
      begin
        wait.until { @browser.find_element(:xpath => layout_path) }.click
        JQuery.wait_for_events_to_finish(@browser)    
        wait.until { @browser.find_element(:xpath => page_selector) }.click
        JQuery.wait_for_events_to_finish(@browser)    
      rescue Exception => e
        Log.logger.info("Caught exception: #{e.inspect}. Attempting to select layout via JS...")
        begin
          js_select_layout(@browser,layout_path,page_selector)
        rescue Exception => e
          Log.logger.info("JS failed: #{e.inspect}")
          res = @browser.execute_script(tb_select(_layout,apply_to))
          raise "Couldn't select the layout even using the TB API." if (res == false)
        end
      end
      JQuery.wait_for_events_to_finish(@browser)    
      #It seems to take a while for the browser to put the new layout into the body tag
      Log.logger.info("Waiting for Layout to appear in html body class")
      begin
        wait.until { @browser.find_element(:xpath => "//body[contains(@class, '#{_layout}')]") }
      rescue StandardError => e
        html_body_class = @browser.find_element(:xpath => "//body").attribute("class")
        raise "After selecting the layout #{_layout.inspect}, the HTML body class didn't contain any layout information: #{html_body_class.inspect}"
      end  
      #TODO: Replace this with a proper wait for something
      sleep 3
    end

    def tb_select(_layout,_apply_to)
      script = <<-SCRIPT
      if ((typeof window.ThemeBuilder != 'undefined') && (typeof window.ThemeBuilder.LayoutEditor != 'undefined') && (typeof window.ThemeBuilder.LayoutEditor.getInstance() != 'undefined')){
        window.ThemeBuilder.LayoutEditor.getInstance().pickLayout(\'#{_layout}\',\'#{_apply_to}\');
        return true;
      }
      return false;
      SCRIPT
      script
    end

    def layout_path_selector(_layout)
      "//div[@onclick=\"ThemeBuilder.LayoutEditor.getInstance().pickLayout(\'#{_layout}\');\"]"
    end

    def this_page_selector(_layout, _apply_to)
      "//div[@onclick=\"ThemeBuilder.LayoutEditor.getInstance().pickLayout(\'#{_layout}\',\'#{_apply_to}\');return false;\"]"
    end

    def js_select_layout(browser,layout,page_selector)
      counter = 0
      l = browser.find_element(:xpath => layout) 
      script = "arguments[0].click(); return true;"
      while counter < 5
        res = @browser.execute_script(script, l)    
        break if (res == true)
        qualify_result(res)
        counter += 1
        @browser.navigate.refresh if counter == 4
        raise "Failed to select the layout via JS!" if counter == 5
      end
      JQuery.wait_for_events_to_finish(@browser)    
      Log.logger.info("Clicking on page selector: #{page_selector}")
      elem = wait.until { @browser.find_element(:xpath => page_selector) }
      script = "arguments[0].click(); return true;"
      counter = 0
      while counter < 5
        res = browser.execute_script(script,elem)    
        break if (res == true)
        qualify_result(res)
        counter += 1
        sleep 1
        raise "Failed to select layout applicator!" if counter == 5
      end
      Log.logger.info("Selected the layout via JS!")
    end

    private :js_select_layout

    def active_tab
      wait = Selenium::WebDriver::Wait.new(:timeout => 10)
      text = "" 
      begin
        text = wait.until { @browser.find_element(:xpath => @active_tab_xpath) }.text
      rescue
        Log.logger.info("Couldn't find an active tab....defaulting to layout")
        text = "Layout"
      end
      return text
    end

    def detect_layout(_page=nil, browser = @browser)
      ThemeBuilder.detect_layout(_page, browser)
    end

    # force start will takeover other active TB sessions.  this is helpful when 
    # a test crashes
    def start_theme_builder(force_start=true, browser=nil)
      force_start = true
      browser = @browser unless browser
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      begin
        JQuery.wait_for_events_to_finish(browser)
        Log.logger.debug("Starting themebuilder")
        Log.logger.info("Waiting for themebuilder button")
        start_btn = wait.until { browser.find_element(:xpath => @theme_editor['start_btn']) }
        wait.until { start_btn.displayed? }
        Log.logger.info("Clicking on themebuilder button")
        start_btn.click

        if force_start
          begin 
            alert = browser.switch_to.alert
            wait.until { alert.displayed? }
            conf_mess = alert.text
            alert.accept
            Log.logger.debug("Looks like there was already an open theme session, overwriting #{conf_mess}")
          rescue Selenium::WebDriver::Error::NoAlertOpenError => e
            Log.logger.info("No confirmation dialog...continuing...")
          ensure
            browser.switch_to.default_content
          end
        end
      rescue StandardError, Selenium::WebDriver::Error::TimeOutError => e
        raise "Error while waiting for themebuilder to load: #{e.message}\n#{e.backtrace}"
      end

      confirmation_text = check_for_confirmation(browser, force_start)
      if confirmation_text and not force_start
        Log.logger.warn("You didn't force the start and there was a confirmation. TB NOT STARTED!")
        $tb_open=false
      else
        wait_for_theme_builder #makes sure theme builder is loaded
        $tb_open=true
      end
      confirmation_text
    end

    def check_for_confirmation(browser, click_ok)
      wait = Selenium::WebDriver::Wait.new(:timeout => 3)
      begin
        dialog = wait.until { browser.find_element(:xpath => "//div[@id='themebuilder-confirmation-dialog']") }
        confirmation_text = dialog.text
        Log.logger.info("Found confirmation: #{confirmation_text.inspect}")
        if click_ok
          Log.logger.info("Clicking 'OK'")
          browser.find_element(:xpath => "//button/span[text()='OK']").click
        else
          Log.logger.info("Clicking 'Cancel'")
          browser.find_element(:xpath => "//button/span[text()='Cancel']").click
        end
      rescue Exception => e
        confirmation_text = nil
        Log.logger.info("No confirmation dialog found.")
      end
      confirmation_text
    end

    # waits for default elemets on the theme builder to be loaded
    # we assume the entire theme builder should then be good to use, we could be wrong 
    def wait_for_theme_builder(browser = @browser)
      #We'll wait for the control veil to be there and then be lifted. After that the TB should be good to go.
      Log.logger.info("Making sure the themebuilder is ready (-> the veil should be insivible).")
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      begin
        wait.until { browser.find_elements(:id => @theme_editor['tb_control_veil_id']).size > 0 }
      rescue
        Log.logger.info "Timeout while waiting for themebuilder veil (took longer than 5 seconds)."
      end
      begin
        wait.until { browser.find_elements(:id => @theme_editor['tb_control_veil_id']).empty? || (!browser.find_element(:id => @theme_editor['tb_control_veil_id']).displayed?) }
      rescue 
        Log.logger.info "Timeout while waiting for themebuilder veil to become invisible  (took longer than 30 seconds after detecting the veil)."
      end
      JQuery.wait_for_events_to_finish(browser)
    end

    def theme_builder_present?(browser = @browser)
      browser.find_elements(:xpath => "//body[contains(@class, 'themebuilder')]").size > 0
    end

    def exit_theme_builder(forced = false)
      Log.logger.info("Exiting Themebuilder (forced: #{forced.inspect})")
      # If/Else Control structure was added here...it is more verbose, but it shouldn't cause the 
      # program to hang the way @browser.element? does when called from within Debug statements
      if theme_builder_present?
        wait_for_theme_builder
        Log.logger.info("Waiting for exit button.")
        wait = Selenium::WebDriver::Wait.new(:timeout => 15)
        begin
          @browser.switch_to.default_content
          exit_btn = wait.until { @browser.find_elements(:id => @theme_editor['exit_btn']) }
        rescue
          Log.logger.info("Couldn't find exit button...")
          ### EMERY TODO!!!!!
        end
        if exit_btn.empty?
          Log.logger.info("No exit button...TB appears to be closed.")
          $tb_open = false
          return
        end
        #check if we should expect a popup box
        if @browser.find_elements(:css => '.theme-modified').size > 0
          possible_confirmation_box = true
          Log.logger.info("We can expect a confirmation box when closing.")
        end
        Log.logger.info("Clicking exit button.")
        wait.until { @browser.find_element(:id => @theme_editor['exit_btn']) }.click
        2.times { JQuery.wait_for_events_to_finish(@browser) }
        sleep 2
        short_wait = Selenium::WebDriver::Wait.new(:timeout => 3)
        found_conf = false
        if possible_confirmation_box
          begin
            alert = wait.until { @browser.switch_to.alert }
            short_wait.until { alert.displayed? }
            conf_mess = alert.text
            alert.accept   ### EMERY TODO : FIGURE OUT IF ACCEPT / DISMISS have differing behaviors
            Log.logger.info("Looks like that theme has already unsaved changes, pressing ok on message: #{conf_mess.inspect}")
            found_conf = true
            @browser.switch_to.default_content
          rescue
            Log.logger.info("Didn't find a confirmation box...")
            consume_possible_confirmation(@browser)
          ensure
            @browser.switch_to.default_content
          end
        end     

        if forced and not found_conf
          begin
            alert = short_wait.until { @browser.switch_to.alert }
            short_wait.until { alert.displayed? }
            conf_mess = alert.text
            alert.accept   ### EMERY TODO : FIGURE OUT IF ACCEPT / DISMISS have differing behaviors
            Log.logger.info("Looks like that theme has already unsaved changes, pressing ok on message: #{conf_mess.inspect}")
            @browser.switch_to.default_content
          rescue 
            Log.logger.info("Didn't find an alert...")
          ensure
            @browser.switch_to.default_content
          end
        end

        #      check_for_confirmation(@browser,true)
        #      consume_possible_confirmation(@browser)

        ####
        #### TODO EMERY : DETERMINE IF NEED TO WAIT FOR PAGELOAD HERE...

      else
        Log.logger.info("Themebuilder was already closed.")
      end
      $tb_open = false
      Log.logger.info("Themebuilder exited sucessfully")
      sleep 0.5
    end

    def unsafe_exit_theme_builder
      Log.logger.info("Doing an unsafe themebuilder exit")
      exit_theme_builder(forced = true)
    end

    def undo
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      begin
        undo_btn = wait.until { @browser.find_element(:xpath => @theme_editor['undo_btn']) }
        wait.until { undo_btn.displayed? }
        undo_btn.click
      rescue
        Log.logger.info("Failed while attempting to undo TB related changes...")
      end
    end

    # will save a theme to a specific name.  
    # TODO: the overwrite is not quire right.  If 
    # cancel is chosen, the status bar will not bump.
    def save_theme_as(themeName, overwrite=true)
      Log.logger.info("Saving theme as #{themeName}")
      #click on the save as button
      Log.logger.info("Waiting for 'save as' button.")
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      begin
        save_as = wait.until { @browser.find_element(:xpath => @theme_editor['save_as_btn']) }
        wait.until { save_as.displayed? }
        Log.logger.info("Clicking on 'save as' button.")
        save_as.click
      rescue
        Log.logger.info("Failed while waiting on save_as button...")
      end

      #write desired theme name
      Log.logger.info("Waiting for save name box.")

      begin
        name_box = wait.until { @browser.find_element(:xpath => @theme_editor['save_name_box']) }
        Log.logger.info("Entering name: #{themeName.inspect}.")
        wait.until { name_box.displayed? }
        name_box.clear
        name_box.send_keys(themeName)
      rescue
        Log.logger.info("Failed while waiting for name box...")
      end

      #press the ok button
      begin
        ok_save = wait.until { @browser.find_element(:xpath => @theme_editor['ok_save_as']) }
        wait.until { ok_save.displayed? }
        ok_save.click
      rescue
        Log.logger.info("failed while trying to ok the save")
      end

      #   sleep 3 # why oh why do I haveto sleep!!!

      begin
        alert = @browser.switch_to.alert
        conf_mess = alert.text
        #####
        ### TODO : DETERMINE IF THIS BLOCK WORKS APPROPRIATELY
        if overwrite
          Log.logger.debug("Looks like there was already a theme with that name saved, overwriting! (confirmation message was: #{conf_mess})")
          alert.accept
        else
          Log.logger.info("Got an alert, and we are not going to override it: #{conf_mess}")
          alert.accept
        end
        @browser.switch_to.default_content
      rescue
        Log.logger.info("Blew up while trying to consume an alert...")
        @browser.switch_to.default_content
      end

      # save only happens if overwrite is chosen.
      self.status_bar_bumped? if overwrite

      begin
        wait.until { @browser.find_element(:xpath => @theme_editor['tb_copied_saved_msg']) }
        Log.logger.debug("Waiting for status message to disappear")
        start_time = Time.now
        wait.until { @browser.find_elements(:xpath => "//div[@id='themebuilder-status' and not(contains(@style, 'display: none;'))]/span[@class='themebuilder-status-message']").empty? }
        Log.logger.debug("Status message is gone (after #{Time.now - start_time} s)")
      rescue
        Log.logger.info("Exploded while waiting for status message to disappear...")
      end
    end

    # Saves the current theme. Not sure why themename might be passed. 
    def save_theme(themeName = '', acceptpublish=true)
      # make sure that veil is not enabled
      self.wait_for_theme_builder
      #click on the save as button
      wait = Selenium::WebDriver::Wait.new(:timeout => 8)
      begin
        save_btn = wait.until { @browser.find_element(:xpath => @theme_editor['save_btn']) }
        wait.until { save_btn.displayed? }
        save_btn.click
      rescue Exception => e
        ### TODO
        Log.logger.info("Caught an error waiting on something...#{e.inspect}")
      end

      #sleep 1 # why oh why do I haveto sleep!!!

      begin
        alert = @browser.switch_to.alert
        conf_mess = alert.text
        Log.logger.debug("Looks like there was already a theme with that name saved, overwriting! (confirmation message was: #{conf_mess})")
        alert.accept if acceptpublish
        alert.accept unless acceptpublish
        @browser.switch_to.default_content
      rescue 
        @browser.switch_to.default_content
      end
      self.status_bar_bumped?
    end

    # publishes a theme of a specificed name
    #
    def publish_theme(themeName, overwrite=true)
      wait = Selenium::WebDriver::Wait.new(:timeout => 10)
      Log.logger.info("Publishing theme as #{themeName.inspect} (overwrite=#{overwrite.inspect})")

      themebuilder_exists = 'window.ThemeBuilder != undefined'
      app_data_exists = "(window.ThemeBuilder.getApplicationInstance() != undefined)"
      theme_exists = "(window.ThemeBuilder.Theme.getSelectedTheme() != undefined)"


      selected_theme_js = 'return window.ThemeBuilder.Theme.getSelectedTheme().getName()'
      wait_script = "return (#{themebuilder_exists} && #{app_data_exists} && #{theme_exists})"
      #We wait for all of our things to be there
      wait.until { @browser.execute_script(wait_script) }

      #When you select a base theme (campaign) and haven't saved it yet. 
      is_undefined = @browser.execute_script('return window.ThemeBuilder.Theme.getSelectedTheme()').nil?

      if is_undefined
        current_theme_name = "Undefined"
      else
        current_theme_name = @browser.execute_script(selected_theme_js)
      end

      Log.logger.info("Got the current theme name from the JS API: #{current_theme_name.inspect}.")     

      #Since we want to publish somethign that will be called named after our own themeName variable
      #We have to save the theme before hand if it has a different name.
      unless [themeName, "Undefined"].include?(current_theme_name)
        Log.logger.info("The currently active theme is called #{current_theme_name}. Calling the save_theme_as method so we're sure to publish the name we wanted #{themeName.inspect}.")
        save_theme_as(themeName, overwrite)
      end

      # make sure that veil is not enabled
      self.wait_for_theme_builder

      #<div id="themebuilder-theme-name">
      #  <span class="theme-name">test *</span>
      #  <span class="last-saved">Last saved
      #    <em class="placeholder">April 20th, 11:35am</em>
      #  </span>

      #click on the publish button
      publish_btn = wait.until { @browser.find_element(:xpath => @theme_editor['publish_btn']) }
      wait.until { publish_btn.displayed? }
      publish_btn.click

      if @browser.find_elements(:xpath => @theme_editor['tb_live_status_msg']).empty? # this seems counter-intuitive, but frankly, it happens.
        Log.logger.info("No Live Status Message after trying to publish theme...was the theme actually published/made live?")
      else
        begin
          #nobody asked us for a new name --> just a save, not a publish
          Log.logger.info("Waiting for the 'is now live' message")
          live_msg = wait.until { @browser.find_element(:xpath => @theme_editor['tb_live_status_msg']) }
          message_text = live_msg.text
          Log.logger.info("Found the 'is now live' message. Text in the message: #{message_text.inspect}.")
        rescue Selenium::WebDriver::Error::TimeOutError => e
          ### TODO
          Log.logger.info("Caught an error waiting for the 'is now live' message: #{e.inspect}")
        end
      end
      Log.logger.info("Finished publishing theme.")
      name = js_get_name(@browser)
      Log.logger.info("Theme name => #{name}")
      name
    end

    def js_get_name(browser)
      script = <<-SCRIPT
      if ((typeof window.ThemeBuilder != 'undefined') && (typeof window.ThemeBuilder.LayoutEditor != 'undefined') && (typeof window.ThemeBuilder.LayoutEditor.getInstance() != 'undefined')){
        var t_name = window.ThemeBuilder.Theme.getSelectedTheme().getName();
        return t_name;
      }
      return false;
      SCRIPT
      result = browser.execute_script(script)
      if result == false
        Log.logger.info("Failed to get the theme name!")
        result = 'fake_theme_name'
      else
        Log.logger.info("Current theme name is #{result}")
      end
      result
    end

    private :js_get_name

    def export_theme(themeName)
      Log.logger.debug("Exporting theme #{themeName}")
      #click on the save as button
      # make sure that veil is not enabled
      self.wait_for_theme_builder
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      begin
        export = wait.until { @browser.find_element(:xpath => @theme_editor['export_btn']) }
        wait.until { export.displayed? }
        export.click
        JQuery.wait_for_events_to_finish(@browser)
        name = wait.until { @browser.find_element(:xpath => @theme_editor['export_name_box']) }
        wait.until { name.displayed? }
        name.clear
        name.send_keys(themeName)
        JQuery.wait_for_events_to_finish(@browser)
        ok_exp = wait.until { @browser.find_element(:xpath => @theme_editor['ok_export']) }
        wait.until { ok_exp.displayed? }
        ok_exp.click
        JQuery.wait_for_events_to_finish(@browser)
        self.status_bar_bumped?
      rescue Exception => e
        ### TODO
        Log.logger.info("Caught an error waiting on something...#{e.inspect}")
      end
    end

    def open_theme(theme_type, theme_name, overwrite=true, basetheme_test = false, browser = @browser)
      Log.logger.info("Opening theme: Type #{theme_type.inspect} | Name: #{theme_name.inspect}")
      self.switch_tab('Themes')
      theme_sel = theme_name.downcase
      if (theme_type =~/Gardens/i)
        open_gardens_theme(theme_sel,overwrite,browser)
      else
        open_personal_theme(theme_sel,overwrite,browser)
      end
      sleep 2
      JQuery.wait_for_events_to_finish(browser)
      Log.logger.info("Theme should be opened...")
      JQuery.wait_for_events_to_finish(browser)
    end

    def open_gardens_theme(name,overwrite,browser)
      wait = Selenium::WebDriver::Wait.new(:timeout => 10)
      active_themes = "//div[contains(@id,'themebuilder-themes-') and (not(contains(@class, 'smart-hidden'))) and (not(contains(@id,'actions')))]"
      raise "We dont seem to be in the themes tab although we switched there ourselves. Did somebody redesign this section?" unless browser.find_elements(:xpath => active_themes).size > 0
      current_section = browser.find_element(:xpath => active_themes).attribute("id").gsub("themebuilder-themes-","").downcase
      case current_section
      when "featured"
        Log.logger.info("Featured themes section is open...good.")
      else
        Log.logger.info("Mythemes section is open...switching to the featured Gardens themes column")
        g_theme = active_themes.gsub("themebuilder-themes-","themebuilder-themes-featured")
        wait.until { browser.find_element(:xpath => "//a[contains(@class, 'action --Choose-a-new-theme')]") }.click
        JQuery.wait_for_events_to_finish(browser)
        consume_possible_confirmation(browser)
        JQuery.wait_for_events_to_finish(browser)
        wait.until { browser.find_element(:xpath => g_theme) }
        Log.logger.info("Successfully switched...")
      end
      consume_possible_confirmation(browser)
      select_wanted_theme(name,overwrite,browser,false)
      choose_button = "//ul[@id='themebuilder-actionlist']/.//a[contains(@class, 'action Choose themebuilder')]"
      Log.logger.info("About to click the choose button...")
      begin
        elem = wait.until { browser.find_element(:xpath => choose_button) }
        wait.until { elem.displayed? }
      rescue Selenium::WebDriver::Error::TimeOutError => e
        raise "Error while waiting for 'Choose' button to show up after opening what is supposed to be a Gardens theme"
      end
      begin
        JQuery.wait_for_events_to_finish(browser)
        wait.until { browser.find_element(:xpath => choose_button) }.click    
        JQuery.wait_for_events_to_finish(browser)  
      rescue StandardError => e
        raise "Error while clicking on the 'choose' button in themebuilder during opening a theme: #{e.message}"
      end
      save_temp_gardens_theme(browser)
      themebuilder_exists = 'return (window.ThemeBuilder != undefined);'
      app_data_exists = "return (window.ThemeBuilder.getApplicationInstance().applicationData != undefined);"
      base_theme_name_js = "return (window.ThemeBuilder.getApplicationInstance().applicationData['base_theme'] == \"#{name.downcase}\");" 
      wait.until { browser.execute_script(themebuilder_exists) && browser.execute_script(app_data_exists) && browser.execute_script(base_theme_name_js) }
      Log.logger.info("Got the data from the JS API (#{base_theme_name_js}).") 
      JQuery.wait_for_events_to_finish(browser)
    end

    private :open_gardens_theme

    def open_personal_theme(name,overwrite, browser = @browser)
      wait = Selenium::WebDriver::Wait.new(:timeout => 10)
      active_themes = "//div[contains(@id,'themebuilder-themes-') and (not(contains(@class, 'smart-hidden'))) and (not(contains(@id,'actions')))]"
      raise "We dont seem to be in the themes tab although we switched there ourselves. Did somebody redesign this section?" unless browser.find_elements(:xpath => active_themes).size > 0
      current_section = browser.find_element(:xpath => active_themes).attribute("id").gsub("themebuilder-themes-","").downcase
      case current_section
      when "mythemes"
        Log.logger.info("Mythemes section is open...good")
      else
        Log.logger.info("Featured themes section is open...switching to the personal themes column.")
        m_theme = active_themes.gsub("themebuilder-themes-","themebuilder-themes-mythemes")
        browser.find_element(:xpath => "//a[contains(@class, 'action Cancel themebuilder-button')]").click 
        begin
          wait.until { browser.find_element(:xpath => m_theme) }
        rescue Exception => e
          raise "Error while switching to the personal themes column: #{e.message}"
        end
        Log.logger.info("Successfully switched...")
      end
      consume_possible_confirmation(browser)
      select_wanted_theme(name,overwrite,browser,true)
      themebuilder_exists = 'return (window.ThemeBuilder != undefined);'
      app_data_exists = "return (window.ThemeBuilder.getApplicationInstance() != undefined);"
      theme_exists = "return (window.ThemeBuilder.Theme.getSelectedTheme() != undefined);"
      selected_theme_js = "return (window.ThemeBuilder.Theme.getSelectedTheme().getName() == \"#{name.downcase}\");"
      wait.until { browser.execute_script(themebuilder_exists) && browser.execute_script(app_data_exists) && browser.execute_script(theme_exists) && browser.execute_script(selected_theme_js) }
      Log.logger.info("Got the data from the JS API #{selected_theme_js}.")     
      JQuery.wait_for_events_to_finish(browser)
    end

    private :open_personal_theme

    ## TODO: GET RID OF THE NON-DETERMINISM OF THIS METHOD
    ##################
    ## POSSIBLE SOLUTION:
    ## def consume_confirmation(browser)
    ##   raise "No confirmation to consume!" unless browser.confirmation?
    ##   # consume confirmation here...
    ##
    ##################
    def consume_possible_confirmation(browser)
      begin
        alert = browser.switch_to.alert
        msg = alert.text
        Log.logger.info("Found a confirmation message, and consuming it: #{msg.inspect}")
        alert.accept
      rescue
        Log.logger.info("No alert present...carrying on")
      ensure
        browser.switch_to.default_content
      end
    end

    private :consume_possible_confirmation

    ###
    # Marc: I changed the behavior a bit, we will now crash when something goes wrong --> fail early!
    # Old behavior:
    # RETURNS TRUE if IT CANNOT FIND THE THEME IT DESIRES. THIS WILL TRIGGER AN EXCEPTION IN THE METHODS THAT CALL THIS ONE
    # RETURNS FALSE if THEME IS FOUND AND CLICKED (but offers no guarantees beyond that)
    def select_wanted_theme(name, overwrite, browser,flag)
      JQuery.wait_for_events_to_finish(browser)
      begin
        theme_name_path = "//li[contains(@class, 'jcarousel-item')]/.//div[@class='label']"
        clickable_text = "//li[contains(@class, 'jcarousel-item')]/.//div[@class='label' and 
        translate(text(), 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz') = '#{name.downcase}']"
        refreshes = 0
        until browser.find_elements(:xpath => clickable_text).size > 0
          Log.logger.info("Theme #{name.inspect} not there yet, refreshing")
          browser.navigate.refresh
          JQuery.wait_for_events_to_finish(browser)
          if (refreshes+=1) > 10
            amount_of_themes = browser.find_elements(:xpath => theme_name_path).size
            theme_names = []
            amount_of_themes.times do |index|
              theme_names << browser.find_element(:xpath => "(#{theme_name_path})[#{index+1}]").text
            end
            raise "Theme #{name.inspect} didn't show up. We found these #{theme_names.size} themes: #{theme_names.inspect}"
          end
        end
        wait = Selenium::WebDriver::Wait.new(:timeout => 10)
        wait.until {  browser.find_element(:xpath => clickable_text) }
        ########### BRING THE THEME INTO VIEW!
        begin
          bring_theme_into_view(clickable_text, browser)
          theme = wait.until {  browser.find_element(:xpath => clickable_text) }
          wait.until { theme.displayed? }
          theme.click
          JQuery.wait_for_events_to_finish(browser)
        rescue
          Log.logger.info("Failed to click the theme conventionally. Attempting to do so via JS....")
          js_click_theme(browser,clickable_text,flag)
        end
        JQuery.wait_for_events_to_finish(browser)
        Log.logger.info("Found the wanted theme (#{name.inspect}).")
        consume_possible_confirmation(browser) if overwrite
        #spinner appears, spinner disappears, page reloads
        JQuery.wait_for_events_to_finish(browser)
        ##TODO: see if more waiting needs to occur
      rescue Exception => e
        Log.logger.info("Couldn't find the wanted theme...")
        raise "Failed to wait for the theme named #{name} (#{e.inspect})"
      end
    end

    private :select_wanted_theme

    def js_click_theme(browser,xpath,flag)
      element = browser.find_element(:xpath => xpath) 
      script = "arguments[0].click(); return true;"
      counter = 0
      while counter < 5
        res = browser.execute_script(script, element)
        break if (res == true)
        qualify_result(res)
        counter += 1
        browser.navigate.refresh
        raise "JS couldn't find or click theme" if counter == 5
      end
      Log.logger.info("Clicked the theme via JS...")
    end

    private :js_click_theme

    def qualify_result(string)
      if string =~ /array/
        Log.logger.info("Didn't find our theme...returned an undefined array!")
      elsif string =~ /element/
        Log.logger.info("Didn't find our theme...element did not exist!")
      elsif string =~ /jQuery/
        Log.logger.info("Didn't find our theme...JQuery was undefined!")
      elsif string =~ /tag/
        Log.logger.info("Didn't find the layout...returned an undefined array!")
      elsif string =~ /option/
        Log.logger.info("Didn't find the layout...couldn't find the options buttons!")
      else
        Log.logger.info("Didn't find the item and didn't classify the result: #{res.inspect}!")
      end
    end

    private :qualify_result

    ########
    #TO BE CALLED AFTER THE THEME HAS BEEN FOUND IN THE DOM! 
    def bring_theme_into_view(theme_xpath,browser)
      Log.logger.info("Trying to bring theme into view.")
      wait = Selenium::WebDriver::Wait.new(:timeout => 10)
      next_button_css = "div.jcarousel-next-horizontal[disabled=false]"
      begin
        next_carousel = wait.until { browser.find_element(:css => next_button_css) }
      rescue Exception => e
        Log.logger.info("Couldn't find an active jcarousel next button. Maybe we're already in view. (#{e.inspect})")
        return
      end
      wanted_theme = wait.until { browser.find_element(:xpath => theme_xpath) }
      max_x, max_y = ThemeBuilder.get_document_bounds(browser)
      theme_location = wanted_theme.location
      theme_x = theme_location.x
      theme_y = theme_location.y
      init_x = theme_x
      init_y = theme_y
      counter = 0
      max_tries = 50
      until (theme_x < max_x) && (theme_y < max_y)  #make this strict
        counter += 1
        raise("Couldn't bring theme into view") if counter > max_tries  # this needs to be large :(
        Log.logger.info("Theme is not yet within the document bounds! Document size: #{max_x}/#{max_y}, Theme is at #{theme_x}/#{theme_y}")
        temp = wait.until { browser.find_element(:css => next_button_css) }
        if temp.attribute("disabled") == true
          Log.logger.info("The jcarousel button is now disabled. This means there is nothing more to scroll and the theme should be present!")
          break
        else
          begin
            Log.logger.info("[#{counter}/#{max_tries}] Attempting to click on the JCarousel next button...")
            browser.find_element(:css => next_button_css).click
          rescue Exception => e
            Log.logger.info("Caught an error while clicking the jcarousel: #{e.inspect}")
            begin 
              @browser.action.move_to(temp,"0","0").click(temp).perform
              #browser.find_element(:xpath => "//div/div/div/div[contains(@class,'jcarousel-next-horizontal')]").click
            rescue Exception => e
              Log.logger.info("Error while clicking jCarousel button #{e.inspect}")
              Log.logger.info("JCarousel button is acting wonky...attempting to click through JS :)")
              element = browser.find_element(:css => next_button_css)
              script = "arguments[0].click();"
              browser.execute_script(script, element)
            end
          end
          JQuery.wait_for_events_to_finish(browser)
          wanted_theme = wait.until { browser.find_element(:xpath => theme_xpath) }
          theme_location = wanted_theme.location
          if theme_location.x >= theme_x
            raise("We tried scrolling the element into view using the jcarousel, but our click didn't change the element's position.")
          end
          theme_x = theme_location.x
          theme_y = theme_location.y
        end
      end
      Log.logger.info("Theme should now be within the document bounds! Max_X => #{max_x}, Max_Y=> #{max_y}, Theme_X => #{theme_x}, Theme_Y => #{theme_y}")
      JQuery.wait_for_events_to_finish(browser)
    end

    private :bring_theme_into_view

    #####
    ## The methodology used here is designed to emulate the way webdriver determines document bounds....
    ## The webdriver methodology lives here (for now): 
    ##        http://code.google.com/p/selenium/source/browse/trunk/javascript/firefox-driver/extension/components/syntheticMouse.js?r=14337&spec=svn14337
    ####
    # def get_document_bounds(browser)
    #   return self.get_document_bounds(browser)
    # end

    def self.get_document_bounds(browser)
      #IM A CHEATER! JS IS FROM HERE: http://andylangton.co.uk/articles/javascript/get-viewport-size-javascript/
      viewport = <<-SCRIPT

      var viewportwidth;
      var viewportheight;

      if (typeof window.innerWidth != 'undefined')
        {
          viewportwidth = window.innerWidth ;
          viewportheight = window.innerHeight ;
        }
      else if (typeof document.documentElement != 'undefined'
        && typeof document.documentElement.clientWidth !=
        'undefined' && document.documentElement.clientWidth != 0)
        {
         viewportwidth = document.documentElement.clientWidth ;
         viewportheight = document.documentElement.clientHeight ;
       }
     else
      {
       viewportwidth = document.getElementsByTagName('body')[0].clientWidth ;
       viewportheight = document.getElementsByTagName('body')[0].clientHeight ;
     }
     return ( "(" + String(viewportwidth) + "," + String(viewportheight) + ")" );

     SCRIPT

     doc = <<-SCRIPT

     var scrollw;
     var scrollh;

     if (typeof document.documentElement != 'undefined' && 
      typeof document.documentElement.scrollHeight != 'undefined' &&
      typeof document.documentElement.scrollWidth  != 'undefined')
      {
        scrollw = document.documentElement.scrollWidth ;
        scrollh = document.documentElement.scrollHeight ;
      }
    else 
      {
        scrollw = 0 ;
        scrollh = 0 ;
      }
      return ( "(" + String(scrollw) + "," + String(scrollh) + ")" );

      SCRIPT

      viewport_coord_string = browser.execute_script(viewport)
      scroll_coord_string   = browser.execute_script(doc)
      view_w, view_h = self.chomp_coords(viewport_coord_string)
      scroll_w, scroll_h = self.chomp_coords(scroll_coord_string)

      #    There is possibly one more thing to check for document height, but for all intents and purposes, this is probably enough (note, there definitely is...)
      #    Log.logger.info("WIDTH: #{scroll_w} #{view_w}")
      #    Log.logger.info("HEIGHT: #{scroll_h} #{view_h}")

      max_w = scroll_w < view_w ? scroll_w : view_w # this seems counter-intuitive, but since we don't know how sel does it, it's best to be conservative...
      max_h = scroll_h < view_h ? view_h : scroll_h # for this,...who gives a shit (most of the document stuff is horizontal, not vertical)
      return max_w, max_h  
    end
    
    def self.chomp_coords(string)
      temp = string.gsub('(','').gsub(')','').split(',')
      return Integer(temp.first),Integer(temp.last)
    end

    def save_temp_gardens_theme(browser)
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      Log.logger.info("Waiting for save form.")
      input_field = "//form[@id='themebuilder-bar-save-form']/.//input[contains(@class, 'name')]"
      begin
        input = wait.until { browser.find_element(:xpath => input_field) }
        wait.until { input.displayed? }
        consume_possible_confirmation(browser)
        Log.logger.info("Element #{input_field} is now visible")
        input.clear
        input.send_keys("temptheme_#{Time.now.to_i}")
      rescue Exception => e
        Log.logger.info("Failed while waiting for/typing into the 'save-theme' box; did a confirmation blow the whole she-bang?!")
        raise "Timed out while waiting for the input field in the 'themebuilder-bar-save-form'. #{e.inspect}"
      end
      JQuery.wait_for_events_to_finish(browser)
      wait.until { browser.find_element(:xpath => "#{@theme_editor['ok_save_as']}/span[text()='OK']") }.click
      Log.logger.info("Waiting 10 s for confirmation.")
      counter = 0
      while ((counter+=1) < 10)
        begin
          alert = browser.switch_to.alert
          wait.until { alert.displayed? }
          msg = alert.text
          Log.logger.info("Found a confirmation message, and consuming it: #{msg.inspect}")
          alert.accept
          browser.switch_to.default_content
          break
        rescue
          browser.switch_to.default_content
          if (counter > 5) && (browser.find_elements(:xpath => "#{@theme_editor['ok_save_as']}/span[text()='OK']").size > 0)
            temp = wait.until { browser.find_element(:xpath => "#{@theme_editor['ok_save_as']}/span[text()='OK']") }
            if temp.displayed? 
              Log.logger.info("Button still present and visible...clicking!")
              temp.click
            end
          end 
        end
        sleep 1
      end
      Log.logger.info("Wait for 'was successfully copied and saved' text")
      copied_msg = wait.until { browser.find_element(:xpath => @theme_editor['tb_copied_saved_msg']) }
      Log.logger.info("Wait for 'was successfully copied and saved' text to disappear")
      wait.until { !copied_msg.displayed? }
      JQuery.wait_for_events_to_finish(browser)
    end

    private :save_temp_gardens_theme

    # Run the dom walker test script and report back the data in the result object
    # putting this here because it might be intereting for some other general case test
    # params:
    # page the relative url of the page to be "walked"
    # element a selector from which to "start" the dom walker test
    #
    # return: an array of elements that failed the test
    #
    # This test will detect if the theme builder is open or not
    # if not, it will open TB

    def run_dom_walker(page, element)
      wait = Selenium::WebDriver::Wait.new(:timeout => 10)
      Log.logger.info("Running dom walker for: '#{element}'")
      self.start_theme_builder unless $tb_open
      @browser.get(@sut_url + page)
      Log.logger.info("Switching tab to 'Styles'")
      self.switch_tab('Styles')
      Log.logger.info("Clicking on 'Font'")
      self.click_in_tab('Font')
      Log.logger.info("Selecting our element: '#{element}'")
      self.select_element(element, :css)
      Log.logger.info("Checking for themebuilder development module.")
      if @browser.find_elements(:id => 'themebuilder-selector-test').size > 0
        Log.logger.info("Themebuilder development module seems to be enabled (good).")
      else
        raise("Themebuilder testing module was not enabled on this gardens installation! Can't run dom walker test.")
      end
      Log.logger.info("Clicking on the themebuilder-selector-test element to start the test")
      sel_test = wait.until { @browser.find_element(:id => 'themebuilder-selector-test') }
      wait.until { sel_test.displayed? }
      sel_test.click 
      Log.logger.info("Waiting for status message to pop up")
      test_status = wait.until { @browser.find_element(:xpath => "//span[@id='themebuilder-selector-test-status']") }

      #these two help making sure that we don't run endlessly if the test gets stuck
      last_tests_remaining = 0
      test_stuck = 0

      while @browser.find_elements(:id => 'themebuilder-selector-test-results').size < 1
        status_message = test_status.text
        tests_remaining = Integer(status_message.scan(/\d+/)[0])
        #we check if our tests actually moved forward or not
        if tests_remaining == last_tests_remaining
          #they didn't move forward --> increment counter
          test_stuck+=1
          #after we incremented 6 times, we just give up on them
          if test_stuck > 6
            error_msg = "Selector tests didn't count down for over a minute (tests remaining: #{tests_remaining})."
            Log.logger.warn(error_msg)
            return [error_msg]
          end
        else
          #since we moved forward, we can reset the counter again
          test_stuck = 0
        end
        #remember what we are at currently and move into the next loop
        last_tests_remaining = tests_remaining
        Log.logger.info("Remaining selector tests: #{tests_remaining} (refreshing in 10s)")
        sleep 10
      end
      Log.logger.info("Themebuilder selector tests are done. Waiting for the results to show up (takes a while)")
      #This takes a while
      new_wait = Selenium::WebDriver::Wait.new(:timeout => 60)
      test_results = new_wait.until { @browser.find_element(:id => 'themebuilder-selector-test-results') }
      result_string = test_results.text
      results = result_string.split('Selector:');
      results.each{|result|
        result.strip!
      }
      results.delete_if{|e| (e.size == 0) || (e =~ /expected: 700; actual: bold/) || (e =~ /Failures encountered/)  }
      if results.empty?
        results = ["success"]
      end
      return results
    end

    ##############
    #############

    #### TODO : select_site_template???


    ##############
    ##############



    #switch to another tab in the theme builder
    # and wait for it's contents to show
    def switch_tab(tab_name)
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      current_tab = self.active_tab()
      if current_tab == tab_name
        Log.logger.info("We are already in tab #{tab_name}")
      else
        begin
          Log.logger.info("Switching to tab #{tab_name.inspect} (currently in #{current_tab.inspect})")
          tab_link_locator = "//div[@id='themebuilder-main']/ul[contains(@class, 'ui-tabs-nav')]/li/a[text()='#{tab_name}']"
          tab_link = wait.until { @browser.find_element(:xpath => tab_link_locator) }
          wait.until { tab_link.displayed? }
          Log.logger.info("Clicked on tab")
          tab_link.click
          successful_path = "//li[contains(@class, 'ui-tabs-selected')]/a[text()='#{tab_name}']"
          success = wait.until { @browser.find_element(:xpath => successful_path) }
          wait.until { success.displayed? }
        rescue
          Log.logger.info("Caught an error while attempting to switch tabs...let's see what happens when we keep going.")
        end
      end
    end

    def select_option(inputName, choiceLabel, timeout = 60)
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      Log.logger.debug("Selecting option #{inputName}, #{choiceLabel}")
      locator = @tabs[self.active_tab][inputName]
      if(!locator)
        raise Exception.new("ThemeBuilder.select_option, the #{@tabs[self.active_tab]} tab does not contain '#{inputName}'" )
      end
      val = locator[0,1]
      case val
      when '/'
        loc = wait.until { @browser.find_element(:xpath => locator) }
      when '#'
        loc = wait.until { @browser.find_element(:css => locator) }
      else
        loc = wait.until { @browser.find_element(:id => locator) }
      end
      flag = false
      loc.find_elements(:xpath => "//option").each { |elem|
        if elem.text.include?(choiceLabel)
          flag = true
          elem.click
          break
        end
      }
      Log.logger.info("Didn't select option for #{choiceLabel}") unless flag
      sleep 1
    end

    # need to specify the element and whether it is xpath, css, etc...
    def select_element(hash_locator, type = :unknown, timeout = 60 )
      wait = Selenium::WebDriver::Wait.new(:timeout => 10)
      Log.logger.debug("Selecting element #{hash_locator}")
      JQuery.wait_for_events_to_finish(@browser)
      case type
      when :css
        loc = wait.until { @browser.find_element(:css => hash_locator) }
      when :xpath
        loc = wait.until { @browser.find_element(:xpath => hash_locator) }
      when :id
        loc = wait.until { @browser.find_element(:id => hash_locator) }
      else
        Log.logger.info("Whoops...didn't recognize the type of locator given. Trying to deduce it, then defaulting to :xpath")
        if hash_locator[0,1] == "/"
          loc = wait.until { @browser.find_element(:xpath => hash_locator) }
        elsif hash_locator[0,1]  == "#"
          loc = wait.until { @browser.find_element(:css => hash_locator) }
        else
          Log.logger.info("Defaulting to :css")
          loc = wait.until { @browser.find_element(:css => hash_locator) }
        end
      end

      Log.logger.info("Ruh roh...our element locator, loc, seems to be nil") if loc.nil?

      10.times do |i|
        Log.logger.info("Clicking on #{loc.inspect} (Try #{i+1}/10)")
        if loc.displayed?
          loc.click
        else
          Log.logger.info("#{loc} is not currently visible...")
          if i == 9
            script = "arguments[0].click();"
            begin
              @browser.execute_script(script,loc)
              Log.logger.info("Clicked the element via JS!")
            rescue Exception => e
              Log.logger.info("Failed to click the element, even via JS. Exception: #{e.inspect}")
            end
          end
        end
        JQuery.wait_for_events_to_finish(@browser)
        #visible border arround element
        if not @browser.find_elements(:xpath => "//div[contains(@class, 'tb-no-select') and contains(@class, 'tb-nav') and contains(@style,'')]").empty?
          Log.logger.info("Selection of #{loc.inspect} was successful!")
          break
        else
          sleep 2
        end
      end

    end


    ##################
    ################

    def type_text(inputName, newText, timeout = 60, browser = @browser)
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      locator = @tabs[self.active_tab][inputName]
      if(!locator)
        raise Exception.new("ThemeBuilder.select_option, the #{@tabs[self.active_tab]} does not contain '#{inputName}'" )
      end
      JQuery.wait_for_events_to_finish(@browser)
      case locator[0,1]
      when '/'
        loc = wait.until { @browser.find_element(:xpath => locator) }
      when '#'
        loc = wait.until { @browser.find_element(:css => locator) }
      else
        loc = wait.until { @browser.find_element(:id => locator) }
      end
      JQuery.wait_for_events_to_finish(@browser)
      loc.click
      loc.clear
      loc.send_keys(newText)
      loc.click
      JQuery.wait_for_events_to_finish(browser)
    end

    # TODO: thie mouse down click is a hack for bold and italic button and should be removed after their fix
    def mouse_down(inputName, timeout = 60)
      locator = @tabs[self.active_tab][inputName]
      Log.logger.debug("Pressing mose down element #{locator}")
      if(!locator)
        raise Exception.new("ThemeBuilder.select_option, the #{@tabs[self.active_tab]} does not contain '#{inputName}'" )
      end
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      case locator[0,1]
      when '/'
        loc = wait.until { @browser.find_element(:xpath => locator) }
      when '#'
        loc = wait.until { @browser.find_element(:css => locator) }
      else
        loc = wait.until { @browser.find_element(:id => locator) }
      end
      JQuery.wait_for_events_to_finish(@browser)
      @browser.action.click_and_hold(loc).perform
      sleep 1
    end

    # returns true if only the last selected tab (active_tab) is visible and false otherwise
    def only_active_tab_is_visible()
      total_active = @browser.find_elements(:xpath => @active_tab_xpath).size
      if total_active == 1
        return true
      else
        puts "WEIRD: #{total_active.inspect}"
      end
    end

    # returns the value of an input element
    def read_input(inputName)
      locator = @tabs[self.active_tab][inputName]
      if(!locator)
        raise Exception.new("ThemeBuilder.read_input, the #{self.active_tab} does not contain '#{inputName}'" )
      end
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      case locator[0,1]
      when '/'
        loc = wait.until { @browser.find_element(:xpath => locator) }
      when '#'
        loc = wait.until { @browser.find_element(:css => locator) }
      else
        loc = wait.until { @browser.find_element(:id => locator) }
      end
      return loc.attribute("value")
    end

    # moves the slider associated with the inputName to the given percentage of its width

    def move_slider_to_percent(inputName, percent, timeout = 60)
      Log.logger.debug("Moving silder for #{inputName} to #{percent} of full")
      locator = @tabs[self.active_tab][inputName]
      Log.logger.info("Using locator #{locator}")
      if(!locator)
        raise Exception.new("ThemeBuilder.move_slider_to_percent, the #{@tabs[self.active_tab]} does not contain '#{inputName}'" )
      end
      JQuery.wait_for_events_to_finish(@browser)
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      case locator[0,1]
      when '/'
        slider  = wait.until { @browser.find_element(:xpath => locator) }
      when '#'
        slider = wait.until { @browser.find_element(:css => locator) }
      else
        slider = wait.until { @browser.find_element(:id => locator) }
      end
      @browser.action.click(slider).click_and_hold(slider).perform
      # retreive the width of the slider
      slider_elem = wait.until { @browser.find_element(:xpath => "//div[contains(@class, 'slider') and contains(@style,'display: block')]") } 
      # look for a magic slider element to be suddenly included
      sliderWidth = slider_elem.size.width
      #move the mouse to desired percentage of the width
      @browser.action.click(slider).click_and_hold(slider).move_to(slider,"#{sliderWidth.to_f*percent}","0").release.perform
      # we now write to a DB o this stuff take finite time to propagate
      JQuery.wait_for_events_to_finish(@browser)
    end

    #given the name appearing to the left of the color input, it picks the color item a,b,c,d,e from the palette
    # makes sure the same color is picked and returns the chosen color that now appears in the color input icon
    def pick_color_from_palette(inputName, desired_item, timeout = 60)
      Log.logger.debug("Pick palette color #{desired_item}, from #{inputName}")
      locator = @tabs[self.active_tab][inputName]
      if(!locator)
        raise Exception.new("ThemeBuilder.pick_color_from_palette, the #{@tabs[self.active_tab]} does not contain '#{inputName}'" )
      end
      JQuery.wait_for_events_to_finish(@browser)
      wait = Selenium::WebDriver::Wait.new(:timeout => timeout)
      case locator[0,1]
      when '/'
        elem = wait.until { @browser.find_element(:xpath => locator) }
      when '#'
        elem = wait.until { @browser.find_element(:css => locator) }
      else
        elem = wait.until { @browser.find_element(:id => locator) }
      end
      elem.click
      #color_locator = "css=.PalettePickerMain:visible .current-palette.palette-list .palette-item.item-#{desired_item}"
      color_locator = "//div[contains(@class, 'PalettePickerMain') and contains(@style, 'display: block')]//div[contains(@class,'current-palette') and contains(@class, 'palette-list')]/div[contains(@class,'palette-item') and contains(@class, 'item-#{desired_item}')]"
      JQuery.wait_for_events_to_finish(@browser)
      palette = wait.until { @browser.find_element(:xpath => color_locator) } 
      palette_color = palette.attribute("style")
      palette_color = /background-color: (.*);/.match(palette_color)[1]
      # click on the desired color from the pallete
      # chosen color on the palette
      #    @browser.click("//div[@id='#{locator}']/preceding-sibling::div//div[@class='current-palette palette-list']/div[@class='palette-item item-#{desired_item}']")
      palette.click
      JQuery.wait_for_events_to_finish(@browser)
      # click ok button on the palette
      # @browser.click("css=.PalettePickerMain .okbutton")
      @browser.find_element(:xpath => "//div[contains(@class, 'PalettePickerMain') and contains(@style, 'display: block')]//button[contains(@class, 'okbutton')]").click
      JQuery.wait_for_events_to_finish(@browser)
      #this is the color we end up with after selection in the palette
      chosen_color = @browser.find_element(:xpath => "//div[@id='#{locator}']").attribute("style")
      chosen_color = /background-color: (.*);/.match(chosen_color)[1]
      # we now write to a DB o this stuff take finite time to propagate
      JQuery.wait_for_events_to_finish(@browser)
      sleep 5
      #are the colors that the palette promissed and the one we got after selection the same?
      if(palette_color != chosen_color)
        raise Exception.new("ThemeBuilder.pick_color_from_palette, the pallete color and picked color do not match" )
      end
      return chosen_color
    end

    def read_css_file(userName, fileName)
      # will throw exception if the required css file is not found
      Log.logger.debug("Current dir: " +Dir.pwd)
      css = File.new("./docroot/sites/all/themes/mythemes/acq_#{userName}_session/#{fileName}", 'r').readlines
      # this leaves the \n at the end of the lines in the array if we decideto clean that up we need to add
      # css.each(|l| l.chomp!)
      puts "css file is : " + css
      return css
    end

    def read_css_font_color(locator)
      read_css_property(locator, "color")
    end

    # returns the font size of the selected element in
    # or given an element class.  get the font size
    def read_css_font_size(class_name)
      res = read_css_property(locator, "font-size")
      return /(.*)[a-z][a-z]/.match(res)[1]
    end

    # get a preoprty of an element. the locator needs to be specific enough to
    # only get one element back  This will accept only css locators.
    # and has rudimentary support for xpath e.g. //div[@id = 'header']
    #example: res = themer.read_css_property('#site-name a', 'font-size')

    def read_css_property(locator, property)
      self.class.read_css_property(@browser, locator, property)
    end

    def self.read_css_property(browser, locator, property)
      t_locator = locator.clone
      t_locator.sub!("@", "") if(locator =~ /^XPath/i || locator =~ /^\/\//)
      JQuery.wait_for_events_to_finish(browser)

      Log.logger.info("Reading CSS Property #{property.inspect} from locator #{t_locator.inspect}")
      js_script = "return (window.jQuery(\"#{t_locator}\").css(\"#{property}\"));"
      retry_counter = 0
      begin
        Log.logger.info("Running script: #{js_script}")
        result = browser.execute_script(js_script)
      rescue Exception => e
        Log.logger.info("Error while grabbing CSS property with jquery: #{e.message}")
        sleep 2
        retry_counter += 1
        if retry_counter < 5
          retry 
        else
          raise "Even after several retries: Error while grabbing CSS property with jquery: #{e.message}"
        end
      end
      result
    end

    def click_in_tab(tabName, timeout = 60)
      click_target = "//div[contains(@class, 'tb-tabs-vert')]/.//ul[contains(@class, 'tabnav')]/li/a[contains(text(),'#{tabName}')]"
      click_target_active = "//div[contains(@class, 'tb-tabs-vert')]/.//ul[contains(@class, 'tabnav')]/li[contains(@class, 'ui-tabs-selected')]/a[contains(text(), '#{tabName}')]"
      Log.logger.info("Waiting for vertical tab with name: #{tabName.inspect}")
      JQuery.wait_for_events_to_finish(@browser)
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      target = wait.until { @browser.find_element(:xpath => click_target) } 
      Log.logger.info("Clicking on vertical tab with name: #{tabName.inspect}")   
      target.click
      JQuery.wait_for_events_to_finish(@browser)
      Log.logger.info("Waiting for tab with name #{tabName.inspect} to become active.")
      wait.until { @browser.find_element(:xpath => click_target_active) }    
    end

    def status_bar_bumped?
      wait = Selenium::WebDriver::Wait.new(:timeout => 15) 
      bumped = false
      Log.logger.info("Detecting status message.")
      begin
        if wait.until { @browser.find_elements(:xpath => @theme_editor['tb_status']) }.empty? # counter-intuitive? maybe...necessary? currently
          Log.logger.info("No status message to detect...")
        else
          Log.logger.info("Waiting for status to be visible.")
          status = wait.until { @browser.find_element(:xpath => @theme_editor['tb_status']) }
          Log.logger.info("Detected status message: (#{status.text.inspect}). Waiting for it to disappear.")
          wait.until { ! status.displayed? }
          Log.logger.info("Detected that status message faded out.")
          bumped = true
        end
      rescue
        bumped = false
        Log.logger.info("The Status bar never bumped")
      end

      # run the check for the not visible again and pause again for
      # double bump situations
      begin
        wait.until {  @browser.find_elements(:xpath => @theme_editor['tb_status']).empty? || (!@browser.find_element(:xpath => @theme_editor['tb_status']).displayed?) }
        #Log.logger.info("Confirmed status message is not visible")
      rescue Exception => e
        Log.logger.warn("Error on the second check for disappeared status bar bump: #{e}")
      end

      #return our findings
      bumped
    end

    # code just to save the actions needed
    # this will bump up and down the element refiner
    def toggle_refinement
      if (@browser.find_elements(:css => '.path-selector-refinement').size > 0)
        p_sel = @browser.find_element(:css => '.path-selector-refinement')
        @browser.action.
        click(p_sel).
        click(p_sel).
        perform
      end
    end

    #####################################################################
    # These all deinitions are for power theming test i.e. 50_DOM_test.rb
    #####################################################################

    # Navigates the DOM navigator to get the parent, child and siblings selected

    def DOM_navigator(nav, timeout = 10)
      Log.logger.debug("Switching the DOM navigator to #{nav}")
      locator = @styles_tab[nav]
      if( @browser.find_elements(:xpath=>locator).size == 0)
        raise Exception.new("Themebuilder.DOM_navigator, @style_tab doesn't contain '#{nav}'")
      end
      wait = Selenium::WebDriver::Wait.new(:timeout => timeout)
      loc = wait.until { @browser.find_element(:xpath => locator) }
      if loc.displayed?
        loc.click
      else
        Log.logger.info("Trying to focus the element via JS!")
        script = "arguments[0].click();"
        @browser.execute_script(script, loc)
      end
    end

    # Switch power theming status and wait for it's contents to show

    def switch_power_theming(power)
      Log.logger.debug("Switching the power thening to #{power}")
      powerval = @browser.find_element(:xpath => "//span[contains(@class, 'power-theming-value')]")
      power_val =  powerval.text
      if(power != "off")
        if(power_val != "on")
          powerval.click
        end
      else
        if(power_val != "off")
          powerval.click
        end
      end
    end

    def get_first_child(node, timeout = 60)
      Log.logger.debug("Getting the first child node of the selected node")
      locator = node
      wait = Selenium::WebDriver::Wait.new(:timeout => timeout)
      wait.until { @browser.find_element(:xpath => locator) }
      fchild = locator + '/*'
      return fchild
    end

    def get_parent(node, wait_for = true, timeout = 60)
      Log.logger.debug("Getting the parent node of the selected node")
      locator = node
      if (wait_for)
        wait = Selenium::WebDriver::Wait.new(:timeout => timeout)
        wait.until { @browser.find_element(:xpath => locator) } 
      end
      parent = locator + '/../..'
      return parent
    end

    def get_next_sibling(node, timeout = 60)
      Log.logger.debug("Getting the next sibling of the selected node")
      locator = node
      wait = Selenium::WebDriver::Wait.new(:timeout => timeout)
      wait.until { @browser.find_element(:xpath => locator) }
      next_sibling =  locator + '/../following-sibling::*'
      return next_sibling
    end

    def get_prev_sibling(node, timeout = 60)
      Log.logger.debug("Getting the previous sibling of the selected node")
      locator = node
      wait = Selenium::WebDriver::Wait.new(:timeout => timeout)
      wait.until { @browser.find_element(:xpath => locator) }
      prev_sibling = locator + '/../preceding-sibling::*'
      return prev_sibling
    end

    def refiner_display
      Log.logger.debug("Check the refiner displays properly")
      locator = "//div[contains(text(), 'Site background')]"
      if(@browser.find_elements(:xpath => locator).size == 0)
        raise Exception.new("It doesn't contain site background link")
      else
        return true
      end
    end

    def themebuilder_present?(browser = @browser)
      Log.logger.info("Checking if themebuilder is present")
      result = browser.find_elements(:xpath => "//body[contains(@class, 'themebuilder')]").size > 0
      Log.logger.info("Themebuilder present: #{result}")
      result
    end

    def check_css_selector_selected(locator, browser = @browser)    
      t_locator = locator.clone
      t_locator.sub!("@", "") if(locator =~ /^XPath/i || locator =~ /^\/\//)
      js_script = "return window.jQuery(\"#{t_locator}\").hasClass('selected');"
      browser.execute_script(js_script)
    end

    ####################################################
    #50_DOM_test.rb defined methods ends here
    ####################################################

    #Deprecated in favor of qa_backdoor.rb
    def image_uploader(user_name, from_url, final_img_name, current_app_server = nil)
      Log.logger.info("Copying image from: #{from_url}")
      current_uri = URI.parse(URI.encode(@browser.current_url))
      current_host = current_uri.host
      current_port = current_uri.port
      qa_backdoor = QaBackdoor.new("http://#{current_host}:#{current_port}/")

      #Grab app server cookie from the browser so we're uploading to the right server 
      #unless current_app_server
      #  cookie_hash = @browser.manage.cookie_named('ah_app_server')
      #  current_app_server = cookie_hash[:value]
      #end
      #Log.logger.info("We're on app server: #{current_app_server}")
      img_to = "sites/#{current_host}/themes/mythemes/acq_#{user_name}_session/images/#{final_img_name}"
      Log.logger.info("Copying image to: http://#{current_host}:#{current_port}/#{img_to}")
      qa_backdoor.inject_file(from_url, img_to)
      js = "return (this.window.ThemeBuilder.Bar.getInstance().getTabObject('themebuilder-style').backgroundEditor.backgroundImageChanged('url(\"images/#{final_img_name}\")'));"
      Log.logger.info("Setting image 'images/#{final_img_name}' as background using javascript")
      res = @browser.execute_script(js)
      Log.logger.info("Evaluated Javascript, done with uploading")
      res
    end


    #Gets the fonts defined in a particular font family
    # @param fam web_safe or font_face
    def font_family(fam='web_safe')
      return @styles_tab['font_families'][fam]
    end

    # In the Advanced tab, clicks on the corresponding action ('hide' or 'delete') of the specified css selector and attribute
    def adjust_css_rule(action, css_selector, css_attribute=nil)
      elem = @browser.find_element(:xpath => get_css_rule_selector(css_selector, css_attribute) + "//div[@class='history-operation history-#{action}']")
      elem.click
    end

    # Gets the status (shown, hidden, or mixed - this is when there are both hidden and shown options within a category for a selector)
    def get_css_rule_status(css_selector, css_attribute=nil)
      show_sel = @browser.find_element(:xpath => get_css_rule_selector(css_selector, css_attribute) + "//div[@class='history-operation history-show']") 
      hide_sel = @browser.find_element(:xpath => get_css_rule_selector(css_selector, css_attribute) + "//div[@class='history-operation history-hide']") 
      show_status = show_sel.attribute("style")
      hide_status = hide_sel.attribute("style")
      Log.logger.info("style for #{show_sel} = " + show_status)
      Log.logger.info("style for #{hide_sel} = " + hide_status)

      return 'mixed' if (show_status =~ /block/ and hide_status =~ /block/)
      return 'shown' if (show_status =~ /none/)
      return 'hidden'
    end

    # to get the correct ID to find a particular css rule in the css history, you have to take out anything thats not a number or letter
    def htmlize_css_selector(css_selector)
      return css_selector.gsub(/[^a-zA-Z0-9]/, "-")
    end

    # finds the actual rule div in the css history
    def get_css_rule_selector(css_selector, css_attribute=nil)
      selector = htmlize_css_selector(css_selector)
      if css_attribute then
        selector = selector + "-#{css_attribute}"
      end
      return @advanced_tab['CSS rule'] + "[@id='selector-#{selector}']"
    end

    def hide_all_rules
      @browser.find_element(:css => @advanced_tab['CSS hideall']).click
    end

    def show_all_rules
      @browser.find_element(:css => @advanced_tab['CSS showall']).click
    end

    def delete_all_rules
      hide_all_rules
      @browser.find_element(:css => @advanced_tab['CSS deleteall']).click
    end
  end
