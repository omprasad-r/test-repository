require 'rubygems'
require "bundler/setup"
require 'rake/testtask'
require 'test/unit'
require 'net/ssh'
require 'net/scp'
require 'json'
require 'helper/00_simpletest_helper'
require 'pp'

class Test00GardensSimpleTest < Test::Unit::TestCase
  include Test00SimpletestHelper
  include Rake::DSL

  def setup
    host = ENV['SUT_URL'] || "#{ENV['SUBDOMAIN']}.gardensqa.acquia-sites.com"
    @host = host.gsub('http://','').chomp('/')
    @key_path = "../instance-setup-fog/keys/gardens_qa_id_rsa"

    @log = Logger.new(STDOUT)
    @log.level = Logger::INFO
  end

  def teardown
    ssh(@host, @key_path) do |session|
      @log.info "Cleaning up ..."
      session.exec!("sudo rm /var/www/*.xml")
      session.exec!("sudo rm /var/www/*.stderr")
    end
  end

  def test_00_run_simpletest
    start_time = Time.now
    tests = get_test_list
    @log.info "Found #{tests.length} tests:\n #{tests.inspect}"

    tests.each { |test| exec_test(test) }
    collect_test_output

    duration = Time.now - start_time
    @log.info "All tests took #{(duration / 60).to_s} minutes"
  end

  def get_test_list
    @log.info "Getting test names"
    stderr = ""

    ssh(@host, @key_path) do |session|
      session.exec!(
        # list all tests
        "sudo -u www-data drush --root=/var/www --uri='http://#{@host}/' test-run --backend > /home/ubuntu/test_list.txt && sleep 10"
      ) do |channel, type, data|
        stderr << data if type == :stderr
      end
    end

    scp(@host, @key_path) do |session|
      session.download!("/home/ubuntu/test_list.txt", "./")
    end

    begin
      stdout = IO.read("./test_list.txt")
      unparsed_json = stdout.match(/^DRUSH_BACKEND_OUTPUT_START>>>(.+)<<<DRUSH_BACKEND_OUTPUT_END/)[1]
      out_json = JSON.parse(unparsed_json)
    rescue => ex
      @log.info "_____________________________"
      @log.info "Standard output:\n #{stdout} \n"
      @log.info "_____________________________"
      @log.info "Standard error:\n #{stderr} \n"
      @log.info "_____________________________"
      @log.info "Current environment:\n"
      pp ENV
      @log.info "_____________________________"

      raise ex
    end

    # grep only single tests no groups and extract the test names
    tests = out_json['output'].split("\n").grep(/^[\s]{3}([\w])/).map do |test|
      test.strip.split(/\s/).first
    end

    tests
  end

  def exec_test(test_name)
    @log.info "_____________________________"
    @log.info "Running test: #{test_name}"

    if TEST_BLACKLIST.include?(test_name)
      @log.info "Found test #{test_name} in blacklist ... bailing out"
    else
      start_time = Time.now
      ssh(@host, @key_path) do |session|
        @log.info session.exec!(
          "sudo -u www-data drush \
            --root=/var/www test-run #{test_name} \
            --uri='http://#{@host}/' \
            --xml 2>&1 | sudo -u www-data tee /var/www/#{test_name}.stderr"
        )
      end
      duration = Time.now - start_time
      @log.info "Finished test: #{test_name}. Duration: #{duration} secs"
    end
  end

  def collect_test_output
    @log.info "Collecting test results"

    ssh(@host, @key_path) do |session|
      @log.info "tar'ing up"
      @log.info session.exec!("mkdir -p ~/junit_reports")
      @log.info session.exec!("cp /var/www/*.xml ~/junit_reports")
      @log.info session.exec!("cp /var/www/*.stderr ~/junit_reports")
      @log.info session.exec!("tar -C ~/ -czf ~/junit_reports.tar.gz junit_reports")
    end

    @log.info "Downloading test results"
    scp(@host, @key_path) do |session|
      session.download!("/home/ubuntu/junit_reports.tar.gz", "./")
    end

    @log.info "Extracting test results"
    sh "tar xzf ./junit_reports.tar.gz"
  end

end
