require 'excon'

Given /^I (?:start to create|start creating) a.? (.*)$/ do |content_type|

  translated_type = translate_name_to_url(content_type)
  visit("/node/add/#{translated_type}")
  raise "Could not find content type #{content_type}" unless has_css?("head > title", :text => /^Create /)
end

Given /^I create a content type named ["']([^"']+)["'] with description ["']([^"']+)["']$/ do |content_type, description|
  puts "Create content type: #{content_type} with description #{description}"
  visit("/admin/structure/types/add")

  within(:css, "form#node-type-form") do
    fill_in("name", :with => content_type)
    fill_in("description", :with => description)
    click_button("edit-submit")
  end

  if has_no_content?("The content type #{content_type} has been added.")
    raise "Could not create content type #{content_type}"
  end
end

Given /^I add a field named ["']([^"']+)["'] of type ["']([^"']+)["'](?: with widget ['"](.*)['"])? to ["']([^"']+)["']$/ do |name, type, widget, content_type|
  puts "Adding field named #{name} of type #{type} with widget #{widget} to #{content_type}"

  translated_type = translate_name_to_url(content_type)
  manage_fields_url = "/admin/structure/types/manage/#{translated_type}/fields"

  visit(manage_fields_url)

  within(:css, "tr#-add-new-field") do
    fill_in("fields[_add_new_field][label]", :with => name)
    select(type, :from => "edit-fields-add-new-field-type")
    select(widget, :from => "edit-fields-add-new-field-widget-type") if widget
  end

  click_button("edit-submit")

  within(:css, "form[id *= 'field-ui-field-']") do
    click_button("edit-submit")
  end

  if has_no_content?("Saved #{name} configuration")
    raise "Could not add field #{name} to #{content_type}"
  end
end

Given /^I change the( teaser)? display type of field ["']([^"']+)["'] for content type ["']([^"']+)["'] to ["']([^"']+)["']$/ do |teaser, field, content_type, display_type|
  translated_type = translate_name_to_url(content_type)
  translated_field = translate_name_to_url(field)
  manage_views_url = "/admin/structure/types/manage/#{translated_type}/display"
  dropdown_id = "edit-fields-field-#{translated_field}-type";

  visit(manage_views_url)

  if teaser
    puts "Selecting teaser display type"
    click_link("Teaser")
    wait_until(15) { has_css?("a.active", :text => "Teaser") }
  end

  within(:css, "form[id *= 'field-ui-display-']") do
    select(display_type, :from => dropdown_id)
    wait_until(15) do
      current_dropdown_id = find(:css, "select[id *= '#{dropdown_id}'].field-formatter-type")["id"]
      dropdown_id != current_dropdown_id
    end unless find("##{dropdown_id} > option[selected]").text == display_type

    click_button("edit-submit")
  end

end

And /^I (start to create|start creating|create) (.*) with the title ['"](.*)['"] and the description ['"](.*)['"]$/ do |finish_or_not, type, title, description|
  step "I start to create #{type.downcase}"
  #This isn't a good solution, but it works for now
  sleep 4 if Capybara.current_driver == :selenium
  step "I disable the rich-text editor"
  step 'I wait for JQuery to be done'
  #This isn't a good solution, but it works for now
  sleep 4 if Capybara.current_driver == :selenium
  step "I fill in 'Title' with '#{title}'"

  desc_element = if has_css?("textarea#edit-body-und-0-value")
                   "edit-body-und-0-value"
                 else
                   "Description"
                 end

  step "I fill in '#{desc_element}' with '#{description}'"

  if finish_or_not == 'create'
    step "I press 'edit-submit'"
    @content_url = current_url
  end
end

And /^I change the (.*) text to ['"](.*)["']$/ do |item, new_text|
  blk = lambda do
    if item == 'description'
      step "I enter '#{new_text}' into the rich text editor"
    else
      step "I fill in '#{item.capitalize}' with '#{new_text}'"
    end
    step "I press 'edit-submit'"
  end

  begin
    wait_until(15) { has_css?("iframe.overlay-element.overlay-active") }
    puts "Switching to overlay iFrame"
    within_class_frame(['overlay-element', 'overlay-active'], &blk)
  rescue Capybara::TimeoutError => e
    puts "Editing the form without switching to an iFrame"
    blk.call
  end
end

And /^I edit the (.*)( using the overlay)?$/ do |content_type, use_overlay|
  #ugly, needs refactoring
  if content_type == 'webform'
    step 'I start to edit the current webform'
  else
    translation = {
      'gallery' => 'Edit gallery',
      'page' => 'Edit',
      'options for all galleries' => 'Edit all galleries',
    }
    raise "Can't find a content type to edit for #{content_type.inspect}" unless translation.key?(content_type)

    if use_overlay
      click_link(translation[content_type])
    else
      url = find(:css, "div#main a", :text => translation[content_type])[:href]
      visit(url)
    end

    step 'I wait for JQuery to be done'
  end
end

And /^I delete the current (.*)$/ do |content_type|
  step "I edit the #{content_type}"
  step "I press 'Delete'"
  step "I press 'Delete'"
  wait_until(30) { page.has_css?('div.status', :text => 'has been deleted.', :visible => true)}
end

Then /^the content should be (gone|available)$/ do |status|
  puts "Checking that #{@content_url.inspect} is #{status}"
  case status
  when 'gone'
    Excon.get(@content_url).status.to_i.should == 404
  when 'available'
    Excon.get(@content_url).status.to_i.should == 200
  else
    raise "Can't check for status: #{status.inspect}"
  end
end

Then /^I should (not )?see content with the title '(.*)'$/ do |should_not_be_there, title|
  if should_not_be_there
    page.should_not have_css("h1#page-title", :text => title)
  else
    page.should have_css("h1#page-title", :text => title)
  end
end

Then /^I should (not )?see content with '(.*)' in the body$/ do |should_not_be_there, text|
  if should_not_be_there
    page.should_not have_css('div#main div.node', :text => text)
  else
    page.should have_css('div#main div.node', :text => text)
  end
end

And /^I delete all (.*) content$/ do |content_type|
  visit('/admin/content')
  select content_type, :from => 'edit-type'
  step "I press 'Filter'"
  find('th.select-all input').click
  select 'Delete selected content', :from => 'edit-operation'
  step "I press 'Update'"
  step "I press 'Delete'"
end
