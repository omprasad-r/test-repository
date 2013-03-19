# Helpers needed in multiple tests go here

puts "Loading helpers.rb"

# ugly workaround for https://github.com/jonleighton/poltergeist/issues/105
def calculated_css_property(selector, property)
  random_part = rand().to_s.delete(".")
  exec_js = "window.jQuery('#{selector}').css('#{property}');"
  page.execute_script("myQAJQueryGalacticSuperGlobalStorageVarWithAReallyLongName_#{random_part} = #{exec_js}")
  page.evaluate_script("myQAJQueryGalacticSuperGlobalStorageVarWithAReallyLongName_#{random_part}")
end

def within_class_frame(classes)
  frame_id = get_frame_id_by_class(classes)
  #seems to only work in selenium :(
  within_frame(frame_id) do
    yield
  end
end

def get_frame_id_by_class(classes)
  frame_id = 0
  all(:css, "iframe").each do |curr_frame|
    return frame_id if classes.all? { |curr_class| curr_frame['class'].include?(curr_class) }
    frame_id += 1
  end
  raise "Couldn't find a frame with the classes: #{classes.inspect}"
end

def get_frame_id_by_title(title)
  frame_id = 0
  all(:css, "iframe").each do |curr_frame|
    return frame_id if curr_frame[:title].match(/#{title}/)
    frame_id += 1
  end
  raise "Couldn't find a frame with the title: #{title}"
end

def round_to(number, digits)
  (number * 10 ** digits).round.to_f / 10 ** digits
end

def translate_name_to_url(name)
  name.downcase.gsub(/(\s|_)/, '-')
end

