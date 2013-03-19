require 'rubygems'
require 'test/unit/assertions'
module Acquia

  module FieldsVerificationMethods
    include Test::Unit::Assertions

    def verify_master_up
      status = self.master_up?(self.master_fqhn)
      assert(status.passed, "The master is not totally up, the following seems to be a problem:\n" +
      status.get_fail_message)
    end

    def verify_servers_up(_site_info)
      Log.logger.info('Verifying servers up')
      all_servers = self.get_site_servers(_site_info)
      all_servers.each{|s|
        server_status = self.server_up?(s)
        assert(server_status.passed, "Server " + s + " does not appear to be up: #{server_status.get_fail_message}")
      }
    end

    def verify_puppet_config(_site_info)
      Log.logger.info('Verifying puppet up')
      all_servers = self.get_site_servers(_site_info)
      all_servers.each{|s|
        status = self.puppet_up?(s)
        assert(status.passed, "Server " + s + " does not appear to have puppet properly configured\n" + status.get_fail_message)
      }
    end

    def verify_webserver_config(_site_info)
      Log.logger.info('Verifying apache httpd up')
      web_servers = self.get_web_servers_external_fqhn(_site_info)
      web_servers.each{|s|
        status = self.webserver_up?(s)
        assert(status.passed, "Server " + s + " does not appear to have apache httpd properly configured\n" + status.get_fail_message)
      }
    end

    def verify_dbserver_config(_site_info)
      Log.logger.info('Verifying mysqld up')
      servers = self.get_db_servers_external_fqhn(_site_info)
      servers.each{|s|
        status = self.dbserver_up?(s)
        assert(status.passed, "Server " + s + " does not appear to have mysqld properly configured\n" + status.get_fail_message)
      }
    end

    def verify_svnserver_config(_site_info)
      Log.logger.info('Verifying svn up')
      servers = self.get_svn_servers(_site_info)
      servers.each{|s|
        status = self.svnserver_up?(s)
        assert(status.passed, "Server " + s + " does not appear to have svn properly configured\n" + status.get_fail_message)
      }
    end

    def verify_balancer_config(_site_info)
      Log.logger.info('Verifying balancer up')
      servers = self.get_balancers(_site_info)
      servers.each{|s|
        status = self.balancer_up?(s)
        assert(status.passed, "Server " + s + " does not appear to have nginx properly configured\n" + status.get_fail_message)
      }
      status = self.balancer_conf_ok?(_site_info)
      assert(status.passed, "Not all the webnodes in the site appear to be in the balancer config\n" + status.get_fail_message)
    end

    def verify_splunk_config(_site_info)
      Log.logger.info('Verifying splunk up')
      all_servers = self.get_site_servers(_site_info)
      all_servers.each{|s|
        status = self.splunk_up?(s)
        assert(status.passed, "Server " + s + " does not appear to have splunk properly configured\n" + status.get_fail_message)
      }
    end

    def verify_correct_netrc(_site_info)
      Log.logger.info('Verifying netrc is correct on all hosts')
      all_servers = self.get_site_servers(_site_info)
      all_servers.each do |s|
        status = self.correct_netrc?(s, (s =~ /^managed/) ? 3 : 1)
        assert(status.passed, "Server " + s + " should have a correct .netrc\n" + status.get_fail_message)
      end

    end

    def verify_all_site_config(site_info)
      verify_puppet_config(site_info)
      verify_webserver_config(site_info)
      verify_dbserver_config(site_info)
      verify_svnserver_config(site_info)
      verify_balancer_config(site_info)
      # we aren't running splunk right now.
#      verify_splunk_config(site_info)
      verify_correct_netrc(site_info)
    end
  end
end
