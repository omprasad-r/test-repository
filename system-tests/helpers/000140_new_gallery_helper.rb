require "rubygems"
require "selenium/webdriver"
require "rspec"
require "galleries_manager.rb"

module Test000140NewGalleryHelper

  # Check the status message of the gallery after deletion of new gallery from Find Content tab.
  def check_delete_recent_gallery_status_message(gal_name)
    gallery_manager = GalleryManager.new(@browser)
    delete_confirm_msg = gallery_manager.delete_recently_added_gallery
    delete_confirm_msg.should include("Deleted 1 posts.")
  end

  # Check the status message of the gallery after performing any action on the gallery through edit gallery tab.
  def check_gallery_status_message(gal_name, action = "deleted")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    begin
      m_t = wait.until { @browser.find_element(:xpath => "//div[contains(@class, 'messages status')]") }
    rescue
      m_t = wait.until { @browser.find_element(:xpath => "//div[contains(@class, 'messages error')]") }
    end
    message = m_t.text
    if (action == "deleted")
      expected_message = "Gallery #{gal_name} has been deleted."
    elsif (action == "blocks_update")
      expected_message = "The block settings have been updated."
    else
      expected_message = "Gallery #{gal_name} has been updated."
    end
    message.should include(expected_message)
  end

  # Changes the location of the gallery blocks to various positions
  # Takes as argument the gallery name and location where the block needs to be shifted.
  def change_blocks_location(gal_name, gallery_manager, move_block_to)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    save_blocks_btn = "//div[@id = 'edit-actions']/input[(@id='edit-submit') and contains(@value, 'Save blocks')]"
    block_id = "//td[contains(text(),'Recent gallery items: #{gal_name}')]"
    block_drop_down = '(' + block_id + '/following-sibling::*[1])/.//select[contains(@class,"block-region-select")]'
    @browser.get(@sut_url + '/admin/structure/block')
    if (@browser.find_elements(:xpath => block_id).size > 0)
      wait.until { @browser.find_element(:xpath => block_drop_down) }
      Log.logger.info("Found our select! Trying to select region: #{move_block_to}")
      wait.until { @browser.find_element(:xpath => "#{block_drop_down}/option[contains(text(),'#{move_block_to}')]") }.click
      sbb = wait.until { @browser.find_element(:xpath =>save_blocks_btn) }
      sbb.click
      self.check_gallery_status_message(gal_name, action = "blocks_update")
    else
      raise "There is no block created for the gallery '#{gal_name}'. Please create a block."
    end
    gallery_manager.return_to_home_page
    wait.until { @browser.find_element(:xpath =>"//a[@href = '/']") }
  end


  # Deletes the galleries and their related media completely.
  def delete_gallery_and_related_media(n, gal_name)
    gallery_manager = GalleryManager.new(@browser)
    gallery_manager.delete_gallery(gal_name)
    self.check_gallery_status_message(gal_name, action = "deleted")
    warning_ct, warning_msg, confirm_ct, confirm_msg = gallery_manager.delete_media(n)
    puts "Got confirmation message: #{confirm_msg.inspect}."
    if (confirm_ct == 0)
      Log.logger.info("No files deleted")
    else
      num_of_del = confirm_msg.scan('was deleted')
      files_deleted = num_of_del.length
      Log.logger.info("Deleted #{files_deleted} files.")
    end
    if (warning_msg == "No Warnings")
      Log.logger.info("All files deleted")
    else
      warning_msg.gsub!(/The file /,'')
      warning_msg.gsub!(/is in use and cannot be deleted./,'')
      warning_msg.gsub!(/ /,', ')
      Log.logger.info("Couldn't delete the following files: #{warning_msg}")
    end
  end

  # Verifies the gallery's settings like cols, rows, media display settings etc for the gallery's first page.
  def verify_changed_gallery_settings(nom, cols, media_display = "Show nothing")
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    Log.logger.info("Starting test for Verification of Gallery Settings done in previous test")
    i = 0
    while i < nom
      image_xpath = "//div[contains(@class, 'media-gallery-sortable-processed') and contains(@class, 'ui-sortable')]/div[contains(@id, 'media-gallery-media-#{i}')]"
      @browser.find_elements(:xpath => image_xpath).should have_at_least(1).items
      @browser.find_element(:xpath => "#{image_xpath}//div[@class='gallery-thumb-inner']/a/img").click
      cbox = wait.until { @browser.find_element(:xpath =>"//div[@id='cboxClose']") } #this element is invis...needs hover or js_click
      Log.logger.info("Clicking cbox via js...")
      begin
        elem = @browser.find_element(:xpath => "//div[@id='cboxClose']")
        script = "arguments[0].click();"
        @browser.execute_script(script, elem)
      rescue Exception => e
        Log.logger.info("Caught an exception: #{e.inspect}")
      end
      if (media_display == "Show nothing")
        # Actual element is "//div[@id='media-gallery-media-#{i}']/div/span/span/span/span[contains(@class, 'media-title')]". Asertion is for relative path.
        @browser.find_elements(:xpath => "#{image_xpath}/.//span[contains(@class, 'media-title')]").should be_empty
        # Actual element is "//div[@id='media-gallery-media-#{i}']/div/a/span/span/span[contains(@class, 'media-title')]". Asertion is for relative path. Difference is a tag.
        @browser.find_elements(:xpath => "#{image_xpath}//a//span[contains(@class, 'media-title')]").should be_empty
      end
      if (media_display == "Show title below")
        # Actual element is "//div[@id='media-gallery-media-#{i}']/div/span/span/span/span[contains(@class, 'media-title')]". Asertion is for relative path.
        @browser.find_elements(:xpath => "#{image_xpath}//span[contains(@class, 'media-title')]").should have_at_least(1).items
      end
      if (media_display == "Show title on hover")
        # Actual element is "//div[@id='media-gallery-media-#{i}']/div/a/span/span/span[contains(@class, 'media-title')]". Asertion is for relative path. Difference is a tag.
        @browser.find_elements(:xpath => "#{image_xpath}//a//span[contains(@class, 'media-title')]").should have_at_least(1).items
      end
      i += 1
    end
    @browser.find_elements(:xpath => "//div[contains(@class, 'content')]/div[contains(@class, 'mg-col-#{cols}')]").should have_at_least(1).items
    # NOTE: This last four assertions will only work if the gallery has number of images more than nom.
    @browser.find_elements(:xpath => "//a[contains(@title, 'Go to page 2')]").should have_at_least(1).items
    @browser.find_elements(:xpath => "//a[contains(@title, 'Go to next page')]").should have_at_least(1).items
    @browser.find_elements(:xpath => "//a[contains(@title, 'Go to last page')]").should have_at_least(1).items
    @browser.find_elements(:xpath => "//div[contains(@class, 'media-gallery-sortable-processed') and contains(@class, 'ui-sortable')]/div[contains(@id, 'media-gallery-media-#{nom}')]").should be_empty
  end

  # Verifies the gallery's presentation settings for each and every image of the gallery present on the first page.
  def verify_changed_gallery_presentation_settings(gal_name, options)
    image_download_chkbox = options[:download]
    media_on_fullpage = options[:media_view] == :fullpage
    media_on_lightbox_desc = options[:lightbox_desc_view]

    wait = Selenium::WebDriver::Wait.new(:timeout => 10)
    Log.logger.info("Starting test for Verification of Gallery's Presentation Settings done in previous test")
    nom = @browser.find_elements(:xpath => "//div[@id='content-area']//div[contains(@id, 'media-gallery-media-')]").size
    Log.logger.info("Found #{nom} gallery items, checking them")
    0.upto(nom - 1) do |i|
      Log.logger.info("Checking gallery item: #{i}")
      image_xpath = "//div[@id='content-area']//div[@id='media-gallery-media-#{i}']"
      @browser.find_elements(:xpath => image_xpath).should have_at_least(1).items
      Log.logger.info("Clicking on Item #{i}")
      @browser.find_element(:xpath => "#{image_xpath}//a[contains(@class,'media-gallery-thumb')]").click
      #wait for the item to pop up
      wait_start = Time.now
      #Wait til the loading garphic is invisible again
      wait.until { @browser.find_element(:xpath =>"//div[@id='cboxLoadingGraphic' and contains(@style,'display: none')]") }
      #check that the media license is visible
      wait.until { @browser.find_element(:xpath =>"//div[@id='colorbox' and not(contains(@style,'display: none'))]//span[@class='media-license medium']") }
      Log.logger.info("Gallery item popped up (#{Time.now - wait_start}s)")
      Log.logger.info("Waiting for the media license class")
      wait.until { @browser.find_element(:xpath =>"//span[@class='media-license medium']") }
      Log.logger.info("Waiting for the lightbox class")
      wait.until { @browser.find_element(:xpath =>"//div[@class='mg-lightbox-detail']") }
      Log.logger.info("Gallery item popped up (#{Time.now - wait_start}s)")
      if image_download_chkbox
        @browser.find_elements(:xpath => "//a[contains(@class, 'gallery-download')]").should have(1).items
      else
        @browser.find_elements(:xpath => "//a[contains(@class, 'gallery-download')]").should be_empty
      end
      if media_on_fullpage
        @browser.find_elements(:xpath => "//div[@id='colorbox' and not(contains(@style,'display: none'))]//div[@id='cboxClose']").should have_at_least(1).items
        @browser.find_elements(:xpath => "//div[@id='colorbox' and not(contains(@style,'display: none'))]//div[@id='cboxContent']").should have_at_least(1).items
        if media_on_lightbox_desc
          @browser.find_elements(:xpath => "//div[@id='colorbox' and not(contains(@style,'display: none'))]//div[contains(@class, 'lightbox-title')]").should have_at_least(1).items
          @browser.find_elements(:xpath => "//div[@id='colorbox' and not(contains(@style,'display: none'))]//div[contains(@class, 'mg-lightbox-description')]").should have_at_least(1).items
        else
          @browser.find_elements(:xpath => "//div[@id='colorbox' and not(contains(@style,'display: none'))]//div[contains(@class, 'lightbox-title')]").should be_empty
          @browser.find_elements(:xpath => "//div[@id='colorbox' and not(contains(@style,'display: none'))]//div[contains(@class, 'mg-lightbox-description')]").should be_empty
        end
        Log.logger.info("Closing item number #{i}")
        @browser.find_element(:xpath => "//div[@id='cboxClose']").click
        start_time = Time.now
        wait.until { @browser.find_element(:xpath =>"//div[@id='colorbox' and contains(@style,'display: none')]") }
        Log.logger.info("Item number #{i} closed after #{Time.now - start_time} s")
      else
        @browser.find_elements(:xpath => "//div[@id='colorbox' and not(contains(@style,'display: none'))]").should have_at_least(1).items
        @browser.find_elements(:xpath => "//div[contains(@class, 'media-gallery-detail')]").should have_at_least(1).items
        @browser.find_elements(:xpath => "//h1[@id = 'page-title']").should have_at_least(1).items
        @browser.find_element(:xpath => "//a[@href = '/content/#{gal_name}']").click
      end
    end
  end

end
