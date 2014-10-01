Then /^I should see (star|rating|percentage) teasers and a (star|rating|percentage) display type on that content$/ do|teaser_type, display_type|
  Test000120FiveStarHelper.verify_display_type(@content_url, {:teaser_type => teaser_type.to_sym, :normal_type => display_type.to_sym})
end

Given /^anonymous voting is enabled$/ do
  puts "Enabling anonymous voting"
  step "the role 'anonymous user' has the permission 'rate content'"
  step "the role 'authenticated user' has the permission 'rate content'"
end

Given /^I vote twice on ["']([^"']+)["']$/ do |content|
  puts "Performing primary votes"
  step "I vote on '#{content}'"
  puts "Performing secondary votes"
  step "I vote on '#{content}'"
end

Given /^I vote on ["']([^"']+)["'](?: using a star count of (\d+))?(?: with ["']([^"']+)["'])?$/ do |content, star_count, widget|
  star_count ||= 5
  widget ||= "Stars (rated while viewing)"
  @global_fivestar_votes = []

  @global_users.each do |user|
    value = (1..star_count).to_a.shuffle.first

    if user[:username] == "__anonymous__"
      step "I log out"
      step "I should be logged out"
    elsif user[:username] == "__qatestuser__"
      step "I am logged in as our testuser"
      step "I should be logged in"
    else
      step "I am logged in as user '#{user[:username]}' with password '#{user[:password]}'"
      step "I should be logged in"
    end
    step "I vote on '#{content}' with the value #{value} and a star count of #{star_count} using the '#{widget}' widget"
  end
end

Given /^I vote on ["']([^"']+)["'] with the value (\d+) and a star count of (\d+) using the ["']([^"']+)["'] widget$/ do |content, value, star_count, widget|
  puts "widget: #{widget}"
  step "I visit the page of content '#{content}'"
  step "I rate the current page using the '#{widget}' widget with a star count of #{star_count} using a value of #{value}"

  @global_fivestar_votes << value.to_i
  puts "Vote with value #{value} was succsessfull"
end

Given /^I rate the current page using the ["']Stars \(rated while viewing\)["'] widget with a star count of (\d+) using a value of (\d+)$/ do |star_count, value|
  value = value.to_i
  star_count = star_count.to_i

  fivestar_div = find(:css, "div.field-type-fivestar")
  select_expression = "select[id ^= 'edit-'].form-select"
  select_id_before = fivestar_div.find(:css, select_expression)['id']

  # star count sanity check
  actual_star_count = fivestar_div.all(:css, "#{select_expression} > option").size - 1
  actual_star_count.should == star_count

  step "I click on 'Give it #{value}/#{star_count}'"

  begin
    # verify that ajax voting succeeded
    wait_until(15) do
      expected_value = (100.0 * value / star_count).ceil

      option_selected = fivestar_div.has_css?(
        "#{select_expression} > option[selected][value='#{expected_value}']"
      )
      select_id_after = fivestar_div.find(:css, select_expression)['id']

      option_selected && (select_id_before != select_id_after)
    end
  rescue Capybara::TimeoutError
    raise "Fivestar vote failed: AJAX voting request timed out (Probably this known fivestar bug: DG-695)"
  end
end

Given /^I rate the current page using the ["']Stars \(rated while editing\)["'] widget with a star count of (\d+) using a value of (\d+)$/ do |star_count, value|
  edit_link = find(:css, "div.tabs ul.tabs.primary a", :text => "Edit")["href"]
  puts "Found edit link: #{edit_link}"
  visit(edit_link)

  step "I click on 'Give it #{value}/#{star_count}'"
  step "I press 'edit-submit'"
end

Given /^I rate the current page using the ["']Select list \(rated while editing\)["'] widget with a star count of (\d+) using a value of (\d+)$/ do |star_count, value|
  edit_link = find(:css, "div.tabs ul.tabs.primary a", :text => "Edit")["href"]
  puts "Found edit link: #{edit_link}"
  visit(edit_link)

  within(:css, "div.field-type-fivestar") do
    dropdown_id = find(:css, "select[id *= 'edit-field-fivestar-'].form-select")["id"]
    if value == "1"
      selected = "1 star"
    else
      selected = "#{value} stars"
    end
    select(selected, :from => dropdown_id)
  end

  step "I press 'edit-submit'"
end

Given /^I change the star count of field ["']([^"']+)["'] for the content type ["']([^"']+)["'] to (\d+)$/ do |field, content_type, star_count|
  translated_field = "field_#{field.downcase.gsub(/(\s|-)/, '_')}"
  translated_content_type = translate_name_to_url(content_type)
  field_url = "/admin/structure/types/manage/#{translated_content_type}/fields/#{translated_field}"

  visit(field_url)

  step "I select '#{star_count}' from select box 'edit-instance-settings-stars'"
  step "I press 'edit-submit'"
end

Then /^I should see correct fivestar voting results$/ do
  step "I should see correct fivestar voting results for widget 'Stars (rated while viewing)'"
end


Then /^I should see correct fivestar voting results for widget ["']Stars \(rated while viewing\)["']$/ do
  values = @global_fivestar_votes

  within(:css, "div[class *= 'field-fivestar']") do
    actual_average = find(:css, "span.average-rating > span").text.to_f
    actual_votes = find(:css, "span.total-votes > span").text.to_i

    puts "Found voting results: average: #{actual_average}, votes: #{actual_votes}"

    expected_votes = values.length
    expected_average = round_to(values.inject { |sum, el| sum + el }.to_f / values.size, 1)

    # Check tolerance since fivestar doesn't calculate correctly
    actual_average.should be_within(0.3).of(expected_average)
    actual_votes.should == expected_votes
  end
end

Then /^I should see correct fivestar voting results for widget ["'](?:Stars|Select list) \(rated while editing\)["']$/ do
   within(:css, "div[class *= 'field-fivestar']") do
    actual_vote = find(:css, "span.average-rating > span").text.to_f
    expected_vote = @global_fivestar_votes.last

    # Check tolerance since fivestar doesn't calculate correctly
    actual_vote.should be_within(0.3).of(expected_vote)
  end
end

Then /^I should see (\d) stars$/ do |star_count|
  fivestar_div = find(:xpath, "//div[contains(@class, 'field-type-fivestar')]")
  select_expression = ".//select[contains(@id, 'edit-') and contains(@class, 'form-select')]"
  actual_star_count = fivestar_div.all(:xpath, "#{select_expression}/option").size - 1
  actual_star_count.should == star_count.to_i
end

Then /^I should see a ["']([^"']+)["'] fivestar display type$/ do |display_type|
  display_selector = ""
  options = {}

  case display_type
  when "As Stars"
    display_selector = "div.star"
  when "Rating"
    display_selector = "div.field-item > div"
    options = { :text => "0/5" }
  when "Percentage"
    display_selector = "div.field-item > div"
    options = { :text => "0%" }
  else
    raise "Don't know how to validate display type #{display_type}"
  end

  page.should have_css(display_selector, options)
end
