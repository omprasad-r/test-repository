require 'rubygems'
require 'bundler/setup'
require 'test/unit'
require 'logger'
require 'net/ssh'
require 'net/scp'
require 'uri'
require '../helpers/gardens_automation.rb'

class Test00GardensCoderReview < Test::Unit::TestCase
  include GardensAutomation::GardensHeadlessTestCase

  KEY_PATH = "../instance-setup-fog/keys/gardens_qa_id_rsa"

  def initialize(test_method_name)
    super(test_method_name)
    @log = Logger.new(STDOUT)
  end

  def test_00_run_coder_review
    parsed_sut_url = URI.parse($config["sut_url"])
    test_host = parsed_sut_url.host

    remote_output_file =  '/tmp/checkstyle-result.xml'
    local_output_file = "#{ENV['WORKSPACE']}/checkstyle-result.xml"

    @log.info("Running coder-review on host #{test_host.inspect}.")

    begin
      Net::SSH.start(test_host, 'ubuntu', :keys => [ KEY_PATH ]) do |session|
        @log.info session.exec!(
          "sudo -u www-data drush --root=/var/www --uri='#{$config["sut_url"]}' coder-review checkstyle > #{remote_output_file}"
        )
      end

      Net::SCP.start(test_host, 'ubuntu', :keys => [ KEY_PATH ]) do |session|
        session.download!(remote_output_file, local_output_file)
      end
    rescue Exception => message
      raise "Could not successfully run coder-review. #{message}\n #{message.backtrace}"
    end

    raise 'Output of coder review has not been created' unless File.exist?(local_output_file)
  end

end
