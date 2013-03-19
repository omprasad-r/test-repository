And /^I add (.*) to the webform$/ do |field_name|
  unless page.has_css?('li.horizontal-tab-button.selected a span', :text => 'Add field')
    find('li.horizontal-tab-button a span', :text => 'Add field').click
  end
  translations = {
    "a Text field" => 'Text field',
    "a Drop-down List" => 'Drop-down list',
    'a File upload field' => 'File upload',
    'an email field' => 'E-mail',
    'Radio buttons' => 'Radio buttons'
  }
  raise "Can't handle input #{field_name}. Typo?" unless translations.key?(field_name)
  translated_name = translations[field_name]
  find('ul.form-builder-fields li a', :text => translated_name).click
  wait_until(30){ page.has_no_content?("Please wait") }
  @last_added_webform_component = translated_name
end


And /^I change the component's (label) to "(.*)"$/ do |item, text|
  step 'I open the field settings tab "Properties"'
  #I have NO idea what actually happens here, but without waiting for the JS we end up having timouts
  step 'I wait for JQuery to be done'
  case item
  when 'label'
    within('div#form-builder-field-configure') do
      fill_in 'title', :with => text
    end
    #There is sometimes an extra space in the label
    wait_until(30){ page.has_xpath?("//label[contains(text(), '#{text}')]") }
  end
end

Given /^the site has webforms? enabled$/ do
  if defined?(WEBFORM_ENABLED)
    puts "Webforms already enabled, restoring snapshot"
    step 'I restore the snapshot "webform"'
  else
    step 'the module webform_alt_ui is enabled'
    step 'I save the state as snapshot "webform"'
    WEBFORM_ENABLED = true
    puts "Done with the webform setup work"
  end
end

Then /^I (should|shouldn't) see a webform with (.*)$/ do |see_or_not, field_type|

  translations = {
    "a Text field" => 'div.webform-component-textfield',
    "a Drop-down List" => 'div.webform-component-select',
    'Radio buttons' => 'div.webform-component-radios'
  }
  raise "Can't find a translation for field type: #{field_type.inspect}" unless translations.key?(field_type)
  css_equivalent = "div#content-area #{translations[field_type]}"
  case see_or_not
  when 'should'
    page.should have_css(css_equivalent)
  when "shouldn't"
    page.should_not have_css(css_equivalent)
  end
end

And /^I open the field settings tab "(.*)"$/ do |tab_name|
  step 'I highlight the recently added webform component'
  enabled_css = 'a[aria-expanded="true"].ui-accordion-header'
  disabled_css = 'a[aria-expanded="false"].ui-accordion-header'
  unless page.has_css?(enabled_css, :text => tab_name)
    find(disabled_css, :text => tab_name).click
    wait_until(10){ page.has_css?(enabled_css, :text => tab_name) }
  end
  step 'I wait for JQuery to be done'
  sleep 4
end

And /^I publish the webform$/ do
  before_url = current_url
  click_button("Publish")
  wait_until(15){ before_url != current_url }
  @content_url = current_url
end

Then /^I should see the text "(.*)" in the webform preview$/ do |text|
  within('div#form-builder') { wait_until(20){ page.has_content?(text) || page.has_xpath?("//input[contains(@value, '#{text}')]") } }
end

Then /^I should (not )?see the webform builder$/ do |should_not|
  if should_not
    page.should_not have_css('div#form-builder')
  else
    page.should have_css('div#form-builder')
  end
end

And /^I highlight the recently added webform component$/ do
  item_selector = case @last_added_webform_component
  when "Text field"
    'div.form-builder-clickable div.webform-component-textfield'
  when "File upload"
    'div.form-builder-clickable div.webform-component-file'
  when "E-mail"
    'div.form-builder-clickable div.webform-component-email'
  when "Drop-down list"
    'div.form-builder-clickable div.webform-component-select'
  else
    raise "I can't handle the item you added last: #{@last_added_webform_component.inspect}"
  end
  #We only highlight it if it wasn't highlighted before
  puts "Selector: #{item_selector}"
  component_to_select = all(:css, item_selector).last
  unless component_to_select.has_xpath?("../div[contains(@class, 'form-builder-active')]")
    component_to_select.click
    # I added these rescue nil's because we sometimes tend to end up with a
    # Capybara::Driver::Webkit::NodeNotAttachedError because the text check is done after the
    # css matching
    wait_until(15){ page.has_css?('div.field-settings-message', :text => 'Loading...') } rescue nil
    wait_until(15){ page.has_no_css?('div.field-settings-message', :text => 'Loading...') } rescue nil
  end
end

When /^I change the component's label alignment to "([^"]*)"$/ do |alignment_selection|
  step 'I open the field settings tab "Properties"'
  within('div#form-builder-field-configure') do
    select(alignment_selection.capitalize, :from => 'edit-title-display')
  end

  case alignment_selection.capitalize
  when "Inline"
    wait_until(20){ page.has_css?('div#form-builder div.webform-component.webform-container-inline') }
  else
    raise "can't verify the alignment: #{alignment_selection}"
  end
end

When /^I change the component's default value to "([^"]*)"$/ do |default_text|
  step 'I open the field settings tab "Properties"'
  within('div#form-builder-field-configure') do
    step 'I wait for JQuery to be done'
    check "default_value-checkbox"
    fill_in 'edit-default-value', :with => default_text
  end
  wait_until(10){ page.has_css?("div#form-builder input[value='#{default_text}']") }
end

When /^I change the component's (prefix|suffix) to "([^"]*)"$/ do |fix, text|
  step 'I open the field settings tab "Display"'
  within('div#form-builder-field-configure') do
    check "field_#{fix}-checkbox"
    step 'I wait for JQuery to be done'
    fill_in "edit-field-#{fix}", :with => text
  end
  wait_until(15){ page.has_css?("span.field-#{fix}", :text => text) }
end


When /^I mark the(?: recently added)? component as required$/ do
  step 'I open the field settings tab "Validation"'
  within('div#form-builder-field-configure') do
    check("edit-required")
  end
  wait_until(15){ page.has_css?("div.form-builder-element span.form-required") }
end


Then /^I should see a webform (.*) field with the text "([^"]*)"$/ do |item, text|
  translation_hash = {
    'text' => 'div.webform-component-textfield',
    'file upload' => 'div.webform-component-file',
    'email' => 'div.webform-component-email',
    'component' => 'div.webform-component',
  }
  raise "Can't find a match for webform item #{item.inspect}" unless translation_hash.key?(item)
  css_class = translation_hash[item]
  page.should have_css(css_class, :text => text)
  #within(css_class) { page.should have_content(text) }
end

Then /^I should see a webform textfield with an inline label alignment$/ do
  page.should have_css("div.webform-component-textfield.webform-container-inline")
end

Then /^I should see a webform textfield with the text "([^"]*)"$/ do |text|
  page.should have_css("div.webform-component-textfield", :text => text)
end

Then /^I should see a webform textfield with a default value of "([^"]*)"$/ do |text|
  page.should have_css("div.webform-component-textfield input[value='#{text}']")
end

Then /^I should see a webform textfield with a (prefix|suffix) of "([^"]*)"$/ do |fix, text|
  page.should have_css("div.webform-component-textfield span.field-#{fix}", :text => text)
end

Then /^I should see a webform textfield which is marked as required$/ do
  page.should have_css("div.webform-component-textfield span.form-required")
end

Then /^I should see a webform drop-down list with the option "(.*)"$/ do |text|
  page.should have_css("div.webform-component-select select option", :text => text)
end

And /^I start to edit the current webform$/ do
  edit_element = find('div.tabs li a', :text => "Edit")
  visit(edit_element['href'])
end


And /^I remove all (.*) from the webform$/ do |components|
  translation = {
    "Text fields" => "div.form-builder-element-textfield"
  }
  css_selector = translation[components]
  raise "Can't deal with component type: #{components}" unless css_selector
  current_amount = all("div#form-builder #{css_selector}").size
  all("div#form-builder #{css_selector}").each do |component|
    puts "Clicking '#{components}' component"
    component.click
    page.has_css?('div.field-settings-message')
    page.has_no_css?('div.field-settings-message')
    first("div#form-builder #{css_selector} a.delete").click
    current_amount -= 1
    wait_until(30) { all("div#form-builder #{css_selector}").size == current_amount }
  end

end

And /^I set the custom webform confirmation to "(.*)"$/ do |text|
  check('edit-confirm-redirect-toggle')
  #open HTML input
  within_fieldset('edit-submission') do
    page.has_css?('div.wysiwyg-tabs-processed div.wysiwyg-tab.disable', :visible => true)
    find('div.wysiwyg-tabs-processed div.wysiwyg-tab.disable').click
    fill_in('edit-confirmation-page-content-value', :with => text)
  end
end

And /^I enable the block creation for this webform$/ do
  within_fieldset('edit-advanced') do
    find('span.fieldset-legend a.fieldset-title').click
    check('edit-block')
  end
end

And /^I start filling out the created webform$/ do
  visit(@content_url)
end

Then /^I should see (\d+) submissions? on the results page of the current webform$/ do |amount|
  step 'I go to the current webform'
  click_link('Results')
  wait_until(15) { page.has_css?('iframe.overlay-active', :visible => true)}
  within_frame(1) do
    all('div.region-content div.content tbody tr').size.should == amount.to_i
    click_link('overlay-close')
  end
end

And /^I add the option "(.*)" to the drop-down list$/ do |text|
  step 'I open the field settings tab "Options"'
  click_link "Add item"
  #Marc: I know it's annoying, but I have no idea what we could wait for.
  #The -processed classes on divs and jquery doesn't seem to work
  sleep 4
  all('input.option-value.form-text').last.set(text)
  wait_until(15){ page.has_css?('div.webform-component-select select option', :text => text)}
end

And /^I (disable|enable) webform submissions for (anonymous user)s?$/ do |dis_en, userclass|
  within_fieldset('edit-role-control') do
    find('span.fieldset-legend a.fieldset-title').click
    case dis_en
    when 'disable'
      uncheck(userclass)
    when 'enable'
      check(userclass)
    end

  end
end

When /^I (disable|enable) webforms for the (.*) content type$/ do |dis_en, content_type|
  step 'I go to the webform content type configuration page'
  checkbox = "edit-node-types-#{content_type}"
  case dis_en
  when 'enable'
    check(checkbox)
  when 'disable'
    uncheck(checkbox)
  end
  step 'I press "Save configuration"'
end

When /^I (disable|enable) the (.*) component for webforms$/ do |dis_en, component|
  step 'I go to the webform content type configuration page'
  checkbox = "edit-components-#{component}"
  case dis_en
  when 'enable'
    check(checkbox)
  when 'disable'
    uncheck(checkbox)
  end
  step 'I press "Save configuration"'
end

Then /^I should (not )?see a webform (.*) component in the "Add field" tab$/ do |should_not, component|
  selector = "div.horizontal-tabs-panes li.ui-draggable a[href*='#{component}']"
  if should_not
    page.should_not have_css(selector)
  else
    page.should have_css(selector)
  end
end

When /^I enable cookies for webform submission tracking$/ do
  step 'I go to the webform content type configuration page'
  find('a.fieldset-title').click
  check('edit-webform-use-cookies')
  step 'I press "Save configuration"'
end
