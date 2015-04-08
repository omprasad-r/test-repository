Then /^I should be logged in$/ do
  page.should have_css('body.logged-in')
end

And /^I log out$/ do
  visit("/user/logout")
  step "I should be logged out"
end

When /^I manually log out$/ do
  page.find("p.welcome-text a").click
  click_link("Logout")
end

Then /^I should be logged out$/ do
  page.should have_css('body.not-logged-in')
end

Given /^I am (manually )?logged in as admin(?:istrator)?$/ do |manually|
  step 'I have a new session'
  if manually == "manually"
    step 'I am manually logged in as user "commonsadmin" with password "commonspass"'
  else
    step 'I am logged in as user "commonsadmin" with password "commonspass"'
  end
end

Given /^I am logged in as our testuser$/ do
  step 'I am logged in as "qatestuser" with the password "ghetto#exits"'
end

Given /^I am logged in as our testuser using the login iframe$/ do
  username = "qatestuser"
  password = "ham#fee"

  click_link('Log in')
  within_class_frame(["overlay-active"]) do
    within(:css, "form#user-login") do
      fill_in('edit-name', :with => username)
      fill_in('edit-pass', :with => password)
      click_button("edit-submit")
    end
  end
end

Given /^I am (manually )?logged in as (?:user )?["'](.*)["'] with (?:the )?password ['"](.*)['"]$/ do |manually, user, password|
  step 'I have a new session'
  #$site_capabilities get set in env.rb
  if ($site_capabilities[:fast_user_switching] && (manually != 'manually'))
    #needs the devel module
    # ./drush dl devel -r /var/www/ -y
    # ./drush en devel -r /var/www/ -y
    # ./drush dl drush_role-7.x-1.x -r /var/www/ -y
    # ./drush role-add-perm 1 "switch users" -r /var/www/
    puts "Using /devel/switch/#{user} to log in"
    step "I visit '/devel/switch/#{user}'"
  else
    step 'I am on "/user"'
    step "I fill in \"edit-name\" with \"#{user}\""
    step "I fill in \"edit-pass\" with \"#{password}\""
    step 'I press "Log in"'
  end
  step 'I should be logged in'
end
