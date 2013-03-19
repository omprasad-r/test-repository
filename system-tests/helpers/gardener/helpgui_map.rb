class HelpGUIMap
  attr_reader :help_link, :help_div, :help_search_textbox, :help_search_button, :help_dropdown_time, 
  :help_search_form, :video_section, :video_page_title, :documentation_section, :documentation_section_title,
  :support_section, :help_section_page_search_textbox, :help_section_page_search_button, :documentation_page_link_menu,
  :documentation_page_title
  
  def initialize()
    @help_link = "//li[contains(@class,'primary-link-help') and a='Help']/a"
    @help_dropdown_time = 2
    #@help_div = "//div[@id='header-wrapper']/div[@id='help-header']"
    @help_div = "//div[@id='help-header']"
    
    @help_search_form = "//form[@id='gardens-help-search-form']"
    @help_search_textbox = "//form[@id='gardens-help-search-form']/input[@type='text']"
    @help_search_button = "//form[@id='gardens-help-search-form']/input[@type='submit']"
    
    @video_section = "//div[@id='help-section-videos']/div"
    @documentation_section = "//div[@id='help-section-documentation']/div"
    @support_section = "//div[@id='help-section-support']/div"
    
    @video_page_title = "//div[h1='Videos']"
    @documentation_page_title = "//div[contains(h1,'')]/h1"
    
    @help_section_page_search_textbox = "//form[@id='search-block-form']//input[contains(@id,'edit-search') and @type='text']"
    @help_section_page_search_button = "//form[@id='search-block-form']//input[@id='edit-submit' and @type='submit']"
    
    @documentation_page_link_menu = "//div[contains(@class,'book-navigation')]/ul[contains(@class,'menu')]"
  end
end