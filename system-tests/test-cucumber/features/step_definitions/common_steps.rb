Then /^I (?:should )?sleep|wait (?:for )(\d+) seconds$/ do |naptime|
  sleep naptime.to_i
end

When /^I click (?:on )?["']([^"']+)['"]$/ do |link|
  click_link(link)
end

And /^I change the window size to (\d+)x(\d+)$/ do |res_x, res_y|
  case Capybara.current_driver
  when :selenium
    Capybara.current_session.driver.browser.manage.window.resize_to(res_x.to_i, res_y.to_i)
  when :webkit
    Capybara.current_session.driver.browser.window.resize_to(res_x.to_i, res_y.to_i)
  else
    raise "Can't resize window using browser: #{Capybara.current_driver.inspect}"
  end
end

Given /^I visit the page of content ["']([^"']+)["']$/ do |content|
  translated_content = translate_name_to_url(content)
  content_url = "/content/#{translated_content}"
  visit(content_url)
end

When /^I am in (.*) browser$/ do |name|
  Capybara.session_name = name
end

When /^(?!I am in)(.*(?= in)) in (.*) browser$/ do |actual_step, name|
  step "I am in #{name} browser"
  step actual_step
end

#The ?: tells us that we don't want to capture that part in a variable
When /^(?:I am|I'm|I) (?:on|viewing|looking at|look at|go to|going to|visit|visiting) ['"]?([^"']+)["']?$/ do |path|
  translation_hash = {
    'the homepage' => '/',
    'the modules page' => '/admin/modules',
    'the galleries page' => '/gallery-collections/galleries',
    'the configuration page' => '/admin/config',
    'the about page' => '/content/about-us',
    'the sample blog post' => '/content/sample-blog-post',
    'that content\'s page' => @content_url,
    'the current gallery' => @content_url,
    'the current webform' => @content_url,
    'the media content administration' => '/admin/content/media',
    'the block configuration page' => '/admin/structure/block',
    'the webform content type configuration page' => '/admin/config/content/webform',
    'the jsunit testrunner page' => '/modules/acquia/jsunit/jsunit/testRunner.html',
    'the metatag settings page' => '/admin/config/search/metatags'
  }
  cleaned_up_path = path.chomp("'").chomp('"').chomp.strip
  resulting_path = translation_hash[cleaned_up_path] || cleaned_up_path
  raise "I don't know how to go to this path: #{resulting_path.inspect}." unless resulting_path.start_with?("/") || resulting_path.start_with?("http")
  #This helps us see that we navigate to the correct content in question
  puts "Navigating to: #{resulting_path.inspect}" if resulting_path == @content_url
  visit(resulting_path)
end

And /^I click on the vertical tab named "([^"']+)"$/ do |tab_name|
  using_wait_time(30) { page.find('li.vertical-tab-button strong', :text => tab_name).click }
  wait_until { page.has_css?('li.vertical-tab-button.selected strong', :text => tab_name) }
end

And /^I wait for JQuery to be done$/ do
  wait_until(30) { page.evaluate_script('jQuery.active') == 0 }
end


When /^I press the key ['"]([^"']+)['"] on element ['"]([^"']+)['"]$/ do |key, element|
  key_translation_hash = {
    'enter' => 13
  }

  key_code = key_translation_hash[key]
  raise "I don't know the keycode for #{key}" if key_code.nil?

  exec_js = """
  var press = jQuery.Event('keypress');
  press.ctrlKey = false;
  press.which = #{key_code};
  jQuery('##{element}').trigger(press);
  """

  page.execute_script(exec_js);
end

Then /^there should be a button called ['"]([^"']+)['"]$/ do |button_name|
  page.should have_button(button_name)
end

Then /^I should be on "([^"']+)"$/ do |path|
  current_path.should == path
end

Then /^I should see the page title ['"]([^"']+)['"]$/ do |page_title|
  using_wait_time(10) { page.should have_css('h1#page-title', :text => page_title) }
end

Then /^I should see the text ['"]([^"']+)['"]$/ do |text|
  using_wait_time(10) { page.should have_content(text) }
end

Then /^I should see the element ['"]([^"']+)['"]$/ do |selector|
  using_wait_time(10) { page.should have_css(selector, :visible => true) }
end

Then /^I should (not )?see the "([^\"]+)" meta element$/ do |negate, name|
  xpath = "//head/meta[@name='#{name.to_s}']"
  page.should (negate ? have_no_xpath(xpath) : have_xpath(xpath))
end

Then /^I should see the "([^\"]+)" meta element with "([^\"]+)" content$/ do |name, content|
  xpath = "//head/meta[@name='#{name.to_s}']"
  find(:xpath, xpath)['content'].should == content.to_s
end

Then /^I should (not )?see ['"]([^"]*)["']( within ['"]([^"]*)["'])?$/ do |presence, text, has_within, css_selector|
  # Select root HTML node if no selector is set
  css_selector ||= "html"
  within(css_selector) do
    if presence
      page.should have_no_content(text)
    else
      page.should have_content(text)
    end
  end
end

Then /^I should (not )?see element ['"]([^"]+)["']( within ['"]([^"']*)["'])?$/ do |presence, element, has_within, css_selector|
  puts "Element: #{element}"

  # Select root HTML node if no selector is set
  css_selector ||= "html"
  within(css_selector) do
    if presence
      page.should have_no_css(element)
    else
      page.should have_css(element)
    end
  end
end


And /^I fill in ["']([^"'']*)["'] with ["']([^"']+)["']$/ do |field, content|
  page.fill_in field, :with => content
end

When /^I press (?:the )?(element )?["']([^"]*)["'](?: with the text ["']([^"']*)["'])?(?: in the ["']([^"]*)["'] section)?$/ do |element, selector, text, scope|
  scope ||= "html"

  opts = { :text => text } if text

  within(scope) do
    if element
      find(selector, opts).click
    else
      click_button(selector)
    end
  end
end

Then /^all images should be present$/ do

  image_urls = []
  bad_results = []

  if @browser

  else
    all(:xpath, "//img[@src]").each do |image_node|
      if image_node['src'].include?("http://")
        image_urls << image_node['src']
      else
        #remove leading / and prepend the current_url
        image_urls << "#{current_url.to_s.chomp("/")}/#{image_node['src'].reverse.chomp("/").reverse}"
      end
    end
  end

  image_urls.each do |image_url|
    http_status = Excon.head(image_url).status
    bad_results << {:http_status => http_status, :checked_url => image_url, :current_page => current_url} if http_status != 200
  end
  bad_results.should == []
end

Then /^there should be no links to ([^"']+)$/ do |bad_link_target|
  page.should_not have_css("a[href*='#{link_target}']")
end

Then /^there should be a link to ([^"']+)$/ do |link_target|
  page.should have_css("a[href*='#{link_target}']")
end

And /^I refresh the page$/ do
  visit(current_path)
end

Given /^I have a new session$/ do
  begin
    Capybara.reset!
  rescue Errno::ECONNREFUSED, StandardError => e
    puts "Error while trying to reset the session: #{e.message}"
    puts "The current driver is: #{Capybara.current_driver}"
    puts "The default driver is: #{Capybara.default_driver}"
    puts "The website reposnds like this:\n#{`curl -I #{Capybara.app_host}`}"
    raise "Error while trying to reset the current session: #{e.message}"
  end
end

And /^I wait for all iframes to disappear$/ do
  wait_until(30) { page.all(:xpath, '//iframe', :visible => true).empty? rescue Selenium::WebDriver::Error::StaleElementReferenceError}
end

And /I should see a message with the text ['"]([^"']+)['"]/ do |text|
  page.find("div.messages").text.should include(text)
end

And /I should see a status message with the text ['"]([^"']+)['"]/ do |text|
  page.find("div.messages.status").text.should include(text)
end

And /I should see an error message with the text ['"]([^"']+)['"]/ do |text|
  page.find("div.messages.error").text.should include(text)
end

And /I should see a link with the text ['"]([^"']+)['"]/ do |text|
  page.should have_xpath("//a[contains(text(), text)]")
end

And /^I should be on a page with the title ['"]([^"']+)['"]$/ do |text|
  page.should have_xpath("/html/head/title[contains(text(), '#{text}')]")
end

And /^I choose ['"]([^"']+)['"]$/ do |radio_button|
  page.choose(radio_button)
end

And /^I select ["']([^"']+)["'] from select box ["']([^"']+)["']$/ do |value, select_box|
  select(value, :from => select_box)
end

And /^I type ["']([^"']+)["'] into ["']([^"']+)["']$/ do |text, target|
  fill_in(target, :with => text)
end

And /^I reload the current page$/ do
  visit(current_url)
  step "I wait for JQuery to be done"
end

Then /^I should see the image ['"]([^"']+)['"]$/ do |image_url|
  page.should have_css("img[src='#{image_url}']")
end

Then /^I should not see an error message$/ do
  page.should_not have_css('div.error')
end

Given /^this hasn't been implemented yet$/ do
  pending
end

And /^I print the response headers$/ do
  puts page.response_headers.inspect
end

Then /^I should see that element ['"]([^"]*)['"] contains ([\w\s]+)?(\d+) elements?$/ do |element, clause, amount|
  puts "Looking for: #{element}"
  el = all(:css, element)

  if clause
    case clause
    when /at least/ then el.should have_at_least(amount).items
    when /at most/ then el.should have_at_most(amount).items
    else raise "Don't know how to deal with clause #{clause}"
    end
  else
    el.should have(amount).items
  end
end

Then /^I should see that the "([^\"]+)" field has "([^\"]+)" content$/ do |locator, content|
  find_field(locator).value.should == content
end

Then /^I should see that the "([^\"]+)" checkbox is (checked|unchecked)$/ do |locator, state|
  find_field(locator).checked?.should == (state == 'checked')
end

And /^I (check|uncheck) the "([^\"]+)" checkbox$/ do |state, locator|
  send(state.to_sym, locator)
end

And /^the "([^\"]+)" field should( not)? be disabled$/ do |locator, negate|
  find_field(locator)['disabled'].should (negate ? be_false : be_true)
end

