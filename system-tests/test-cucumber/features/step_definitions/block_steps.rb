Then /^I should (not )?see a block with the name "(.*)" in the block list$/ do |should_not_be_there, block_name|
  step 'I go to the block configuration page'
  if should_not_be_there
    page.should_not have_xpath("//td[@class='block' and contains(text(),'#{block_name}')]")
  else
    page.should have_xpath("//td[@class='block' and contains(text(),'#{block_name}')]")
  end
end