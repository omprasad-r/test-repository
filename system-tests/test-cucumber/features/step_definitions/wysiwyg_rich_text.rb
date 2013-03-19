require 'digest/bubblebabble' #make random things that look like words

Given /^I enable the rich-text editor$/ do
  step 'I wait for JQuery to be done'
  find(:css, "div.wysiwyg-tab.enable").click
  step 'I wait for JQuery to be done'
  wait_until(30) { find('td.cke_contents iframe').visible? }
end

Given /^I disable the rich-text editor$/ do
  step 'I wait for JQuery to be done'
  wait_until(30) { find('td.cke_contents iframe').visible? }
  step 'I wait for JQuery to be done'
  #Disabling WYSIWYG
  find(:css, "div.wysiwyg-tab.disable").click
  #Waiting for textarea to be visible
  wait_until(30) { find("div.textarea-processed textarea.text-full.form-textarea").visible? }
end


Given /^I enter a text with (\d+) words and (\d+) paragraphs into the rich text editor$/ do |word_count, paragraph_count|
  puts "Entering a tect with #{word_count} words and #{paragraph_count} paragraphs"

  word_count = word_count.to_i
  paragraph_count = paragraph_count.to_i

  words = 0.upto(word_count).map { |num| Digest.bubblebabble(rand(36**8).to_s(36)).split('-') }
  paragraphs = 0.upto(paragraph_count).map { |num| words.shuffle[0..rand(9)].join(' ') }

  content = paragraphs.map { |paragraph| "<p>#{paragraph}</p>" }.join("")

  step "I enter '#{content}' into the rich text editor"
end

Given /^I enter ["']([^"']+)["'] into the rich text editor$/ do |content|
  exec_js = <<-SCRIPT
    CKEDITOR.instances[Drupal.wysiwyg.activeId].setData('#{content}');
    jQuery('textarea#' + Drupal.wysiwyg.activeId).text('#{content}');
  SCRIPT

  page.execute_script(exec_js)
end

Given /^I add the ["']([^"']+)["'] image to the rich text editor$/ do |image_name|
  if image_name.start_with?('http://')
    image_link = image_name
  else
    image_prefix = "http://testimages.drupalgardens.com/sites/testimages.drupalgardens.com/files/styles/media_gallery_large/public/"
    image_link = image_prefix + image_name
  end

  find(:css, "td#cke_top_edit-body-und-0-value a.cke_button_media").click

  within_class_frame(["media-modal-frame", "ui-dialog-content", "ui-widget-content"]) do
    click_link("media-tab-media_internet-link")
    fill_in("edit-embed-code", :with => image_link)
    click_button("edit-submit--2")
    wait_until(30) { has_no_css?("input.media-add-from-url") }
  end

  within_class_frame(["media-modal-frame", "ui-dialog-content", "ui-widget-content"]) do
    wait_until { has_css?("input#edit-alt[value = '#{image_name}']") }
    find(:css, "a.button.fake-ok", :text => "Submit").click
  end

  wait_until { has_no_css?("iframe.media-modal-frame") }
end

Then /^I should see the ["']([^"']+)["'] image embedded in the rich text editor$/ do |image_name|
  editor_frame = get_frame_id_by_title("edit-body-und-0-value")
  within_frame(editor_frame) do
    wait_until do
      image_url = find(:css, "body img", :visible => true)[:src]
      image_url.match(/#{image_name}/)
    end
  end
end

Then /^I should see the ["']([^"']+)["'] thumbnail embedded in the page$/ do |image_name|
  name = File.basename(image_name, ".*")
  using_wait_time(10) { page.should have_css("div.media-thumbnail-frame > img[src *= '#{name}']", :visible => true) }
end
