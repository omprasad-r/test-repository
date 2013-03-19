require "rubygems"
require "selenium/webdriver"

class GardensManager

  attr_reader :export_path, :gardensmgr

  def initialize(_browser,_url=nil)
    @browser = _browser
    @gardensmgr = GardensManagerGM.new()
    @sut_url = _url || $config['sut_url']
  end
  
  # This method is defined to switch the browser session from gardener to gardens.

  def login(current_page, login_info, password_info)
    wait = Selenium::WebDriver::Wait.new(:timeout => 30)
    @browser.get("#{@sut_url}#{current_page}")
    wait.until { @browser.find_element(:xpath => @gardensmgr.login_reg_link) }.click
    frame = wait.until { @browser.find_element(:xpath => @gardensmgr.overlay_frame) }
    @browser.switch_to.frame(frame)
    temp = wait.until { @browser.find_element(:xpath => @gardensmgr.login_txtbox) }
    temp.clear
    temp.send_keys(login_info)
    temp = @browser.find_element(:xpath => @gardensmgr.password_txtbox)
    temp.clear
    temp.send_keys(password_info)
    @browser.find_element(:xpath => @gardensmgr.login_btn).click
    @browser.switch_to.default_content 
    @browser.get("#{@sut_url}#{current_page}")  # Sometimes site doesn't login unless until user refreshes the page.
  end
  
  class GardensManagerGM
    attr_reader :login_reg_link, :overlay_frame
    attr_reader :password_txtbox, :login_txtbox, :login_btn
    
    def initialize()
      @login_reg_link = 'link=Login or register'
      @overlay_frame = '//iframe[contains(@class, "overlay-element overlay-active")]'
      @login_txtbox = '//input[@id="edit-name"]'
      @password_txtbox = '//input[@id="edit-pass"]'
      @login_btn = '//input[@id="edit-submit"]'
    end
  end
end
