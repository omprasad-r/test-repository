
SITE_ELEMENT_TRANSLATION_HASH = {
  "site name link" => "#site-name a",
  "rotating banner" => "#banner",
  "header region" => "div#header-region",
  "site slogan" => "p#site-slogan",
  "about link" => "div.content ul.menu li.leaf > a[href = '/content/about-us']",
  "blog link" => "div.content ul.menu li.leaf > a[href = '/blog']"
}

SITE_ELEMENT_XPATH_TRANSLATION_HASH = {
  "site name link" => "//h1[contains(@id,'site-name')]",
  "about link" => "//div[contains(@class,'content')]//ul[contains(@class,'menu')]//li[contains(@class,'leaf')]/a[contains(@href,'/about-us')]",
  "blog link" => "//div[contains(@class,'content')]//ul[contains(@class,'menu')]//li[contains(@class,'leaf')]/a[contains(@href,'/blog')]"
}

THEME_BUILDER_TABS =  ['styles', 'brand', 'layout', 'advanced', 'theme']

Given /^I open the theme builder$/ do
  puts "Opening theme builder"
  find(:css, "div#toolbar a#toolbar-link-admin-appearance").click

  if page.has_css?("div.ui-dialog-buttonset")
    puts "Found theme builder active draft dialog. Pressing OK."
    find(:css, "div.ui-dialog-buttonset > button.ui-button", :text => "OK").click
  end

  has_css?("div#themebuilder-main", :visible => true)
end

Given /^I close the theme builder$/ do
  has_css?("div#themebuilder-main", :visible => true)
  puts "Closing theme builder"
  click_button("themebuilder-exit-button")
  has_no_css?("div#themebuilder-main")
end

Given /^I switch to(?: the)? theme builder tab ["']([^"']+)["']$/ do |tab_name|
  puts "Selecting tab '#{tab_name}'"
  tab_css = "div#themebuilder-main > ul.ui-tabs-nav > li > a"
  find(:css, tab_css, :text => tab_name).click
  success_css = "li.ui-tabs-selected > a"
  has_css?(success_css, { :visible => true, :text => tab_name })
  step "I wait for JQuery to be done"
end

Given /^I switch to(?: the)? vertical theme builder tab ["']([^"']+)["']$/ do |vertical_tab_name|
  puts "Selecting #{vertical_tab_name} in the current tab"
  vtab_css = "div.tb-tabs-vert ul.tabnav > li > a"
  find(:css, vtab_css, :text => vertical_tab_name).click
  # This could potentially skip the dialog if it takes some time to pop up. We
  # haven't had any problems with this yet.
  step "I accept the modal dialog"
  success_css = "div.tb-tabs-vert ul.tabnav > li.ui-tabs-selected > a"
  has_css?(success_css, :visible => true, :text => vertical_tab_name)
  step "I wait for JQuery to be done"
end

And /^I accept the modal dialog$/ do
  # Use a rescue statement to avoid throwing an exception if the dialog doesn't
  # exist.
  begin
    # Only directly use the selenium API for accepting the alert dialog when
    # not using the poltergeist driver.
    page.driver.browser.switch_to.alert.accept unless Capybara.current_driver == :poltergeist
  rescue
    nil
  end
end

Given /^I select layout ([\w]+) (for all pages|for this page)?$/ do |layout, apply_page|
  puts "Selecting layout #{layout} #{apply_page}"
  step "I switch to the theme builder tab 'Layout'"
  apply_to = if apply_page.include?("all pages"); "all" else "single"; end

  find(:css, "div#layouts_list div[onclick *= \"pickLayout('fixed-#{layout}')\"].layout-fixed-#{layout}").click
  find(:css, "div.options div[onclick *= \"pickLayout('fixed-#{layout}','#{apply_to}')\"].applyoption").click

  has_css?("body[class *= '#{layout}']")
  step "I wait for JQuery to be done"
end

Given /^I select element ["']([^"']+)["']$/ do |site_element|
  puts "Selecting element #{site_element}"

  selector = SITE_ELEMENT_TRANSLATION_HASH[site_element] || site_element
  element_to_select = find(:css, selector, :visible => true)
  already_selected = element_to_select['class'].include?('selected')
  if already_selected
    puts "Element is already selected"
  else
    success_css = "div[style *= 'top'].tb-no-select.tb-nav"
    wait_until(15) do
      #We sometimes have to click more than once if JS is not ready yet.
      #This could also be solved by a "sleep 2"
      element_to_select.click
      css_is_there = has_css?(success_css)
      selected_class = element_to_select['class'].include?('selected')
      css_is_there && selected_class
    end
  end

  # Save the element for later use
  @currently_selected_element = selector
end

# Works only with selenium due to usage of native methods
Given /^I save the current theme$/ do
  puts "Saving current theme"
  within(:css, "div#themebuilder-main") { click_link("Save") }
  step "I accept the modal dialog"
  has_css?("div#themebuilder-theme-name > span.theme-visibility", :text => "(Live - everyone can see this")
end

Given /^I save the current theme as ["']([^"']+)["']$/ do |theme_name|
  puts "Saving theme as #{theme_name}"
  within(:css, "div#themebuilder-save") do
    click_link("Save as")
  end

 has_css?("div#export-dialog", :visible => true)

  within(:css, "div.themebuilder-dialog") do
    fill_in("name", :with => theme_name)
    find(:css, "div.ui-dialog-buttonpane span.ui-button-text", :text => "OK").click
  end

  tb_status_message_css = "div#themebuilder-status > span.themebuilder-status-message"
  has_css?(tb_status_message_css, { :visible => true, :text => "was successfully copied and saved." })

  wait_until(15) do
    status_message_gone = has_css?(tb_status_message_css, :visible => false)
    theme_name_updated = has_css?("div#themebuilder-theme-name > span.theme-name", :text => /#{theme_name[0...12]}/)
    status_message_gone && theme_name_updated
  end
end

Given /^I publish the current theme$/ do
  puts "Publishing current theme"
  within(:css, "div#themebuilder-main") { click_link("Publish") }
  has_css?("div#themebuilder-theme-name > span.theme-visibility", :text => "(Live - everyone can see this")
end

Given /^I preview the current theme$/ do
  "noop"
end

Given /^I save_as the current theme$/ do
  step "I save the current theme as 'test_theme_save_as'"
  step "I load the theme 'test_theme_save_as'"
end

Given /^I save timestamped theme with prefix ["']([^"']+)["']$/ do |prefix|
  step "I save the current theme as '#{ prefix }_#{ Time.now.to_i }'"
end

Given /^I load the (Gardens )?theme ["']([^"']+)["']$/ do |gardens, theme_name|
  puts "Loading theme #{theme_name}"
  step "I reload the current page"
  has_css?("div#themebuilder-main", :visible => true)
  step "I switch to the theme builder tab 'Themes'"

  if gardens
    find(:css, "ul#themebuilder-actionlist a[class $= 'action --Choose-a-new-theme']").click
    has_css?("ul#themebuilder-actionlist a[class *= 'action --Choose-a-new-theme']", :visible => true)
  end

  # Using css selectors currently not pssible due to nokogiri bug
  # https://github.com/tenderlove/nokogiri/issues/451
  active_themes_xpath = "//div[contains(@id,'themebuilder-themes-') and (not(contains(@class, 'smart-hidden'))) and (not(contains(@id,'actions')))]"
  raise "We dont seem to be in the themes tab. Did somebody redesign this section?" unless all(:xpath, active_themes_xpath).size > 0
  current_section = find(:xpath, active_themes_xpath)["id"].gsub("themebuilder-themes-","").downcase
  step "I select '#{theme_name}' from themes list"

  detect_js_call = if gardens
                     "window.ThemeBuilder.getApplicationInstance().applicationData['base_theme'] === '#{theme_name}'"
                   else
                     "window.ThemeBuilder.Theme.getSelectedTheme().getName() === '#{theme_name}'"
                   end
  detect_theme_js = <<-END
    (function() {
      try { return #{detect_js_call}; } catch(e) { return false; }
    }());
  END

  wait_until(15) { evaluate_script(detect_theme_js) }

  if gardens
    find(:css, "div#themebuilder-themes-featured-modal a", :text => "Choose").click
    temp_name = "temp_theme_#{theme_name}"
    within(:css, "div.themebuilder-dialog") do
      fill_in("name", :with => temp_name)
      click_button("OK")
    end

    has_css?("div#themebuilder-theme-name > span.theme-name", :text => /#{ temp_name[0...12] }/)
  end
end

Given /^I select ["']([^"']+)["'] from themes list$/ do |theme_name|
  theme_name = theme_name.downcase
  puts "Selecting theme #{theme_name}"
  theme_label_css = "li[id $= '#{theme_name.gsub(/[\s|-]/, "_")}'].jcarousel-item div.label"
  theme_css_options = { :text => /#{theme_name}/i, :visible => true }
  find(:css, theme_label_css, theme_css_options).click
  step "I wait for JQuery to be done"
end

Given /^I ([\w]+) and reload the current theme$/ do |action|
  step "I #{action} the current theme"
  step "I reload the current page"
end

# This step is using native webdriver calls and works therefore only with selenium
# TODO: Look into porting this step to element.trigger and/or native JS calls
Given /^I move the ["']([^"']+)["'] slider to (\d+) percent$/ do |slider_name, percent|
  puts "Moving #{slider_name} slider to #{percent}"
  slider = find(:css, "div#themebuilder-main input[id^='#{slider_name}']").native
  browser = page.driver.browser
  browser.action.click(slider).click_and_hold(slider).perform

  slider_element = find("div.slider-container[style *= 'display: block']")
  slider_target_position = slider_element.native.size.width.to_f * percent.to_f / 100

  puts "Moving slider to position: #{slider_target_position}"
  browser.action.click(slider).click_and_hold(slider).move_to(
    slider, "#{slider_target_position}", "0"
  ).release.perform
end

Given /^I set the (\w+) (\w+) spacing to (\d+)$/ do |spacing, edge, value|
  suffix = spacing == "border" ? "-width" : ""
  step "I type '#{value}' into 'tb-style-#{spacing}-#{edge}#{suffix}'"
end

Given /^I set the (\w+)(?: (\w+))? spacing to (\d+) percent using the slider$/ do |spacing, edge, percent|
  suffix = spacing == "border" ? "-width" : ""
  if edge
    step "I move the 'tb-style-#{spacing}-#{edge}#{suffix}' slider to #{percent} percent"
  else
    step "I move the 'tb-style-#{spacing}' slider to #{percent} percent"
  end
end

Given /^I pick item ["']([^"']+)["'] from color picker ["']([^"']+)["']$/ do |desired_item, picker_name|
  puts "Picking color #{desired_item} from picker #{picker_name}"
  find(:css, "div#themebuilder-main div##{picker_name}.colorSelector").click

  within(:css, "div[style *= 'display: block'].PalettePickerMain") do
    color_css = "div.current-palette.palette-list > div.palette-item.item-#{desired_item}"
    color_element = find(:css, color_css)
    color_element.click
    click_button("OK")
  end
end

Given /^I change(?: the)? font ([\w]+) to ['"](.*)["']$/ do |type, value|
  puts "Setting font #{type} to #{value}"
  element_id = "style-font-#{type}"

  case type
  when "family"
    step "I select '#{value}' from select box '#{element_id}'"
  when "size"
    step "I type '#{value}' into '#{element_id}'"
  when "color"
    step "I pick item '#{value}' from color picker '#{element_id}'"
  when "weight", "style", "decoration"
    is_set_to_normal = value == 'normal'
    is_active = find(:css, "button##{element_id}")['class'].include?("ui-state-active")

    needs_change = is_set_to_normal == is_active
    step "I press element 'button##{element_id}' in the '#themebuilder-font-editor' section" if needs_change
  else raise "Don't know how to deal with type: #{type}"
  end

  step "I wait for JQuery to be done"
end

Given /^I publish multiple themes with the names:$/ do |table|
  table.hashes.each do |theme|
    step "I select layout abc for all pages"
    step "I select layout acb for all pages"
    step "I save the current theme as '#{theme['theme_name']}'"
    step "I publish the current theme"
    step "I should see layout acb on 'the homepage'"
  end
end

Given /^I run the theme builder selector test$/ do
  step("I press 'themebuilder-selector-test'")

  test_status = find(:css, "span#themebuilder-selector-test-status")
  last_tests_remaining = 0
  tests_stuck = 0

  while all(:css, "div#themebuilder-selector-test-results").size < 1
    tests_remaining = test_status.text.scan(/\d+/)[0].to_i
    if tests_remaining == last_tests_remaining
      tests_stuck += 1
      raise "Selector tests didn't count down for over a minute (tests remaining: #{tests_remaining})." if tests_stuck > 6
    else
      tests_stuck = 0
    end
    last_tests_remaining = tests_remaining
    sleep 10
  end

  has_css?("div#themebuilder-selector-test-results", :visible => true)
end

Given /^I change element ["']([^"']+)["'] to color ["']([^"']+)["'] and font size ["']([\d]+)["']$/ do |element, color, percent|
  step "I switch to the theme builder tab 'Styles'"
  step "I switch to the vertical theme builder tab 'Font'"
  step "I select element '#{element}'"
  step "I pick item '#{color}' from color picker 'style-font-color'"
  step "I move the 'style-font-size' slider to #{percent} percent"
end

Given /^I ([\w]+) the CSS rule for element ["']([^"']+)["']$/ do |action, element|
  id_element = element.gsub(/[^\w\d]/, "-")
  action_selector = "div#css-history tr#selector-#{id_element} div.history-operation.history-#{action}"
  action_element = find(:css, action_selector)
  action_element.click

  if action == "delete"
    has_no_css?(action_selector)
  else
    wait_until(15) { !action_element.visible? }
  end
end

Given /^the role ["']([^"']+)["'] has the permission to see all theme builder tabs$/ do |role|
  THEME_BUILDER_TABS.each do |theme|
    step "the role '#{role}' has the permission 'access themebuilder #{theme} tab'"
  end
end

Given /^I ([\w]+) all CSS rules$/ do |action|
  selector =  "#history-#{action}-all"
  selector = selector + "-hidden" if action == "delete"
  find(:css, selector).click
end

Given /^I enable theme builder power theming$/ do
  find(:css, "div#themebuilder-main div.power-theming-label > span.power-theming-value").click
end

Given /^I enable theme builder CSS display$/ do
  find(:css, "div#themebuilder-main div.natural-language-label > span.natural-language-value").click
end

Given /^I press the theme builder navigation arrow to (.*)$/ do |direction|
  direction = direction.capitalize
  find(:css, "div[title = '#{direction}']").click
end

Given /^I remember the ([\w\s]+) of ['"](.*)["'] as ['"](.*)["']$/ do |property_name, site_element, var_name|
  selector = SITE_ELEMENT_TRANSLATION_HASH[site_element] || site_element
  css_property = property_name.gsub(/\s/, "-")
  property = calculated_css_property(selector, css_property)

  raise "Can't store #{property_name} of #{element}: No property returned" if property.nil?
  instance_variable_set("@#{var_name}".to_sym, property)
end

Given /^I set the background image to ["']([^"']+)["']$/ do |image_name|
  puts "Uploading image #{image_name}"

  if image_name.start_with?('http://')
    image_link = image_name
  else
    image_prefix = "http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/"
    image_link = image_prefix + image_name
  end
  local_image_name = URI.parse(image_link).path.split("/").last
  local_image_path = "#{Dir.tmpdir}/#{local_image_name}"
  File.open(local_image_path, 'w') {|f| f.write(Excon.get(image_link).body) }
  attach_file("files[styleedit]", local_image_path)

  wait_until(30) do
    has_image = has_css?("div.background-image-selection-panel div.background-image > img", :visible  => true)
    has_delete_link = has_css?("div.background-image-control > a#background-remove:not(.ui-state-disabled)")
    has_image && has_delete_link
  end
end

Given /^I remove the background image$/ do
  within(:css, "div.background-image-control") { click_link("Remove") }
  # Webdriver looses focus after clicking a link that is outside the viewport
  find(:css, @currently_selected_element).click if Capybara.current_driver == :selenium
  wait_until(15) { find(:css, "div#themebuilder-style-background div.background-image > img")[:src].empty? }
end

Then /^I should see layout ([\w]+) on ['"](.*)["']$/ do |layout, page|
  puts "Detecting layout #{layout} on #{page}"
  step "I go to '#{page}'"
  has_css?("body")
  body_class = find(:css, "body")[:class]
  match = body_class.to_s.match(/body-layout-fixed-([\w]{1,3})/)
  match[1].should == layout
end

Then /^I should only see the currently active tab$/ do
  active = all(:css, "div#themebuilder-main > ul > li.ui-tabs-selected")
  active.should have(1).items
end

Then /^I should see that ["']([^"']+)["'] is the currently active vertical tab$/ do |tab_name|
  page.should have_css("div#themebuilder-advanced > div.tb-tabs-vert > ul > li.ui-tabs-selected.ui-state-active a", :text => tab_name)
end

Then /^I should see that(?: the)? ["']([^"']+)["'] equals ["']([^"']+)["'] ([\w\s\-]+)$/ do |site_element, tb_control_name, type|
  selector = SITE_ELEMENT_TRANSLATION_HASH[site_element] || site_element
  #border has a -width suffix, that's why we just look for the prefix string
  if tb_control_name.match(/border-(top|bottom|left|right)$/)
    css_for_element = "div#themebuilder-main *[id^='#{tb_control_name}']"
    type = "#{type}-width"
  else
    #We can't always use the prefix, the font sliders have 3 hits for the prefix
    css_for_element = "div#themebuilder-main ##{tb_control_name}"
  end

  expected_element = find(:css, css_for_element, :visible => true)
  raise "Can't find element with css #{css_for_element.inspect}" unless expected_element

  expected = case type
             when "font size" then expected_element.value
             when "font family" then expected_element.value.delete("\"'").split(",")
             when /^(margin|padding|border) (top|bottom|left|right)(-width)?$/ then expected_element.value
             when "font weight" then expected_element[:class].match(/ui-state-active/) ? "bold" : "normal"
             when "font style" then expected_element[:class].match(/ui-state-active/) ? "italic" : "normal"
             when "text decoration" then expected_element[:class].match(/ui-state-active/) ? "underline" : "none"
             when "text transform" then expected_element[:class].match(/ui-state-active/) ? "uppercase" : "none"
             when "text align" then expected_element[:id].split("-").last
             when /^((border (\w+ )?)|background )?color/ then expected_element[:style].match(/background-color: (.*);/)[1]
             else raise "Don't know how to deal with type: #{type}"
             end
  raise "Couldn't get value for expected_element: #{css_for_element.inspect}" if expected.to_s.empty?
  expected_css_property = "#{type.gsub(' ', '-')}"
  actual = calculated_css_property(selector, expected_css_property).gsub("px", '')

  # Workaround since ff returns font weight as number
  if type == "font weight"
    actual = "normal" if actual == "400"
    actual = "bold" if actual == "700"
    # Get around inconsistent font family declarations
  elsif type == "font family"
    actual = actual.delete("\"'").split(",")
  end
  actual.should == expected
end

Then /^I should see that remembered value ['"](.*)["']( not)? equals ['"](.*)["']$/ do |name1, not_equals, name2|
  actual = instance_variable_get("@#{name1}".to_sym)
  expected = instance_variable_get("@#{name2}".to_sym)

  if not_equals
    actual.should_not == expected
  else
    actual.should == expected
  end
end

Then /^I should see that ["']([^"']+)["'] has the correct background image set$/ do |site_element|
  selector = SITE_ELEMENT_TRANSLATION_HASH[site_element] || site_element

  background_url = find(:css, "div.background-image > img")[:src]
  expected_bg = if background_url.empty? then "none" else "url(\"#{background_url}\")" end
  actual_bg = calculated_css_property(selector, "background-image")
  actual_bg.should == expected_bg

  repeat_element = find(:css, "div.background-repeat-panel div[id ^= 'background-repeat-'].enabled")
  expected_repeat = repeat_element[:id].gsub(/^background-repeat-/, "")
  actual_repeat = calculated_css_property(selector, "background-repeat")
  actual_repeat.should == expected_repeat

  attachment_element = find(:css, "div.background-attachment-panel div[id ^= 'background-attachment-'].enabled")
  expected_attachment = attachment_element[:id].gsub(/^background-attachment-/, "")
  actual_attachment = calculated_css_property(selector, "background-attachment")
  actual_attachment.should == expected_attachment
end

Then /^I should see that the theme folder contains theme ["']([^"']+)["']$/ do |theme_name|
  puts "Verifying theme folder structure"

  theme_dir_name = "acq_#{theme_name}"
  theme_dir_root = "./sites/#{$config['sut_host']}/themes/mythemes/"

  backdoor = QaBackdoor.new($config['sut_url'])
  dir_listing = backdoor.list_directory_content(theme_dir_root)
  dir_listing.should include(theme_dir_name)

  theme_info_file_contents = backdoor.list_file_content("#{theme_dir_root}#{theme_dir_name}/#{theme_dir_name}.info")
  theme_info_file_contents.to_s.should include("name = '#{theme_name}'")

  filelist = backdoor.list_directory_content("#{theme_dir_root}#{theme_dir_name}")
  filelist.should include("local.js")
  filelist.map{ |filename| filename.include?(".css") }.size.should > 5
  filelist.map{ |filename| filename.include?(".tpl") }.size.should > 10
  filelist.should include("images")
end

Then /^I should see that the theme builder selector test finished successfully$/ do
  puts "Verifying tb selector test"
  result_text = find(:css, "#themebuilder-selector-test-results").text
  results = result_text.split("Selector:").map { |res| res.strip }

  filtered_results = results.select do |res|
    contains_something = res.size < 0
    font_weight_fail = res.match(/expected: 700; actual: bold/)
    failure_encountered = res.match(/Failures encountered/)

    contains_something && !(font_weight_fail || failure_encountered)
  end

  filtered_results.should be_empty

end

Then /^I should see that the CSS history rule for ["']([^"']+)["'] is( not)? present$/ do |element, not_present|
  puts "Verifying CSS rule for #{element}"
  step "I wait for JQuery to be done"
  if not_present
    page.should_not have_css("th.history-selector.history-element-text", :text => element)
  else
    page.should have_css("th.history-selector.history-element-text", :text => element)
  end
end

Then /^I should see that all CSS history rules are( not)? hidden$/ do |not_hidden|
  all(:css, "div#css-history tr.history-table-row.history-selector-row").each do |row|
    if not_hidden
      row.should have_css("div.history-operation.history-hide", :visible => true)
    else
      row.should have_css("div.history-operation.history-show", :visible => true)
    end
  end
end

Then /^I should (not )?see the theme builder tab ["']([^"']+)["']$/ do |not_present, tab_name|
  puts "Verifying tab #{tab_name}"
  selector = "ul.tabnav > li.ui-state-default > a"

  if not_present
    page.should_not have_css(selector, :text => /^#{tab_name.capitalize}/, :visible => true)
  else
    page.should have_css(selector, :text => /^#{tab_name.capitalize}/, :visible => true)
  end
end

Then /^I should see all theme builder tabs$/ do
  THEME_BUILDER_TABS.each do |tab|
    step "I should see the theme builder tab '#{tab}'"
  end
end

Then /^I should see all theme builder navigation arrows$/ do
  ["left", "right", "top", "bottom"].each do |direction|
    page.should have_css("div.tb-nav.#{direction}.tb-nav-enabled", :visible => true)
  end
end

Then /^I should see that the ([\w\s]+) of ["']([^"']+)["'] is selected$/ do |selection, site_element|
  selector = SITE_ELEMENT_XPATH_TRANSLATION_HASH[site_element] || site_element
  selection_xpath =  case selection
                     when "parent" then "/../.."
                     when "first child" then "/*"
                     when "next sibling" then "/../following-sibling::*"
                     when "previous sibling" then "/../preceding-sibling::*"
                     else raise "Don't know how to handle selection #{selection}"
                     end

  verification_xpath = selector + selection_xpath + "[contains(@class, 'selected') and not(contains(@class, 'tb-no-select'))]"

  page.should have_xpath(verification_xpath)
end

Then /^I should see the DOM navigation refiner display$/ do
  page.should have_css(
    "div#path-selector div.path-element-inner > div.path-element-label",
    :text => /site background/i)
end

Then /^I should see that the displayed CSS selector is selected$/ do
  current_selector = find("div.path-selector-label > span.path-selector-value").text
  page.should have_css(current_selector + ".selected")
end

Then /^I should be able to navigate to the top of the page$/ do
  while page.has_no_css?("#page.selected")
    step "I press the theme builder navigation arrow to select the parent element"
    step "I should see that the displayed CSS selector is selected"
  end
end

Then /^I should be able to navigate to the top of the page and set the following attributes for each element:$/ do |attribute_table|
  while page.has_no_css?("#page.selected")
    step "I press the theme builder navigation arrow to select the parent element"
    current_selector = find("div.path-selector-label > span.path-selector-value").text

    # set attributes and save values
    attribute_table.hashes.each do |hsh|
      attribute = hsh["attribute"]
      stored_attribute = attribute.gsub(/\s/, "_")
      value = hsh["value"]

      tb_control_name = "style-#{attribute.gsub(/\s/, "-")}"
      step "I change #{attribute} to '#{value}'"
      step "I should see that '#{current_selector}' equals '#{tb_control_name}' #{attribute}"
      step "I remember the #{attribute} of '#{current_selector}' as 'expected_navigation_#{stored_attribute}'"
    end

    step "I publish the current theme"
    step "I close the theme builder"
    step "I open the theme builder"
    step "I switch to the theme builder tab 'Styles'"
    step "I switch to the vertical theme builder tab 'Font'"
    step "I select element '#{current_selector}'"

    # verify attributes set before
    attribute_table.hashes.each do |hsh|
      attribute = hsh["attribute"]
      stored_attribute = attribute.gsub(/\s/, "_")
      step "I remember the #{attribute} of '#{current_selector}' as 'actual_navigation_#{stored_attribute}'"
      step "I should see that remembered value 'actual_navigation_#{stored_attribute}' equals 'expected_navigation_#{stored_attribute}'"
    end

  end
end

Then /^I should see that the ["']([^"']+)["'] background image was uploaded successfully$/ do |image_name|
  img_selector = "div#themebuilder-main div.background-image-selection-panel div.background-image > img"
  page.should have_css(img_selector, :visible => true)
  img_url = find(img_selector)[:src]
  img_url.should match(/#{image_name.split(".")[0]}/)
  Excon.head(img_url).status.should == 200
end

Then /^I should see the active draft message$/ do
  page.should have_css("div#themebuilder-confirmation-dialog.message.ui-dialog-content",
                       :text => "An active draft exists from a previous session")
end

And /^the update button should be (enabled|disabled)$/ do |state|
  within(:css, 'div#themebuilder-main div.update-button-wrapper') do
    css = 'a.disabled'
    page.should (state == 'enabled' ? have_no_css(css) : have_css(css))
  end
end

And /^the control veil should be (enabled|disabled)$/ do |state|
  within(:css, 'div#themebuilder-wrapper') do
    css = 'div#themebuilder-control-veil.on'
    page.should (state == 'enabled' ? have_css(css) : have_no_css(css))
  end
end

Then /^I should see themebuilder-loader appear and disappear$/ do
  tb_loader = 'div.themebuilder-loader'
  page.should have_css(tb_loader)
  page.should have_no_css(tb_loader)
end
