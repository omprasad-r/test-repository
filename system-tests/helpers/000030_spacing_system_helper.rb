require "rubygems"
require "selenium/webdriver"
require "rspec"
require "theme_builder.rb"


module Test000030SpacingSystemHelper
  # helper test for edge data entry.
  # save_type = nil (no save), save (save and reopen the saved theme), publish (publish and exit TB)
  def edge_width_data_entry_test(save_type = nil, theme_name = "testtheme", new_theme = 'sparks')
    theme_name = "#{theme_name}_#{Time.now.to_i}" if theme_name == "testtheme"
    Log.logger.info("Starting edge data entry #{save_type} test")
    login($config['user_accounts']['qatestuser']['user'], $config['user_accounts']['qatestuser']['password'])
    case save_type
    when "save"
      edge_width_data_entry_save_test(theme_name)
    when "publish"
      edge_width_data_entry_publish_test(theme_name, new_theme)
    when nil
      edge_width_data_entry_plain_test()
    end
  end
  
  
  def edge_width_data_entry_plain_test()
    Log.logger.info("Starting edge data entry save test")
    spacings = ['margin', 'padding', 'border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    elements.each_with_index do |element, element_index|
      Log.logger.info("Working on the Element: #{element} [#{element_index+1}/#{elements.size}]")
      spacings.each_with_index do |spacing, space_index|
        Log.logger.info("Working using a spacing of: #{spacing} [#{space_index+1}/#{spacings.size}]")
        edges.each_with_index do |edge, edge_index|
          Log.logger.info("Working on the edge: #{edge} [#{edge_index+1}/#{edges.size}]")
          ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
            Log.logger.info("Testing Element #{element.inspect} [#{element_index+1}/#{elements.size}] with spacing #{spacing.inspect} [#{space_index+1}/#{spacings.size}] and edge #{edge.inspect} [#{edge_index+1}/#{edges.size}]")
            edge_prop = edge_sel = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
            themer.switch_tab('Styles')
            themer.click_in_tab('Spacing')
            Log.logger.info("Selecting element: css=#{element}")
            themer.select_element(element, :css)
            type_width = (self.rand_width*100).to_i
            themer.type_text(edge_sel, type_width.to_s)
            set_spacing_size = themer.read_input(edge_sel)
            res = themer.read_css_property(element, edge_prop)
            effective_spacing_size = /(.*)[a-z][a-z]/.match(res)[1]
            #, "The effective spacing size on the HTML (#{effective_spacing_size.inspect} (#{res.inspect})) isn't what we set (#{set_spacing_size.inspect}) for #{spacing}, edge #{edge}, element #{element}")
            set_spacing_size.should == effective_spacing_size
            Log.logger.info("Assertion succeeded, moving on to next item.")
          end
        end
      end
    end
  end
  
  def edge_width_data_entry_save_test(theme_name)
    Log.logger.info("Starting edge data entry save test")
    spacings = ['margin', 'padding', 'border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    elements.each_with_index do |element, element_index|
      Log.logger.info("Working on the Element: #{element} [#{element_index+1}/#{elements.size}]")
      spacings.each_with_index do |spacing, space_index|
        Log.logger.info("Working using a spacing of: #{spacing} [#{space_index+1}/#{spacings.size}]")
        edges.each_with_index do |edge, edge_index|
          Log.logger.info("Working on the edge: #{edge} [#{edge_index+1}/#{edges.size}]")
          ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
            Log.logger.info("Testing Element #{element.inspect} [#{element_index+1}/#{elements.size}] with spacing #{spacing.inspect} [#{space_index+1}/#{spacings.size}] and edge #{edge.inspect} [#{edge_index+1}/#{edges.size}]")
            edge_sel = edge_prop = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
            themer.switch_tab('Styles')
            themer.click_in_tab('Spacing')
            Log.logger.info("Selecting element: css=#{element}")
            themer.select_element(element, :css)
            type_width = (self.rand_width*100).to_i
            themer.type_text(edge_sel, type_width.to_s)
            set_spacing_size = themer.read_input(edge_sel)
            #save the theme
            themer.save_theme_as(theme_name)
            #directly open the saved theme
            themer.open_theme('My Themes', theme_name)
            res = themer.read_css_property(element, edge_prop)
            effective_spacing_size = /(.*)[a-z][a-z]/.match(res)[1]
            #"The effective spacing size on the HTML (#{effective_spacing_size.inspect}) (#{res.inspect}) isn't what we set (#{set_spacing_size.inspect}) for #{spacing}, edge #{edge}, element #{element}")
            set_spacing_size.should == effective_spacing_size
            Log.logger.info("Assertion succeeded, moving on to next item.")
          end
        end
      end
    end
  end
  
  
  def edge_width_data_entry_publish_test(theme_name, new_theme)
    Log.logger.info("Starting edge data entry publish test")
    spacings = ['margin', 'padding', 'border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    elements.each_with_index do |element, element_index|
      Log.logger.info("Working on the Element: #{element} [#{element_index+1}/#{elements.size}]")
      spacings.each_with_index do |spacing, space_index|
        Log.logger.info("Working using a spacing of: #{spacing} [#{space_index+1}/#{spacings.size}]")
        edges.each_with_index do |edge, edge_index|
          Log.logger.info("Working on the edge: #{edge} [#{edge_index+1}/#{edges.size}]")
          Log.logger.info("Testing Element #{element.inspect} [#{element_index+1}/#{elements.size}] with spacing #{spacing.inspect} [#{space_index+1}/#{spacings.size}] and edge #{edge.inspect} [#{edge_index+1}/#{edges.size}]")
          edge_sel = edge_prop = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
          expected_spacing_size = nil
          ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
            themer.open_theme('Gardens', new_theme)
            themer.switch_tab('Styles')
            themer.click_in_tab('Spacing')
            
            Log.logger.info("Selecting element: css=#{element}")
            themer.select_element(element, :css)
            type_width = (self.rand_width*100).to_i
            Log.logger.info("Typing new width: #{type_width.inspect} in #{edge_sel.inspect}")
            themer.type_text(edge_sel, type_width.to_s)
            expected_spacing_size = themer.read_input(edge_sel)
            themer.publish_theme(theme_name)
          end
            
          res = ThemeBuilder.read_css_property(@browser, element, edge_prop)
          begin
            effective_spacing_size = /(.*)[a-z][a-z]/.match(res)[1] 
          rescue StandardError
            raise "Couldn't figure out the spacing size for #{element.inspect} from this: #{res.inspect}"
          end
          #"The effective spacing size on the HTML (#{effective_spacing_size.inspect}) isn't what we set (#{expected_spacing_size.inspect}) for #{spacing}, edge #{edge}, element #{element}")
          expected_spacing_size.should == effective_spacing_size
          Log.logger.info("----Expected size #{expected_spacing_size.inspect} was the one we found, cool!----")
        end
      end
    end
  end
    
  
  
  
  def edge_width_slider_test(save_type = nil, theme_name = 'testtheme', new_theme = 'campaign')
    theme_name = "#{theme_name}_#{Time.now.to_i}" if theme_name == "testtheme"
    Log.logger.info("Starting edge slider #{save_type} test")
    login($config['user_accounts']['qatestuser']['user'], $config['user_accounts']['qatestuser']['password'])
    case save_type
    when "save"
      edge_width_slider_save_test(theme_name)
    when "publish"
      edge_width_slider_publish_test(theme_name, new_theme)
    when nil
      edge_width_slider_plain_test()
    end   
  end
  
  
  
  def edge_width_slider_plain_test()
    spacings = ['margin', 'padding', 'border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    elements.each do |element|
      spacings.each do |spacing|
        edges.each do |edge|
          Log.logger.info("Testing #{spacing} #{edge} for #{element}")
          edge_sel = edge_prop = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
          ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
            themer.switch_tab('Styles')
            themer.click_in_tab('Spacing')
            themer.select_element(element, :css)
            themer.move_slider_to_percent(edge_sel, self.rand_width)
            slider_spacing_size = themer.read_input(edge_sel)            
            res = themer.read_css_property(element, edge_prop)
            effective_spacing_size = /(.*)[a-z][a-z]/.match(res)[1]
            #"The effective spacing size on the HTML #{effective_spacing_size.inspect} (#{res.inspect}) and the size from the slider #{slider_spacing_size.inspect} for #{spacing} and edge #{edge} do not match for element #{element}")
            slider_spacing_size.should == effective_spacing_size
          end
        end
      end
    end
  end
  
  def edge_width_slider_save_test(theme_name)
    spacings = ['margin', 'padding', 'border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    elements.each do |element|
      spacings.each do |spacing|
        edges.each do |edge|
          Log.logger.info("Testing #{spacing} #{edge} for #{element}")
          edge_sel = edge_prop = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
          ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
            themer.switch_tab('Styles')
            themer.click_in_tab('Spacing')
            themer.select_element(element, :css)
            themer.move_slider_to_percent(edge_sel, self.rand_width)
            slider_spacing_size = themer.read_input(edge_sel)
            
            themer.save_theme_as(theme_name)
            themer.open_theme('My Themes', theme_name)
            
            res = themer.read_css_property(element, edge_prop)
            effective_spacing_size = /(.*)[a-z][a-z]/.match(res)[1]
            #, "The effective spacing size on the HTML #{effective_spacing_size.inspect} and the size from the slider #{slider_spacing_size.inspect} for #{spacing} and edge #{edge} do not match for element #{element}")
            slider_spacing_size.should == effective_spacing_size
          end
        end
      end
    end
  end
  
  
  # helper test for edge sliders.
  # save_type = nil (no save), save (save and reopen the saved theme), publish (publish and exit TB)
  def edge_width_slider_publish_test(theme_name, new_theme)
    spacings = ['margin', 'padding', 'border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    elements.each do |element|
      spacings.each do |spacing|
        edges.each do |edge|
          Log.logger.info("Testing #{spacing} #{edge} for #{element}")
          edge_sel = edge_prop = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
          expected_slider_spacing_size = nil
          ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
            themer.open_theme('Gardens', new_theme)
            themer.switch_tab('Styles')
            themer.click_in_tab('Spacing')
            themer.select_element(element, :css)
            themer.move_slider_to_percent(edge_sel, self.rand_width)
            expected_slider_spacing_size = themer.read_input(edge_sel)
            themer.publish_theme(theme_name)
          end
            
          res = ThemeBuilder.read_css_property(@browser, element, edge_prop)
          effective_spacing_size = /(.*)[a-z][a-z]/.match(res)[1]
          #"The effective spacing size on the HTML #{effective_spacing_size.inspect} (#{res.inspect})  and the size from the slider #{expected_slider_spacing_size.inspect} for #{spacing} and edge #{edge} do not match for element #{element}.")
          expected_slider_spacing_size.should == effective_spacing_size
        end
      end
    end
  end
  
 
  
  def corner_width_slider_test(save_type = nil, theme_name = 'testtheme', new_theme = 'impact')
    theme_name = "#{theme_name}_#{Time.now.to_i}" if theme_name == "testtheme"
    Log.logger.info("Starting corner slider #{save_type} test")
    login($config['user_accounts']['qatestuser']['user'], $config['user_accounts']['qatestuser']['password'])    
    case save_type
    when "save"
      corner_width_slider_save_test(theme_name)
    when "publish"
      corner_width_slider_publish_test(theme_name, new_theme)
    when nil
      corner_width_slider_plain_test()
    end  
  end
  
  
  
  def corner_width_slider_plain_test()
    spacings = ['margin', 'padding', 'border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    elements.each do |element|
      spacings.each do |spacing|
        Log.logger.info("Testing corner slider for #{spacing} of #{element}")
        ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
          themer.switch_tab('Styles')
          themer.click_in_tab('Spacing')
          themer.select_element(element, :css)
          themer.move_slider_to_percent(spacing, self.rand_width)
          # Doing some hard coding for edge_sel, as it doesn't affect our final test motive, to make sure all four edge values are equal.
          edge_sel = "#{spacing}-top#{'-width' if spacing == 'border'}".gsub("NIL","")
          slider_spacing_size = themer.read_input(edge_sel)          
          edges.each do |edge|
            edge_prop = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
            res = themer.read_css_property(element, edge_prop)
            effective_spacing_size = /(.*)[a-z][a-z]/.match(res)[1]
            #"The effective spacing size on the HTML #{effective_spacing_size} and the size from the slider #{slider_spacing_size} for #{spacing} and edge #{edge} do not match for element #{element}")
            slider_spacing_size.should == effective_spacing_size
          end
        end
      end
    end
  end
  
  
  def corner_width_slider_save_test(theme_name = 'testtheme')
    spacings = ['margin', 'padding', 'border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    elements.each do |element|
      spacings.each do |spacing|
        Log.logger.info("Testing corner slider for #{spacing} of #{element}")
        ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
          themer.switch_tab('Styles')
          themer.click_in_tab('Spacing')
          themer.select_element(element, :css)
          themer.move_slider_to_percent(spacing, self.rand_width)
          # Doing some hard coding for edge_sel, as it doesn't affect our final test motive, to make sure all four edge values are equal.
          edge_sel = "#{spacing}-top#{'-width' if spacing == 'border'}".gsub("NIL","")
          slider_spacing_size = themer.read_input(edge_sel)
          themer.save_theme_as(theme_name)
          themer.open_theme('My Themes', theme_name)          
          edges.each do |edge|
            edge_prop = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
            res = themer.read_css_property(element, edge_prop)
            spacing_matches = /(.*)[a-z][a-z]/.match(res)
            if spacing_matches
              effective_spacing_size = spacing_matches[1]
            else
              raise "Can't detect spacing size for element #{edge_prop.inspect}. reading css resulted in: #{res.inspect}"
            end
            
            #"The effective spacing size on the HTML #{effective_spacing_size} (#{res.inspect}) and the size from the slider #{slider_spacing_size} for #{spacing} and edge #{edge} do not match for element #{element}")
            slider_spacing_size.should == effective_spacing_size
          end
        end
      end
    end
  end
  
  # helper test for corner sliders.
  # save_type = nil (no save), save (save and reopen the saved theme), publish (publish and exit TB)
  def corner_width_slider_publish_test(theme_name, new_theme)
    spacings = ['margin', 'padding', 'border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    elements.each do |element|
      spacings.each do |spacing|
        expected_slider_spacing_size = nil
        ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
          Log.logger.info("Testing corner slider for #{spacing} of #{element}")
          themer.open_theme('Gardens', new_theme)
          themer.switch_tab('Styles')
          themer.click_in_tab('Spacing')
          themer.select_element(element, :css)
          themer.move_slider_to_percent(spacing, self.rand_width)
          # Doing some hard coding for edge_sel, as it doesn't affect our final test motive, to make sure all four edge values are equal.
          edge_sel = "#{spacing}-top#{'-width' if spacing == 'border'}".gsub("NIL","")
          expected_slider_spacing_size = themer.read_input(edge_sel)
          themer.publish_theme(theme_name)
        end
        edges.each do |edge|
          edge_prop = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
          res = ThemeBuilder.read_css_property(@browser, element, edge_prop)
          effective_spacing_size = /(.*)[a-z][a-z]/.match(res)[1]
          #"The effective spacing size on the HTML #{effective_spacing_size} and the size from the slider #{expected_slider_spacing_size} for #{spacing} and edge #{edge} do not match for element #{element}")
          expected_slider_spacing_size.should == effective_spacing_size
        end
      end
    end
  end
    
  # helper for the multi edge tests since each test does almost exactly the same thing
  # save_type = nil (no save), save (save and reopen the saved theme), publish (publish and exit TB)
  def multi_edge_entry_test(save_type = nil, theme_name = 'testtheme', new_theme = 'sonoma')
    theme_name = "#{theme_name}_#{Time.now.to_i}" if theme_name == "testtheme"
    Log.logger.info("Starting edge data entry #{save_type} test")
    login($config['user_accounts']['qatestuser']['user'], $config['user_accounts']['qatestuser']['password'])
    case save_type
    when "save"
      multi_edge_entry_save_test(theme_name)
    when "publish"
      multi_edge_entry_publish_test(theme_name, new_theme)
    when nil
      multi_edge_entry_plain_test()
    end  
  end
  
  def multi_edge_entry_plain_test()
    spacings = ['margin', 'padding', 'border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    elements.each do |element|
      ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
        themer.switch_tab('Styles')
        themer.click_in_tab('Spacing')
        themer.select_element(element, :css)
        css_spacings = Hash.new
        spacings.each do |spacing|
          edges.each do |edge|
            edge_sel = edge_prop = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
            type_width = (self.rand_width*100).to_i
            themer.type_text(edge_sel, type_width.to_s)
            set_spacing_size = themer.read_input(edge_sel)
            css_spacings[edge_prop] = set_spacing_size
          end
        end
        css_spacings.each_pair{|prop, set_size|
          res = themer.read_css_property(element, prop)
          effective_spacing_size = /(.*)[a-z][a-z]/.match(res)[1]
          #"The effective spacing size on the HTML #{effective_spacing_size} (#{res.inspect}) and the size from the entry #{set_size} do not match for element #{element}")
          set_size.should == effective_spacing_size
        }
      end
    end
  end
  
  def multi_edge_entry_save_test(theme_name)
    spacings = ['margin', 'padding', 'border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    elements.each do |element|
      ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
        themer.switch_tab('Styles')
        themer.click_in_tab('Spacing')
        themer.select_element(element, :css)
        css_spacings = Hash.new
        spacings.each do |spacing|
          edges.each do |edge|
            edge_sel = edge_prop = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
            type_width = (self.rand_width*100).to_i
            themer.type_text(edge_sel, type_width.to_s)
            set_spacing_size = themer.read_input(edge_sel)
            css_spacings[edge_prop] = set_spacing_size
          end
        end
        themer.save_theme_as(theme_name)
        themer.open_theme('My Themes', theme_name)
        css_spacings.each_pair{|prop, set_size|
          res = themer.read_css_property(element, prop)
          effective_spacing_size = /(.*)[a-z][a-z]/.match(res)[1]
          #"The effective spacing size on the HTML #{effective_spacing_size} and the size from the entry #{set_size} do not match for element #{element}")
          set_size.should == effective_spacing_size
        }
      end
    end
  end
  
  
  # helper for the multi edge tests since each test does almost exactly the same thing
  # save_type = nil (no save), save (save and reopen the saved theme), publish (publish and exit TB)
  def multi_edge_entry_publish_test(theme_name, new_theme)
    spacings = ['margin', 'padding', 'border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    elements.each do |element|
      css_spacings = Hash.new
      ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
        themer.open_theme('Gardens', new_theme)
        themer.switch_tab('Styles')
        themer.click_in_tab('Spacing')
        themer.select_element(element, :css)
        spacings.each do |spacing|
          edges.each do |edge|
            edge_sel = edge_prop = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
            type_width = (self.rand_width*100).to_i
            themer.type_text(edge_sel, type_width.to_s)
            set_spacing_size = themer.read_input(edge_sel)
            css_spacings[edge_prop] = set_spacing_size
          end
        end
        themer.publish_theme(theme_name)
      end
      css_spacings.each_pair{|prop, set_size|
        res = ThemeBuilder.read_css_property(@browser, element, prop)
        effective_spacing_size = /(.*)[a-z][a-z]/.match(res)[1]
        #, "The effective spacing size on the HTML #{effective_spacing_size} and the size from the entry #{set_size} do not match for element #{element}")
        set_size.should == effective_spacing_size
      }
    end
  end
  
  
  # helper for the border color tests since each test does almost exactly the same thing
  # save_type = nil (no save), save (save and reopen the saved theme), publish (publish and exit TB)
  def border_color_test(save_type = nil, theme_name = 'testtheme', new_theme = 'broadway')
    theme_name = "#{theme_name}_#{Time.now.to_i}" if theme_name == "testtheme"
    Log.logger.info("Starting border color test with save_type: #{save_type.inspect}")
    login($config['user_accounts']['qatestuser']['user'], $config['user_accounts']['qatestuser']['password'])
    case save_type
    when "save"
      border_color_save_test(theme_name)
    when "publish"
      border_color_publish_test(theme_name, new_theme)
    when nil
      border_color_plain_test()
    end  
  end
  
  
  def border_color_plain_test()
    spacings = ['border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    colors = ['a','b','c','d','e']
    elements.each do |element|
      spacings.each do |spacing|
        edges.each do |edge|
          ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
            Log.logger.info("Testing #{spacing} #{edge} for #{element}")
            edge_to_change = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
            themer.switch_tab('Styles')
            themer.click_in_tab('Spacing')
            themer.select_element(element, :css)
            themer.move_slider_to_percent(edge_to_change, self.rand_width)
            chosen_color = themer.pick_color_from_palette('BorderColor', colors[rand(colors.size)])
            edge_prop = "#{spacing}-#{edge}#{'-color' if spacing == 'border'}"
            effective_color = themer.read_css_property(element, edge_prop)
            #"The effective border color on the HTML #{effective_color} and the color from the theme builder #{chosen_color} for #{spacing} and edge #{edge} do not match for element #{element}")
            chosen_color.should == effective_color
          end
        end
      end
    end
  end
  
  
  def border_color_save_test(theme_name)
    spacings = ['border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    colors = ['a','b','c','d','e']
    elements.each do |element|
      spacings.each do |spacing|
        edges.each do |edge|
          ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
            Log.logger.info("Testing #{spacing} #{edge} for #{element}")
            edge_sel = spacing + '-' + edge
            edge_prop = "#{spacing}-#{edge}#{'-color' if spacing == 'border'}"
            edge_to_change = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
            themer.switch_tab('Styles')
            themer.click_in_tab('Spacing')
            themer.select_element(element, :css)
            themer.move_slider_to_percent(edge_to_change, self.rand_width)
            chosen_color = themer.pick_color_from_palette('BorderColor', colors[rand(colors.size)])
            themer.save_theme_as(theme_name)
            themer.open_theme('My Themes', theme_name)
            effective_color = themer.read_css_property(element, edge_prop)
            #"The effective border color on the HTML #{effective_color} and the color from the theme builder #{chosen_color} for #{spacing} and edge #{edge} do not match for element #{element}")
            chosen_color.should == effective_color
          end
        end
      end
    end
  end
  
  def border_color_publish_test(theme_name, new_theme)
    spacings = ['border']
    edges = ['top', 'bottom', 'left', 'right']
    elements = ['div#header-region']
    colors = ['a','b','c','d','e']
    elements.each do |element|
      spacings.each do |spacing|
        edges.each do |edge|
          chosen_color = nil
          ThemeBuilder.ensure_open_and_close_tb(@browser) do |themer|
            Log.logger.info("Testing #{spacing} #{edge} for #{element}")
            themer.open_theme('Gardens', new_theme)
            themer.switch_tab('Styles')
            themer.click_in_tab('Spacing')
            themer.select_element(element, :css)
            edge_to_change = "#{spacing}-#{edge}#{'-width' if spacing == 'border'}".gsub("NIL","")
            themer.move_slider_to_percent(edge_to_change, self.rand_width)
            chosen_color = themer.pick_color_from_palette('BorderColor', colors[rand(colors.size)])
            themer.publish_theme(theme_name)
          end
          edge_prop = "#{spacing}-#{edge}#{'-color' if spacing == 'border'}"
          effective_color = ThemeBuilder.read_css_property(@browser, element, edge_prop)
          #"The effective border color on the HTML #{effective_color} and the color from the theme builder #{chosen_color} for #{spacing} and edge #{edge} do not match for element #{element}")
          chosen_color.should == effective_color
        end
      end
    end
  end
    
  # returns a 2 sig fig 0.1 - 1.0 random value
  def rand_width
    srand()
    width = rand(90).to_f / 100 + 0.1
    return width
  end
end
