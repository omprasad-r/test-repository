
TYPEKIT_KEY = "gdm0zpb"

Given /^the typekit font management is enabled$/ do
  puts "Enabling typekit management"
  step "the module font_management is enabled"
  step "I enable the typekit integration and add the key '#{TYPEKIT_KEY}' to the configuration"
end

Given /^I disable the typekit integration$/ do
  puts "Disabling typekit integration"
  visit("/admin/config/user-interface/font-management")
  uncheck("edit-font-management-typekit-enable")
  click_button("edit-submit")
  wait_until(15) { has_content?("The configuration options have been saved") }
end

Given /^I enable the typekit integration and add the key ["']([^"']+)["'] to the configuration$/ do |typekit_key|
  visit("/admin/config/user-interface/font-management")
  check("edit-font-management-typekit-enable")
  fill_in("edit-font-management-typekit-key", :with => typekit_key)
  click_button("edit-submit")
  wait_until(15) { has_content?("The configuration options have been saved") }
end

Given /^I select the font ["']([^"']+)["'] for element ["']([^"']+)["']$/ do |font, element|
  step "I switch to theme builder tab 'Styles'"
  step "I switch to vertical theme builder tab 'Font'"
  step "I select element '#{element}'"
  step "I select '#{font}' from select box 'style-font-family'"
end
