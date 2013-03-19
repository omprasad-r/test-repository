require 'yaml'

class ModuleManagerCapy
  #To deal with the cpapybara 1.0 vs 0.4 changes
  begin
    include Capybara::DSL
  rescue NameError
    include Capybara
  end

  def initialize
    module_file = File.dirname(File.expand_path(__FILE__)) + '/module_list.yml'
    @modules = YAML.load_file(module_file)
  end

  def get_blacklist
    @modules['blacklist']
  end

  def get_whitelist
    @modules['whitelist']
  end

  def get_approved_module_groups
    @modules['modulegroups'].map { |mod| mod.strip.downcase }
  end

  def get_modules(options = {})
    visit_modules_page
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
      ["Devel node access", "Devel", "Devel generate"].each {|mod| found_modules.delete(mod)}
    end

    found_modules
  end

  def get_modules_types
    visit_modules_page
    found_module_types = []
    all(:css, "a.fieldset-title").each do |module_type|
      found_module_types << module_type.text.downcase.gsub('hide','').capitalize
    end
    found_module_types.map { |mod| mod.strip.downcase }
  end

  def switch_module_state(module_name)
    visit_modules_page

    find("form#system-modules").all(:css, "tbody tr").each do |table_row|
      mod_name = table_row.find(:css, "label").text
      if mod_name == module_name
        mod_label_for_attribute = table_row.find(:css, "label")['for']
        mod_enabled = table_row.find(:css, "input[type='checkbox']").checked?
        #different in akephalos vs selenium
        mod_clickable = ["false", nil].include?(table_row.find(:css, "input[type='checkbox']")[:disabled])
        raise "Module #{module_name} can't be checked/unchecked, it is disabled" unless mod_clickable

        if mod_enabled
          Log.logger.info("Unchecking module: #{module_name} (#{mod_label_for_attribute.inspect}).")
          page.uncheck(mod_label_for_attribute)
        elsif !mod_enabled
          Log.logger.info("Checking module: #{module_name} (#{mod_label_for_attribute.inspect}).")
          page.check(mod_label_for_attribute)
        end

      end
    end

    submit_module_changes
  end

  private

  def visit_modules_page
    visit("/admin/modules") unless page.current_path.to_s.include?("/admin/modules")
  end

  def submit_module_changes
    page.click_button("edit-submit")
    if page.has_content?("Some required modules must be enabled")
      Log.logger.info("Apparently there are dependencies, clicking continue")
      page.click_button("Continue")
    end
    raise "Couldn't find confirmation after submitting module change" unless page.has_content?('The configuration options have been saved.')
  end


end
