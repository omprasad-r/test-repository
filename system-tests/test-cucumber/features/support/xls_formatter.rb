require 'rubygems'
require 'erb'
require 'cucumber/formatter/ordered_xml_markup'
require 'cucumber/formatter/duration'
require 'cucumber/formatter/io'
require 'axlsx'

module Acquia
  module Formatter
    class XLS
      include ERB::Util # for the #h method
      include Cucumber::Formatter::Duration
      include Cucumber::Formatter::Io

      def initialize(step_mother, path, options)
        @io = ensure_file(path, "xls")
        @step_mother = step_mother
        @options = options
        @buffer = {}
        @p = create_axlsx(@io)
        @workbook = @p.workbook
        @feature_number = 0
        @scenario_number = 0
        @step_number = 0
        @header_red = nil
        @delayed_messages = []
        @img_id = 0
        @sheet_stats = {:tester_count => 20, :feature_rows => Array.new, :scenario_rows => Array.new}
      end

      def embed(src, mime_type, label)
        case(mime_type)
        when /^image\/(png|gif|jpg|jpeg)/
          embed_image(src, label)
        end
      end

      def embed_image(src, label)
        id = "img_#{@img_id}"
        @img_id += 1
        @builder.span(:class => 'embed') do |pre|
          pre << %{<a href="" onclick="img=document.getElementById('#{id}'); img.style.display = (img.style.display == 'none' ? 'block' : 'none');return false">#{label}</a><br>&nbsp;
          <img id="#{id}" style="display: none" src="#{src}"/>}
        end
      end


      def before_features(features)
        @step_count = get_step_count(features)
        @wb = @p.workbook
        @sheet = @wb.add_worksheet
        @sheet.add_row(Array.new(7) + ["Total"])
        @sheet.add_row(["Enterprise Gardener", "https://gardener.utest.acquia-sites.com"] + Array.new(5) + ["Pass"])
        @sheet.add_row(["SMB Gardener", "https://gardener.gsteamer.acquia-sites.com"] + Array.new(5) + ["Fail"])
        @sheet.add_row(["Total Steps", @step_count] + Array.new(5) + ["Not run"])
        testers = []
        @sheet_stats[:tester_count].times{ |i| testers.push("Tester #{i + 1}")}
        header_row = ["Functional Area", "Test Cases", "Test Steps", "Expected Result", "History", "", "", "Keep Empty"]
        header_row = header_row + testers
        @sheet.add_row(header_row, :style => feature_separator_style)
        @sheet.add_row()
        @sheet.add_row()
        @sheet.add_row(Array.new(4) + ["Last Pass", "Last Fail", "Time Run(ever)"])
        @sheet_stats[:header] = {:last_row => @sheet.rows.last.index}
      end

      def after_features(features)
#        @sheet.col_style(0, feature_column_style, 3)
        STDOUT.puts "In after features"
        print_stats(features)
        STDOUT.puts "Done with stats, printing worksheet"
        @sheet.col_style(0, feature_column_style)
        @p.serialize(@io.path)
      end

      def before_feature(feature)
        @exceptions = []
        feature_separator_style
        feature_column_style
#        @sheet.add_row(["This is empty and before feature"])
      end

      def after_feature(feature)
#        @sheet.add_row(["This is after feature"])
      end
  
      def before_comment(comment)
        # before comment
      end

      def after_comment(comment)
        # after comment
      end
  
      def comment_line(comment_line)
        # od not leave a comment at the mement
#        @sheet.add_row([comment_line])
      end
  
      def after_tags(tags)
        @tag_spacer = nil
      end
  
      def tag_name(tag_name)
        # do nto add tags
#        @sheet.add_row([tag_name])
      end
  
      def feature_name(keyword, name)
        STDOUT.puts "Adding a feature row that is styled."
        @sheet.add_row(Array.new(20), :style => feature_separator_style)
        @sheet_stats[:feature_rows].push(@sheet.rows.last.index)
        STDOUT.puts "Row index: #{@sheet.rows.last.index}"
#        sheet.[ri] feature_separator_style
        STDOUT.puts "Done Styling"

        lines = name.split(/\r?\n/)
        return if lines.empty?
        @sheet.add_row([lines[0] ])
        lines[1..-1].each do |line|
          @sheet.add_row([line.strip])
        end
      end
  
      def before_background(background)
        # @in_background = true
        # @builder << '<div class="background">'
      end
  
      def after_background(background)
        # @in_background = nil
        # @builder << '</div>'
      end
  
      def background_name(keyword, name, file_colon_line, source_indent)
        @listing_background = true
          @sheet.rows.last.add_cell(keyword)
      end

      def before_feature_element(feature_element)
        @scenario_number+=1
        @scenario_red = false
        css_class = {
          Cucumber::Ast::Scenario        => 'scenario',
          Cucumber::Ast::ScenarioOutline => 'scenario outline'
        }[feature_element.class]
#        @builder << "<div class='#{css_class}'>"
      end

      def after_feature_element(feature_element)
#        @builder << '</div>'
        @open_step_list = true
      end

      def scenario_name(keyword, name, file_colon_line, source_indent)
        @sheet.add_row(Array.new(1))
        @sheet_stats[:scenario_rows].push @sheet.rows.last
        @sheet.rows.last.add_cell(name)
        STDOUT.puts "There is a scenario on row #{@sheet.rows.last.index}"
      end
  
      def before_outline_table(outline_table)
#        @outline_row = 0
#        @builder << '<table>'
      end
  
      def after_outline_table(outline_table)
#        @builder << '</table>'
#        @outline_row = nil
      end
      
      def before_examples(examples)
#         @builder << '<div class="examples">'
      end
      
      def after_examples(examples)
#        @builder << '</div>'
      end

      def examples_name(keyword, name)
        @sheet.rows.last.add_cell("#{keyword}: #{name}", :b => true)
        @first_cell = true
      end
  
      def before_steps(steps)
#        @builder << '<ol>'
      end
  
      def after_steps(steps)
#        @builder << '</ol>'
      end

      def before_step(step)
        @step_id = step.dom_id
        @step_number += 1
        @step = step
      end

      def after_step(step)
        move_progress
      end

      def before_step_result(keyword, step_match, multiline_arg, status, exception, source_indent, background, file_colon_line)
        @step_match = step_match
        @hide_this_step = false
        if exception
          if @exceptions.include?(exception)
            @hide_this_step = true
            return
          end
          @exceptions << exception
        end
        if status != :failed && @in_background ^ background
          @hide_this_step = true
          return
        end
        @status = status
        return if @hide_this_step
        set_scenario_color(status)      
#        @builder << "<li id='#{@step_id}' class='step #{status}'>"            
      end

      def after_step_result(keyword, step_match, multiline_arg, status, exception, source_indent, background, file_colon_line)
#        return if @hide_this_step
#        # print snippet for undefined steps
#        if status == :undefined
#          step_multiline_class = @step.multiline_arg ? @step.multiline_arg.class : nil
#          @sheet.add_row([@step_mother.snippet_text(@step.actual_keyword,step_match.instance_variable_get("@name") || '',step_multiline_class)])
#        end
#        print_messages
      end

      def step_name(keyword, step_match, status, source_indent, background, file_colon_line)
        @step_matches ||= []
        background_in_scenario = background && !@listing_background
        @skip_step = @step_matches.index(step_match) || background_in_scenario
        @step_matches << step_match

        unless @skip_step
          build_step(keyword, step_match, status)
        end
      end

      def exception(exception, status)
        build_exception_detail(exception)
      end

      def extra_failure_content(file_colon_line)
        @snippet_extractor ||= SnippetExtractor.new
        "<pre class=\"ruby\"><code>#{@snippet_extractor.snippet(file_colon_line)}</code></pre>"
      end

      def before_multiline_arg(multiline_arg)
        return if @hide_this_step || @skip_step
        if Cucumber::Ast::Table === multiline_arg
#          @builder << '<table>'
        end
      end
  
      def after_multiline_arg(multiline_arg)
        return if @hide_this_step || @skip_step
        if Cucumber::Ast::Table === multiline_arg
#          @builder << '</table>'
        end
      end

      def doc_string(string)
#        return if @hide_this_step
#        @sheet.add_row(["in doc_string", string])
      end
  
  
      def before_table_row(table_row)
        @row_id = table_row.dom_id
        @col_index = 0
        return if @hide_this_step
#        @builder << "<tr class='step' id='#{@row_id}'>"
      end
  
      def after_table_row(table_row)
#        return if @hide_this_step
#        print_table_row_messages
#        if table_row.exception
#          @sheet.add_row(["failed", h(format_exception(table_row.exception)) ])
#          set_scenario_color_failed
#        end
#        if @outline_row
#          @outline_row += 1
#        end
#        @step_number += 1
#        move_progress
      end

      def table_cell_value(value, status)
        return if @hide_this_step
        
        @cell_type = @outline_row == 0 ? :th : :td
        attributes = {:id => "#{@row_id}_#{@col_index}", :class => 'step'}
        attributes[:class] += " #{status}" if status
        build_cell(@cell_type, value, attributes)
        set_scenario_color(status)
        @col_index += 1
      end

      def puts(message)
        @delayed_messages << message
      end
      
      def print_messages
#        return if @delayed_messages.empty?
#        @delayed_messages.each do |ann|
#          @sheet.add_row([ann])
#        end
#        empty_messages
      end
      
      def print_table_row_messages
#        return if @delayed_messages.empty?
#        
#        @sheet.add_row([@delayed_messages.join(", ")])
#        empty_messages
      end
      
      def empty_messages
        @delayed_messages = []
      end

      protected

      def build_exception_detail(exception)
#        backtrace = Array.new
#        message = exception.message
#        if defined?(RAILS_ROOT) && message.include?('Exception caught')
#          matches = message.match(/Showing <i>(.+)<\/i>(?:.+) #(\d+)/)
#          backtrace += ["#{RAILS_ROOT}/#{matches[1]}:#{matches[2]}"] if matches
#          matches = message.match(/<code>([^(\/)]+)<\//m)
#          message = matches ? matches[1] : ""
#        end
#
#        unless exception.instance_of?(RuntimeError)
#          message = "#{message} (#{exception.class})"
#        end
#
#        @sheet.add_row(["Exception Message", message])
#        backtrace = exception.backtrace
#        backtrace.delete_if { |x| x =~ /\/gems\/(cucumber|rspec)/ }
#        @sheet.add_row(["Backtrace", backtrace_line(backtrace.join("\n"))])
#        
#        extra = extra_failure_content(backtrace)
#        @sheet.add_row([extra]) unless extra == ""
      end

      def set_scenario_color(status)
        if status == :undefined or status == :pending
          set_scenario_color_pending
        end
        if status == :failed
          set_scenario_color_failed
        end
      end
      
      def set_scenario_color_failed
        STDOUT.puts "Setting Color failed"
#        @builder.script do
#          @builder.text!("makeRed('cucumber-header');") unless @header_red
#          @header_red = true
#          @builder.text!("makeRed('scenario_#{@scenario_number}');") unless @scenario_red
#          @scenario_red = true
#        end
      end
      
      def set_scenario_color_pending
        STDOUT.puts "Setting Color Pending"
#        @builder.script do
#          @builder.text!("makeYellow('cucumber-header');") unless @header_red
#          @builder.text!("makeYellow('scenario_#{@scenario_number}');") unless @scenario_red
#        end         
      end

      def get_step_count(features)
        count = 0
        features = features.instance_variable_get("@features")
        features.each do |feature|
          #get background steps
          if feature.instance_variable_get("@background")
            background = feature.instance_variable_get("@background")
            background.init
            background_steps = background.instance_variable_get("@steps").instance_variable_get("@steps")
            count += background_steps.size
          end
          #get scenarios
          feature.instance_variable_get("@feature_elements").each do |scenario|
            scenario.init
            #get steps
            steps = scenario.instance_variable_get("@steps").instance_variable_get("@steps")
            count += steps.size

            #get example table
            examples = scenario.instance_variable_get("@examples_array")
            unless examples.nil?
              examples.each do |example|
                example_matrix = example.instance_variable_get("@outline_table").instance_variable_get("@cell_matrix")
                count += example_matrix.size
              end
            end

            #get multiline step tables
            steps.each do |step|
              multi_arg = step.instance_variable_get("@multiline_arg")
              next if multi_arg.nil?
              matrix = multi_arg.instance_variable_get("@cell_matrix")
              count += matrix.size unless matrix.nil?
            end
          end
        end
        return count
      end

      def build_step(keyword, step_match, status)
        step_name = step_match.format_args(lambda{|param| %{#{param}}})
          @sheet.rows.last.add_cell( "#{keyword} #{step_name}")
          @sheet.add_row(Array.new(2))
#        if keyword  =~ /Then/
#          @sheet.rows[@sheet_stats[:scenario_rows].last.index].add_cell("#{keyword} #{step_name}")
#          @sheet.add_row(Array.new(2))
#        else
#          @sheet.add_row(Array.new(2))
#        end
#        step_file = step_match.file_colon_line
#        step_file.gsub(/^([^:]*\.rb):(\d*)/) do
#          if ENV['TM_PROJECT_DIRECTORY']
#            step_file = "<a href=\"txmt://open?url=file://#{File.expand_path($1)}&line=#{$2}\">#{$1}:#{$2}</a> "
#          end
#        end
#        @sheet.add_row([step_file])
      end

      def build_cell(cell_type, value, attributes)
        cell_val = value
        cell_val = '<' + value + '>' if @first_cell
        @sheet.add_row(Array.new(2) + [cell_val])
        @first_cell = false
      end

      def move_progress
#        @builder << " <script type=\"text/javascript\">moveProgressBar('#{percent_done}');</script>"
      end

      def percent_done
        result = 100.0
        if @step_count != 0
          result = ((@step_number).to_f / @step_count.to_f * 1000).to_i / 10.0
        end
        result
      end

      def format_exception(exception)
        (["#{exception.message}"] + exception.backtrace).join("\n")
      end

      def backtrace_line(line)
        line.gsub(/\A([^:]*\.(?:rb|feature|haml)):(\d*).*\z/) do
          if ENV['TM_PROJECT_DIRECTORY']
            "<a href=\"txmt://open?url=file://#{File.expand_path($1)}&line=#{$2}\">#{$1}:#{$2}</a> "
          else
            line
          end
        end
      end

      def print_stats(features)
        STDOUT.puts("In print stats")
        @sheet.add_row(["Finished in " + format_duration(features.duration) + "seconds"])
        STDOUT.puts("Done formatting duration")
        @sheet.add_row(["Totals " + print_stat_string(features)])
      end

      def print_stat_string(features)
        string = String.new
        string << dump_count(@step_mother.scenarios.length, "scenario")
        scenario_count = print_status_counts{|status| @step_mother.scenarios(status)}
        string << scenario_count if scenario_count
        string << "\n"
        string << dump_count(@step_mother.steps.length, "step")
        step_count = print_status_counts{|status| @step_mother.steps(status)}
        string << step_count if step_count
      end

      def print_status_counts
        counts = [:failed, :skipped, :undefined, :pending, :passed].map do |status|
          elements = yield status
          elements.any? ? "#{elements.length} #{status.to_s}" : nil
        end.compact
        return " (#{counts.join(', ')})" if counts.any?
      end

      def dump_count(count, what, state=nil)
        [count, state, "#{what}#{count == 1 ? '' : 's'}"].compact.join(" ")
      end

      def create_axlsx(io)
        Axlsx::Package.new
#        Cucumber::Formatter::OrderedXmlMarkup.new(:target => io, :indent => 0)
      end

      def example_header_style
        unless @example_header_style
          @wb.styles do |s|
            @example_header_style = s.add_style :b => true
          end
        end
      end

      def feature_separator_style
        unless @feature_separator_style
          @wb.styles do |s|
            STDOUT.puts "Adding style to #{s.to_s}"
            @feature_separator_style = s.add_style :bg_color => ACQUIA_BLUE, :b => true
          end
        end
        STDOUT.puts "using #{@feature_separator_style.to_s}"
        @feature_separator_style
      end

      def feature_column_style
        unless @feature_column_style
          @wb.styles do |s|
            STDOUT.puts "Adding style to #{s.to_s}"
            @feature_column_style = s.add_style :bg_color => ACQUIA_BLUE, :b => true
          end
        end
        STDOUT.puts "using #{@feature_column_style.to_s}"
        @feature_column_style
      end

      # colors
      ACQUIA_BLUE = "99ccff"
      RED = "ff0000"
      GREEN = "00ff00"
      BLUE = "0000ff"

      class SnippetExtractor #:nodoc:
        class NullConverter; def convert(code, pre); code; end; end #:nodoc:
        begin; require 'syntax/convertors/html'; @@converter = Syntax::Convertors::HTML.for_syntax "ruby"; rescue LoadError => e; @@converter = NullConverter.new; end

        def snippet(error)
          raw_code, line = snippet_for(error[0])
          highlighted = @@converter.convert(raw_code, false)
          highlighted << "\n<span class=\"comment\"># gem install syntax to get syntax highlighting</span>" if @@converter.is_a?(NullConverter)
          post_process(highlighted, line)
        end

        def snippet_for(error_line)
          if error_line =~ /(.*):(\d+)/
            file = $1
            line = $2.to_i
            [lines_around(file, line), line]
          else
            ["# Couldn't get snippet for #{error_line}", 1]
          end
        end

        def lines_around(file, line)
          if File.file?(file)
            lines = File.open(file).read.split("\n")
            min = [0, line-3].max
            max = [line+1, lines.length-1].min
            selected_lines = []
            selected_lines.join("\n")
            lines[min..max].join("\n")
          else
            "# Couldn't get snippet for #{file}"
          end
        end

        def post_process(highlighted, offending_line)
          new_lines = []
          highlighted.split("\n").each_with_index do |line, i|
            new_line = "<span class=\"linenum\">#{offending_line+i-2}</span>#{line}"
            new_line = "<span class=\"offending\">#{new_line}</span>" if i == 2
            new_lines << new_line
          end
          new_lines.join("\n")
        end

      end
    end
  end
end
