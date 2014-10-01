require 'capybara'
require 'capybara/dsl'

module JQuery
  def self.wait_for_events_to_finish
    Capybara.wait_until { Capybara.page.evaluate_script('jQuery.active') == 0 }
  end
end