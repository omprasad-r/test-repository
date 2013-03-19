$LOAD_PATH << File.dirname(__FILE__)
require "rubygems"
require 'digest/bubblebabble' #make random things that look like words
require "acquia_qa/log"
require "selenium/webdriver"
require 'acquia_qa/ssh'
require 'tempfile'
#require 'acquia_qa/qaapi'
require 'jquery.rb'

class GalleryManager
  attr_reader :export_path, :galmgr

  def initialize(_browser, _url=nil)
    @browser = _browser
    @export_path = '/tmp'
    @galmgr = GalleryManagerGM.new()
    @ssh.extend Acquia::SSH
  end


  def add_num_galleries( _browser = @browser, _num = 8)
    base_names = []
    _num.times do
      base_names << (self.get_random_string + self.utc_timestamp.to_s)
    end
    image_url_list = ["http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/herpderphorse.jpg"]
    resultant_names = []
    base_names.each do |name|
      resultant_names << self.create_new_gallery(name,_browser)
      self.add_images(name, image_url_list, _browser)
    end
    if base_names.length != resultant_names.length
      Log.logger.info("base_names and resultant_names appear to be different...")
    end
    merge_arrays = base_names - resultant_names
    if (merge_arrays.length != 0)
      Log.logger.info("the difference of the two arrays is non-zero...")
    end
    return resultant_names
  end

  # Open the tab to add gallery into the list of galleries
  def open_add_gallery_tab(_browser = @browser)
    Log.logger.debug("Opening Add Galleries Tab")
    _browser.get("#{$config["sut_url"]}/node/add/media-gallery")
    self.wait_for_add_gallery_tab #makes sure Modules Tab is loaded
  end

  # Returns the user to the home page
  def return_to_home_page(_browser = @browser)
    Log.logger.debug("Returning Home Page")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    wait.until { _browser.find_element(:xpath => @galmgr.home) }.click
  end

  # waits for two default elemets on the Galleries Tab to be loaded
  def wait_for_add_gallery_tab(_browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    wait.until { _browser.find_element(:xpath => @galmgr.save_gallery_btn) }  #make sure Add Gallery Tab is loaded (needed for refresh page)
    wait.until { _browser.find_element(:xpath => @galmgr.gallery_title) }
    JQuery.wait_for_events_to_finish(_browser)
  end

  # Opens the Galleries link
  def open_galleries_link(browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 10)
    Log.logger.info("Waiting for galleries link to show up, then clicking")
    browser.switch_to.default_content
    JQuery.wait_for_events_to_finish(browser)
    begin
      temp = wait.until { browser.find_element(:xpath => @galmgr.galleries_link) }
      Log.logger.info("Found link...clicking")
      wait.until { temp.displayed? }
      browser.find_element(:xpath => @galmgr.galleries_link).click
    rescue Exception => e
      Log.logger.info("Blew up somewhere...#{e.inspect}")
      #raise "Error while waiting for galleries links ('#{@galmgr.galleries_link}') to appear: #{e.message}\n#{e.backtrace}"
      require 'uri'
      url = "#{$config["sut_url"]}/gallery-collections/galleries"
      Log.logger.info("Attempting to open #{url}")
      browser.navigate.to(URI.encode(url))
    end
    JQuery.wait_for_events_to_finish(browser)
  end

  # Gallery names must not contain any spaces, if it does replace it with '-'

  def open_gallery(gallery_name, browser = @browser)
    #ATTENTION : drupal tends to delete common words like "at" out of the title URL!!!
    open_galleries_link(browser)
    gallery_name = gallery_name.to_s.downcase.gsub(" ",'-') # Replace spaces with '-' if any
    #this link will contain the actual name of the gallery and the slug in the link
    Log.logger.info("Opening our gallery...")
    browser.get("#{$config["sut_url"]}/content/#{gallery_name}")
  end


  def gallery_exists?(gallery_name, browser = @browser)
    #ATTENTION : drupal tends to delete common words like "at" out of the title URL!!!
    open_galleries_link(browser)
    gallery_name = gallery_name.to_s.downcase.gsub(" ",'-') # Replace spaces with '-' if any
    #this link will contain the actual name of the gallery and the slug in the link
    locator = "//a[contains(@href,'#{gallery_name}')]"
    while (browser.find_elements(:xpath => "//li[contains(@class, 'pager-last')]//a[contains(text(), 'last')]").size > 0 and browser.find_elements(:xpath => locator).size < 1) do
      Log.logger.info("Didn't find our gallery (#{gallery_name.inspect}, #{locator.inspect}), but there was a 'next' button --> clicking")
      browser.find_element(:xpath => "//li[contains(@class, 'pager-next')]//a[contains(text(), 'next')]").click
      Log.logger.info("We are on the next gallery page")
    end
    browser.find_elements(:xpath => locator).size > 0
  end

  # Open the page to Edit all galleries from the tab under Galleries page, Assuming user on the galleries page (or changes to galleries page)
  def open_edit_all_galleries_link(browser = @browser)
    browser.get("#{$config["sut_url"]}/galleries") unless browser.current_url.split('/').last == "galleries"
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    begin
      elem = wait.until { browser.find_element(:xpath => @galmgr.edit_all_galleries_tab) }
    rescue Timeout::Error => e
      raise "Timeout during 'open_edit_all_galleries_link' in the galleries manager. The 'edit_all_galleries_tab' (path: #{@galmgr.edit_all_galleries_tab.inspect}) didn't seem to be there."
    end
    elem.click
    JQuery.wait_for_events_to_finish(browser)
  end

  # Open the page to Edit all galleries from the Gallery settings link under Configuration Tab, Assuming user on the home page.

  def open_edit_all_galleries_by_config(browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Opening 'edit all galleries' area")
    wait.until { browser.find_element(:xpath => @galmgr.config_tab)  }.click
    frame = wait.until { browser.find_element(:xpath => @galmgr.edit_overlay_frame) }
    browser.switch_to.frame(frame)
    wait.until { browser.find_element(:xpath => @galmgr.gallery_settings)  }.click
    browser.switch_to.default_content
    Log.logger.info("Done opening 'edit all gallers' area")
  end

  # Returns the url of the page where all media is stored.
  def content_media_url
    return 'admin/content/media'
  end

  # Creates new gallery by choosing any random name or user provided name and returns the name of the gallery

  def create_new_gallery(gallery_name = "", gallery_desc = "",_browser = @browser)
    pre_string = "node New Gallery "
    word_count = 1 + rand(2) #1 or two words
    para_count = 1 + rand(5)

    #Create a bunch of random words
    bunch_of_words = []
    25.times{ bunch_of_words += Digest.bubblebabble(rand(36**8).to_s(36)).split('-') }
    #Create a title string with 1 or 2 words.
    title_string = bunch_of_words.shuffle[0..word_count].join(' ')
    #Create para_count paragraphs with random words.
    paragraphs = []
    para_count.times { paragraphs << (bunch_of_words.shuffle[0..rand(9)].join(' ')) }
    paragraphs = paragraphs.join("\n")


    desc_string = pre_string + paragraphs
    Log.logger.info("Creating New Gallery")
    self.open_galleries_link(_browser)

    wait = Selenium::WebDriver::Wait.new(:timeout => 15)

    on_frame = false
    if (_browser.find_elements(:xpath => "//a[contains(@href, '/node/add/media-gallery')]").size > 0)
      _browser.find_element(:xpath => "//a[contains(@href, '/node/add/media-gallery')]").click
      frame = wait.until { _browser.find_element(:xpath => "//iframe[contains(@class,'overlay-active')]") }
      _browser.switch_to.frame(frame)
      on_frame = true
    else
      self.open_add_gallery_tab
    end
    JQuery.wait_for_events_to_finish(_browser)
    title = wait.until { _browser.find_element(:xpath => @galmgr.gallery_title) }
    if (gallery_name == "")
      title.clear
      title.send_keys(title_string)
    else
      title.clear
      title.send_keys(gallery_name)
      title_string = gallery_name
    end
    JQuery.wait_for_events_to_finish(_browser)
    ## desc => in iframe
    frame = wait.until { _browser.find_element(:xpath => "//iframe[contains(@title,'edit-media-gallery-description')]") }
    _browser.switch_to.frame(frame)
    e = wait.until { _browser.find_element(:xpath => "//body[@class='cke_show_borders']") }
    begin
      wait.until { e.displayed? }
    rescue
      Log.logger.info("Element not displayed??! Don't care...")
    end
    e.clear
    e.send_keys(desc_string)
    _browser.switch_to.default_content
    if on_frame
      frame = wait.until { _browser.find_element(:xpath => "//iframe[contains(@class,'overlay-active')]") }
      _browser.switch_to.frame(frame)
    end
    wait.until { _browser.find_element(:xpath => @galmgr.save_gallery_btn) }.click
    Log.logger.info("New gallery named '#{title_string}' is created.")
    JQuery.wait_for_events_to_finish(_browser)
    _browser.switch_to.default_content
    return title_string
  end

  # Edits the Title and Description of the specified gallery under Edit Gallery Tab.

  def edit_gallery_title_desc(gallery_name = "test", gallery_title = "samplegallery", gallery_desc = "Its a sample gallery",_browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Editing '#{gallery_name}' Gallery properties (Title + Desc).")
    self.open_gallery(gallery_name)
    wait.until { _browser.find_element(:xpath => @galmgr.edit_gallery) }.click
    frame = wait.until { _browser.find_element(:xpath => @galmgr.edit_overlay_frame) }
    _browser.switch_to.frame(frame)
    temp = wait.until { _browser.find_element(:xpath => @galmgr.gallery_title) }
    temp.clear
    temp.send_keys(gallery_title)
    wait.until { _browser.find_element(:xpath => @galmgr.gallery_desc) }
    self.type_text_in_wysiwyg_editor(gallery_desc)
    ## we need to be on the overlay at this point of execution
    wait.until { _browser.find_element(:xpath => @galmgr.save_gallery_btn) }.click
    _browser.switch_to.default_content
  end

  def type_text_in_wysiwyg_editor(body_text, _browser = @browser)
    ## CONTEXT: we are coming here from the OVERLAY
    Log.logger.info("TODO: determine why this method is non-deterministic?!!!!!")
    sleep 1
    JQuery.wait_for_events_to_finish(_browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 10)
    frame = wait.until { _browser.find_element(:xpath => ".//iframe[contains(@title,'edit-media-gallery-description')]") }
    _browser.switch_to.frame(frame)
    current_text = _browser.find_element(:xpath => ".#{@galmgr.ckeditor_body}").text
    Log.logger.info("Found text in description #{current_text.inspect}")
    
    Log.logger.info("Deleting current text (#{current_text.size} characters)")
    delete_presses = 0
    while (_browser.find_element(:xpath =>  ".#{@galmgr.ckeditor_body}").text.size > 0) do
      _browser.find_element(:xpath =>  ".#{@galmgr.ckeditor_body}").clear
      delete_presses += 1
      #Just to make sure we're not running for hours
      if (delete_presses > (100 * current_text.size)) #just to make sure we don't run endlessly (not seen it happen, but nobody likes endless loops
        Log.logger.warn("Something weird happened when trying to delete the description text. Currently left: #{_browser.find_element(:xpath => @galmgr.ckeditor_body).text}")
        break
      end
    end

    Log.logger.info("Typing #{body_text.inspect}")
    item = _browser.find_element(:xpath => ".#{@galmgr.ckeditor_body}")
    item.send_keys(body_text)
    count = 0
    until _browser.find_element(:xpath =>  ".#{@galmgr.ckeditor_body}").text.include?(body_text)
      Log.logger.info("Didn't find our text in the item!!! Retrying once!")
      item = _browser.find_element(:xpath => ".#{@galmgr.ckeditor_body}")
      _browser.action.double_click(item).perform
      item.send_keys(body_text)
      Log.logger.info("Current text: #{item.text.inspect}")
      if not item.text.include?(body_text)
        item.send_keys(body_text)
      end
      count += 1
      sleep 1
      if count > 10 and not item.text.include?(body_text)
        raise "Typing text into a WYSIWYG is failing in overlay (method: type_text_in_wysiwyg_editor; line: 269)"
      end
    end
    _browser.switch_to.default_content
    overlay = wait.until { _browser.find_element(:xpath => @galmgr.edit_overlay_frame) }
    _browser.switch_to.frame(overlay) ## we must return to the overlay!!
    JQuery.wait_for_events_to_finish(_browser)
  end

  # Edits the Gallery Settings of the specified gallery under Edit Gallery Tab.

  def edit_gallery_settings(gallery_name = "test", cols = "4", rows = "3", media_display = "Show title on hover", _browser = @browser)
    Log.logger.info("Editing '#{gallery_name}' Gallery properties (Settings).")
    self.open_gallery(gallery_name)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    wait.until { _browser.find_element(:xpath => @galmgr.edit_gallery) }.click
    frame = wait.until { _browser.find_element(:xpath => @galmgr.edit_overlay_frame) }
    _browser.switch_to.frame(frame)
    wait.until { _browser.find_element(:xpath => @galmgr.edit_gallery_cols) }.find_elements(:xpath => "//option[@value]").each { |e|
      next unless e.text == cols ;  e.click ; break ;
    }
    temp = _browser.find_element(:xpath => @galmgr.edit_gallery_rows)
    temp.clear
    temp.send_keys(rows)
    _browser.find_element(:xpath => @galmgr.edit_gallery_media_display).find_elements(:xpath => "//option[@value]").each { |e|
      next unless e.text == media_display ;  e.click ; break ;
    }
    wait.until { _browser.find_element(:xpath => @galmgr.save_gallery_btn) }.click
    _browser.switch_to.default_content
  end

  # Reads the Gallery Settings of the specified gallery under Edit Gallery Tab.
  def read_gallery_settings(gallery_name = "test", _browser = @browser)
    Log.logger.info("Reading '#{gallery_name}' Gallery properties.")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    self.open_gallery(gallery_name)
    wait.until { _browser.find_element(:xpath => @galmgr.edit_gallery) }.click
    frame = wait.until { _browser.find_element(:xpath => @galmgr.edit_overlay_frame) }
    _browser.switch_to.frame(frame)
    cols = wait.until { _browser.find_element(:xpath => "//div[contains(@class,'form-item')]/select[contains(@id,'edit-media-gallery-columns-und')]/option[@selected='selected']") }.text
    rows = _browser.find_element(:xpath => @galmgr.edit_gallery_rows).attribute("value")
    media_display = wait.until { _browser.find_element(:xpath => "//div[contains(@class,'form-item')]/select[contains(@id,'edit-media-gallery-image-info-where')]/option[@selected='selected']")}.text
    _browser.find_element(:xpath => @galmgr.close_overlay).click
    #make sure we're on a loaded page
    _browser.switch_to.default_content
    Log.logger.info("Cols: #{cols} rows: #{rows} media_display: #{media_display}")
    return cols, rows, media_display
  end

  # Edits the Gallery's Presentation Settings under Edit Gallery Tab.
  # Takes 4 arguments, First: gallery_name, Second: desired download settings, Third: desired fullpage view settings
  # lightbox_view will be true, iff fullpage view is false. Therefore we dont need that as argument, it will be depended upon desired fullpage view settings
  # Fourth argument will be lightbox with desc view, it can only be true iff fullpage_view is false. Otherwise it will be false by default.

  def edit_gallery_presentation_settings(gal_name = "test", options = {})
    Log.logger.info("Editing '#{gal_name}' Gallery Presentation: #{options.inspect}")

    download = options[:download]
    media_view = options[:media_view]
    lightbox_desc_view = options[:lightbox_desc_view]

    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    self.open_gallery(gal_name)
    wait.until { @browser.find_element(:xpath => @galmgr.edit_gallery) }.click
    frame = wait.until { @browser.find_element(:xpath => @galmgr.edit_overlay_frame) }
    @browser.switch_to.frame(frame)

    # Downloading of Image action
    download_checkbox = wait.until { @browser.find_element(:xpath => @galmgr.image_download_chkbox) }
    if download != download_checkbox.selected?
      Log.logger.info("Toggling download settings, download: #{download}")
      download_checkbox.click
    else
      Log.logger.info("Download was already set to: #{download}. No need to toggle")
    end

    # Full Page/Lightbox View action
    Log.logger.info("Setting media presentation view to #{media_view}")
    if media_view == :lightbox
      lightbox_radio = @browser.find_element(:xpath => @galmgr.media_on_lightbox)
      lightbox_radio.click
      self.check_lightbox_view_settings(lightbox_desc_view)
    elsif media_view == :fullpage
      fullpage_radio = @browser.find_element(:xpath => @galmgr.media_on_fullpage)
      fullpage_radio.click
    end

    wait.until { @browser.find_element(:xpath => @galmgr.save_gallery_btn) }.click

    @browser.switch_to.default_content
  end

  # This method will be called only from edit_gallery_presentation_settings after changing to lightbox view.
  # We are coming here after making the lighbox view as true, therefore it remains true.
  # No need to check for lightbox view. Check only for Lightbox desc view only.

  def check_lightbox_view_settings(lightbox_desc_view)
    lightbox_desc_checkbox = @browser.find_element(:xpath => @galmgr.media_on_lightbox_desc)

    if lightbox_desc_view != lightbox_desc_checkbox.selected?
      Log.logger.info("Toggling lightbox desc, lightbox_desc_view: #{lightbox_desc_view}")
      lightbox_desc_checkbox.click
    else
      Log.logger.info("Lightbox desc was already set to: #{lightbox_desc_view}. No need to toggle")
    end
  end

  # Reads the current gallery_presentation_settings  of specified gallery where second argument can be any combination of these four.
  # ["image_download_chkbox", "media_on_fullpage", "media_on_lightbox", "media_on_lightbox_desc"]). Returns the value of them.

  def read_gallery_presentation_settings(gal_name = "test")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Reading gallery's presentation settings")
    self.open_gallery(gal_name)
    Log.logger.info("Successfully opened the gallery, waiting for and clicking on 'edit' link now")
    wait.until { @browser.find_element(:xpath => @galmgr.edit_gallery) }.click
    Log.logger.info("Removing overlay part of the URL:")
    url = @browser.current_url
    Log.logger.info("Before: #{url}")
    # Removing the overlay from the url.
    url.gsub!("content/#{gal_name}#overlay=",'')
    Log.logger.info("After: #{url}")
    Log.logger.info("Opening: #{url.inspect}")
    @browser.get(url)
    Log.logger.info("Successfully opened, waiting for image download checkbox now")
    wait.until { @browser.find_element(:xpath => @galmgr.image_download_chkbox) }
    Log.logger.info("Checkbox found, getting rest of the properties")
    download = @browser.find_element(:xpath => @galmgr.image_download_chkbox).selected?
    fullpage_view = @browser.find_element(:xpath => @galmgr.media_on_fullpage).selected?
    lightbox_view = @browser.find_element(:xpath => @galmgr.media_on_lightbox).selected?
    lightbox_desc_view = @browser.find_element(:xpath => @galmgr.media_on_lightbox_desc).selected?

    if fullpage_view
      media_view = :fullpage
    elsif lightbox_view
      media_view = :lightbox
    end

    self.return_to_home_page

    {
      :download => download,
      :media_view => media_view,
      :lightbox_desc_view => lightbox_desc_view
    }
  end

  def switch_to_last_pagination_page(_browser = @browser)
    if _browser.find_elements(:xpath => "//li[contains(@class,'pager-last')]/a[@href]").size > 0
      Log.logger.info("clicking on the 'last' link, apparently there are a lot of media items already O_o")
      start_time = Time.now
      _browser.find_element(:xpath => "//li[contains(@class,'pager-last')]/a[@href]").click
      Log.logger.info("... and we're on the last page (Took #{Time.now - start_time} seconds to switch here)")
      return true
    else
      return false
    end
  end

  # Adding media items into the gallery. Takes two arguments: Gallery Name and array of Media URL's which are supposed to be added
  # Gallery names must not contain any spaces, if it does replace it with '-'
  # media_xpath is long enough due to presence of blocks of other gallery on the page(their items also share same ids), which makes the counting wrong

  def add_media_action(gallery_name, media_url_list = ["http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/150x62XMasNeon.gif"], _browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Adding New Images into the Gallery '#{gallery_name}'.")
    #    media_xpath = "//div[contains(@id, 'media-gallery-media-')]//a[@href and contains(@class, 'media-gallery-thumb')]"
    media_xpath = "//div[contains(@class, 'media-gallery-sortable-processed ui-sortable')]/div[contains(@id, 'media-gallery-media-')]"
    Log.logger.info("Opening Gallery #{gallery_name}")
    self.open_gallery(gallery_name)
    media_url_list.each do |media_url|
      Log.logger.info("Working on media item: #{media_url}")
      switch_to_last_pagination_page
      #Get the amount of items so we later know which ID something that gets added will get (IDs start at 0, so it will get the id 'nom')
      nom = Integer(_browser.find_elements(:xpath => media_xpath).size)
      Log.logger.info("Found #{nom} media item(s).")

      Log.logger.info("Waiting for 'Add media' link.")
      wait.until { _browser.find_element(:xpath => @galmgr.add_media) }
      #we have to wait for the WHOLE page to be loaded and it might even refresh itself for some reason.
      #if we don't sleep, we sometimes run into the page refreshing after we clicked on the add media link
      JQuery.wait_for_events_to_finish(_browser)
      Log.logger.info("Clicking on'Add media' link.")
      _browser.find_element(:xpath => @galmgr.add_media).click
      Log.logger.info("Waiting for Overlay.")
      wait.until { _browser.find_element(:xpath => "//div[@class='ui-widget-overlay']") }
      Log.logger.info("Waiting for media browser iframe.")
      start_time = Time.now
      frame = wait.until { _browser.find_element(:xpath => @galmgr.media_upload_frame) }
      Log.logger.info("Found media browser iframe after #{Time.now - start_time} seconds.")
      JQuery.wait_for_events_to_finish(_browser)
      if _browser.find_elements(:xpath => @galmgr.media_upload_frame).size > 0 #perhaps unnecessary?
        Log.logger.info("Switching to media browser iframe.")
        _browser.switch_to.frame(_browser.find_element(:xpath => @galmgr.media_upload_frame))
      else
        raise "Media Browser frame seems to have disappeared after we had waited for it!"
      end
      Log.logger.info("Waiting for media upload tab link")
      wait.until { _browser.find_element(:xpath => @galmgr.embed_image_video) }.click
      #Wait til the content of the tab is actually visible
      Log.logger.info("Waiting for media upload tab to be visible")
      wait.until { _browser.find_element(:xpath => "//div[@id='media-tab-upload' and not(@class='ui-tabs-hide')]") }
      Log.logger.info("Waiting for text input field and entering media URL: (#{media_url})")
      temp = wait.until { _browser.find_element(:xpath => @galmgr.url_textbox) }
      temp.clear
      temp.send_keys(media_url)
      Log.logger.info("Waiting for and clicking on submit button")
      wait.until { _browser.find_element(:xpath => @galmgr.submit_url_btn) }.click
      Log.logger.info("Waiting for embed popup to disappear")
      #      wait.until { ! _browser.find_element(:xpath => @galmgr.embed_image_video) }
      Log.logger.info("Switching back to main frame.")
      _browser.switch_to.default_content
      Log.logger.info("Waiting for overlay to disappear.")
      wait.until {  _browser.find_elements(:xpath => "//div[@class='ui-widget-overlay']").empty? }
      Log.logger.info("Overlay is gone.")
      #Our newly inserted element will show up shortly EITHER on a new paginated page or within this context
      partial_file_name = media_url.split('/').last.split(".").first.gsub("watch?v=", "")
      Log.logger.info("Waiting for the media-gallery-media-#{nom} we just added to show up (with a link inside and an img src that points to a file with #{partial_file_name.inspect}).")
      #TODO: Find out if this can be handled by an "OR" in the xpath
      start_time = Time.now
      if switch_to_last_pagination_page()
        nom = Integer(_browser.find_elements(:xpath => media_xpath).size)
        Log.logger.info("Found #{nom} media items on the last page.")
      end
      inserted_media_item_path = "//img[contains(@src, '#{partial_file_name}')]"
      #//div[contains(@id, 'media-gallery-media-#{nom}')].//div[contains(@class, 'media-gallery-draggable-processed')].//a[@href and contains(@class, 'media-gallery-thumb')].
      Log.logger.info("Waiting for #{inserted_media_item_path}")
      wait.until { _browser.find_element(:xpath => inserted_media_item_path) }
      Log.logger.info("Inserted media item 'media-gallery-media-#{nom}' showed up (after #{Time.now - start_time} seconds).")
    end #end each-loop
    JQuery.wait_for_events_to_finish(_browser)
    Log.logger.info("Done with media file insertion")
  end


  # Adding Images into the gallery. Takes two arguments: Gallery Name and array of Image URL's which are supposed to be added
  # Gallery names must not contain any spaces, if it does replace it with '-'

  def add_images(gallery_name, image_url_list = ["http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/150x62XMasNeon.gif"],_browser = @browser)
    Log.logger.info("Adding New Images into the Gallery.")
    add_media_action(gallery_name, image_url_list, _browser)
  end


  # Adding Videos into the gallery. Takes two arguments: Gallery Name and array of Youtube Video URL's which are supposed to be added
  # Gallery names must not contain any spaces, if it does replace it with '-'

  def add_videos(gallery_name, video_url_list = ["http://www.youtube.com/watch?v=Jr-ubPWN7n4", "http://www.youtube.com/watch?v=K-KkSiIjlm0"], _browser = @browser)
    Log.logger.info("Adding New Videos into the Gallery.")
    add_media_action(gallery_name, video_url_list, _browser)
  end

  # List element ids of all media present on the page for overlay opens after clicking Edit Media tab, so that we can change their values.
  # Element Ids for changing the following properties of the various images: title, description, tag and license settings
  # Takes as argument the only gallery name that too without spaces, spaces(if any) will be replaced by '-'

  def list_media_details(gallery_name = "test", _browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Listing Media properties and ids to edit it (for Gallery: #{gallery_name.inspect})")
    self.open_gallery(gallery_name)
    wait.until { _browser.find_element(:xpath => @galmgr.edit_media) }.click
    frame = wait.until { _browser.find_element(:xpath => @galmgr.edit_overlay_frame) }
    _browser.switch_to.frame(frame)
    edit_media_ids = Hash.new
    i = 1
    wait.until { _browser.find_element(:xpath => "//div[contains(@id, 'media-edit-')]") }
    med_ct = Integer(_browser.find_elements(:xpath => "//div[contains(@id, 'media-edit-')]").size)
    Log.logger.info("Found editable images: #{med_ct}")
    while i < med_ct+1
      media_url = _browser.find_element(:xpath => "//div[contains(@id, 'media-edit-#{i}')]//img[contains(@class, '')]").attribute("src")
      Log.logger.info("Working on image number #{i}: #{media_url.inspect}")
      if (i == 1)
        if(_browser.find_elements(:xpath => "//select[@id = 'edit-field-license-und']").size < 1)
          media_license_setting = ""
          media_type = "video"
        else
          media_license_setting = "//select[@id = 'edit-field-license-und']"
          media_type = "image"
        end
        media_title = "//input[@id = 'edit-media-title-und-0-value']"
        media_description = "//textarea[@id = 'edit-media-description-und-0-value']"
        media_tag = "//input[@id = 'edit-field-tags-und']"
      else
        if(_browser.find_elements(:xpath => "//select[@id = 'edit-field-license-und--#{i}']").size < 1)
          media_license_setting = ""
          media_type = "video"
        else
          media_license_setting = "//select[@id = 'edit-field-license-und--#{i}']"
          media_type = "image"
        end
        media_title = "//input[@id = 'edit-media-title-und-0-value--#{i}']"
        media_description = "//textarea[@id = 'edit-media-description-und-0-value--#{i}']"
        media_tag = "//input[@id = 'edit-field-tags-und--#{i}']"
      end

      Log.logger.info("image number #{i} media-type: #{media_type.inspect}")
      Log.logger.info("image number #{i} media-title: #{media_title.inspect}")
      edit_media_ids[media_url] = Hash.new() unless (edit_media_ids[media_url])
      edit_media_ids[media_url][:license] = media_license_setting
      edit_media_ids[media_url][:tag] = media_tag
      edit_media_ids[media_url][:desc] = media_description
      edit_media_ids[media_url][:title] = media_title
      edit_media_ids[media_url][:type] = media_type
      i += 1
    end
    return edit_media_ids
  end

  # Changes the Current License Settings of the image to the desired license
  # Takes as argument the following 4:
  # First img_path usually "http://#{self.new_sites[0]}.#{$SUT_HOST}/sites/#{self.new_sites[0]}.#{$SUT_HOST}/files/styles/square_thumbnail/public/"
  # Second img_name must be in array if multiple images needs same license change. Images must be on the same page
  # And test is good for images uploaded with title having no spaces. If there are spaces please copy the exact name of image from Firebug.
  # Third Gallery name that too without spaces, spaces will be replaced by '-'
  # Fourth: Desired License Settings

  # NOTE: this img_path is different from img_path used in get_current_license_settings

  def change_img_license_settings(img_path, img_name, gallery_name, license_to_set, _browser = @browser)
    img_properties_ids = self.list_media_details(gallery_name)
    img_name.each do |img|
      img_url = img_path + img
      Log.logger.info("Setting license for #{img_url}")
      if !img_properties_ids.key?(img_url)
        Log.logger.info("Can't find the Image in the Hash that tells me what license to chose: #{img_url}")
        img_name_without_extension = img_url.split("/").last.split(".").first
        Log.logger.info("Image Name that we're looking for: #{img_name_without_extension}")
        possible_matches = img_properties_ids.keys.select { |url| url.include?(img_name_without_extension) }
        Log.logger.info("Found possible matches: #{possible_matches.size}")
        if !possible_matches.first.nil?
          Log.logger.info("possible match: #{possible_matches.first}, let's just use that one")
          img_url = possible_matches.first
        else
          raise("Can't change license on image because Image URL #{img_url} isn't a valid key in the img_properties_ids hash: #{img_properties_ids.inspect}")
        end
      end
      license_setting_id = img_properties_ids[img_url][:license]
      flag = false
      Log.logger.info("license setting id: #{license_setting_id} License to set: #{license_to_set}")
      _browser.find_element(:xpath => " #{license_setting_id}/option[text()='#{license_to_set}']").click
    end
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    JQuery.wait_for_events_to_finish(_browser)
    wait.until { _browser.find_element(:xpath => @galmgr.edit_media_submit) }.click
  end

  # Returns the Current License Settings of the image
  # Takes as argument the following:
  # First img_path which is usually "http://#{self.new_sites[0]}.#{$SUT_HOST}/sites/#{self.new_sites[0]}.#{$SUT_HOST}/files/styles/media_gallery_thumbnail/public/"
  # Second img_name without extension and test is good for images uploaded with title having no spaces
  # Third Gallery name that too without spaces, spaces will be replaced by '-'

  # NOTE: this img_path is different from img_path used in change_img_license_settings

  def get_current_license_settings(img_path, img_name, gallery_name, _browser = @browser)
    #we split because we sometimes run the test when an image with the same name is already on the server
    #In this case, our image gets renamed. For the test: image_0.jpg is just as fine as image.jpg
    #--> http://blablablabla.bla/bla/bla/image instead of http://blablablabla.bla/bla/bla/image.jpg
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    img_url = img_path + img_name.split(".").first
    image_name_without_extension = img_name.split(".").first
    image_link = @galmgr.image_link(image_name_without_extension)
    Log.logger.info("Getting current image license settings. Looking for: #{image_link}")
    self.open_gallery(gallery_name)
    Log.logger.info("Waiting for image url to appear #{image_link.inspect}")
    link = wait.until { _browser.find_element(:xpath => image_link) }
    Log.logger.info("Clicking on the image URL and then waiting for #{@galmgr.image_license_info}")
    link.click
    wait.until { _browser.find_element(:xpath => @galmgr.image_license_info) }
    current_img_license = ""
    n = Integer(_browser.find_elements(:xpath => @galmgr.image_license_info).size)
    1.upto(n) do |i|
      val = _browser.find_element(:xpath => "#{@galmgr.image_license_info}[#{i}]").attribute("title")
      if (i == 1)
        current_img_license = val
      else
        current_img_license = current_img_license + ", " + val
      end
    end
    return current_img_license
  end

  # Counts the media type in the gallery, number of images and number of videos. Takes gallery name (w/o spaces) as argument.

  def count_media_type(gallery_name = "test", _browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Counting Images & Videos present on the 1st page of gallery '#{gallery_name}'")
    self.open_gallery(gallery_name)
    wait.until { _browser.find_element(:xpath => @galmgr.edit_media) }.click
    vid_ct = Integer(_browser.find_elements(:xpath => "//div[contains(@id, 'media-edit-')]//div[contains(@class, 'container-media_youtube')]").size)
    img_ct = Integer(_browser.find_elements(:xpath => "//div[contains(@id, 'media-edit-')]//div[contains(@class, 'container-image')]").size)
    return vid_ct, img_ct
  end

  # Creates new gallery block and takes as input gallery name, block rows and columns.

  def create_gallery_block(gal_name, cols = 3, rows = 5, _browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Creating the block for '#{gal_name}' Gallery.")
    self.open_gallery(gal_name)
    wait.until { _browser.find_element(:xpath => @galmgr.edit_gallery) }.click
    frame = wait.until { _browser.find_element(:xpath => @galmgr.edit_overlay_frame) }
    _browser.switch_to.frame(frame)
    wait.until { _browser.find_element(:xpath => @galmgr.gallery_block_link) }.click
    box = _browser.find_element(:xpath => @galmgr.gallery_block_chkbox)
    if not box.selected?
      box.click
    else
      Log.logger.info("Block was already created, changing columns and rows of the block now")
    end
    _browser.find_element(:xpath => "#{@galmgr.gallery_block_cols}/option[text()='#{cols}']").click
    temp = _browser.find_element(:xpath => @galmgr.gallery_block_rows)
    temp.clear
    temp.send_keys(rows)
    wait.until { _browser.find_element(:xpath => @galmgr.save_gallery_btn) }.click
    _browser.switch_to.default_content
  end

  # Sorts out the content in Find Content tab by the updated date and time in ascending or descending order as required. By Default sorts in ascending.

  def content_sorter_path(type)
    "//table[contains(@class, 'sticky-enabled')]/thead/tr/th/a[@title = 'sort by #{type}']"
  end

  def sort_content(ascending = true, browser = @browser)
    if ascending
      direction = :asc
    else
      direction = :desc
    end
    sort_content_by(direction, "Updated", browser)
  end

  def sort_content_by(direction, type, browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.debug("Sorting content by type: #{type.inspect}")
    wait.until { browser.find_element(:xpath => content_sorter_path(type)) }

    if browser.find_elements(:xpath => "//a[@title = 'sort by #{type}']/img").size < 1
      Log.logger.info("We don't sort by #{type.inspect} so far, changing that.")
      wait.until { browser.find_element(:xpath => content_sorter_path(type)) }.click
    end
    sort_elem = wait.until { browser.find_element(:xpath => "//a[@title = 'sort by #{type}']/img") }
    opposite_of_current_sorting = sort_elem.attribute("title")
    if opposite_of_current_sorting == 'sort descending'
      current_sorting = :asc
    elsif opposite_of_current_sorting == 'sort ascending'
      current_sorting = :desc
    end

    Log.logger.info("We're currently sorting according to #{type.inspect}: #{current_sorting.inspect}")
    if (current_sorting != direction)
      Log.logger.info("Switching sorting direction.")
      wait.until { browser.find_element(:xpath => content_sorter_path(type)) }.click
    else
      Log.logger.info("Already sorting correctly.")
    end
  end

  # Filter the content by content type, but this is particularly used for gallery only

  def filter_content(type = "Gallery", _browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    wait.until { _browser.find_element(:xpath => @galmgr.filter_btn) }
    _browser.find_element(:xpath => "#{@galmgr.content_type}/option[text()='Gallery']").click
    JQuery.wait_for_events_to_finish(_browser)
    wait.until { _browser.find_element(:xpath => @galmgr.filter_btn) }.click
    JQuery.wait_for_events_to_finish(_browser)
  end

  # Deletes any number of media in the Find Content/Media, provided the media is not in use. Implemented to avoid repition of images and videos in the library.
  # Takes as argument number of media you want to delete. It deletes only number of media starting in the ascending order
  # Or Number of media starting in the descending order of their updation time.
  # Its good if you can mention you want to delete the most recent or oldest media. Otherwise it will delete the most recent media.

  def delete_media(n=1, ascending = true, _browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Deleting the last #{n} media items")
    _browser.get("#{$config["sut_url"]}/#{self.content_media_url}")
    self.sort_content
    1.upto(n) do |i|
      Log.logger.info("Selecting Element #{i}")
      wait.until { _browser.find_element(:xpath => "//*[@id='media-admin']//tbody/tr[#{i}]//input") }.click
    end
    self.delete_media_action(_browser)

    # Check for any warnings, if any file is in use.

    warning_message = ""
    if (_browser.find_elements(:xpath => @galmgr.warning_message).size > 0)
      i = 1
      if (_browser.find_elements(:xpath => @galmgr.warning_message + "/ul/li").size < 1)
        warning_ct = 1
        warning_message = warning_message + _browser.find_element(:xpath => @galmgr.warning_message).text
      else
        warning_ct = Integer(_browser.find_elements(:xpath => "#{@galmgr.warning_message}/ul/li").size)
        while i < (warning_ct+1)
          warning_message = warning_message + _browser.find_element(:xpath => "#{@galmgr.warning_message}/ul/li[#{i}]").text
          i += 1
        end
      end
    else
      warning_ct = 0
      warning_message = "No Warnings"
    end
    # Check for any status messages, whether the media has been deleted.

    confirm_message = ""
    if (_browser.find_elements(:xpath => @galmgr.confirmation_message).size > 0)
      i = 1
      if (_browser.find_elements(:xpath => "#{@galmgr.confirmation_message}/ul/li").size < 1)
        confirm_msg_ct = 1
        confirm_message = confirm_message + _browser.find_element(:xpath => @galmgr.confirmation_message).text
      else
        confirm_msg_ct = Integer(_browser.find_elements(:xpath => "#{@galmgr.confirmation_message}/ul/li").size)
        while i < confirm_msg_ct+1
          confirm_message = confirm_message + _browser.find_element(:xpath => "#{@galmgr.confirmation_message}/ul/li[#{i}]").text
          i += 1
        end
      end
    else
      confirm_msg_ct = 0
      confirm_message = "No Media deleted"
    end

    _browser.switch_to.default_content
    self.return_to_home_page
    return warning_ct, warning_message, confirm_msg_ct, confirm_message
  end


  def delete_all_youtube_videos(browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Deleting the video media items")
    browser.get("#{$config["sut_url"]}/#{self.content_media_url}")
    sort_content_by(:desc, "Type", browser)
    xpath_counter = 0
    found_yt_vids = false
    while (xpath_counter += 1)
      Log.logger.info("Waiting for Element #{xpath_counter}")
      if browser.find_elements(:xpath => "(//*[@id='media-admin']//tbody/tr)[#{xpath_counter}]").size < 1
        Log.logger.info("No more elements.")
        break
      else
        text = browser.find_element(:xpath => "(//*[@id='media-admin']//tbody/tr)[#{xpath_counter}]").text
        if text.include?("video/youtube")
          found_yt_vids = true
          Log.logger.info("Found 'video/youtube' content. Selecting Element #{xpath_counter}")
          wait.until { browser.find_element(:xpath => "(//*[@id='media-admin']//tbody/tr)[#{xpath_counter}]/.//input") }.click
        else
          if found_yt_vids
            break
          else
            Log.logger.info("Checking for youtube videos")
          end
        end
      end
    end
    Log.logger.info("Done selecting videos to delete.")

    unless found_yt_vids
      Log.logger.info("No youtube vids to delete...")
      self.return_to_home_page
      return
    end

    self.delete_media_action(browser)
    # Check for any warnings, if any file is in use.
    warning_message = ""
    if (browser.find_elements(:xpath => @galmgr.warning_message).size > 0)
      i = 1
      if (browser.find_elements(:xpath => "#{@galmgr.warning_message}/ul/li").size < 1)
        warning_ct = 1
        warning_message = warning_message + browser.find_element(:xpath => @galmgr.warning_message).text
      else
        warning_ct = Integer(browser.find_elements(:xpath => "#{@galmgr.warning_message}/ul/li").size)
        while i < warning_ct+1
          warning_message = warning_message + browser.find_element(:xpath => "#{@galmgr.warning_message}/ul/li[#{i}]").text
          i += 1
        end
      end
    else
      warning_ct = 0
      warning_message = "No Warnings"
    end

    browser.switch_to.default_content
    self.return_to_home_page
    if warning_ct != 0
      Log.logger.warn("Received warning: #{warning_message}")
    end
  end
  # Perform the delete media action after selecting the particular media through checkbox

  def delete_media_action(_browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    item = wait.until { _browser.find_element(:xpath => @galmgr.action) }
    if not item.displayed?
      Log.logger.info("Select bar is not visible...you haven't checked any content to delete!")
      return
    end
    wait.until { _browser.find_element(:xpath => "#{@galmgr.action}/option[contains(@value,'delete')]") }.click
    JQuery.wait_for_events_to_finish(@browser)
    Log.logger.info("Clicking delete button.")
    temp = wait.until { _browser.find_element(:xpath => @galmgr.confirm_delete) }
    wait.until { temp.displayed? }
    temp.click
    JQuery.wait_for_events_to_finish(@browser)
    next_body_text = wait.until {  _browser.find_element(:xpath => "//body") }.text
    if next_body_text.include?("Error 503 Service Unavailable")
      raise "Got an error while trying to delete media items."
    end

    #are you sure?
    Log.logger.info("Waiting for and consuming confirmation.")
    temp = wait.until { _browser.find_element(:xpath => @galmgr.confirm_delete) }
    wait.until { temp.displayed? }
    temp.click
    JQuery.wait_for_events_to_finish(@browser)
  end

  # Deletes the gallery completly through Edit gallery tab on the gallery. Takes gallery name (w/o spaces) as argument.

  def delete_gallery(gallery_name, _browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 10)
    Log.logger.info("Deleting the '#{gallery_name}' Gallery.")
    Log.logger.info("Opening gallery #{gallery_name.inspect}.")
    self.open_gallery(gallery_name)
    JQuery.wait_for_events_to_finish(_browser)
    Log.logger.info("Waiting for 'edit' link.")
    temp = wait.until { _browser.find_element(:xpath => @galmgr.edit_gallery) }
    wait.until { temp.displayed? }
    temp.click
    JQuery.wait_for_events_to_finish(_browser)
    frame = wait.until { _browser.find_element(:xpath => "//iframe[contains(@class,'overlay-active')]") }
    _browser.switch_to.frame(frame)
    temp = wait.until { _browser.find_element(:xpath => @galmgr.delete_gallery_btn) }
    wait.until { temp.displayed? }
    temp.click
    JQuery.wait_for_events_to_finish(_browser)
    temp = wait.until { _browser.find_element(:xpath => @galmgr.confirm_delete_gallery_btn) }
    wait.until { temp.displayed? }
    temp.click
    _browser.switch_to.default_content
    JQuery.wait_for_events_to_finish(_browser)
  end

  # Deletes the most recent gallery completly through Find Content tab. Takes no argument, as it deletes the most recent content.
  # And returns the confirmation message, which confirms the gallery deletion.

  def delete_recently_added_gallery(_browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 5)
    Log.logger.info("Deleting the just added Gallery.")
    wait.until { _browser.find_element(:xpath => @galmgr.find_content_link) }.click
    JQuery.wait_for_events_to_finish(_browser)
    frame = wait.until { _browser.find_element(:xpath => @galmgr.edit_overlay_frame) }
    _browser.switch_to.frame(frame)
    self.filter_content(type = "Gallery")
    self.sort_content
    JQuery.wait_for_events_to_finish(_browser)
    wait.until { _browser.find_element(:xpath => @galmgr.first_gallery_checkbox) }.click
    JQuery.wait_for_events_to_finish(_browser)
    confirm_message = self.delete_content_action
    _browser.switch_to.default_content
    self.return_to_home_page
    return confirm_message
  end

  # Performs the delete action for deleting any content and returns confirmation message for deletion of content

  def delete_content_action(_browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 8)
    flag = false
    wait.until { _browser.find_element(:xpath => @galmgr.action) }.find_elements(:xpath => "//*").each {|e|
      next unless e.text.include?('Delete selected content') ; flag = true ; e.click ; break ;
    }
    Log.logger.info("Failed to select 'delete selected content'") unless flag
    JQuery.wait_for_events_to_finish(_browser)
    _browser.find_element(:xpath => @galmgr.findcontent_update_btn).click
    wait.until { _browser.find_element(:xpath => @galmgr.confirm_delete) }.click
    confirm_message = wait.until { _browser.find_element(:xpath => @galmgr.confirmation_message) }.text
    return confirm_message
  end

  # Method to edit all galleries settings to user required settings.
  # Takes as argument new_title = which can be any text in form of string, new_desc = can be any text in form of string,
  # desc_format = can be one of following: "Plain Text", "Safe HTML", "Full HTML", "Filtered HTML" in form of string,
  # new_url = can be any text in form of string, new_cols = can be any +ve integer b/w 2-10, new_rows = can be any +ve integer,
  # new_display = can be one of following: "Show title on hover", "Show nothing", "Show title below" in form of string.

  def edit_all_galleries(new_title = "Galleries", desc_format = "Plain text", new_desc = "",
                         new_url = "galleries", new_cols = 4, new_rows = 3, new_display = "Show title on hover", _browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 10)
    Log.logger.info("Editing All Galleries")
    Log.logger.info("Waiting for gallery overlay frame")
    frame = wait.until { _browser.find_element(:xpath => @galmgr.eag_overlay_frame) }
    _browser.switch_to.frame(frame)
    Log.logger.info("Waiting for the 'Gallery settings' title. (to make really sure we're on the right iframe)")
    #<h1 id="overlay-title">Gallery settings</h1>
    wait.until { _browser.find_element(:xpath => "//h1[@id='overlay-title' and text()='Gallery settings']") }
    Log.logger.info("Waiting for eag_title")
    already_retried = false
    temp = wait.until { _browser.find_element(:xpath => @galmgr.eag_title) }
    temp.clear
    temp.send_keys(new_title)
    JQuery.wait_for_events_to_finish(_browser)
    Log.logger.info("Selecting eac_desc_format (#{desc_format.inspect} in text-format list.")
    elm = wait.until { _browser.find_element(:xpath => "#{@galmgr.eag_desc_format}/option[text()='#{desc_format}']") }
    wait.until { elm.displayed? }
    elm.click
    if desc_format == "Plain text"
      Log.logger.info("Waiting for text 'Allowed tags: None' to show up (Because 'Plain text' option was selected)")
      wait.until { _browser.find_element(:xpath => "//div[@class='wysiwyg-none-header']//span[contains(text(), 'None')]") }
    else
      JQuery.wait_for_events_to_finish(_browser)
    end
=begin
    begin
      _browser.find_element(:xpath => @galmgr.eag_url_path).send_keys(new_url)
##### THIS HAS CHANGED DUE TO THE NEW SEO SHIT
    rescue
      Log.logger.info("TF!!")
      gets
    end
=end
    Log.logger.info("Setting columns to: #{new_cols}")
    wait.until { _browser.find_element(:xpath => "#{@galmgr.edit_gallery_cols}/option[text()='#{new_cols}']") }.click
    Log.logger.info("Setting row to: #{new_rows}")
    r_ows = _browser.find_element(:xpath => @galmgr.edit_gallery_rows)
    r_ows.clear
    r_ows.send_keys(new_rows)
    wait.until { _browser.find_element(:xpath => "#{@galmgr.edit_gallery_media_display}/option[text()='#{new_display}']") }.click
    if (desc_format == "Plain text")
      Log.logger.info("Typing in new description ")
      temp = wait.until { _browser.find_element(:xpath => @galmgr.eag_desc) }
      temp.clear
      temp.send_keys(new_desc)
    else
      _browser.switch_to.default_content
      Log.logger.info("Selecting ckeditor frame")
      frame = wait.until { _browser.find_element(:xpath => @galmgr.ckeditor_frame) }
      _browser.switch_to.frame(frame)
      Log.logger.info("Waiting for ckeditor body")
      wait.until { _browser.find_element(:xpath => @galmgr.ckeditor_body) }
      Log.logger.info("Typing in new description")

      type_text_in_wysiwyg_editor(new_desc)

      _browser.switch_to.default_content
      frame = wait.until { _browser.find_element(:xpath => @galmgr.edit_overlay_frame) }
      _browser.switch_to.frame(frame)
    end
    Log.logger.info("Clicking on save gallery button")
    wait.until { _browser.find_element(:xpath => @galmgr.save_gallery_btn) }.click
    Log.logger.info("Waiting for confirmation message")
    confirmation_msg = wait.until { _browser.find_element(:xpath => @galmgr.confirmation_message) }.text
    Log.logger.info("Got confirmation message: #{confirmation_msg.inspect}")
    frame = wait.until { _browser.find_element(:xpath => @galmgr.close_overlay) }
    Log.logger.info("Clicking on close overlay link")
    begin
      alert = _browser.switch_to.alert
      Log.logger.info("Encountered an alert while closing overlay...perhaps I shouldn't be using the overlay!!!!")
      alert.accept
    rescue
      Log.logger.info("No alert present...continuing")
    end
    wait.until { _browser.find_element(:xpath => @galmgr.close_overlay) }.click
    _browser.switch_to.default_content
    return confirmation_msg
  end

  # Reads the present values of the edit_all_galleries various settings. Takes as input which settings value needs to be checked.
  # Argument will be among these: title, description, url, rows, cols, media_display, desc_format.

  def read_edit_all_galleries_values(property, _browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 5)
    Log.logger.info("Reading Edit all gallery's #{property} settings")
    frame = wait.until { _browser.find_element(:xpath => @galmgr.eag_overlay_frame) }
    _browser.switch_to.frame(frame)
    case property
    when 'title'
      title = @galmgr.eag_title
      val = self.get_value(title)
    when 'description'
      description = @galmgr.eag_desc
      val = self.get_value(description)
    when 'url'
      url = @galmgr.eag_url_path
      val = self.get_value(url)
    when 'rows'
      rows = @galmgr.edit_gallery_rows
      val = self.get_value(rows)
    when 'desc_format'
      desc_format = @galmgr.eag_desc_format
      val = self.get_value(desc_format, label = true)
    when 'cols'
      cols = "//div[contains(@class,'form-item')]/select[contains(@id,'edit-media-gallery-columns-und')]/option[@selected='selected']"
      val = self.get_value(cols)
    when 'media_display'
      media_display = "//div[contains(@class,'form-item')]/select[contains(@id,'edit-media-gallery-image-info-where')]/option[@selected='selected']"
      val = self.get_value(media_display)
    else
      Log.logger.info("Wrong Property Name mentioned '#{property}', Accepted are one of the following: title, description, desc_format, url, cols, rows, media_display ")
      val = nil
    end
    _browser.switch_to.default_content
    return val
  end

  # Related to read_edit_all_galleries_values to retrieve values of particular element

  def get_value(element, label = false, _browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 5)
    elem = wait.until { _browser.find_element(:xpath => element) }
    val = nil
    text = nil
    if (label)
      val = elem.find_element(:xpath => "//option[@selected]").text
    else
      val = elem.attribute("value")
      text = elem.text
      Log.logger.info("Value: #{val} Text: #{text}")
    end
    return val if text.nil? or text.empty? or (text == val)
    return text
  end

  # Count the total number of galleries on the specific page

  def count_number_of_galleries(_browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 5)
    Log.logger.info("Counting the number of galleries present on the galleries first page.")
    nog = wait.until { _browser.find_elements(:css => 'div#main div.media-gallery-item') }.size
    return nog
  end

  # Extracts the title of the gallery from the currently opened gallery page

  def get_galleries_title(_browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 5)
    actual_title = wait.until { _browser.find_element(:xpath => @galmgr.galleries_title) }.text
    return actual_title
  end

  # Extract the decription of the gallery from currently opened gallery page

  def get_galleries_desc(_browser = @browser)
    JQuery.wait_for_events_to_finish(_browser)
    desc_present = _browser.find_elements(:xpath => @galmgr.galleries_desc).size > 0
    if(desc_present)
      actual_desc = _browser.find_element(:xpath => @galmgr.galleries_desc).text
      return actual_desc
    else
      return nil
    end
  end

  # Extract the url of the currently opened gallery

  def get_galleries_url(_browser = @browser)
    actual_url = _browser.current_url
    return actual_url
  end

  # Deletes all the galleries present on the page by checking the select all checkbox.
  # Emery: should we deprecate this?
  def delete_all_galleries( _browser = @browser)
    Log.logger.info("Function is deprecated...calling 'destroy_existing_galleries'")
    return destroy_existing_galleries(_browser)
=begin
    wait = Selenium::WebDriver::Wait.new(:timeout => 10)
    Log.logger.info("Deleting all the galleries all together.")
    _browser.get("#{$config["sut_url"]}/admin/content")
    _browser.switch_to.default_content
    self.filter_content(type = "Gallery")
    nodes_to_select = _browser.find_elements(:xpath => "//input[contains(@id, 'edit-nodes-')]").size
    nog = _browser.find_elements(:xpath => "//tbody/tr/td[text() = 'Gallery']").size
    if(nodes_to_select == nog)
      begin
        item = wait.until { _browser.find_element(:xpath => @galmgr.select_all_galleries) }
        wait.until { item.displayed? }
        item.click #USED TO BE CLICK_AT(1,1)
      end
      delete_confirm_msg = self.delete_content_action
      return delete_confirm_msg, nog
    else
      Log.logger.info("Can't delete galleries, as the filtering to select only galleries is not working.")
      return nil, nog
    end
=end
  end

  def destroy_existing_galleries(_browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 10)
    flag          = false
    delete_button = "//input[@id = 'edit-submit' and @value = 'Delete']"
    msg_box       = "//div[@class = 'messages status']"
    table         = "//table[contains(@class,'sticky-table')]"
    table_row     = "#{table}/tbody/tr"
    _browser.get("#{$config["sut_url"]}/admin/content")
    self.filter_content(type = "Gallery")
    wait.until { _browser.find_element(:xpath => table) }
    rows = wait.until { _browser.find_elements(:xpath => table_row) }.size
    return unless rows > 0
    (1..rows).each { |row|
      row_type = "(#{table_row}[#{row}]/td[3])"
      chk_box  = "(#{table_row}[#{row}]/td[1]/div/input[@type='checkbox'])"
      flag = self.check_row(_browser, row_type, chk_box) || flag
    }
    return unless flag
    begin
      elem = wait.until { _browser.find_element(:xpath => "//div/select[@id = 'edit-operation']/option[@value='delete']") }
      elem.click
    rescue Exception => e
      Log.logger.info("Blew up trying to delete the items...JS click!")
      elem = wait.until { _browser.find_element(:xpath => "//div/select[@id = 'edit-operation']/option[@value='delete']") }
      script = "arguments[0].click();"
      _browser.execute_script(script, elem)
      JQuery.wait_for_events_to_finish(_browser)
    end
    wait.until { _browser.find_element(:xpath => "//input[@id='edit-submit--2']") }.click
    JQuery.wait_for_events_to_finish(_browser)
    unless _browser.find_element(:xpath => "//body").text.include?("cannot be undone")
      Log.logger.info("Couldn't verify deletion in progress")
    end
    temp = wait.until { _browser.find_element(:xpath => delete_button) }
    wait.until { temp.displayed? }
    temp.click
    JQuery.wait_for_events_to_finish(_browser)
    text_flag = _browser.find_element(:xpath => "//body").text.include?("Deleted")
    msg_flag = _browser.find_elements(:xpath => msg_box).size > 0
    unless text_flag && msg_flag
      Log.logger.info("Deletion unsuccessful...")
    end
    msg_text = _browser.find_element(:xpath => msg_box).text
    Log.logger.info("Returning to root page...")
    _browser.get($config["sut_url"])
    return msg_text, rows
  end

  def check_row( _browser, row_type, row_tick)
    gall = _browser.find_element(:xpath => row_type)
    box  = _browser.find_element(:xpath => row_tick)
    if (gall.text.downcase.include?("gallery"))
      box.click unless box.selected?
      return true
    end
    return false
  end

  # Retrieves the names of the galleries present on the page from their respective url, one's having space in it will be replaced by '-'
  # EMERY: I think we should deprecate the function below in favor of "get_gallery_names"
  def get_galleries_names_on_page(_browser = @browser)
    Log.logger.info("Getting gallery link-names on the page")
    i = 1
    galleries_list = []
    galleries_xpath = "//div[contains(@class, 'media-gallery-collection')]/div"
    nog = Integer(_browser.find_elements(:xpath => galleries_xpath).size)
    Log.logger.info("Found #{nog} galleries")
    while i <= nog
      JQuery.wait_for_events_to_finish(_browser)
      gal_name = _browser.find_element(:xpath => "//div[contains(@class, 'media-gallery-collection')]/div[#{i}]").attribute("about")
      gal_name.gsub!(/\/content\//,'')
      i += 1
      galleries_list << gal_name
    end
    return galleries_list
  end

  def get_gallery_names(_browser = @browser)
    _browser.get("#{$config["sut_url"]}/galleries") unless _browser.current_url.split('/').last.include?('galleries')
    children = self.compose_gallery_path("/child::*")
    unless _browser.find_elements(:xpath => children).size > 0
      Log.logger.info("It appears there are no galleries to collect...returning")
      return
    end
    index = 1
    names = []
    until  _browser.find_elements(:xpath => self.gallery_element_by_index(index)).size < 1
      element = self.gallery_element_by_index(index)
      names << self.find_gallery_name(_browser, element)
      index += 1
    end
    return names
  end

  def get_gallery_index(_browser,_name)
    _browser.get("#{$config["sut_url"]}/galleries") unless _browser.current_url.split('/').last.include?('galleries')
    children = self.compose_gallery_path("/child::*")
    unless _browser.find_elements(:xpath => children).size > 0
      Log.logger.info("It appears there are no galleries to collect...returning")
      return
    end
    index = 1
    until  _browser.find_elements(:xpath => self.gallery_element_by_index(index)).size < 1
      element = self.gallery_element_by_index(index)
      return index if _name.include?(self.find_gallery_name(_browser, element))
      index += 1
    end
    return -1
  end

  def gallery_element_by_index(_index)
    "(//div[@class='content']/div[contains(@class,'media-gallery-sortable-processed ui-sortable') and contains(@class,'media-gallery-collection')]/div[contains(@id,'node-')])[#{_index}]"
  end

  def compose_gallery_path(_extension = "")
    "(//div[contains(@class,'media-gallery-sortable-processed ui-sortable')]#{_extension})"
  end

  def node_path(_name, relative = false)
    return "/div[contains(@class,'node-media-gallery') and contains(@about,#{_name})]" unless relative
    return "//div[contains(@class,'node-media-gallery') and contains(@about,#{_name})]"
  end

  def find_gallery_node_id(_browser,_name)
    _node_path = self.node_path(_name)
    _element_path = self.compose_gallery_path(_node_path)
    node_id = _browser.find_element(:xpath => _element_path).attribute("id")
    Log.logger.info("Node ID: #{node_id}")
    return node_id
  end

  def find_gallery_name(_browser, _element)
    about = _browser.find_element(:xpath => _element).attribute("about")
    return (about.gsub("/content/",""))
  end

  # Checks the status message of the gallery after creation of new gallery.
  def check_create_gallery_status_message(gal_name, _browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 10)
    status_message_gallery_name = "//div[@class = 'messages status']/em"
    actual_gallery_title = wait.until { _browser.find_element(:xpath => status_message_gallery_name) }.text
    return actual_gallery_title
  end

  # Delete all the medis on the page, except the default created by the anonymous user

  def delete_media_created_by_currentuser(_browser = @browser)
    Log.logger.info("Deleting all the media created by testuser named qaadmin")
    _browser.get("#{$config["sut_url"]}/admin/content/media")
    user = _browser.find_element(:xpath => "//a[@title='User account']/strong").text
    n = _browser.find_elements(:xpath => "//td[5]/a[text() = '#{user}']/../preceding-sibling::*[4]/div/input").size
    i = 1
    while i < n + 1
      wait.until { _browser.find_element(:xpath => "//td[5]/a[text() = '#{user}']/../preceding-sibling::*[4]/div[#{i}]/input") }.click
      JQuery.wait_for_events_to_finish(_browser)
      i += 1
    end
    self.delete_media_action
  end

  # Check the status message of the gallery after performing any action on the gallery through edit gallery tab.

  def check_gallery_status_message(gal_name, action = "deleted", _browser = @browser)
    wait = Selenium::WebDriver::Wait.new(:timeout => 10)
    message = wait.until { _browser.find_element(:xpath => "//div[contains(@class, 'messages status')]") }.text
    if (action == "deleted")
      expected_message = "Gallery #{gal_name} has been deleted."
    elsif (action == "blocks_update")
      expected_message = "The block settings have been updated."
    else
      expected_message = "Gallery #{gal_name} has been updated."
    end
    Log.logger.info("Confirmation Message '#{message}' not correct, therefore #{action} action gallery might not have been executed.") unless expected_message.include?(message)
  end

  def utc_timestamp
    Time.now.to_i
  end

  def get_random_string(length=12)
    string = ""
    chars = ("a".."z").to_a
    length.times do
      string << chars[rand(chars.length-1)]
    end
    return string
  end

  # GUI MAP

  class GalleryManagerGM
    attr_reader :home
    attr_reader :gallery_title, :gallery_desc
    attr_reader :edit_gallery_cols, :edit_gallery_rows
    attr_reader :edit_gallery_media_display
    attr_reader :image_download_chkbox
    attr_reader :media_on_fullpage, :media_on_lightbox, :media_on_lightbox_desc
    attr_reader :save_gallery_btn
    attr_reader :galleries_link
    attr_reader :add_media
    attr_reader :media_upload_frame
    attr_reader :embed_image_video, :url_textbox, :submit_url_btn, :cancel_url_btn
    attr_reader :findcontent_update_btn, :action
    attr_reader :content_type, :filter_btn
    attr_reader :sort_asc
    attr_reader :first_gallery_checkbox
    attr_reader :confirm_delete, :confirmation_message, :warning_message
    attr_reader :eag_overlay_frame, :edit_gallery, :edit_media, :delete_gallery_btn, :confirm_delete_gallery_btn
    attr_reader :edit_media_submit, :edit_overlay_frame
    attr_reader :find_content_link, :find_content_media_link
    attr_reader :gallery_block_link, :gallery_block_chkbox, :gallery_block_cols, :gallery_block_rows
    attr_reader :image_license_info
    attr_reader :close_overlay

    attr_reader :edit_all_galleries_tab, :config_tab, :gallery_settings
    attr_reader :galleries_title, :galleries_desc
    attr_reader :eag_title, :eag_desc, :eag_desc_format, :eag_url_path
    attr_reader :select_all_galleries, :ckeditor_frame, :ckeditor_body

    def image_link(img_url)
      return "//div[contains(@id, 'media-gallery-media-')]//a/img[contains(@src, '#{img_url}')]"
    end

    def initialize()
      @home = '//a[@href = "/"]'
      @find_content_link = '//a[@href = "/admin/content"]'
      @find_content_media_link = '//a[@href = "/admin/content/media"]'
      @gallery_title = "//input[@id='edit-title']"
      @gallery_desc = '//textarea[contains(@id, "edit-media-gallery-description-und-0-value")]'
      @edit_gallery_cols = '//select[@id = "edit-media-gallery-columns-und"]'
      @edit_gallery_rows = '//input[@id = "edit-media-gallery-rows-und-0-value"]'
      @edit_gallery_media_display = '//select[contains(@id, "edit-media-gallery-image-info-where-und")]'
      @image_download_chkbox = '//input[contains(@id, "edit-media-gallery-allow-download-und")]'
      @media_on_fullpage = '//input[contains(@id, "edit-media-gallery-format-und-node")]'
      @media_on_lightbox = '//input[contains(@id, "edit-media-gallery-format-und-lightbox")]'
      @media_on_lightbox_desc = '//input[contains(@id, "edit-media-gallery-lightbox-extras-und")]'
      @galleries_link = '//a[contains(@href, "/gallery-collections/")]'
      #      @add_media = '//a[text()="Add media"]'
      #'-processed' is added after Javascript grabs the link. After that, we get the fancy popover for embedding images/videos
      #Without JS: <a class="media-gallery-add launcher" href="/media/browser?render=media-popup">Add media</a>
      #With JS: <a href="/media/browser?render=media-popup" class="media-gallery-add launcher media-gallery-add-processed">Add media</a>
      @add_media = '//a[@href and contains(@class, "media-gallery-add-processed")]'

      @media_upload_frame = '//iframe[@id = "mediaBrowser"]'
      @embed_image_video = '//a[@href = "#media-tab-media_internet"]/span' #[text() = "Embed image/video"]' Bug related to this AN-19634, will fix this later
      @url_textbox = '//input[@id = "edit-embed-code"]'
      @submit_url_btn = '//input[@id = "edit-submit--2"]'
      @cancel_url_btn = '//div[@id = "edit-actions"]/a[text() = "Cancel"]'
      @findcontent_update_btn = '//input[@id = "edit-submit--2"]'
      @action = '//select[@id = "edit-operation"]'
      @content_type = '//select[@id="edit-type"]'
      @filter_btn = '//input[(@id = "edit-submit") and (@value = "Filter")]'
      @sort_asc = '//table[contains(@class, "sticky-enabled")]/thead/tr/th/a[@title = "sort by Updated"]'
      @first_gallery_checkbox = '//table[contains(@class, "sticky-enabled")]/tbody/tr[1]/td/div/input'
      @confirm_delete = '//input[@id = "edit-submit"]'
      @confirmation_message = '//div[contains(@class, "messages status")]'
      @warning_message = '//div[contains(@class, "messages warning") and not(contains(@class, "email-verification-reminder")) and not (contains(@class, "twitter-alert"))]'
      @save_gallery_btn = '//input[@id="edit-submit"]'
      @edit_gallery = '//div[@id="main"]//a[text() = "Edit gallery"]'
      @edit_media = "//*[contains(text(),'Edit media')]"
      @delete_gallery_btn = '//input[@id = "edit-delete"]'
      @confirm_delete_gallery_btn = '//input[(@id = "edit-submit") and (@value = "Delete")]'
      @edit_media_submit = '//input[contains(@id, "edit-submit")]'
      @edit_overlay_frame = '//iframe[contains(@class,"overlay-element") and contains(@class, "overlay-active")]'
      @eag_overlay_frame = "//iframe[@title = 'Gallery settings dialog']"
      @gallery_block_link = '//a[@href = "#"]/strong[text() = "Blocks"]'
      @gallery_block_chkbox = '//input[@id="edit-media-gallery-expose-block-und"]'
      @gallery_block_cols = '//select[@id="edit-media-gallery-block-columns-und"]'
      @gallery_block_rows = '//input[@id="edit-media-gallery-block-rows-und-0-value"]'
      @image_license_info = '//div[@class = "media-gallery-detail-info"]/span[contains(@class, "media-license")]/span'
      @close_overlay = "//*[contains(text(),'Close overlay')]"

      @edit_all_galleries_tab = "//*[contains(text(),'Edit all galleries')]"
      @config_tab = '//a[contains(@href, "/admin/config")]'
      @gallery_settings = "//*[contains(text(),'Gallery settings')]"
      @galleries_title = '//h1[@id = "page-title"]'
      @galleries_desc = '//div[contains(@class, "taxonomy-term-description")]/p'
      @eag_title = '//input[contains(@id, "edit-name")]'
      @eag_desc = '//textarea[@id="edit-description-value"]'
      @eag_desc_format = '//select[@id="edit-description-format--2"]'
      @eag_url_path = '//input[@id="edit-path-alias"]'
      @select_all_galleries = '//th[contains(@class, "select-all")]/input'
      @ckeditor_frame = '//iframe[contains(@title, "Rich text editor, edit-description-value,")]'
      @ckeditor_body = '//body[@class = "cke_show_borders"]'
    end
  end
end
