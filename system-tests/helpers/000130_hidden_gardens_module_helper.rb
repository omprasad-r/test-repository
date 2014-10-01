module Test000130HiddenGardensModuleHelper

  def user_mod_types
    module_file = File.dirname(File.expand_path(__FILE__)) + '/module_list.yaml'
    @modules = YAML.load_file(module_file)
    @modules['modulegroups']
  end

  #These modules should show up
  def whitelist
    module_file = File.dirname(File.expand_path(__FILE__)) + '/module_list.yaml'
    @modules = YAML.load_file(module_file)
    @modules['whitelist']
  end

  #Things on the blacklist shouldn't show up (e.g. scarecrowed)
  def blacklist
    module_file = File.dirname(File.expand_path(__FILE__)) + '/module_list.yaml'
    @modules = YAML.load_file(module_file)
    @modules['blacklist']
  end

end
