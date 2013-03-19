And /^I add (\d+) random (images?|videos?) to the site$/ do |amount, vid_img|
  case vid_img
  when /images?/
    media_url_list =
    [
      "http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/460x311corn_harvester.png",
      "http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/herpderphorse.jpg",
      "http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/77x96jupiter-planet.jpg",
      "http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/425x725wheat-harvest.jpg",
      "http://testimages.drupalgardens.com/media/432/download/92x120flail_0.png",
      "http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/150x62XMasNeon.gif"
    ]

  when /videos?/
    media_url_list = ['http://www.youtube.com/watch?v=doDCdqaHBZ8', 'http://www.youtube.com/watch?v=VHO9uZX9FNU', 'http://www.youtube.com/watch?v=XZm7Q71F83U']
  end

  amount.to_i.times do
    step 'I go to the media content administration'
    click_link('Add file')
    within_frame('mediaBrowser') do
      puts "Switched iframe, clicking embed url link"
      wait_until(30) { first('a#media-tab-media_internet-link', :visible => true) }
      #crashes on webkit :(
      click_link('media-tab-media_internet-link')
      current_media_item = media_url_list.shuffle.pop
      raise "ran out of unique media items to add." unless current_media_item
      fill_in 'edit-embed-code', :with => current_media_item
      find(:css, 'div.content input[value=Submit]').click
    end
  end


end



And /^I add (\d+) random (images?|videos?) to the gallery$/ do |amount, vid_img|

  case vid_img
  when /images?/
    media_url_list =
    [
      "http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/460x311corn_harvester.png",
      "http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/herpderphorse.jpg",
      "http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/77x96jupiter-planet.jpg",
      "http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/425x725wheat-harvest.jpg",
      "http://testimages.drupalgardens.com/media/432/download/92x120flail_0.png",
      "http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/150x62XMasNeon.gif"
    ]

  when /videos?/
    media_url_list = [
      'http://www.youtube.com/watch?v=doDCdqaHBZ8',
      'http://www.youtube.com/watch?v=VHO9uZX9FNU',
      'http://www.youtube.com/watch?v=XZm7Q71F83U' ]
  end

  amount.to_i.times do
    visit(@content_url)
    last_page_button = first("a[title='Go to last page'].active")
    last_page_button.click if last_page_button
    before_count = all('div#main div.media-gallery-item').size
    puts "Detected #{before_count} images"
    after_count = before_count + 1
    click_link('Add media')
    within_frame('mediaBrowser') do
      puts "Switched iframe, clicking embed url link"
      wait_until(30) { has_css?('a#media-tab-media_internet-link', :visible => true) }
      #crashes on webkit :(
      click_link('media-tab-media_internet-link')
      current_media_item = media_url_list.shuffle!.pop
      raise "ran out of unique media items to add." unless current_media_item
      fill_in 'edit-embed-code', :with => current_media_item
      find(:css, 'div.content input[value=Submit]').click
    end

    # explicitly wait for iframe to disappear
    wait_until(15) { has_no_css?("iframe#mediaBrowser") }

    last_page_button.click if last_page_button
    puts "Waiting for image number #{after_count} to be present"
    wait_until(30) do
      last_page_button = first("a[title='Go to last page'].active")
      if last_page_button
        last_page_button.click
        #If we switched pages for the first time, we only expect 1 item
        after_count = 1 if all('div#main div.media-gallery-item').size == 1
      end
      all('div#main div.media-gallery-item').size == after_count
    end
    puts "Found image number #{after_count}"
  end
end

And /^I switch the gallery to display media in (full screen|a lightbox)$/ do |mode|
  unless current_url == @content_url
    puts "Navigating to the main gallery URL #{@content_url} (currently: #{current_url})"
    visit(@content_url)
  end
  step 'I edit the gallery'
  case mode
  when 'full screen'
    choose('Show media on a full page')
  when 'a lightbox'
    choose('Show media in a lightbox')
  end
  step 'I press "Save"'
end

And /^I click on (the first|a random) image in the gallery$/ do |which_image|
  case which_image
  when "the first"
    first('div#main div.media-gallery-item img').click
  when "a random"
    all('div#main div.media-gallery-item img').shuffle.first.click
  end
end

Then /^I should see a fullscreen image$/ do
  using_wait_time(10) {page.should have_css('div.media-gallery-detail img')}
end

Then /^I should see a lightbox image$/ do
  using_wait_time(10) {page.should have_css('div.lightbox-stack img')}
end

Then /^I should see (\d+) (?:images|videos) in the gallery$/ do |expected_amount|
  visit(@content_url)
  amount = 0

  # Debug work around
  wait_until(15) do
    amount = all('div#main div.media-gallery-item').size
    puts "Found images: #{amount}"
    amount > 0
  end
  #first pagination page
  number_of_media_items = all('div#main div.media-gallery-item').size
  #following pagination pages
  next_page_button = first("a[title='Go to next page'].active")
  while next_page_button
    next_page_button.click
    number_of_media_items += all('div#main div.media-gallery-item').size
    next_page_button = first("a[title='Go to next page'].active")
  end
  number_of_media_items.should == expected_amount.to_i
end

And /^I change the license setting of all images to "(.*)"$/ do |license|
  capitalized_license = license.split(" ").map{|item| item.capitalize}.join(" ")
  click_link('Edit media')
  #Will probably only work with selenium, but since we don't have names for the iframes...
  within_frame(2) do
    all("select[id^='edit-field-license-und']").each do |item|
      select(capitalized_license, :from => item[:id])
    end
    step 'I press "Save"'
  end
  step 'I wait for all iframes to disappear'
end

Then /^I should see the "(.*)" license logo$/ do |license|
  capitalized_license = license.split(" ").map{|item| item.capitalize}.join(" ")
  using_wait_time(10) { page.should have_css("span.media-license span[title='#{capitalized_license}']") }
end

And /^I change the title for the all galleries page to "(.*)"$/ do |new_title|
  click_link('Edit all galleries')
  wait_until(25) { page.has_css?('iframe.overlay-active', :visible => true) }
  #This one only has 1 iframe depth... weird
  within_frame(1) do
    fill_in 'Title', :with => new_title
    step 'I press "Save"'
    click_link('overlay-close')
  end
  step 'I wait for all iframes to disappear'
end

And /^I change the gallery layout to (\d+)x(\d+)$/ do |columns, rows|
  raise "The minimum number of columns is 2" if columns.to_i < 2
  raise "The maximum number of columns is 10" if columns.to_i > 10
  click_link('Edit gallery')
  #Will probably only work with selenium, but since we don't have names for the iframes...
  within_frame(2) do
    select(columns, :from => 'edit-media-gallery-columns-und')
    fill_in 'edit-media-gallery-rows-und-0-value', :with => rows
    step 'I press "Save"'
  end
  step 'I wait for all iframes to disappear'
end

Then /^I should see a gallery with a (\d+)x(\d+) layout$/ do |columns, rows|
  #This actually just checks for the correct div for the given amount of columns
  #and the theoretical maximum number of images
  page.should have_css("div#content-area div.mg-col-#{columns}")
  page.all('div#content-area div.media-gallery-media-item-thumbnail').size.should <= (columns.to_i * rows.to_i)
end


And /^I (disable|enable) the block creation for this gallery$/ do |dis_en|
  click_link('Edit gallery')
  #Will probably only work with selenium, but since we don't have names for the iframes...
  within_frame(2) do
    step 'I click on the vertical tab named "Blocks"'
    checkbox_id = 'edit-media-gallery-expose-block-und'
    case dis_en
    when "enable"
      check(checkbox_id)
    when "disable"
      uncheck(checkbox_id)
    end
    step 'I press "Save"'
  end
  step 'I wait for all iframes to disappear'
end

And /I delete (and confirm )?the most recent image using the content administration$/ do |confirm|
  step "I delete #{confirm}the recent 1 images using the content administration"
end

And /I delete (and confirm )?the recent (\d+) images using the content administration$/ do |confirm, amount|
  step 'I go to the media content administration'
  #This means we are currently in a view that is sorted descending
  wait_until(30) { page.has_css?("a[title='sort by Updated'] img", :visible => true) }
  first("a[title='sort by Updated'] img")[:title].should == 'sort ascending'
  all('tbody tr input.form-checkbox')[0..(amount.to_i - 1)].each{ |checkbox| check(checkbox[:id]) }
  select('Delete', :from => 'operation')
  step 'I press "Submit"'
  step 'I press "Delete"' if confirm
end

Then /^I should be able to drag an image from the last position to the first position$/ do
  original_images = all("div#main img[typeof='foaf:Image']")
  original_first_image = original_images.first
  original_last_image = original_images.last
  original_last_image.drag_to(original_first_image)
  new_images = all("div#main img[typeof='foaf:Image']")
  new_first_image = new_images.first
  step 'I wait for JQuery to be done'
  original_last_image[:src].should == new_first_image[:src]
end

Then /^I should be able to drag images to the (first|last) position$/ do |pos|
  amount_of_images = all("div#main img[typeof='foaf:Image']").size
  amount_of_images.times do
    original_images = all("div#main img[typeof='foaf:Image']")
    original_image = original_images.send(pos)
    #pick an image that ISN'T the one in the front
    original_random_image = original_images[1..-1].shuffle.first
    puts "Good news, '#{original_random_image[:src].split('/').last}'! You're going to the #{pos} position!"
    original_random_image.drag_to(original_image)
    new_images = all("div#main img[typeof='foaf:Image']")
    new_image = new_images.send(pos)
    step 'I wait for JQuery to be done'
    original_random_image[:src].should == new_image[:src]
  end
end


