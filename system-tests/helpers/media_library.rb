require "rubygems"
require "selenium/webdriver"
require "acquia_qa/log"

class MediaLibrary

  attr_reader :mlgm
  
  def initialize(_browser=nil, _site_info=nil)
    @browser = _browser
    @site_info = _site_info
    @mlgm = MediaLibraryGM.new()
  end
  
  # Uploads the image from the 'From URL' tab and takes one argument
  # _image_url is the url from where the image will be uploaded
  def upload_image_from_url(_image_url)
    wait = Selenium::WebDriver::Wait.new(:timeout => 15)
    b_frame = @browser.find_element(:xpath => @mlgm.media_browser_frame)
    @browser.switch_to.frame(b_frame)
    wait.until { @browser.find_element(:xpath => @mlgm.image_from_url) }.click
    temp = wait.until { @browser.find_element(:xpath => @mlgm.url_edit) }
    temp.clear
    temp.send_keys(_image_url)
    wait.until { @browser.find_element(:xpath => @mlgm.submit_image) }.click
    s_frame = wait.until { @browser.find_element(:xpath => @mlgm.media_style_frame) }
    @browser.switch_to.frame(s_frame)
    wait.until { @browser.find_element(:xpath => @mlgm.confirm_image_submit) }.click
    @browser.switch_to.default_content
  end

  class MediaLibraryGM
    attr_reader :media_browser_frame
    attr_reader :image_from_url, :url_edit, :submit_image    
    attr_reader :media_style_frame, :confirm_image_submit
    
    def initialize()
      @media_browser_frame = '//iframe[@id = "mediaBrowser"]'
      
      @image_from_url = '//a[contains(@href, "#media-tab-media_internet")]'
      @url_edit = '//input[@id = "edit-embed-code"]'
      @submit_image = '//input[@id = "edit-submit--2"]'
      
      @media_style_frame = '//iframe[@id = "mediaStyleSelector"]'
      @confirm_image_submit = '//form[@id="media-format-form"]/a[text()="Submit"]'
      
    end
  end
  
end
