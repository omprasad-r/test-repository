require 'yaml'

GARDENS_MODULES = YAML.load_file(File.dirname(File.expand_path(__FILE__)) + '/../../../helpers/module_list.yaml')


def get_modules(options = {})
  found_modules = {}

  find("form#system-modules").all(:css, "tbody tr").each do |table_row|
    mod_name = table_row.find(:css, "label").text
    mod_enabled = table_row.find(:css, "input[type='checkbox']").checked?
    #different in akephalos vs selenium
    mod_clickable = ["false", nil].include?(table_row.find(:css, "input[type='checkbox']")[:disabled])
    #in case somebody only wants the clickable ones
    if (!options[:only_clickable] or (mod_clickable and options[:only_clickable]))
      found_modules[mod_name] = mod_enabled
    end
  end

  #return the list
  raise "Couldn't find any modules" if found_modules.empty?

  unless options[:include_devel_module]
    ["Devel node access", "Devel", "Devel generate"].each { |mod| found_modules.delete(mod) }
  end

  found_modules
end

Given /^I switch the state of module ['"]([^"']+)['"]$/ do |module_name|
  puts "Switching the module state of #{module_name}"

  find("form#system-modules").all(:css, "tbody tr").each do |table_row|
    mod_name = table_row.find(:css, "label").text

    if mod_name == module_name
      mod_label_for_attribute = table_row.find(:css, "label")['for']
      mod_enabled = table_row.find(:css, "input[type='checkbox']").checked?

      #different in akephalos vs selenium
      mod_clickable = ["false", nil].include?(table_row.find(:css, "input[type='checkbox']")[:disabled])
      raise "Module #{module_name} can't be checked/unchecked, it is disabled" unless mod_clickable

      if mod_enabled
        $logger.info("Unchecking module: #{module_name} (#{mod_label_for_attribute.inspect}).")
        page.uncheck(mod_label_for_attribute)
      elsif !mod_enabled
        $logger.info("Checking module: #{module_name} (#{mod_label_for_attribute.inspect}).")
        page.check(mod_label_for_attribute)
      end
    end
  end
end

Given /^I save the current module configuration$/ do
  step "I press 'edit-submit'"

  # confirm when a modules has dependencies
  if has_css?("div.content", :text => /You must enable the .* module to install .*/)
    step "I press 'edit-submit'"
  end
end

Then /^I should see all whitelisted modules$/ do
  found_modules = get_modules.keys
  whitelist = GARDENS_MODULES['visible_modules']
  modules_on_whitelist_that_are_not_present = whitelist - found_modules
  #Some modules from the whitelist are missing it this is not empty
  modules_on_whitelist_that_are_not_present.should == []
end

Then /^I should (not )?see the "(.*)" module$/ do |should_not_be_present, module_name|
  present_modules = get_modules.keys
  if should_not_be_present
    present_modules.should_not include(module_name)
  else
    present_modules.should include(module_name)
  end
end

Then /^I shouldn't see any non-whitelisted modules$/ do
  installed_modules = get_modules
  found_modules = installed_modules.keys
  whitelist = GARDENS_MODULES['visible_modules']
  modules_that_are_present_but_not_on_the_whitelist = found_modules - whitelist
  modules_that_are_present_but_not_on_the_whitelist.should == []
end

Then /^I shouldn't see any unexpected module categories$/ do
  found_module_types = all(:css, "a.fieldset-title").map do |module_type|
    module_type.text.downcase.gsub('hide','').capitalize.strip.downcase
  end

  approved_mod_types = GARDENS_MODULES['modulegroups'].map { |mod| mod.strip.downcase }
  found_module_types.should == approved_mod_types
end


Then /^I want to be able to randomly enable and disable modules$/ do
  puts "getting clickable modules"
  clickable_modules = get_modules(:only_clickable => true)
  puts "Found #{clickable_modules.size} clickable modules"
  random_selection = Hash[clickable_modules.to_a.shuffle[1..5]]
  puts "Random selection for this test: #{random_selection.inspect}"
  successful_change_text = "The configuration options have been saved."

  random_selection.each do |dg_module|
    module_name = dg_module[0]
    module_currently_enabled = dg_module[1]
    puts "Working on module #{module_name} (currently enabled: #{module_currently_enabled})."
    step "I switch the state of module '#{module_name}'"
    step "I save the current module configuration"
    step "I should see a status message with the text '#{successful_change_text}'"
    step "I switch the state of module '#{module_name}'"
    step "I save the current module configuration"
    step "I should see a status message with the text '#{successful_change_text}'"
  end
end
