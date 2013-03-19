require '../helpers/qa_backdoor.rb'

Given /^the module (.*) is (enabled|disabled)$/ do |module_name, action|
  raise "No QA backdoor detected. Bailing out." unless $site_capabilities[:backdoor]
  module_name = module_name.delete("\"'")
  puts "#{action.gsub(/ed$/, 'ing')} module #{module_name.inspect}."
  backdoor = QaBackdoor.new($config['sut_url'])

  if action == "enabled"
    backdoor.enable_module(module_name)
  elsif action == "disabled"
    backdoor.disable_module(module_name)
  end

  puts "Module #{module_name.inspect} #{action}."
end

Given /^I save (?:this|the) state as snapshot ['"]([^"]*)['"]$/ do |snapshot_name|
  raise "No QA backdoor detected. Bailing out." unless $site_capabilities[:backdoor]
  snapshot_name = snapshot_name.delete("\"'")
  puts "Saving snapshot of current state as: #{snapshot_name.inspect}."
  backdoor = QaBackdoor.new($config['sut_url'])
  backdoor.create_snapshot(snapshot_name)
end


Given /^I (?:load|restore) the snapshot ['"](.*)['"]$/ do |snapshot_name|
  raise "No QA backdoor detected. Bailing out." unless $site_capabilities[:backdoor]
  snapshot_name = snapshot_name.delete("\"'")
  backdoor = QaBackdoor.new($config['sut_url'])
  puts "Loading snapshot: #{snapshot_name.inspect}."
  backdoor.restore_snapshot(snapshot_name)
end

Given /^I (uninstall|install) the module drush calls ["'](.*)["']$/ do |operation, module_drush_name|
  raise "No QA backdoor detected. Bailing out." unless $site_capabilities[:backdoor]
  backdoor = QaBackdoor.new($config['sut_url'])
  case operation
  when "install"
    backdoor.install_module(module_drush_name)
  when "uninstall"
    backdoor.uninstall_module(module_drush_name)
  end
end

Given /^the user ["']([^"']+)["'] (has|does not have) the role ["']([^"']+)["']$/ do |user, has, role|
  raise "No QA backdoor detected. Bailing out." unless $site_capabilities[:backdoor]
  backdoor = QaBackdoor.new($config['sut_url'])

  if has == 'has'
    puts "Adding role #{role} to #{user}"
    backdoor.add_user_role(user, role)
  else
    puts "Removing role #{role} from #{user}"
    backdoor.remove_user_role(user, role)
  end
end

Given /^the role ["']([^"']+)["'] (has|does not have) the permission ["']([^"']+)["']$/ do |role, has, permission|
  raise "No QA backdoor detected. Bailing out." unless $site_capabilities[:backdoor]
  backdoor = QaBackdoor.new($config['sut_url'])

  if has == 'has'
    puts "Adding permission #{permission} to #{role}"
    backdoor.role_add_permission(role, permission)
  else
    puts "Removing permission #{permission} from #{role}"
    backdoor.role_remove_permission(role, permission)
  end
end

Given /^a new user ["']([^"']+)["'] with the password ["']([^"']+)["']$/ do |name, password|
  puts "Creating new user #{name} with password #{password}"
  raise "No QA backdoor detected. Bailing out." unless $site_capabilities[:backdoor]
  backdoor = QaBackdoor.new($config['sut_url'], :logger => $logger)
  backdoor.create_user(name, password)
end

Given /^(\d+) users( and an anonymous one)?(?: with role ["']([^"']+)["'])? on my site$/ do |amount, anonymous, role|
  @global_users = (1..amount.to_i - 1).map do |count|
    username = "drupal_user_#{count}"
    password = "#{username}_Passw0rd!"
    step "a new user '#{username}' with the password '#{password}'"
    step "the user '#{username}' has the role '#{role}'" if role

    { :username => username, :password => password }
  end

  if anonymous
    @global_users.push({ :username => "__anonymous__" })
  else
    @global_users.push({ :username => "__qatestuser__" })
  end
end

Then /^I should( not)? see the ['"]([^"']+)['"] theme directory$/ do |negation, dir|
  raise "No QA backdoor detected. Bailing out." unless $site_capabilities[:backdoor]
  backdoor = QaBackdoor.new($config['sut_url'])
  contents = backdoor.list_directory_content("./sites/#{$config['sut_host']}/themes/mythemes/")

  if negation
    contents.should_not include(dir)
  else
    contents.should include(dir)
  end
end

Then /^I should( not)? see the files ['"]([^"']+)['"] in the ['"]([^"']+)['"] theme directory$/ do |negation, files, theme_dir|
  raise "No QA backdoor detected. Bailing out." unless $site_capabilities[:backdoor]
  backdoor = QaBackdoor.new($config['sut_url'])
  expected_files = files.split(",").map { |el| el.strip }
  contents = backdoor.list_directory_content("./sites/#{$config['sut_host']}/themes/mythemes/#{theme_dir}/")

  expected_files.each do |file|
    if negation
      contents.should_not include(file)
    else
      contents.should include(file)
    end
  end
end
