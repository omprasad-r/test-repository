$LOAD_PATH << File.dirname(__FILE__)
require 'rubygems'
require 'acquia_qa/log'
require 'capybara'
require 'capybara/dsl'
require 'jquery_capybara.rb'

class ContentTypeCapy

  def initialize(options = {})
    Log.logger.info "Creating new content type #{options.inspect}"
    name = options[:name]

    init_new(options) unless options[:already_exists]

    @content_type_name = name
    @content_type_url = "/admin/structure/types/manage/#{name}"
    @display_options_url = "#{@content_type_url}/display"
    @create_content_url = "/node/add/#{@content_type_name.gsub('_', '-')}"
  end


  def init_new(options = {})
    Capybara.visit("/admin/structure/types/add")
    name = options[:name]

    Log.logger.info "Filling in options: #{options.inspect}"
    Capybara.within(:xpath, "//form[@id='node-type-form']") do
      Capybara.fill_in("name", :with => name)
      Capybara.fill_in("description", :with => options[:desc])
      Capybara.click_button("edit-submit")
    end

    if Capybara.has_no_content?("The content type #{name} has been added.")
      raise "Could not create content type"
    end
  end

  private :init_new

  def add_new_field(options = {})
    Log.logger.info "Creating new field: #{options.inspect}"

    ContentFieldCapy.new(
      :name => options[:field_name],
      :type => options[:field_type],
      :containing_type_url => @content_type_url
    )
  end

  def display_options(options = {}, &block)
    Log.logger.info "Setting display_options options for #{@content_type_name}: #{block.inspect}"
    is_teaser = options[:is_teaser] || false

    Log.logger.info "Visiting: #{@display_options_url}"
    Capybara.visit(@display_options_url)

    if is_teaser
      Log.logger.info "Selecting teaser display type"
      Capybara.click_link("Teaser")
      Capybara.wait_until(15) { Capybara.has_xpath?("//a[@class='active' and text()='Teaser']") }
    end

    Capybara.within(:xpath, "//form[contains(@id, 'field-ui-display-')]") do
      # Set module options
      block.call

      Capybara.click_button("edit-submit")
    end

    if Capybara.has_no_content?("Your settings have been saved")
      raise "Could not change display options for #{@content_type_name}"
    end

  end

  def create_content(options = {})
    Log.logger.info "Creating new content #{options.inspect}"

    Log.logger.info "Visiting #{@create_content_url}"
    Capybara.visit(@create_content_url)

    Capybara.within(:xpath, "//form[contains(@id, '-node-form')]") do
      JQuery.wait_for_events_to_finish
      Capybara.wait_until(30) { Capybara.find(:css, "td.cke_contents iframe").visible? }
      JQuery.wait_for_events_to_finish

      Log.logger.info "Disabling WYSIWYG"
      Capybara.find(:xpath, ".//div[contains(@class, 'wysiwyg-tab disable')]").click
      Capybara.wait_until(30) { Capybara.find(:css, "div.textarea-processed textarea.text-full.form-textarea").visible? }

      Capybara.fill_in("title", :with => "Test title for type #{@content_type_name}")
      Capybara.fill_in("edit-body-und-0-summary", :with => "Summary for type #{@content_type_name}")
      Capybara.fill_in("edit-body-und-0-value", :with => "<p>Test content for type #{@content_type_name}</p>")

      Log.logger.info "Saving new content"
      Capybara.click_button("edit-submit")
    end

    if Capybara.has_no_content?("has been created.")
      raise "Could not create content of type #{@content_type_name}"
    end

    Capybara.current_url
  end

end

class ContentFieldCapy

  attr_reader :field_name

  def initialize(options = {})
    @field_name = options[:name]
    @field_type = options[:type]

    manage_fields_url = "#{options[:containing_type_url]}/fields"
    @field_url = "#{manage_fields_url}/field_#{@field_name}"

    Log.logger.info "Visiting #{manage_fields_url}"
    Capybara.visit(manage_fields_url)

    Capybara.within(:xpath, "//tr[@id='-add-new-field']") do
      Capybara.fill_in("fields[_add_new_field][label]", :with => @field_name)
      Capybara.fill_in("fields[_add_new_field][field_name]", :with => @field_name)
      Capybara.select(@field_type, :from => "edit-fields-add-new-field-type")
    end

    Log.logger.info "Saving field #{options.inspect}"
    Capybara.click_button("edit-submit")

    Log.logger.info "Saving settings on #{Capybara.current_url}"
    Capybara.within(:xpath, "//form[contains(@id, 'field-ui-field-')]") do
      Capybara.click_button("edit-submit")
    end

    if Capybara.has_no_content?("Saved #{@field_name} configuration")
      raise "Could not add field #{options.inspect}"
    end
  end

  def change_widget_type(widget_type)
    Log.logger.info "Changing field #{@field_name} to widget type #{widget_type}"
    widget_url = "#{@field_url}/widget-type"

    Log.logger.info "Visiting: #{widget_url}"
    Capybara.visit(widget_url)

    Capybara.within(:xpath, "//form[@id='field-ui-widget-type-form']") do
      begin
        Capybara.select(widget_type, :from => "edit-widget-type")
        Capybara.click_button("edit-submit")
      rescue Capybara::ElementNotFound
        raise "Field #{@field_name} has no widget type #{widget_type}"
      end
    end

    if Capybara.has_no_content?("Changed the widget for field #{@field_name}")
      raise "Could not change widget type for field #{@field_name}"
    end
  end

  def change_options(&block)
    Log.logger.info "Setting field options for #{@field_name}: #{block.inspect}"

    Log.logger.info "Visiting: #{@field_url}"
    Capybara.visit(@field_url)

    Capybara.within(:xpath, "//form[contains(@id, 'field-ui-field-')]") do
      # Set module options
      block.call unless block.nil?

      Capybara.click_button("edit-submit")
    end

    if Capybara.has_no_content?("Saved #{@field_name} configuration")
      raise "Could not change options for field #{@field_name}"
    end
  end

end
