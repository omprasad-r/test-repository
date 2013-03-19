puts "loading env.rb"
require 'bundler/setup'
require 'uri'
require 'tmpdir'
require 'date'
require 'capybara'
require 'capybara/dsl'
require "capybara/cucumber"
require 'capybara/poltergeist'
require "selenium-webdriver"
require "base64"
require "excon"
require 'capybara-screenshot'
require "log4r"
require 'rbconfig'

if RbConfig::CONFIG['target_os'].include?("darwin")
  os = 'osx'
elsif RbConfig::CONFIG['target_os'].include?("linux")
  os = 'linux'
else
  os = 'unknown_os'
end

phantom_path =  "features/support/dependencies/phantomjs/#{ os }/bin/phantomjs"

if File.exists?(phantom_path)
  bundled_phantomjs = phantom_path
else
  detected_phantomjs_version = `phantomjs --version`.to_f
  unless detected_phantomjs_version >= 1.8
    raise "Couldn't find a phantomjs version >= 1.8. (--> http://code.google.com/p/phantomjs/downloads/list)"
  end
end

puts "Using phantomJS: #{ bundled_phantomjs || "System installation #{ detected_phantomjs_version }" }"

Capybara.register_driver :poltergeist do |app|
  options = {
    :js_errors => false,
    :window_size => [1280, 768]
  }
  options[:phantomjs] = bundled_phantomjs if defined?(bundled_phantomjs)
  Capybara::Poltergeist::Driver.new(app, options)
end

Capybara.default_wait_time = 30
Capybara.default_driver = :selenium

def check_availibilty(url, expected_status)
  begin
    return (Excon.head(url).status == expected_status)
  rescue Exception => e
    puts "Couldn't determine feature: #{e.message}"
    return false
  end
end

#This is how we point the test at a certain domain
unless ENV['SUT_URL']
  puts "No SUT_URL environment variable found. We need this so we know which URL points to our system under test."
  exit(1)
end

# Log only to stdout
$logger = Logger.new STDOUT

$config = {}
$config['sut_url'] = ENV['SUT_URL'].chomp('/')
$config['sut_host'] = URI.parse($config['sut_url']).host
$config['user_accounts'] = {'qatestuser' => {'user' => 'qatestuser'}}

#Capybara.default_driver = :webkit
Capybara.app_host = $config['sut_url']

Before do
  # Set default window size
  case Capybara.current_driver
  when :selenium
    Capybara.current_session.driver.browser.manage.window.resize_to(1280, 768)
  when :webkit
    Capybara.current_session.driver.browser.window.resize_to(1280, 768)
  end
end

$site_capabilities = {}
$site_capabilities[:fast_user_switching] = check_availibilty("#{$config['sut_url']}/devel/switch", 302)
$site_capabilities[:backdoor] = check_availibilty("#{$config['sut_url']}/qa_reset.php", 200)
puts "Site capabilites: #{$site_capabilities.inspect}"
World(Capybara)

# Marc: I have NO idea why this is started to be necessary, but if we don't have this line
# Cucumber will finish and then test-unit will pop up and immediately die complaning that
# It can't find most of the cuke features
# Similar: https://github.com/cucumber/cucumber-rails/issues/88
# Note: doesn't seem to happen on Ruby 1.9 according to Sven
Test::Unit.run = true unless RUBY_VERSION.include?("1.9")
