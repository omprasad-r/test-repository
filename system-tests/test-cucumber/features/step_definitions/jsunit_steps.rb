Then /^the jsunit tests should pass$/ do
  step 'I am on the jsunit testrunner page'
  within_frame("mainFrame") do
    within_frame("mainData") do
      fill_in("testFileName", :with => "#{ $config['sut_host'] }/jsunit")
      click_button("Run")
    end
  end
  within_frame("mainFrame") do
    within_frame("mainStatus") do
      wait_until(300) { find(:css, "div#content").has_content?("Done") }
    end
  end
    
  tests = nil
  errors = nil
  fails = nil
  
  within_frame("mainFrame"){ within_frame("mainCounts"){ within_frame("mainCountsRuns"){ tests = text.match(/\d+/).to_s.to_i } } }
  tests.should be > 0
  within_frame("mainFrame"){ within_frame("mainCounts"){ within_frame("mainCountsErrors"){ errors = text.match(/\d+/).to_s.to_i } } }
  within_frame("mainFrame"){ within_frame("mainCounts"){ within_frame("mainCountsFailures"){ fails = text.match(/\d+/).to_s.to_i } } }
  problems = nil
  within_frame("mainFrame"){ within_frame("mainErrors"){ problems = find("select[name='problemsList']").text.gsub(/failedjsunit/, "failed\njsunit").gsub(/errorjsunit/, "error\njsunit")} }
  puts problems unless problems.empty?
  
  if errors > 0 || fails > 0
    error_details = nil
    within_frame("mainFrame"){ within_frame("mainErrors"){ click_button('Show all') } }
    within_window(page.driver.browser.window_handles.last) do
      error_details = page.text
    end
    puts error_details
    #This is kind of a hack. This will never be empty, but if we have errors or fails, we'll want this in the logs
    error_details.should == ''
    errors.should == 0
    fails.should == 0
    
  end
    
end