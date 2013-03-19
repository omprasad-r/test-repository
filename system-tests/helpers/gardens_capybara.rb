require 'rubygems'
require 'net/http'
require 'uri'
require 'capybara'
require 'capybara/dsl'
require 'acquia_qa/log'

class InvalidSiteName < StandardError; end

class GardensManagerCapy
  #To deal with the cpapybara 1.0 vs 0.4 changes
  begin
    include Capybara::DSL
  rescue NameError
    include Capybara
  end

  def initialize
    #nothing yet
  end

  def self.login(options = {})
    if $site_capabilities[:fast_user_switching] && options[:manual] != true
      username = options[:username] || $config['user_accounts']['qatestuser']['user']
      Log.logger.info("Using /devel/switch/#{username} to log in")
      Capybara.visit("/devel/switch/#{username}")
    else
      Log.logger.info("Using manual login")
      login_manual(options)
    end
  end

  def self.login_manual(options = {})
    #This seems to be a bit flaky with some drivers if we haven't visited a site before
    begin
      where_we_came_from = Capybara.current_url
      Capybara.visit('/') if where_we_came_from.include("about:")
    rescue StandardError
      Capybara.visit('/')
      where_we_came_from = Capybara.current_url
    end

    username = options[:username] || $config['user_accounts']['qatestuser']['user']
    password = options[:password] || $config['user_accounts']['qatestuser']['password']
    forced = (options[:forced] == true)
    do_not_go_back = (options[:do_not_go_back] == true)
    Log.logger.info("Logging in #{"(forced!) " if forced}using capybara and the #{Capybara.current_driver.inspect} driver (currently on: #{where_we_came_from.inspect})")

    #If we don't force a log-in AND we are logged in already, don't do anything
    if self.logged_in? and not forced
      Log.logger.info("We seem to be logged in already.")
    else
      if forced and self.logged_in?
        Log.logger.info("Logging out (forced).")
        Capybara.visit('/user/logout') if self.logged_in?
        Capybara.visit('/logout') if self.logged_in?
      end

      if linked_to_gardener?(where_we_came_from)
        Log.logger.info("Detected a gardens site connected to a gardener, using the '/gardener/login' path")
        Capybara.visit '/gardener/login'
      else
        Log.logger.info("Detected a good old standalone drupal site, using the '/user' path")
        Capybara.visit "/user"
      end

      Log.logger.info("Logging in via site: #{Capybara.current_url}")

      Capybara.within(:xpath, "//form[@id='user-login']") do
        Log.logger.info("Entering credentials for #{username.inspect}")
        Capybara.fill_in 'edit-name', :with => username
        Capybara.fill_in 'edit-pass', :with => password
        Log.logger.info("Clicking user-login button.")
        Capybara.click_button("edit-submit")
      end

      raise "Login as #{username.inspect} was not successful." unless self.logged_in?

      Log.logger.info("Login as #{username.inspect} was successful.")
    end
    Capybara.visit(where_we_came_from) unless do_not_go_back
  end

  def self.linked_to_gardener?(where_we_came_from)
    gardens_servers = /\.gsteamer\.|\.gcurrent\.|\.drupalgardens\.com/
    the_gardener_itself = "gardener."

    where_we_came_from.match(gardens_servers) and not where_we_came_from.include?(the_gardener_itself)
  end

  def login(options = {})
    GardensManagerCapy.login(options)
  end

  def self.logged_in?
    #sometimes current_path is nil, sometimes a empty string, depends on the driver
    return false if Capybara.page.current_path.to_s.empty?
    #as opposed to 'not-logged-in'
    Capybara.page.has_xpath?("//body[contains(@class, ' logged-in')]")
  end

  def logged_in?
    GardensManagerCapy.logged_in?
  end

  def self.logout
    raise "We're not logged in --> we don't have to log out" unless logged_in?
    Log.logger.info("Logging out")
    Capybara.visit('/user/logout')
    Capybara.visit('/logout') if logged_in?
  end

  def self.make_gardens_site_as_user(options = {})
    username = options[:username]
    password = options[:password]
    site_name = options[:sitename]
    template = options[:template]
    unless options[:do_not_log_in]
      Log.logger.info("Logging in as #{username.inspect}.")
      GardensManagerCapy.login(:username => username, :password => password, :forced => true)
    end
    Capybara.visit('/create-site')

    #enter site name
    Capybara.within(:xpath, "//form[@id='gardens-signup-create-form']") do
      Log.logger.info("Entering sitename #{site_name.inspect}.")
      Capybara.fill_in 'edit-site-prefix', :with => site_name

      #sadly, this doens't seem to work with akephalos: Capybara.using_wait_time(10) {raise "Not a valid site name" unless Capybara.page.has_css?("p#site-prefix-status.valid")}
      begin
        Capybara.wait_until {Capybara.page.has_css?("p#site-prefix-status.valid")}
        Log.logger.info("Got the little green 'valid sitename' pill for #{site_name.inspect}.")
      rescue Capybara::TimeoutError
        raise InvalidSiteNameError
      end

      Log.logger.info("Clicking submit button for sitename #{site_name.inspect}.")
      Capybara.click_button("edit-submit")
    end
    #select site template
    Log.logger.info("Selecting template #{template.inspect} for #{site_name.inspect}.")
    begin
      Capybara.find(:xpath, "//form[@id='gardens-signup-create-form']")
      Log.logger.info("Found the signup/create form..")
    rescue Exception => e
      raise "Couldn't find the signup form: #{e.inspect} #{e.backtrace}"
    end
    Capybara.within(:xpath, "//form[@id='gardens-signup-create-form']") do
      Capybara.find(:xpath, ".//div[@class='site-template']").click
      Capybara.find(:xpath, ".//div[@class='site-template' and contains(h2,'#{template}')]").click
      Capybara.click_button("edit-submit")
    end
  end

  def self.create_qatest_user(options = {})
    GardensManagerCapy.create_user(
      :log_in_as_admin  => options[:log_in_as_admin],
      :username         => $config['user_accounts']['qatestuser']['user'],
      :password         => $config['user_accounts']['qatestuser']['password'],
      :email            => "qa.001@acquia.com",
      :roles            => ['site builder', 'acquia engineering'],
      :smb_roles        => ['drupalgardens.com site owner', 'platform admin', 'acquia engineering']
    )
  end

  def self.create_user(options = {})
    Log.logger.info("Creating new user: #{options.inspect}")
    login_admin_if_required(options)

    username = options[:username]
    password = options[:password]
    email    = options[:email]
    roles    = options[:roles] || []
    smb_roles = options[:smb_roles] || []

    #Sometimes we don't care, e.g. when we try to create a user that is already there and we're fine with the old one
    fail_on_error = options[:fail_on_error] || false

    #Marc: more of an ugly hack to fix gardens system tests.
    if (linked_to_gardener?(Capybara.current_url) or Capybara.current_url.include?('://gardener.'))
      Capybara.visit("#{get_gardener_url}/admin/user/user/create")
    else
      Capybara.visit("/admin/people/create")
    end

    Log.logger.info("We're on #{Capybara.current_url}.")

    Capybara.within(:xpath, "//form[contains(@id, 'user-register')]") do
      Log.logger.info("Entering credentials for #{username.inspect}")
      Capybara.fill_in('edit-name', :with => username)
      Capybara.fill_in('edit-mail', :with => email)
      Capybara.fill_in('edit-pass-pass1', :with => password)
      Capybara.fill_in('edit-pass-pass2', :with => password)

      Log.logger.info("Enabling roles: #{roles.inspect}")
      if roles.reject{|role| Capybara.has_checked_field?(role)}.empty?
        Log.logger.info("Enabling roles: #{roles.inspect}")
        roles.each do |role|
          Capybara.check(role)
        end
      else
        Log.logger.info("Seems we are on SMB gardens. Enabling roles: #{smb_roles.inspect}")
        smb_roles.each do |role|
          Capybara.check(role)
        end
      end

      Log.logger.info("Clicking user-login button.")
      Capybara.click_button("edit-submit")
    end

    if Capybara.has_no_content?("Created a new user account for #{username}") and Capybara.has_no_content?("have been sent to your e-mail address.")
      if fail_on_error
        raise "Couldn't create user #{username}"
      else
        Log.logger.warn("User creation didn't succeed, but apparently we don't care (:fail_on_error wasn't set to true).")
      end
    end

    { :username => username, :password => password }
  end

  def self.get_gardener_url
    #This should work if we're on the gardener itself.
    if Capybara.current_url.to_s.include?("://gardener.")
      extracted_url = Capybara.current_url
    else
      extracted_url = get_gardener_url_from_redirect
    end

    parsed_uri = URI.parse(extracted_url)
    "#{parsed_uri.scheme}://#{parsed_uri.host}"
  end

  def self.get_gardener_url_from_redirect
    gardener_login_url = "#{$config['sut_url'].to_s.chomp('/')}/gardener/login"
    http_body = Net::HTTP.get(URI.parse(gardener_login_url))
    extracted_url = http_body.match(/form action="([^\"]*)"/)

    raise "Can't extract gardener URL: No redirect URL found on #{gardener_login_url}" if extracted_url.nil?

    extracted_url[1]
  end

  def self.login_admin_if_required(options = {})
    if options[:log_in_as_admin]
      GardensManagerCapy.login(
        :username => $config['user_accounts']['gardener_admin']['user'],
        :password => $config['user_accounts']['gardener_admin']['password']
      )
    end
    raise "Couldn't login successfully!" unless self.logged_in?
  end

  def self.change_subscription_type(options = {})
    Log.logger.info("Changing subscription type: #{options.inspect}")
    raise "Can't change subscription type for standalone drupal installations." unless linked_to_gardener?(Capybara.current_url)

    login_admin_if_required(options)

    Capybara.visit("#{get_gardener_url}/node/#{options[:site_nid]}/edit")

    Capybara.within(:xpath, "//form[@id='node-form']") do
      Capybara.select(options[:type], :from => "edit-field-subscription-product-nid-nid")
      Capybara.fill_in("edit-field-db-cluster-id-0-value", :with => 42)

      Capybara.within(:xpath, "//div[@id='edit-field-inactive-status-value-completed-wrapper']") do
        Capybara.choose('completed')
      end
    end

    Capybara.click_button('edit-submit')
  end

end
