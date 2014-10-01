$LOAD_PATH << File.dirname(__FILE__)
require "rubygems"
require "selenium/webdriver"
require "media_library.rb"
require 'jquery.rb'
require 'tempfile'
require 'digest/bubblebabble' #make random things that look like words

class NodeBuilder
  
  def initialize(_browser, _url=nil)
    @browser = _browser
    @nbgm = NodeBuilderGM.new()
    @sut_url = _url || $config['sut_url']
  end
  
  # add a node of type _type
  # This will fail disasterously if the title or body element does not exist
  # Uniq add a uniq string if needed
  # _og_list is an array of the og groups to subscribe to.
  #  a random number of paragraphs will be added
  def add_og_post_node(_type, _uniq=nil, _og_list = nil)
    pre_string = "node (#{_type}#{_uniq}) - "
    word_count = 1 + rand(10)
    para_count = 1 + rand(10)
    
    bunch_of_words = []
    25.times{ bunch_of_words += Digest.bubblebabble(rand(36**8).to_s(36)).split('-') }
    
    title_string = bunch_of_words.shuffle[0..word_count].join(' ')
    
    paragraphs = []
    para_count.times { paragraphs << (bunch_of_words.shuffle[0..rand(9)].join(' ')) }
    paragraphs = paragraphs.join("\n")
    
    body_string = pre_string + paragraphs

    wait = Selenium::WebDriver::Wait.new(:timeout => 15)

    @browser.get(@sut_url + @nbgm.path_add_node(_type))
    temp = wait.until { @browser.find_element(:xpath => @nbgm.fld_title) }
    wait.until { temp.displayed? }
    temp.clear
    temp.send_keys(title_string)
    Log.logger.info("WYSIWYG is in an iframe...does switching frames allow me to send keys to it??!")
    frame = @browser.find_element(:xpath => @nbgm.wysiwyg_editor_frame)
    @browser.switch_to.frame(frame)
    temp = wait.until { @browser.find_element(:xpath => "//body") }
    wait.until { temp.displayed? }
    temp.clear
    temp.send_keys(body_string)
    @browser.switch_to.default_conter
    if (nil!= _og_list)
      _og_list.each{|og_name|
        wait.until { @browser.find_element(:xpath => @nbgm.cbx_og_audience(og_name)) }.click
      }
    end

    self.save_node
  end
  
  # add a node of type _type
  # This will fail disasterously if the title or body element does not exist
  # or if the content creator is user 1 or an admin
  # _og_list is an array of the og groups to subscribe to.
  #  a random number of paragraphs will be added
  # 
  def add_og_group_node(_type, _og_name, _og_list = nil)
    word_count = 1 + rand(10)
    para_count = 1 + rand(10)
    pre_string = "node (#{_type}) - "
    title_string = _og_name
    
    bunch_of_words = []
    25.times{ bunch_of_words += Digest.bubblebabble(rand(36**8).to_s(36)).split('-') }
    
    desc_string = bunch_of_words.shuffle[0..word_count].join(' ')
    
    paragraphs = []
    para_count.times { paragraphs << (bunch_of_words.shuffle[0..rand(9)].join(' ')) }
    paragraphs = paragraphs.join("\n")
    
    body_string = pre_string + paragraphs

    wait = Selenium::WebDriver::Wait.new(:timeout => 15)

    @browser.get(@sut_url + @nbgm.path_add_node(_type))
    temp = wait.until { @browser.find_element(:xpath => @nbgm.fld_title) }
    wait.until { temp.displayed? }
    temp.clear
    temp.send_keys(title_string)
    temp = wait.until {  @browser.find_element(:xpath => @nbgm.fld_og_description) }
    wait.until { temp.displayed? }
    temp.clear
    temp.send_keys(desc_string)
    temp = wait.until { @browser.find_element(:xpath => @nbgm.fld_body) }
    wait.until { temp.displayed? }
    temp.clear
    temp.send_keys(body_string)
    wait.until { @browser.find_element(:id => @nbgm.rb_og_member_open) }.click
    JQuery.wait_for_events_to_finish(@browser)
    wait.until { @browser.find_element(:xpath => @nbgm.btn_node_save) }.click
  end
  
  # Add Lorem Ipsum generated node of type _type
  # _uniq will add a uniq string to the begining of the content
  # opts is a hash of other things to chage; e.g . {'authored_by' => 'foo, '}
  # only authored_by is accepted at the momemnt
  def add_node(_type, _uniq=nil, _opts = nil)
    Log.logger.debug("Adding node of type #{_type}")
    pre_string = "node (#{_type}) - #{_uniq} -"
    word_count = 1 + rand(10)
    para_count = 1 + rand(10)
    
    
    bunch_of_words = []
    25.times{ bunch_of_words += Digest.bubblebabble(rand(36**8).to_s(36)).split('-') }
    
    title_string = bunch_of_words.shuffle[0..word_count].join(' ')
    
    paragraphs = []
    para_count.times { paragraphs << (bunch_of_words.shuffle[0..rand(9)].join(' ')) }
    paragraphs = paragraphs.join("\n")
    
    body_string = pre_string + paragraphs
    @browser.get(@sut_url + @nbgm.path_add_node(_type))
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    title_el = wait.until { @browser.find_element(:xpath => @nbgm.fld_title) } 
    wait.until { title_el.displayed? }
    title_el.clear
    title_el.send_keys(title_string)
    #TODO: FIX THIS, ADDING TEXT IN BODY ELEMENT DOESN'T WORK
    frame = wait.until { @browser.find_element(:xpath => @nbgm.wysiwyg_editor_frame) }
    @browser.switch_to.frame(frame)
    temp = wait.until { @browser.find_element(:xpath => @nbgm.wysiwyg_body_text) }
    wait.until { temp.displayed? }
    temp.clear
    temp.send_keys(body_string)
    @browser.switch_to.default_content
    if (nil != _opts)
      if (_opts['authored_by'])
        begin
          temp = wait.until { @browser.find_element(:id => @nbgm.fld_authored_by) }
          wait.until { temp.displayed? }
        rescue
          wait.until { @browser.find_element(:xpath => "//a[@href='#' and contains(text(),'Authoring information')]") }.click
          JQuery.wait_for_events_to_finish(@browser)
          temp = wait.until { @browser.find_element(:id => @nbgm.fld_authored_by) }
          wait.until { temp.displayed? }
        ensure
          temp.clear
          temp.send_keys(_opts['authored_by'])
        end
      end
    end
    Log.logger.info("Waiting for save button")
    sv_btn = wait.until { @browser.find_element(:xpath => @nbgm.btn_node_save) }
    Log.logger.info("Clicking on save button")
    sv_btn.click
    JQuery.wait_for_events_to_finish(@browser)
  end
  
  # _image_url is the url of the image to be uploaded 
  # Add Lorem Ipsum generated node of type _type
  # _uniq will add a uniq string to the begining of the content
  # opts is a hash of other things to change; e.g . {'authored_by' => 'foo, '}
  # only authored_by is accepted at the moment
  
  def wysiwyg_add_node(_type, _image_url=nil)
    Log.logger.debug("Adding node of type #{_type}")
    pre_string = "node (#{_type}) -"
    mlgm = MediaLibrary.new(@browser)
    word_count = 1 + rand(2)
    para_count = 1 + rand(10)
    
    
    bunch_of_words = []
    25.times{ bunch_of_words += Digest.bubblebabble(rand(36**8).to_s(36)).split('-') }
    
    title_string = bunch_of_words.shuffle[0..word_count].join(' ')
    
    paragraphs = []
    para_count.times { paragraphs << (bunch_of_words.shuffle[0..rand(9)].join(' ')) }
    paragraphs = paragraphs.join("\n")
    
    body_string = pre_string + paragraphs
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    @browser.get(@sut_url + @nbgm.path_add_node(_type))
    w_t = wait.until { @browser.find_element(:xpath => @nbgm.wysiwyg_title) }
    wait.until { w_t.displayed? }
    w_t.clear
    w_t.send_keys(title_string)
    w_f = wait.until { @browser.find_element(:xpath => @nbgm.wysiwyg_editor_frame) }
    @browser.switch_to.frame(w_f)
    temp = wait.until { @browser.find_element(:xpath => @nbgm.wysiwyg_body_text) }
    wait.until { temp.displayed? }
    temp.clear
    temp.send_keys(body_string)
    # Only one image upload should be allowed for each node created
    retried = false
    Log.logger.info("Attempting to upload an image...")
    begin
      if (_image_url != nil)
        wait.until { @browser.find_element(:xpath => @nbgm.insert_image) }.click
        JQuery.wait_for_events_to_finish(@browser)
        mlgm.upload_image_from_url(_image_url)
        wait.until { @browser.find_element(:xpath => @nbgm.main_page_uploaded_image) }
      end
    rescue
      Log.logger.info("Failed to find the insertion image stuff in the iframe...Retrying on default content")
      @browser.switch_to.default_content
      unless retried
        retried = true
        retry  
      end
    ensure
      @browser.switch_to.default_content
    end
  end
  
  def save_node
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    if @browser.find_elements(:xpath => @nbgm.btn_node_save).size > 0
      #A little bit of debugging...
      Log.logger.info("No need to waiting for save-button, already there")
    else
      Log.logger.info("Waiting for save-button: #{@nbgm.btn_node_save}")
      wait.until { @browser.find_element(:xpath => @nbgm.btn_node_save) }
    end
    Log.logger.info("Clicking on save-button (#{@nbgm.btn_node_save}) and waiting for page loaded event")
    wait.until { @browser.find_element(:xpath => @nbgm.btn_node_save) }.click
    JQuery.wait_for_events_to_finish(@browser)
    Log.logger.info("Done with saving!")
  end
  
  def check_image_uploaded
    #
    if @browser.find_elements(:xpath => "//div[@id='overlay-content']").size > 0
      Log.logger.info("Overlay detected, selecting editor frame")
      frame = @browser.find_element(:xpath => @nbgm.wysiwyg_editor_frame)
      @browser.switch_to.frame(frame)
      if(@browser.find_elements(:xpath => @nbgm.overlay_uploaded_image).size > 0)
        Log.logger.info("Found image (inside overlay), clicking")
        @browser.find_element(:xpath => @nbgm.overlay_uploaded_image).click
        Log.logger.info("Selecting top frame")
        @browser.switch_to.default_content
        return true
      else
        Log.logger.info("Didn't find image in the overlay")
        Log.logger.info("Selecting top frame")
        @browser.switch_to.default_content
        return false
      end
    else
      Log.logger.info("Didn't detect an overlay, no need to select other frame")
      if(@browser.find_elements(:xpath => @nbgm.main_page_uploaded_image).size > 0)
        Log.logger.info("Found image (not inside overlay)")
        return true
      else
        Log.logger.info("Didn't find image on the main page")
        return false
      end
    end

  end
  
  # Only one image upload should be allowed for each node created
  def check_view_image_uploaded
    if(!@browser.find_elements(:xpath => @nbgm.main_page_uploaded_image).empty?)
      return true
    else
      return false
    end
  end
  
  def check_text_view
    @browser.find_element(:xpath => @nbgm.view_content).text
  end
  
  def enable_wysiwyg(input = true)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    @browser.switch_to.default_content
    
    #No iframes -> no overlay
    amount_of_iframes = @browser.find_elements(:css => 'iframe').size
    if amount_of_iframes < 2
      Log.logger.debug "Found #{amount_of_iframes} iframes, too few to trigger using overlay mode."
      overlay = false 
    else
      Log.logger.debug "Found #{amount_of_iframes} iframes, using overlay mode."
      overlay = true
    end

    if overlay
      wait.until { @browser.find_element(:css => 'iframe.overlay-active').displayed? }
      iframe = @browser.find_element(:css => 'iframe.overlay-active')
      @browser.switch_to.frame(iframe)    
    end
    if(!input and @browser.find_elements(:xpath => @nbgm.active_html).empty?)
      Log.logger.info("Changing node body textarea to HTML format.")
      wait.until { @browser.find_element(:css => 'div.form-item-body-und-0-value div.wysiwyg-tab.disable').displayed? }
      fmat = @browser.find_element(:css => 'div.form-item-body-und-0-value div.wysiwyg-tab.disable')
      #fmat = wait.until { @browser.find_element(:xpath => @nbgm.html_format) }
      fmat.click
      wait.until { @browser.find_element(:xpath => @nbgm.active_html) }
    elsif(!input and @browser.find_elements(:xpath => @nbgm.active_html).size > 0)
      Log.logger.info("Node body textarea already in HTML format.")
    elsif(input and @browser.find_elements(:xpath => @nbgm.active_wysiwyg).size > 0)
      Log.logger.info("Node body textarea already in WYSIWYG format.")
    else
      Log.logger.info("Changing node body textarea to WYSIWYG format.")
      
      wmat = wait.until { @browser.find_element(:css => 'div.form-item-body-und-0-value div.wysiwyg-tab.enable') }
      wmat.click
      wait.until { @browser.find_element(:xpath => @nbgm.active_wysiwyg) }
    end
    Log.logger.info("Done setting WYSIWIG to: #{input}")
    @browser.switch_to.default_content if overlay
  end
  
  def get_textarea_value(element)
    @browser.execute_script("return window.document.element.value;")
  end
  
  
  class NodeBuilderGM
    
    attr_reader :fld_title
    attr_reader :fld_body
    attr_reader :fld_authored_by
    attr_reader :fld_og_description
    attr_reader :cbx_og_public
    attr_reader :rb_og_member_open
    attr_reader :btn_node_save
    
    attr_reader :add_new_content, :wysiwyg_title, :edit_summary, :wysiwyg_body
    attr_reader :font_bold, :font_italic, :font_underline, :justify_left, :justify_center, :justify_right,
      :bullet_list, :number_list, :increase_indent, :decrease_indent, :btn_undo, :btn_redo, :hyperlink,
      :unlink, :text_color, :block_quote, :insert_hor_line, :erase_format, :format_text, :format_open_btn,
      :insert_table, :spell_check, :insert_image, :toolbox_collapser
    
    attr_reader :wysiwyg_editor_frame, :wysiwyg_editor, :wysiwyg_body_text, :wysiwyg_resizer, :preview_page
    attr_reader :main_page_uploaded_image, :overlay_uploaded_image, :view_content
    attr_reader :html_format, :wysiwyg_format, :active_html, :active_wysiwyg
        
    def initialize()
      @fld_title = '//input[contains(@id,"edit-title")]'
      #      @fld_body = 'edit-body'
      @fld_body = '//textarea[contains(@id,"edit-body") and contains(@id,"value")]'
      @fld_authored_by = 'edit-name'
      @fld_og_description = 'edit-og-description'
      
      # only open memberships are useful at the moment
      @rb_og_member_open = 'edit-og-selective-0'
      # og may be joined during registration
      @cbx_og_registration_form = 'edit-og-register'
      @cbx_og_private = 'edit-og-private'
      @cbx_og_public = 'edit-og-public'
      @btn_node_save = '//div[@id="edit-actions"]/input[@id="edit-submit"]'
      
      # WYSIWYG Editor elements
      @html_format = '//div[text() = "HTML"]'
      @wysiwyg_format = '//div[text() = "WYSIWYG"]'
      @active_html = '//div[contains(@class, "wysiwyg-active") and text() = "HTML"]'
      @active_wysiwyg = '//div[contains(@class, "wysiwyg-active") and text() = "WYSIWYG"]'
      @add_new_content = 'link=Add new content'
      @wysiwyg_title = '//input[@id = "edit-title"]'
      @edit_summary = 'link=Edit summary'
      @wysiwyg_body = '//input[@id = "edit-body"]'
      
      @font_bold = '//a[contains(@class, "cke_button_bold")]'
      @font_italic = '//a[contains(@class, "cke_button_italic")]'
      @font_underline = '//a[contains(@class, "cke_button_underline")]'
      @justify_left = '//a[contains(@class, "cke_button_justifyleft")]'
      @justify_center = '//a[contains(@class, "cke_button_justifycenter")]'
      @justify_right = '//a[contains(@class, "cke_button_justifyright")]'
      @bullet_list = '//a[contains(@class, "cke_button_bulletedlist")]'
      @number_list = '//a[contains(@class, "cke_button_numberedlist")]'
      @increase_indent = '//a[contains(@class, "cke_button_outdent cke_")]'
      @decrease_indent = '//a[contains(@class, "cke_off cke_button_indent")]'
      @btn_undo = '//a[contains(@class, "cke_button_undo")]'
      @btn_redo = '//a[contains(@class, "cke_button_redo")]'
      @hyperlink = '//a[contains(@class, "cke_button_link")]'
      @unlink = '//a[contains(@class, "cke_button_unlink")]'
      @text_color = '//a[contains(@class, "cke_button_textcolor")]'
      @block_quote = '//a[contains(@class, "cke_button_blockquote")]'
      @insert_hor_line = '//a[contains(@class, "cke_button_horizontalrule")]'
      @erase_format = '//a[contains(@class, "cke_button_removeFormat")]'
      @format_text = '//span[@class = "cke_text cke_inline_label"]'
      @format_open_btn = '//span[@class = "cke_openbutton"]'
      @insert_table = '//a[contains(@class, "cke_button_table")]'
      @spell_check = '//a[contains(@class, "cke_button_checkspell")]'
      @insert_image = '//a[contains(@class, "cke_button_media")]'
      @toolbox_collapser = '//a[contains(@class, "cke_toolbox_collapser")]'
      
      @wysiwyg_editor_frame = '//*[contains(@title, "edit-body-und-0-value")]'
      @wysiwyg_editor = '//html[@dir = "ltr"]'
      @wysiwyg_body_text = '//body[@class = "cke_show_borders"]'
      @wysiwyg_resizer = '//div[@class = "cke_resizer"]'
      @btn_node_preview = '//input[@id = "edit-preview" and @value = "Preview"]'
      # <img typeof="foaf:Image" src="http://gardens.trunk:8080/sites/gardens.trunk/files/styles/large/public/acquia-logo-med_3.gif" />
      @main_page_uploaded_image = '//img[@typeof="foaf:Image"]'
      @overlay_uploaded_image = "//img[@typeof='foaf:Image' and contains(@class, 'media-image ')]"
      @view_content = '//div[contains(@class, "field-item even")]'
    end
    
    def path_add_node(_type)
      type = _type.gsub(/_/,'-')
      return '/node/add/' + type
    end
    
    #def wysiwyg_path_add_node(_type)
    # type = _type.gsub(/_/,'-')
    #return '#overlay=node/add/' + type
    #end
    
    # get the check box for a specific OG grop
    def cbx_og_audience(_og_node_name)
      # possible xpath pattterms to select a node who 
      # //input[parent::label[contains(.,'OG1')] and contains(@id, 'edit-og-groups')]
      # //input[parent::label[contains(.,'#{_og_node}')] and contains(@id, 'edit-og-groups')]
      "//input[parent::label[contains(.,\'#{_og_node_name}\')] and contains(@id, \'edit-og-groups\')]"
    end    
  end
  
end
