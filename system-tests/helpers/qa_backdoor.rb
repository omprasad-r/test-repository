require 'rubygems'
require 'bundler/setup'
require 'excon'
require 'uri'

class QaBackdoor

  def initialize(base_url, options = {})
    @options = options
    @base_url = base_url + "/qa_reset.php?operation="
  end

  def create_snapshot(snapshot_name = "default")
    get_and_log(@base_url + "create_snapshot&snapshot_name=#{snapshot_name}")
  end

  def restore_snapshot(snapshot_name = "default")
    get_and_log(@base_url + "restore_snapshot&snapshot_name=#{snapshot_name}")
  end

  def cleanup_installation
    get_and_log(@base_url + "cleanup_installation")
  end

  def create_user(username, password)
    username = URI.escape(username)
    password = URI.escape(password)
    get_and_log(@base_url + "create_user&username=#{username}&password=#{password}")
  end

  def add_user_role(username, role)
    role =  URI.escape(role)
    get_and_log(@base_url + "add_user_role&username=#{username}&role=#{role}")
  end

  def remove_user_role(username, role)
    role =  URI.escape(role)
    get_and_log(@base_url + "remove_user_role&username=#{username}&role=#{role}")
  end

  def role_add_permission(role, permission)
    role =  URI.escape(role)
    permission =  URI.escape(permission)
    get_and_log(@base_url + "role_add_permission&role=#{role}&permission=#{permission}")
  end

  def role_remove_permission(role, permission)
    role =  URI.escape(role)
    permission =  URI.escape(permission)
    get_and_log(@base_url + "role_remove_permission&role=#{role}&permission=#{permission}")
  end

  def download_module(module_name)
    get_and_log(@base_url + "download_module&module_name=#{module_name}")
  end

  def enable_module(module_name)
    get_and_log(@base_url + "enable_module&module_name=#{module_name}")
  end

  def disable_module(module_name)
    get_and_log(@base_url + "disable_module&module_name=#{module_name}")
  end

  def install_module(module_name)
    get_and_log(@base_url + "install_module&module_name=#{module_name}")
  end

  def uninstall_module(module_name)
    get_and_log(@base_url + "uninstall_module&module_name=#{module_name}")
  end

  def list_directory_content(directory_name)
    get_and_log(@base_url + "list_directory_content&directory_name=#{directory_name}").split("\n")
  end

  def list_file_content(file_name)
    get_and_log(@base_url + "list_file_content&file_name=#{file_name}")
  end

  def inject_file(file_from, file_to)
    get_and_log(@base_url + "inject_file&file_from=#{file_from}&file_to=#{file_to}")
  end

  private

  def get_and_log(url)
    logger.debug("Requesting URL #{url}")
    response = Excon.get(url)
    raise "Error: #{response.body}" if (response.body.include?("Exception") && response.body.include?("(line "))

    unless response.status == 200
      logger.error(response.body)
      raise "Error connecting to #{url}: #{response.status}"
    end
    logger.debug(response.inspect)

    response.body
  end

  def post_file_and_log(url, file_path)
    response = Excon.post(url, :body => IO.read(file_path))
    raise "Error: #{response.body}" if (response.body.include?("Exception") && response.body.include?("(line "))
    logger.error(response.body) unless response.status == 200
    logger.debug(response.inspect)
    response.body
  end

  def logger
    @options[:logger] || NilLogger
  end

  class NilLogger
    def self.method_missing(*stuff)
    end
  end

end
