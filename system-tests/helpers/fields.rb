$LOAD_PATH << File.dirname(__FILE__)
require 'rubygems'
require 'resolv'
require 'json'
require 'net/ssh'
require 'test/unit'
require 'xmlrpc/client'
require 'acquia_qa/os'
require 'acquia_qa/log'
require 'acquia_qa/ssh'
require 'acquia_qa/configuration'
require 'fields/verifications'
require 'fields/status'
require 'fields/exception'

require 'net/http'

#We might not want to use the monkeypatches every time.
if $do_not_monkeypatch_http_lib or ENV.key?('do_not_monkeypatch_http_lib')
  Log.logger.debug "Ignoring the HTTP monkeypatches in fields.rb"
else
  Log.logger.debug "Adding monkeypatches to Net:HTTP (fields.rb)"
  module Net
    class HTTP
      # Net::HTTP.connect raises getaddrinfo failures for servers whose
      # DNS entry we have have already confirmed resolves
      # correctly. Test to see if the problem is different DNS servers
      # returning different results.
      alias_method :real_connect, :connect
      def connect
        begin
          real_connect
        rescue SocketError => e
          if (e.message =~ /^getaddrinfo:/)
            Log.logger.info("getaddrinfo error for #{conn_address()}")
            [1, 2, 3, 4].each {|i|
              # TODO: We should just delete this whole error check.  But for
              # now, poll each of the Dynect authority servers to see if each of
              # the servers have the same A record.
              cmd = "dig @ns#{i}.p07.dynect.net #{conn_address()}"
              dig = `#{cmd}`
              Log.logger.info("#{cmd}:\n#{dig}")
            }
          end
          e2 = SocketError.new("Net::HTTP.connect(#{conn_address()}, #{conn_port()}): #{e.message}")
          e2.set_backtrace(e.backtrace)
          raise e2
        end
      end
    end

    # Net::HTTP's read_status_line calls BufferedIO::readline calls
    # BufferedIO::readuntil("\n") without an option to pass readuntil's ignore_eof
    # option. readuntil calls rbuf_fill which calls sysread which will raise
    # an EOFError if it sees eof before the newline. This is less useful than
    # read_status_line's normal HTTPBadResponse error which actually shows the
    # invalid status line. So, change read_status_line to use readuntil with
    # ignore_eof.
    # We will additionally add the details for the Socket of the failed connection 
    # to our Exception. The peeraddr call returns something like this: 
    # ["AF_INET", 443, "www-ilg.verisign.net", "69.58.181.89"]
  
    class << HTTPResponse
      def read_status_line(sock)
        str = sock.readuntil("\n", true).chop # ignore_eof
        m = /\AHTTP(?:\/(\d+\.\d+))?\s+(\d\d\d)\s*(.*)\z/in.match(str) or
          raise HTTPBadResponse, "wrong status line: #{str.dump} | #{sock.io.peeraddr.inspect}"
        m.captures
      end
    end 
  end
end

# Ruby 1.8.6 has a bug in which raising Resolv::ResolvTimeout triggers an
# error "wrong number of arguments (0 for 1)" because class Interrupt requires
# a message. It is fixed in 1.8.7 which we aren't using yet. This works around
# it. See http://redmine.ruby-lang.org/repositories/revision/ruby-186?rev=28029.
if RUBY_VERSION == "1.8.6"
  class Resolv::ResolvTimeout
    def initialize
      super("timeout in resolv.rb")
    end
  end
end

class Fields
  # Acquia::SSH defines methods for connecting to remote systems:
  #
  # ssh_long_session, ssh, ssh_connect, ssh_run (deprecated), scp,
  #   long_session_params, secure_params
  include Acquia::SSH

  # Acquia::Config defines methods for reading YAML structs or files,
  #  and for dumping variables in YAML format.
  #
  # write_inputs, write_inputs_from_hash, read_inputs
  #
  include Acquia::Config

  # OS (part of itlib) defines methods for executing commands on the
  #  local system:
  #
  # OS.new_instance, run, run2, run_quietly, attempt, watch
  include OS

  # Methods for verifying the status of various pieces of test infrastructure
  include Acquia::FieldsVerificationMethods

  SERVER_STATUS_NORMAL = 0
  SERVER_STATUS_ALLOCATED = 1
  SERVER_STATUS_KILLED = 3
  SITE_WEB_ACTIVE = 1000
  SITE_WEB_INACTIVE = 1001
  SITE_WEB_DEPLOY = 1002
  SERVER_SERVICE_STATUS_INACTIVE = 1
  SERVER_SERVICE_STATUS_ACTIVE = 2

  def initialize(stage=nil)
    if (stage)
      fields_stage(stage)
    end
  end

  # Read the given _conf_file and use Acquia::Config#read_inputs to
  # add new methods to (or redefine old methods on) this object, one
  # for each of the key names in the file.
  #
  # Methods which are likely to be (re?)defined by the test_set.yaml
  # file in Hosting include:
  #
  # site
  # site_type
  # fields_stage_id
  # fields_root
  # master_credentials
  # jmeter_label
  # region
  # availability_zone
  # instance_type
  # security_groups
  # is_ubuntu
  #
  # Obviously, if someone adds a key to that file, or mistypes the
  # name of a key, or loads a different file, this list might be
  # different. If someone loads a file containing a key named
  # "instance_size", it will override the existing instance_size
  # method.
  def set_fields_env(_conf_file)
    read_inputs(_conf_file)
    self.set_environment_variables
  end

  # This is total insanity.  But by moving this to its own method, at least we don't
  # have to pass in a configuration file.  Calling classes could potentially set the
  # variables from the outside and then call this.  It's really ugly either way.
  #MARC: About to deprecate this
  def set_environment_variables
    ENV['site'] = site
    ENV['FIELDS_STAGE'] = fields_stage
    Log.logger.debug('fields_stage: ' + ENV['FIELDS_STAGE'])
  end

  # The master server's external fully qualified domain name.
  def master_fqhn
    "master.e.#{self.fields_stage}.f.e2a.us"
  end

  # The puppetmaster's internal fully-qualified domain name.
  def puppetmaster_fqhn
    "master.i.#{self.fields_stage}.f.e2a.us"
  end

  # The parent domain where public domain names for sites are kept.
  def sites_domain
    "acquia-sites.com"
  end

  # Return the stage in which the tests are being run. This will be
  # given by the FIELDS_STAGE environment variable, except:
  #
  # -- If the stage argument is non-nil, set the FIELDS_STAGE
  # environment variable to stage and return it.
  #
  # -- If FIELDS_STAGE is empty, set it based on the fields_stage_id from
  # the configuration file plus the name of the current user (given by
  # ENV['USER']) and return the result.
  def fields_stage(stage=nil)
    unless (stage)
      #Wisdom of Nik:
      # well that plan was that you cannot launch something that had a stage of ''
      # so at the min you would get gardener.mseeger.acquia-sites.com
      # I think the stage Id at one point was the account
      stage  = ENV['FIELDS_STAGE'] || "#{ENV['USER']}-forgot-to-set-the-fields-id"
    end
    ENV['FIELDS_STAGE'] = stage
    return stage
  end

  # The size of instance to launch. If defined in the test_set.yaml,
  # use that instance type, otherwise default to small.
  def instance_size
    inst_s = nil
    begin
      if self.instance_type
        inst_s = self.instance_type
      else
        inst_s = 'm1.small'
      end
      Log.logger.debug("Instances of type #{self.instance_type} will be created")
    rescue NoMethodError
      inst_s = 'm1.small'
      Log.logger.debug("instance type was not defined, defaulting to small")
    end

    return inst_s
  end

  # redefine the XMLRPC call method to retry on failure.
  # TODO: how to strip out max_tries and waittimes so they can be passed in
  class XMLRPC::Client
    alias_method :real_call, :call2

    def call2(cmd, *args)
      max_tries = 3
      waittime = 5
      t = 0
      begin
        t += 1
        return real_call(cmd, *args)
      rescue StandardError, Timeout::Error => rpc_e
        Log.logger.debug("XMLRPC Attempt #{t} failed, trying again in #{waittime} seconds")
        if (t < max_tries)
          sleep waittime
          retry
        else
          Log.logger.warn("XML RPC command failed after #{t} tries.  The last message was #{rpc_e.message}")
          raise rpc_e
        end
      end
    end
  end

  # returns "the" XMLRPC client; there can only be one
  def fields_xmlrpc
    unless (@xmlrpc)
      #        auth = IT.new.fieldsrpc_credentials.get
      #        master_api_url = "https://#{auth.login}:#{auth.password}@master.e." + fields_stage + '.f.e2a.us/xmlrpc.php'
      @xmlrpc =  XMLRPC::Client.new2(self.master_api_url)
    end
    return @xmlrpc
  end

  def master_api_url
    raise "$config is not available to create master_api_url" if $config.nil?
    raise "fields_stage key not found in $config" unless $config.key?('fields_stage')
    auth = IT.new.fieldsrpc_credentials.get
    api_url = "https://#{auth.login}:#{auth.password}@master.e." + $config['fields_stage'] + '.f.e2a.us/xmlrpc.php?format=none'
    return api_url
  end

  # the path to the fields-provision command
  def fields_provision_cmd
    raise "$config is not available to create fields_provision_cmd" if $config.nil? 
    raise "fields_root key not found in $config" unless $config.key?('fields_root')
    return 'php ' + $config['fields_root'] + '/fields-provision.php '
  end

  # Return the basic set of servers for all site types. This effectively
  # declares all the servers that are launched by the system tests.
  def standard_servers
    { 'svn' => 1, 'bal' => 2, 'web' => 2, 'dbmaster' => 2, 'ded' => 2, 'managed' => 1, 'backup' => 1 }
  end

  # Return the fileserver cluster which the standard servers should use.
  def standard_fs_cluster_name
    'test-fs-cluster-1'
  end

  # Allocate Hosting servers, including any additional setup such as
  # clustering, adding bricks, etc. Allow EC2 to choose the availability
  # zone within Fields::region to improve the odds of launching.
  #
  # @param type
  #   The server type to allocate
  # @param num
  #   The number of servers to allocate
  # @param prefix
  #   The host name prefix to use.
  # @return
  #   The allocated, unlaunched server records.
  def allocate_servers(type, num, opts = {})
    opts = {
      :prefix => type.to_s,
      :region => self.region,
      :availability_zone => self.availability_zone,
      :instance_size => self.instance_size,
      :fs_cluster_name => standard_fs_cluster_name}.merge(opts)
    servers = fields_xmlrpc.call2('acquia.fields.allocate.servers', type, num.to_s,
      { 'prefix' => opts[:prefix],
        'region' => opts[:region],
        'avail_zone' => '*',
        'ami_type' => opts[:instance_size]})

    servers.each do |id, server|
      fields_xmlrpc.call2('acquia.fields.save', 'server_tag',
        { 'server_id' => id, 'tag' => 'system-test' })
    end

    # If we just allocated db servers, cluster them. Make the evens encrypt
    # the primary volume and the odds encrypt the secondary volume.
    if (type =~ /dbmaster|ded/)
      fields_xmlrpc.call2('acquia.fields.allocate.db.cluster', servers)
      servers.each do |id,srv|
        vol_type = (id.to_i % 2 == 0) ? 'primary' : 'secondary'
        Log.logger.info("Encrypting #{srv['name']} (#{id}) #{vol_type} volume")
        self.run fields_provision_cmd + "--server-volume-encrypt #{srv['name']}:#{vol_type}:on"
      end
    end

    # Add servers to an FS cluster, creating it if necessary
    if (type =~ /bal|web|ded|managed/)
      cluster = fields_xmlrpc.call2('acquia.fields.find.one', 'fs_cluster', { 'name' => opts[:fs_cluster_name] })
      if cluster.nil? || cluster.empty?
        cluster = { 'name' => opts[:fs_cluster_name], 'server_ids' => servers.keys }
      else
        cluster['server_ids'] = servers.keys.concat(cluster['server_ids']).uniq
      end
      cluster_id = fields_xmlrpc.call2('acquia.fields.save', 'fs_cluster', cluster)
      # update the local server records to reflect the FS cluster assignment
      servers.each_key do |sid|
        servers[sid]['fs_cluster_id'] = cluster_id
      end
    end

    # If we just allocated balancers or dedicated servers, add a brick.
    # Encrypt the even ones.
    if (type =~ /bal|ded/)
      servers.each { |id, srvr| self.add_brick(srvr['name'], id.to_i % 2 == 0) }
    end

    # Create a balancer cluster out of the balancers.
    if (type == 'bal')
      bal_cluster = {
        'ec2_region' => servers.values.first['ec2_region'],
        'ec2_availability_zone' => servers.values.first['ec2_availability_zone'],
      }
      bal_cluster_id = fields_xmlrpc.call2('acquia.fields.save', 'bal_cluster', bal_cluster)
      servers.each { |id, srv| servers[id]['bal_cluster_id'] = bal_cluster_id }
      fields_xmlrpc.call2('acquia.fields.save.multiple', 'server', servers)
    end

    Log.logger.info("Allocated #{num} #{type}s: " + servers.map { |id,s| s['name']  }.join(','))
    return servers
  end

  # Launch all unlaunched servers.
  #
  # @param find_opts
  #   A list of RPC Find options for the servers to be launched
  # @return
  #   The launched server records.
  def launch_servers(find_opts = {})
    # By default, we'll launch all allocated servers.
    find_opts = find_opts.merge({
        'ec2_id' => 'create'
      })

    # Retrieve the servers that will be launched.
    to_launch = fields_xmlrpc.call2('acquia.fields.find', 'server', find_opts)
    to_launch_names = to_launch.map{|id,s| s['name']}.join(',')
    Log.logger.info("Launching servers: " + to_launch_names)

    arg = "--launcher --region #{self.region} --parallel --server #{to_launch_names} --dns-wait-time 3600"
    launch_cmd = fields_provision_cmd + arg
    Log.logger.info("Launch command: [#{launch_cmd}]")

    curr_try = 0
    max_tries = 3
    launched = false
    while (!launched && curr_try < max_tries)
      curr_try += 1
      (ec, result) = self.attempt launch_cmd
      puts result
      launched = (result !~ /Launch\s+failure/i && ec == 0)
      if (!launched)
        Log.logger.warn("Launch servers #{to_launch_names} failed on try #{curr_try} retrying")
      end
    end
    if (!launched)
      raise "Could not launch servers #{to_launch_names} in #{max_tries} tries"
    end

    Log.logger.info("Servers #{to_launch_names} successfully launched on try #{curr_try}")

    # Return the launched server records
    fields_xmlrpc.call2('acquia.fields.find', 'server',
      { 'id' => { 'op' => 'IN', 'value' => to_launch.map { |id,s| s['id'] }}})
  end

  # Given a list of servers, verify that they are working. Services
  # are verified based on server type.
  #
  # @param servers
  #   Hash of servers to verify, id => record.
  # @return
  #   TODO: Throw instead of assert.
  def verify_servers(servers)
    Log.logger.info("Verifying servers: " + servers.map{|id,s| s['name']}.join(', '))
    servers.each { |id,s|

      Log.logger.info("Checking server #{id}:#{s['name']} has valid EC2 ID [#{s['ec2_id']}]")
      assert_match(/^i-[a-f0-9]+$/, s['ec2_id'], "Server #{s['name']} has an invalid EC2 ID [#{s['ec2_id']}]")

      Log.logger.info("Checking server #{id}:#{s['name']} has valid FQHN #{s['external_fqhn']}")
      assert_not_equal('', s['external_fqhn'], "Server #{s['name']} has no FQHN")

      # Verify that the server's DNS entry is correct. The TTL is 60 seconds.
      condition = lambda {
        begin
          public_ip = Resolv.getaddress(s['external_fqhn'])
          Log.logger.info("#{s['external_fqhn']} resolves to #{public_ip}")
          s['external_ip'] == public_ip
        rescue Resolv::ResolvTimeout
          Log.logger.info("#{site_info['default_fqdn']} resolves to <timeout>")
          false
        end
      }
      CommonUtil.wait_until(90, 1, condition, true, "#{s['external_fqhn']} to resolve to #{s['external_ip']}")

      # Verify all servers are up
      server_status = self.server_up?(s['external_fqhn'])
      assert(server_status.passed, "Server #{s['name']} does not appear to be up: #{server_status.get_fail_message}")

      # Verify puppet on all servers
      status = self.puppet_up?(s['external_fqhn'])
      assert(status.passed, "Server #{s['name']} does not appear to have puppet properly configured\n" + status.get_fail_message)

      # Now puppet has finished, verify that the default launch keys are no
      # longer installed. This assumes the person running the tests has a
      # personal key in the authorized_keys file managed by puppet. We've added
      # buildbot's key.
      ssh(s['external_fqhn']) {|ss|
        (ec,out) = ss.attempt('grep -E "hosting|gardens" /root/.ssh/authorized_keys*')
        assert_equal(1, ec, "Server #{s['name']} has no default ssh keys installed")
      }

      # Verify netrc on all servers
      status = self.correct_netrc?(s['external_fqhn'], (s['name'] =~ /^managed/) ? 3 : (s['name'] =~ /^backup/ ? 3 : 1))
      assert(status.passed, "Server #{s['name']} has an incorrect .netrc:\n" + status.get_fail_message)

      # Verify splunk on all servers
      # TODO: Remove this? We aren't using splunk.
      #      status = self.splunk_up?(s['external_fqhn'])
      #      assert(status.passed, "Server #{s['name']} does not appear to have splunk properly configured\n" + status.get_fail_message)

      # Verify services based on server type
      if (s['type'] =~ /web|ded|staging|managed/)
        status = self.webserver_up?(s['external_fqhn'])
        assert(status.passed, "Server #{s['name']} does not appear to have apache httpd properly configured\n" + status.get_fail_message)
      end

      if (s['type'] =~ /svn/)
        status = self.svnserver_up?(s['external_fqhn'])
        assert(status.passed, "Server #{s['name']} does not appear to have svn properly configured\n" + status.get_fail_message)
      end

      if (s['type'] =~ /bal/)
        status = self.balancer_up?(s['external_fqhn'])
        assert(status.passed, "Server #{s['name']} does not appear to have nginx properly configured\n" + status.get_fail_message)
      end

      if (s['type'] =~ /ded|staging|dbmaster/)
        status = self.dbserver_up?(s['external_fqhn'])
        assert(status.passed, "Server #{s['name']} does not appear to have mysqld properly configured\n" + status.get_fail_message)
      end
    }
  end

  # Verify all the servers in the list of standard servers
  #
  # @return
  #   TODO: Throw, not assert.
  def verify_standard_servers
    all_servers = {}
    @servers.each { |type,srvs|
      all_servers.merge!(srvs)
    }
    verify_servers(all_servers)
  end

  # Retrieve or allocate server records for all of the standard servers.
  #
  # Returns a nested hash of server records, keyed by server type and server ID.
  def get_standard_servers

    return @servers unless @servers.nil?

    # Find all system-test servers that we previously created.
    server_tags = fields_xmlrpc.call2('acquia.fields.find', 'server_tag', { 'tag' => 'system-test' })
    server_ids = server_tags.map {|id,s_t| s_t['server_id']}

    @servers = {}
    allocated = false
    standard_servers.each { |type, num|
      @servers[type] = {}

      # Find servers with the correct type that we previously allocated.
      fields_xmlrpc.call2('acquia.fields.find', 'server', { 'type' => type, 'status' => { 'op' => '!=', 'value' => SERVER_STATUS_KILLED }, 'id' => { 'op' => 'IN', 'value' => server_ids }}).each { |id,s|
        @servers[type][id] = s
      }

      # If any are still waiting to be launched (e.g. we aborted a previous
      # run), notice.
      allocated ||= ! (@servers[type].values.find_all { |s| s['status'] == SERVER_STATUS_ALLOCATED.to_s }.empty?)

      # If fewer than we want exist, allocate more.
      if (@servers[type].length < num)
        to_alloc = num - @servers[type].length
        opts = {}
        opts[:fs_cluster_name] = standard_fs_cluster_name if type == 'bal'
        if type == 'ded'
          # give each dedicated server its very own fileserver cluster as server and client
          @servers[type] ||= {}
          to_alloc.times do |i|
            opts[:fs_cluster_name] = "dedicated-fs-#{i}"
            @servers[type].merge!(allocate_servers(type, 1, opts))
          end
        else
          @servers[type] = allocate_servers(type, to_alloc, opts)
        end
        allocated = true
      end
    }

    # If we allocated any servers, launch and verify them all right now.
    # Do this after the loop above so all servers launch in parallel.
    if (allocated)
      launched_servers = launch_servers
      launched_servers.each {|id,s|
        Log.logger.info("Updating server record for server ID #{s['id']}")
        # Update @servers to contain the launched server records.
        @servers[s['type']][s['id']] = s
      }
      verify_servers(launched_servers)
    end

    return @servers
  end

  # Create a site if it does not exist. Optionally make sure it is set up
  # on its servers.
  #
  # TODO: This is really create_or_reuse_and_optionally_verify_site().
  # Rename to create_or_reuse_site() and make callers call verify_site()
  # themselves.
  #
  # @param _type
  #   Site type: shared, dedicated, private, or gardens.
  # @param _site_name
  #   Site name
  # @param verify = true
  #   If true, verify the site sets up correct or raise an exception.
  # @param _extra_args = {}
  #   Extra arguments passed to fields-provision. Deprecated.
  def create_or_verify_site(type, site_name, verify = true, _extra_args = {})
    site_info = get_site_info(fields_xmlrpc, site_name)
    if (site_info.nil?  || 0 == site_info.size)
      Log.logger.info("Site #{site_name} does not exist; creating.")
      site_info = create_site(type, site_name, _extra_args)
    else
      # The IP address of our balancers may change (e.g. during our elastic
      # IP tests). Sites created after the change will use the new IP because
      # create.site sets the FQDN, but sites created before that change will
      # still point to the old IP address. Save such sites again to update
      # their DNS.
      Log.logger.info("Site #{site_name} exists; re-saving to update default FQDN DNS.")
      fields_xmlrpc.call2('acquia.fields.save', 'site', site_info)
    end

    if (verify)
      verify_site(site_info)
    end
  end

  # Create a new site with a unique name.
  #
  # @param type
  #   Site type: shared, dedicated, private, or gardens.
  # @param prefix
  #   Site name prefix
  # @return
  #   The site record
  def create_new_site(type, prefix, opts = {})
    # Find a unique site name. Try the prefix first.
    site_name = get_random_site_name(prefix)
    Log.logger.info("Creating site #{site_name}")
    create_site(type, site_name, {}, opts)
  end

  # Return a random name for a site that begins with the given
  # prefix. Try the prefix as a bare word first. Do not return
  # a name that matches any existing site.
  #
  # - +prefix+ A string to include at the beginning of the site name.
  def get_random_site_name(prefix)
    site_name = prefix
    while (site_exists?(site_name))
      site_name = prefix + rand(999).to_s
      Log.logger.info("Trying unique site name: #{site_name}")
    end
    site_name
  end

  # Verify that a site is configured on all of its servers.
  #
  # @param site_info
  #   The site record.
  # @return
  #   The fully populated site record. Throws an exception on failure.
  def verify_site(site_info)
    # SVN creation can take a minute, so do it now.
    site_name = site_info['name']
    site_host = "#{site_name}.#{fields_stage}.#{sites_domain}"
    get_svn_servers(site_info).each {|s|
      Log.logger.info("Site #{site_name}: fields-config-svn on #{s}")
      ssh(s) { |ss|
        ss.watch(180, 1, '/usr/local/sbin/fields-config-svn.php')
      }
    }

    # Verify SVN repo exists. We just ran f-c-svn; waiting isn't really needed.
    condition = lambda {
      site_info = get_site_info(nil, site_name)
      return !site_info['svn']['internal'].empty?
    }
    CommonUtil.wait_until(180, 3, condition, true, "#{site_name} repo to exist")

    # Wait for site to be configured on each web node...
    get_web_servers_external_fqhn(site_info).each {|s|
      Log.logger.info("Site #{site_name}: waiting for virtual host on #{s} to say 'This is an Acquia Hosting web site'")
      srv = Net::HTTP.new(s)
      condition = lambda {
        res = srv.get("/index.html?verify_web=#{s}", { 'Host' => site_host, 'Cookie' => 'NO_CACHE=1' })
        return (res.code.to_i == 200 && res.body =~ /This is an Acquia Hosting web site/i)
      }
      CommonUtil.wait_until(180, 3, condition, true, "site #{site_name} virtual host on #{s}")

      ssh(s) do |ss|
        # Verify site-php files are created.
        dir = "/var/www/site-php/#{site_info['name']}"
        # Old-style settings include files.
        assert_ssh(ss, "stat #{dir}/D5-#{site_info['name']}-settings.inc")
        assert_ssh(ss, "stat #{dir}/D6-#{site_info['name']}-settings.inc")
        assert_ssh(ss, "stat #{dir}/D7-#{site_info['name']}-settings.inc")
        # New-style settings include files.
        assert_ssh(ss, "stat #{dir}/D5-#{site_info['stage']}-#{site_info['db']['role']}-settings.inc")
        assert_ssh(ss, "stat #{dir}/D6-#{site_info['stage']}-#{site_info['db']['role']}-settings.inc")
        assert_ssh(ss, "stat #{dir}/D7-#{site_info['stage']}-#{site_info['db']['role']}-settings.inc")
      end
    }

    # ... and balancers.
    get_balancers(site_info).each {|s|
      Log.logger.info("Site #{site_name}: waiting for virtual host on #{s} to say 'This is an Acquia Hosting web site'")
      srv = Net::HTTP.new(s)
      condition = lambda {
        res = srv.get("/index.html?verify_bal=#{s}", { 'Host' => site_host, 'Cookie' => 'NO_CACHE=1' })
        return (res.code.to_i == 200 && res.body =~ /This is an Acquia Hosting web site/i)
      }
      CommonUtil.wait_until(180, 3, condition, true, "site #{site_name} virtual host on #{s}")
    }

    # Wait for all dbs to be created on all db servers.
    site_info['databases'].each {|dbid,db|
      Log.logger.info("Site #{site_name}: waiting for db #{db['name']}")
      site_info['db_servers'].each {|srvid,srvname|
        srv = site_info['db_servers'][srvid]
        condition = lambda {
          ec = nil
          ssh(srv['external_fqhn']) {|ss|
            (ec,out) = ss.attempt("mysql -e \"show databases\" | grep #{db['name']}")
          }
          ec == 0
        }
        CommonUtil.wait_until(90, 15, condition, true, "site #{site_name} db #{db['name']} on #{srv['name']}")
      }
    }

    # Verify that gfs is working across all web servers.
    token = Time.now.to_i.to_s + rand().to_s
    ssh(site_info['web_servers'].values.first['external_fqhn']) {|ss|
      (ec, out) = ss.attempt("echo #{token} > /mnt/gfs/#{site_info['name']}/verify-token-#{token}")
      assert_equal(0, ec, "writing verify-token")
    }
    site_info['web_servers'].each {|id,srv|
      ssh(srv['external_fqhn']) {|ss|
        (ec, out) = ss.attempt("cat /mnt/gfs/#{site_info['name']}/verify-token-#{token}")
        assert_equal(token, out.strip, "verify-token on server #{srv['external_fqhn']}")
      }
    }

    if (true)
      # Verify that the site's DNS entry is current. This makes our system
      # tests a canary for our DNS provider's API.
      assert_equal('1', site_info['dns_current'], "#{site_name}'s DNS is current")

      # Verify that the site's default domain has an A record for one of its
      # balancers. The TTL is 60 seconds, but we're seeing a lot of failures
      # here, so let's try 5 minutes plus a little.
      condition = lambda {
        begin
          public_ip = Resolv.getaddress(site_info['default_fqdn'])
          Log.logger.info("#{site_info['default_fqdn']} resolves to #{public_ip}")
          site_info['balancers'].map {|id,s| s['external_ip']}.include?(public_ip)
        rescue Resolv::ResolvTimeout
          Log.logger.info("#{site_info['default_fqdn']} resolves to <timeout>")
          false
        end
      }
      CommonUtil.wait_until(310, 1, condition, true, "#{site_info['default_fqdn']} resolves to one of its balancer's IPs")
    end

    return get_site_info(nil, site_info['name'])
  end

  # Accept a hash whose keys are options for the create-site fields-provision
  # command (e.g. "--svn") and whose values are the values for each key. Join
  # these up into an argument string for fields-provision and return that
  # string.
  def join_site_create_options(opts)
    opts.keys.inject('') do |str, k|
      str += k + ' ' + opts[k] + ' '
    end
  end

  # Return the fields-provision options to create site_name of site_type.
  #
  # @param site_name
  #   The site name to create.
  # @param site_type
  #   shared, dedicated, private, or gardens.
  # @return
  #   fields-provision command-line options to create the site. For now,
  # this uses the explicit server options --svn, --webs, etc. Later, it
  # may use --tag.
  def _create_site_options(site_name, site_type, opts = { })
    opts = { :join => TRUE }.merge(opts)

    get_standard_servers

    # Create the command-line options based on site_type
    site_options = {}
    site_options['--create-site']  = site_name
    if opts[:parent_site].nil?
      site_options['--svn'] = @servers['svn'].values.first['name']
    else
      site_options['--parent-site'] = opts[:parent_site]
      site_options['--stage'] = opts[:stage] || 'test';
    end
    site_options['--bals'] = @servers['bal'].map {|id,s| s['name']}.join(',')
    case site_type
    when /^shared$/, /^private$/
      site_options['--webs'] = @servers['web'].map {|id,s| s['name']}.join(',')
      site_options['--db'] = @servers['dbmaster'].values.first['name']
    when /^dedicated$/
      site_options['--webs'] = @servers['ded'].values.first['name']
      site_options['--db'] = @servers['ded'].values.first['name']
    when /^gardens$/
      site_options['--webs'] = @servers['managed'].values.first['name']
      site_options['--db'] = @servers['dbmaster'].values.first['name']
      site_options['--svn-repo'] = "ACQUIA_GARDENS-REPOSITORY"
    else
      raise "unknown site type #{site_type}"
    end
    if opts[:join]
      return join_site_create_options(site_options)
    else
      return site_options
    end
  end

  # Create a new hosting site, assigning it to servers based on type.
  #
  # @param _type
  #   Site type: shared, dedicated, private, gardens. This means nothing
  #   other than what servers the site is assigned to.
  # @param _site_name
  #   Site name. Behavior is undefined if the site already exists.
  # @param _extra_args
  #   Extra arguments for fields-provision.php.  Deprecated.
  # @return
  #   The site record. Raises an exception if fields-provision fails.
  def create_site(_type, _site_name=site, _extra_args = {}, _opts = {})
    if (!validate_site_name(_site_name))
      raise "Site name: #{_site_name} is not valid"
    end

    # optionaly set more arguments.
    extra = '';
    _extra_args.each {|key, value|
      extra += ' --' + key + ' ' + value
    }

    Log.logger.info("Creating site #{_site_name} of type #{_type}")
    cmd = fields_provision_cmd + _create_site_options(_site_name, _type, _opts) + ' ' + extra
    Log.logger.info("Creation command: #{cmd}")
    begin
      result  = self.run cmd
    rescue => details
      Log.logger.warn("the creation command failed: #{details.message}\n #{details.backtrace}")
      raise details
    end

    self.get_site_info(nil, _site_name)
  end

  #return true if there is a brink on the server passed in.
  # the brick might still not be launched
  # takes the name of the server (not the fqhn)
  def has_brick?(_server)

    has_brick = false
    server = fields_xmlrpc.call2('acquia.fields.find.one', 'server', {'name' => _server})
    server_id = server['id']
    bricks = fields_xmlrpc.call2('acquia.fields.find', 'gluster_brick', { 'server_id' => server_id})
    Log.logger.info("array_of_bricks: #{bricks.to_yaml}")

    if (!bricks.nil? && bricks.size != 0)
      has_brick = true
      Log.logger.debug("Server:#{_server}. has_brick: #{has_brick}")
    end
    return has_brick

  end

  # adds a brick to a server. does not "launch it"
  # will only pass in defaults.
  def add_brick(_server_name, _encrypted, _snap_id = nil, _device = nil, _repl_id = nil)
    arg = "--add-brick --server #{_server_name}"
    if (_snap_id)
      arg += " --snap-id #{_snap_id}"
    end

    if (_device)
      arg += " --device #{_device}"
    end

    if (_repl_id)
      arg += " --repl-id #{_repl_id}"
    end

    if (_encrypted)
      Log.logger.info("Encrypting #{_server_name}'s brick")
      arg += " --encrypted"
    end

    Log.logger.info("Adding brick to server #{_server_name}")
    cmd = fields_provision_cmd + arg
    Log.logger.debug("launcher cmd: #{cmd}")
    (ec, result)  = self.attempt cmd
    puts result
    if(result =~ /error/i)
      raise "Error adding brick to server #{_server_name}."
    end
    return result
  end

  # launches created bricks
  def launch_bricks
    arg = '--launch-bricks'
    Log.logger.info("Launching created bricks")
    cmd = fields_provision_cmd + arg
    Log.logger.debug("launcher cmd: #{cmd}")
    (ec, result)  = self.attempt cmd
    puts result
    return result
  end

  # launches a site in fields
  def launch_site(_site_name=site)
    max_tries = 3
    launched = false
    arg = '--launcher --parallel'
    Log.logger.info("Launching site #{_site_name}")
    cmd = fields_provision_cmd + arg
    Log.logger.debug("launcher cmd: #{cmd}")
    result = ''
    curr_try = 0
    until launched
      curr_try += 1
      if (curr_try >= max_tries)
        result = "Could not launch site #{site} in #{curr_try} tries"
        return result
      end
      (ec, result)  = self.attempt cmd
      puts result
      unless (result =~ /Launch\s+failure/i  || ec != 0)
        Log.logger.info("Site #{_site_name} was successfully launched on try #{curr_try}")
        launched = true
      else
        Log.logger.warn("Launch of site #{_site_name} failed on try #{curr_try} retrying")
      end
    end
    return result
  end

  # Terminate all servers matching a query.
  def terminate_server(_query)
    Log.logger.info("Terminating servers #{_query}")
    cmd = fields_provision_cmd + " --terminate #{_query} --parallel"
    (ec, result)  = self.attempt cmd
    Log.logger.debug("Termination result: #{result}")
    return result
  end

  # terminates servers that have _server anywhere in the name
  def terminate_servers(_server)
    arg = '%' + _server + '%'
    return terminate_server(arg)
  end

  # validates a sites name currently will only check for length until
  # there is a boolean
  def validate_site_name(_sitename=site)
    is_valid = false
    if (_sitename.length > 16 ||
          _sitename =~ /^[0-9]/ ||
          _sitename =~ /\W|_/)
      is_valid = false
    else
      is_valid = true
    end
    return is_valid
  end

  # gets the site id from the site info
  # throws a  NilSiteInfoError if we get back an empty site info object
  # throws a CorruptSiteIfoError if the server keys are missing hostnames

  def get_site_id(_site_info)
    site_id = _site_info['id']
    return site_id
  end
  # gets the stiename from the site info
  def get_site_name(_site_info)
    site_name = _site_info['name']
    return site_name
  end

  # Return 80, or the per-site vhost port.
  def get_site_vhost_port(_site_info)
    port = '80'
    if (!_site_info['config_settings']['vhost'].nil? && !_site_info['config_settings']['vhost']['port'].nil?) then
      port = _site_info['config_settings']['vhost']['port']
    end
    return port
  end

  # returns the info from a fields site by name
  def get_site_info(_client=nil, _sitename=site)
    site_info = nil
    if (_client==nil)
      _client = fields_xmlrpc
    end
    site_info  = _client.call2('acquia.fields.get.site.info', _sitename)
    unless (site_info)
      # TODO: This function ostensibly throws NilsiteInfoError when the site is
      # not found. However, get.site.info returns NULL in that case which
      # XML-RPC turns into the empty string which Ruby treats as true, so
      # the exception has never been raised. The new JSON encoding gets this
      # right so the function started throwing the exception, but now none
      # of the calling code expects that, so we just return "" like we did
      # before. Fix this.
      #Log.logger.warn("Site info returned an empty object")
      #raise NilSiteInfoError, "Empty site info object"
      return ""
    end
    return site_info
  end

  #

  # simply checks to see if the site exists in fields
  def site_exists?(_site_name)
    s_exists = true
    site_info = get_site_info(fields_xmlrpc, _site_name)
    if (site_info.nil?  || 0 == site_info.size)
      return false
    end
    return s_exists
  end

  # checks to see if the site exists in fields, that there is a
  # server for each of the necessary pieces. Servers have already been
  # verified.
  def site_up?(_site_name)
    if (! site_exists?(_site_name))
      return false
    end
    site_info = get_site_info(fields_xmlrpc, _site_name)
    dbs = get_db_servers_external_fqhn(site_info)
    if (0 == dbs.length)
      return false
    end
    webs = get_web_servers_external_fqhn(site_info)
    if (0 == webs.length)
      return false
    end
    bals = get_balancers(site_info)
    if (0 == bals.length)
      return false
    end
    svns = get_svn_servers(site_info)
    if (1 != svns.length)
      return false
    end
    return true
  end

  # Return an array of values in the site info have that are in the
  # various servers fields. Only consider servers with status NORMAL,
  # since this function is usually used by (e.g.)
  # get_web_servers_external_fqhn and a otherwise a single killed
  # server will prevent it from working forever (due to
  # require_fqhns). This also means a single unlaucnhed server will
  # also prevent it from working, but that at least is not a
  # permanent condition, and is something the user might want to be
  # warned about.
  #
  # Throw an exception if the result is empty becasue
  # usually you want something back.
  def get_servers_data(_site_info, _s_type, _d_type)
    if (_s_type !~/db_servers|web_servers|balancers|svn_server/)
      raise _s_type + ' is not a supported server type'
    end
    s_attr = Array.new
    s = _site_info[_s_type]
    s.each{|id,val|
      if (val['status'] == SERVER_STATUS_NORMAL.to_s && val[_d_type])
        s_attr.push(val[_d_type])
      end
    }
    if (0 == s_attr.size)
      raise "No attribute #{_d_type} for server type #{_s_type} was found"
    end
    return s_attr
  end

  # given the server name in fields-  e.g. bal-9, ded-1 ....
  # get the value for the attribute.  e.g.
  # external_fqhn ...
  def get_server_attribute_by_name(_server_name, _attr)
    Log.logger.info("Requesting the #{_attr} for server #{_server_name}")
    result = fields_xmlrpc.call2('acquia.fields.list.servers.by.name', _server_name)
    Log.logger.info("XML_RPC output for server #{_server_name} is: #{result.inspect}")
    server_attribute = result[result.keys.to_s][_attr]
    if (nil == server_attribute)
      raise "No attribute #{_attr} for server #{_server_name} was found"
    end
    Log.logger.info("Server #{_server_name} has #{_attr}: #{server_attribute}")
    return server_attribute
  end

  # returns an hash in the site info
  # of the users info.
  # throws an exception if the result is empty becasue
  # usually you want something back
  # this is the same as get_servers_data and refactoring should be considered.
  def get_users_data(_site_info)
    s_type = 'users'
    s = _site_info[s_type]
    return s
  end

  # returns an array of the web names, NOT useful for ssh ...
  def get_web_servers_name(_site_info)
    return get_servers_data(_site_info, 'web_servers', 'name')
  end

  # returns an array of the DBs extenral fqhn sueful for ssh ...
  def get_db_servers_external_fqhn(_site_info)
    dbs_fqhn = require_fqhns('db', get_servers_data(_site_info, 'db_servers', 'external_fqhn'))
    return dbs_fqhn
  end
  # returns an array of the DBs extenral fqhn sueful for ssh tunneling...
  def get_db_servers_internal_fqhn(_site_info)
    dbs_fqhn = require_fqhns('db', get_servers_data(_site_info, 'db_servers', 'internal_fqhn'))
    return dbs_fqhn
  end

  # returns an array of the DBs extenral fqhn sueful for ssh ...
  def get_web_servers_external_fqhn(_site_info)
    webs_fqhn = require_fqhns('web', get_servers_data(_site_info, 'web_servers', 'external_fqhn'))
    return webs_fqhn
  end

  # returns an array of the DBs extenral fqhn useful for ssh ...
  def get_web_servers_internal_fqhn(_site_info)
    webs_fqhn = require_fqhns('web', get_servers_data(_site_info, 'web_servers', 'internal_fqhn'))
    return webs_fqhn
  end

  def get_web_servers_internal_ip(_site_info)
    webs_fqhn = require_fqhns('web', get_servers_data(_site_info, 'web_servers', 'internal_ip'))
    return webs_fqhn
  end

  def require_fqhns(type, fqhns)
    fqhns.each {|s| raise type + " server does not have a external address, site cannot be tested, please reset system" if s.empty?}
    return fqhns
  end

  # gets the junk that is "repo" based.  i.e. the svn junk,
  # DB connection info...
  def get_repo_data(_site_info, _r_type, _d_type)
    unless _r_type == 'svn' || _r_type == 'db'
      raise _r_type + ' is not a supported type'
    end
    r_data = Array.new
    repo = _site_info[_r_type]
    repo.each do |k,v|
      r_data.push(v) if k == _d_type
    end
    if (0 == r_data.size)
      raise "No attribute #{_d_type} for repo type #{_r_type} was found"
    end

    return r_data

  end

  # returns an array of the balancers for a site.
  # This currently assumes that the svn repo and the
  # balancer are the same machine
  def get_balancers(_site_info)
    balancers_fqhn = require_fqhns('bal', get_servers_data(_site_info, 'balancers', 'external_fqhn'))
  end

  def get_balancers_names(_site_info)
    balancers_fqhn = self.get_servers_data(_site_info, 'balancers', 'name')
  end

  # returns an array of the svn servers for a site.
  def get_svn_servers(_site_info)
    svns_fqhn = require_fqhns('svn', get_servers_data(_site_info, 'svn_server', 'external_fqhn'))
  end

  def get_db_user(_site_info)
    user = Array.new
    user = get_repo_data(_site_info, 'db', 'user')
    return user[0]
  end

  def get_db_name(_site_info)
    name = Array.new
    name = get_repo_data(_site_info, 'db', 'name')
    return name[0]
  end

  def get_db_pass(_site_info)
    pass = Array.new
    pass = get_repo_data(_site_info, 'db', 'pass')
    return pass[0]
  end

  # get the slave user.  _site_name at the moment is not necesary and will be ignored
  def get_repli_user(_site_name=nil)
    return 'slave'
  end
  # get the slave password
  def get_repli_pass(_site_name=nil)
    return 'jlJ6LntHJE'
  end

  # gets an array of the uniq servers that comprise a site.
  def get_site_servers(_site_info)
    dbs = get_db_servers_external_fqhn(_site_info)
    webs = get_web_servers_external_fqhn(_site_info)
    bals = get_balancers(_site_info)
    svns = get_svn_servers(_site_info)
    all_servers = dbs + webs + bals + svns
    uniq_servers = all_servers.uniq
    return uniq_servers
  end

  # reboots a host will wait 5 minutes
  def reboot_host(_server, _waittime = 300, _polltime = 5)
    Log.logger.info("Rebooting #{_server}")
    ssh(_server){|s|
      cmd = 'reboot'
      s.exec!(cmd)
    }
    sleep 5 # just to make sure
    timeout = Time.now + _waittime
    while (server_up?(_server))
      sleep(_polltime)
      if (Time.now > timeout)
        raise Exception.new("Maximum number of attempts to reached waiting for server to stop")
      end
    end
    timeout = Time.now + _waittime
    while (!server_up?(_server))
      sleep(_polltime)
      if (Time.now > timeout)
        raise Exception.new("Maximum number of attempts to reached waiting for server to start")
      end
    end
  end

  #Look for strings in /var/log/messages* for warnings|errors|notices... (case insensitive)
  # Pretty hard coded and heavy handed. strip out the kernel errors that happen on all machines
  # these seem to happen a lot
=begin
/var/log/messages-20090324:Mar 24 17:28:37 domU-12-31-39-03-CD-95 kernel: PCI: Fatal: No config space access function found
/vaog/messages-20090324:Mar 24 17:28:37 domU-12-31-39-03-CD-95 kernel: powernow-k8: BIOS error - no PSB or ACPI _PSS objects
/var/log/messages-20090706:Jul  6 07:51:32 eve kernel: PCI: Fatal: No config space access function found
/var/log/messages-20090706:Jul  6 07:51:32 eve kernel: powernow-k8: BIOS error - no PSB or ACPI _PSS objects
/var/log/messages-20090706:Jul  6 07:51:32 eve kernel: EXT3-fs warning: checktime reached, running e2fsck is recommended
/var/log/messages-20090706:Jul  6 07:52:53 single-1 puppetd[1264]: (//Node[default]/acquia-util::common/openssh52/Exec[openssh-5.2p1]/returns) warning: /etc/ssh/sshd_config created as /etc/ssh/sshd_config.rpmnew
/var/log/messages-20090706:Jul  6 07:58:33 single-1 puppetd[1264]: (//Node[default]/acquia-util::common/splunk/Exec[splunk-install]/returns) warning: peer certificate won't be verified in this SSL session
/var/log/messages-20090706:Jul  6 07:58:33 single-1 puppetd[1264]: (//Node[default]/acquia-util::common/splunk/Exec[splunk-install]/returns) warning: /tmp/splunk-3.4.8-54309.i386.rpm: Header V3 DSA signature: NOKEY, key ID 653fb112
/var/log/messages-20090706:Jul  6 09:05:53 single-1 httpd[3537]: [error] [client 75.150.65.1] File does not exist: /var/www/html/DEFAULT-VHOST/favicon.ico
/var/log/messages-20090706:Jul  6 09:05:56 single-1 httpd[3538]: [error] [client 75.150.65.1] File does not exist: /var/www/html/DEFAULT-VHOST/favicon.ico
=end

  #TODO: ALL of these ignores need to be checked at some point to make sure they are ok
  def ignore_known_errors(_message, _ignores=nil)
    all_ignores = ['No config space access function found',
      # associated with the new 32-bit kernel 20100428
      'failed to get a good estimate for loops_per_jiffy',
      'uses 32-bit capabilities \(legacy support in use\)',
      'Disabling barriers, trial barrier write failed',

      'no PSB or ACPI _PSS objects',
      'checktime reached, running e2fsck is recommended',
      'netsnmp_assert.+duplicate.+agent_registry.+netsnmp_subtree_load',
      'puppetd.+openssh.+warning:.+sshd_config created as .+/sshd_config.rpmnew',
      'puppetd.+splunk-install.+peer certificate won\'t be verified in this SSL session',
      'puppetd.+splunk-install.+NOKEY',
      # puppet 25.4: DEPRECATION NOTICE: Files found in modules without specifying 'modules' in file path will be deprecated in the next major release.  Please fix module 'httpd' when no 0.24.x clients are present
      'puppetmasterd.+DEPRECATION NOTICE',
      # puppet 25.4: complains about /var/lib/puppet/lib/ being empty on first run after deb package install. Not important.
      'puppetd.+\(\/File\[\/var\/lib\/puppet\/lib\]\) Failed',
      'httpd.+File does not exist',
      'kernel.+probe.+uvesafb.+failed with error',
      'kernel.+process.+nginx.+deprecated sysctl',
      'kerne.+Failure registering.+primary security',
      'kernel.+vbe_init.+failed',
      'kernel.+failed.+v86d',
      'kernel.+VBE info.+ailed',
      'kernel.+failed.+cpufreq',
      'nrpe.+Could not get.+entry',
      'Communication error.+acquia_xmlrpc_call',
      'surpress_https_warnings.rb',
      'configtoolkit not available',
      'console-kit-daemon.+Invali.+argument',
      'console-kit-daemon.+active.+console',
      'console-kit-daemon.+waiting for native console',
      # The next two errors are due to a bug in the debian mysql-server-5.0
      # postinst script. The command to pipe SQL for filling in the timezone
      # tables lacks a "USE mysql" statement.
      'mysqld_safe.+No database selected', #  debian package install problem
      'mysqld_safe.+Aborting', # debian package install problem
      'mysql-server-postinst.+No database selected', #  debian package install problem
      'mysql-server-postinst.+Aborting', # debian package install problem
      'mysql.user.+root.+without password',
      # This warning occurs because we now created /var/lib/mysql as a symlink
      # to /vol/ebs1/mysql, owned by root, before installing mysql-server-5.0.
      # The postinst script chowns it to mysql after this warning.
      'adduser:.+Warning:.+The home directory `/var/lib/mysql\' does not belong',
      'warning=.+fields_config_rpc_retry',
      # TODO: We no longer use CPAN to install nagios plugins, remove these.
      'cpan_install_nagios_plugin.*LWP failed with code\[404\]',
      'cpan_install_nagios_plugin.*LWP failed with code\[500\]',
      'cpan_install_nagios_plugin.*Warning: prerequisite',
      'cpan_install_nagios_plugin.*WARNING: LICENSE is not a known parameter',
      'cpan_install_nagios_plugin.*Warning: No md5 checksum for Nagios-Plugin-0.34.tar.gz',
      'cpan_install_nagios_plugin.*fail-like\.',
      'cpan_install_nagios_plugin.*fail-more\.',
      'cpan_install_nagios_plugin.*fail\.',
      'cpan_install_nagios_plugin.*fail_one\.',
      'cpan_install_nagios_plugin.*is_deeply_fail\.',
      'cpan_install_nagios_plugin.*sample_tests\/five_fail\.plx',
      'cpan_install_nagios_plugin.*sample_tests\/one_fail\.plx',
      'cpan_install_nagios_plugin.*sample_tests\/too_few_fail\.plx',
      'cpan_install_nagios_plugin.*sample_tests\/two_fail\.plx',
      'cpan_install_nagios_plugin.*tbt_05faildiag\.',
      'cpan_install_nagios_plugin.*tbt_06errormess\.',
      'cpan_install_nagios_plugin.*Warning: This index file is \d+ days old\.',
      'Use of undefined constant ERROR_REPORTING_DISPLAY.*simpletest.function.inc',
      # Known harmless error on Hardy from 'modprobe sha256'.
      'Error inserting padlock_aes.*: No such device',
      # Correct output from luks-setup includes this error. The script exits
      # with status 1 if it actually fails.
      'Command failed: No key available with this passphrase',
      # This occurs on ded servers which have no partner.
      'Could not establish db replication after 100 attempts',
      # This occurs when a brand new backup server is brought up
      'Cannot perform split-brain check: cannot find cluster info',
    ]
    if (_ignores)
      all_ignores.concat(_ignores)
    end
    status = Acquia::FieldsStatus.new
    ignore_pattern = all_ignores.join('|')
    #    tmp_outputs = _message.split("\n")
    #    tmp_outputs.each{|line|
    pattern = '\w+\s+\d+\s+\d+:\d+:\d+\s+\S+\s+([-a-zA-Z0-9]+).+?:(.+)'
    _message.each{|line|
      key  = line.gsub(/#{pattern}/,'\1:\2')
      Log.logger.debug("key: #{key}")
      unless (line=~/#{ignore_pattern}/)
        status.add_fail(key)
      end
    }
    return status
  end

  #Look for strings in /var/log/messages* for warnings|errors|notices... (case insensitive)
  # ignores is an array of patterns like.
  #    ['fields-config-svn.+No such file',
  #    'fields-config-bal.+line 52',
  #    'fields-config-bal.+line 85',
  #    'fields-config-svn.+line 76',
  #    'fields-config-svn.+line 98',
  #    'fields-config-svn.+line 101']

  def get_log_errors(_server, _grep_patt = 'notice|warn|error|fatal|backup:|fail|alert=', _ignores=nil )
    status = Acquia::FieldsStatus.new
    syslog = syslog_id(_server)
    ssh(_server){|s|
      cmd = "ls -1rt /var/log/#{syslog}* | xargs egrep -hi \'#{_grep_patt}\'"
      (exit_code, str_output, str_err) = s.attempt(cmd)
      status = self.ignore_known_errors(str_output, _ignores)
    }
    return status
  end

  #returns true if the /var/log/messages* has things the look like errors in them
  # warn, notice, error, fatal, backup: (becasue there is an error here)
  # this is a tad fragile as if te logging is not correct, we will not see the error
  def log_has_errors?(_server, _ignores=nil , _grep_patt = 'notice|warn|error|fatal|backup:|fail')
    syslog = syslog_id(_server)
    status = self.get_log_errors(_server, _grep_patt, _ignores)
    if (!status.passed)
      Log.logger.warn("/var/log/#{syslog} on #{_server} has problems.")
      Log.logger.warn("#{status.get_fail_message}")
    end
    return status
  end

  # checks that the nettica ip address and the
  # external ip address from amazon match
  # _server is the external_fqhn
  def ip_address_match?(_server)
    status = Acquia::FieldsStatus.new
    server_rec = fields_xmlrpc.call2('acquia.fields.find.one', 'server', {'external_fqhn' => _server, 'status' => 0})
    ec2_ext_fqhn = server_rec['external_ec2']
    external_ip = server_rec['external_ip']
    ec2_ext_ip = Resolv.getaddress(ec2_ext_fqhn)
    server_ip = Resolv.getaddress(_server)
    if (server_ip != external_ip)
      status.add_fail("#{_server} dns entry #{server_ip} does not match the external_ip #{external_ip}")
    end
    if (server_ip != ec2_ext_ip)
      status.add_fail("#{_server} dns entry #{server_ip} does not match the ec2 dns entry #{ec2_ext_ip}")
    end
    return status
  end

  # checks to see that a server is up by running /bin/true via ssh
  # validates that the ip addreses for the external ec2_address and
  # the external_fqhn (nettica) match
  # _server is the external fqhn
  def server_up?(_server)
    Log.logger.info('Verifying server ' + _server)
    status = Acquia::FieldsStatus.new
    ip_status = self.ip_address_match?(_server)
    unless (ip_status.passed)
      status.add_fail(ip_status.get_fail_message)
    end
    cmd = "/bin/true || echo 'Server is not functional, no /bin/true'"
    begin
      ssh(_server){|s|
        output = s.exec!(cmd)
        if (output =~ /not functional/)
          status.add_fail("No /bin/true found on server server is not functional")
        end
      }
    rescue Errno::ECONNREFUSED
      status.add_fail("SSH connection to #{_server} was refused")
    end
    return status
  end

  def servers_up?(_all_servers)
    servers_status = Acquia::FieldsStatus.new()
    _all_servers.each{|s|
      Log.logger.info("Checking if server " + s + " is up")
      s_status = server_up?(s)
      if (!s_status.passed)
        Log.logger.warn("Server " + s + " does not appear to be up")
        servers_status.add_fail = s_status.get_fail_message
      end
    }
    return servers_status
  end

  def syslog_id(_server)
    (_server =~ /^master/ ? 'daemon.log' : 'syslog')
  end

  # Verifies that netrc is correct on the machine.
  def correct_netrc?(_server, entry_count = 1, _waittime = 600)
    status = Acquia::FieldsStatus.new
    ssh(_server){|s|
      # Be careful not to display netrc content. s.attempt logs all output!
      (ec, out) = s.attempt('grep -c "machine fields.rpc.acquia.com" /root/.netrc')
      out.rstrip!
      if (ec != 0)
        status.add_fail("Server #{_server} is missing /root/.netrc")
        Log.logger.warn("Server #{_server} is missing /root/.netrc")
      end
      if (out != "1")
        status.add_fail("Server #{_server} is missing RPC entry in /root/.netrc")
        Log.logger.warn("Server #{_server} is missing RPC entry in /root/.netrc")
      end
      if (_server !~ /^master/)
        # Be careful not to display netrc content. s.attempt logs all output!
        (ec, out) = s.attempt('grep -c "machine" /root/.netrc')
        out.rstrip!
        if (ec != 0 || out.to_i != entry_count)
          status.add_fail("Server #{_server} has wrong number of entries (#{out}, not #{entry_count}) in /root/.netrc")
          Log.logger.warn("Server #{_server} has wrong number of entries (#{out}, not #{entry_count}) in /root/.netrc")
        end
      end
    }
    return status
  end

  # checks to see that splunk is live and properly configured
  def splunk_up?(_server, _waittime = 600)
    splunk_up = true
    state = Acquia::FieldsStatus.new()
    ssh(_server){|s|
      Log.logger.info("Verifying splunkd running")
      cmd = 'ps -ef'
      # Make sure splunkd is running and as user splunk.
      pattern = '^splunk .+ splunkd -p 8089'
      if (!action_complete?(s, cmd, pattern, _waittime))
        state.add_fail('No splunkd')
      end
      Log.logger.info("Verifying splunk uid")
      # Puppet should have created the splunk user with uid 520.
      cmd = 'id -u splunk'
      pattern = '520'
      if (!action_complete?(s, cmd, pattern, _waittime))
        state.add_fail('Bad splunk uid')
      end
    }
    unless (state.passed)
      Log.logger.info("Problem with splunk configuration: #{state.get_fail_message}")
    end
    return state
  end

  # checks to see that puppet is live and properly configured
  def puppet_up?(_server, waittime = 1200)
    Log.logger.info("Checking that puppet is up on #{_server}")
    syslog = syslog_id(_server)
    Log.logger.info("examining #{syslog}")
    pingtime = 5
    lastloglines = 500
    #ubuntu is syslog, fedora is messages
    status = Acquia::FieldsStatus.new()
    ssh(_server){|s|
      Log.logger.info("Verifying puppetd in cron on #{_server}")
      cmd = 'crontab -l'
      pattern = 'puppetd'
      if (!action_complete?(s, cmd, pattern, waittime, pingtime))
        status.add_fail("No puppetd running on #{_server}")
      end
      Log.logger.info("Verifying puppetd conf on #{_server}")
      cmd = 'cat /etc/puppet/puppet.conf'
      #      pattern = "master.i.#{fields_stage}.f.e2a.us"
      pattern = self.puppetmaster_fqhn
      if (!action_complete?(s, cmd,pattern))
        status.add_fail("Bad puppet master config on #{_server}")
      end
      Log.logger.info("Verifying CA has been signed for #{_server}")
      # we need to transform server name to internal name
      internal_server = _server
      if (internal_server =~ /^master\.e\.(.*)$/)
        internal_server = "master.i.#{$1}"
        Log.logger.info("Internal servername #{internal_server}")
      end
      ssh(self.master_fqhn){|m|
        cmd = 'puppetca --list --all'
        pattern = '\+\s+' + internal_server
        if (!action_complete?(m, cmd, pattern, 60))
          status.add_fail("The puppet certificate was never signed for #{internal_server}")
        end
      }
      Log.logger.info("Verifying puppetd execution on #{_server}")
      cmd = "grep \'Finished catalog run\' /var/log/#{syslog}*"
      pattern = 'Finished catalog run'
      if (!action_complete?(s, cmd, pattern, waittime))
        status.add_fail("Puppetd never finished its configuration run on #{_server}")
        cmd ='tail -n ' + lastloglines.to_s + " /var/log/#{syslog}*"
        output = s.exec!(cmd)
        Log.logger.info("Last #{lastloglines.to_s} lines from /var/log/#{syslog}* were:\n" + output)
      end
    }
    if status.passed
      Log.logger.info("Puppet appears to have completed a run on #{_server}")
    else
      Log.logger.warn("Problem with puppet configuration: #{status.get_fail_message}")
    end
    return status
  end

  def clear_syslogs
    all_servers = self.fields_xmlrpc.call2('acquia.fields.list.local.servers', '%').values
    all_servers.each {|server|
      syslog = syslog_id(server['name'])
      ssh(server['external_fqhn']) {|remote|
        remote.exec!("cat /dev/null > /var/log/#{syslog}*")
      }
    }
  end

  # checks to see that the web server is live and more or less properly configured
  # looks for httpd process, vhost-main.conf, a curl ofthe local web server is happy
  # the web servers fields-config-web.php exists and is not 0 size
  # crontab has an entry for the web server
  def webserver_up?(_server)
    if (self.is_ubuntu)
      ws_string = 'apache2'
    else
      ws_string = 'httpd'
    end

    webserver_up = true
    state = Acquia::FieldsStatus.new()
    ssh(_server){|s|
      Log.logger.info("Verifying #{ws_string} process is running")
      cmd = 'ps -ef'
      pattern = "#{ws_string}"
      if (!action_complete?(s, cmd,pattern))
        state.add_fail("No #{ws_string} process")
      end
      Log.logger.info("Verifying #{ws_string} vhost-main default vhost config is installed")
      cmd = "cat /etc/#{ws_string}/conf.d/vhost-main.conf"
      pattern = 'ExtendedStatus.On'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail('Bad webserver vhost-main Virtual Host')
      end
      cmd = "cat /etc/#{ws_string}/conf.d/vhost-main.conf"
      pattern = '<Location.+\/server-status>'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail('Bad webserver vhost-main server-status location Directory directive')
      end
      Log.logger.info("Verifying #{ws_string} web server is answering")
      cmd = 'curl http://localhost/server-status'
      pattern = '<html>|<\/html>'
      if (!action_complete?(s, cmd, pattern))
        state.add_fail('Bad webserver ping')
      end
      Log.logger.info("Verifying #{ws_string} fields-config-web.php script is installed")
      config_bin_path = '/usr/local/sbin/'
      config_bin = 'fields-config-web.php'
      config_full_path = config_bin_path+config_bin
      cmd = 'test -f '+ config_full_path +' -a -s '+ config_full_path+' && echo '+ config_bin + ' script ok'
      pattern = config_bin + ' script ok'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail(config_bin +' not ok')
      end
      Log.logger.info("Verifying #{ws_string} fields-config-web.php crontab is installed")
      cmd = 'crontab -l'
      pattern = config_full_path.gsub(/\//, '\/')
      if (!action_complete?(s, cmd,pattern))
        state.add_fail(config_bin +' crontab not ok')
      end
    }
    unless (state.passed)
      Log.logger.warn("Problem with webserver configuration: #{state.get_fail_message}")
    end
    return state
  end

  # checks to see that the db server is live and more or less properly configured
  # looks for mysqld process, my.cnf, a curl ofthe local web server is happy
  # the web servers fields-config-web.php exists and is not 0 size
  # crontab has an entry for the web server
  def dbserver_up?(_server)
    if (self.is_ubuntu)
      my_cnf_path = '/etc/mysql/my.cnf'
    else
      my_cnf_path = '/etc/my.cnf'
    end

    server_up = true
    state = Acquia::FieldsStatus.new()
    ssh(_server){|s|
      Log.logger.info("Verifying mysqld running")
      cmd = 'ps -ef'
      pattern = 'mysqld'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail('No mysqld process')
      end
      Log.logger.info("Verifying mysqld my.cnf")
      cmd = 'test -f '+ my_cnf_path +' -a -s '+ my_cnf_path+' && echo '+ my_cnf_path + ' ok'
      pattern = my_cnf_path.gsub(/\//,'\/') + ' ok'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail('Bad dbserver my.cnf')
      end
      Log.logger.info("Verifying mysqld fields config php")
      config_bin_path = '/usr/local/sbin/'
      config_bin = 'fields-config-db.php'
      #      config_bin = 'fields-config-hosts.php'
      config_full_path = config_bin_path+config_bin
      cmd = 'test -f '+ config_full_path +' -a -s '+ config_full_path+' && echo '+ config_bin + ' script ok'
      pattern = config_bin + ' script ok'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail(config_bin +' not ok')
      end
      # We used to run fields-config-db.php here to make sure /root/.my.cnf was
      # set up before connecting to mysqld. However, puppet runs ah-set-db-root
      # now, and we've already verified puppet has succeeded.
      Log.logger.info("Verifying mysqld/puppet crontab")
      cmd = 'crontab -l'
      pattern = config_full_path.gsub(/\//, '\/')
      if (!action_complete?(s, cmd,pattern))
        state.add_fail(config_bin +' crontab not ok')
      end
      Log.logger.info("Verifying mysqld connection")
      cmd = 'mysql -e \'show databases;\''
      pattern = '\s+mysql\s+'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail('Cannot connect to mysqld')
      end
    }
    unless (state.passed)
      Log.logger.warn("Problem with dbserver configuration: #{state.get_fail_message}")
    end
    return state
  end

  # Starts the db process on _server.  This assumes that the process is mysqld
  # and can be started/stopped via /etc/init.d/mysqld start|stop and is a wrapper
  # around db_control sleeps are to hope the damn thing has shut down
  def start_db(_server, _why=nil)
    Log.logger.info("Starting DB on #{_server}: #{_why}")
    self.db_control(_server, 'start', _why)
    sleep 5
  end

  # Stops the db process on _server.  This assumes that the process is mysqld
  # and can be started/stopped via /etc/init.d/mysqld start|stop and is a wrapper
  # around db_control sleeps are to hope the damn thing has shut down
  def stop_db(_server, _why=nil)
    Log.logger.info("Stopping DB on #{_server}: #{_why}")
    self.db_control(_server, 'stop', _why)
    sleep 5
  end

  #restart the db on server _server, this explictly stops and starts rather than
  # /etc/initd/... restart becasue problems have been seen with restart behaving
  # oddly (empirically)
  def restart_db(_server, _why=nil)
    Log.logger.info("Restarting DB on #{_server}: #{_why}")
    self.stop_db(_server, _why)
    self.start_db(_server, _why)
  end

  # starts/stops _server based on _arg
  # currently valid states are start|stop
  def db_control(_server, _arg, _why=nil)
    if (self.is_ubuntu)
      init_script = 'mysql'
    else
      init_script = 'mysqld'
    end
    self.service_control(_server, init_script, _arg, _why)
  end

  # Stops the Apache server process on _server using the /etc/init.d script,
  # and waits up to 30 seconds for it to be gone.
  def stop_httpd(_server, _why=nil)
    Log.logger.info("Stopping httpd on #{_server}: #{_why}")
    self.httpd_control(_server, 'stop', _why)
    httpd = httpd_process_name(_server)
    ssh(_server) {|ss|
      ss.watch(30, 1, 'pgrep '+httpd+' >/dev/null; test "$?" = "1"')
    }
  end

  # Starts the httpd process on _server using the /etc/init.d script, and
  # waits up to 30 seconds for it to be running.
  def start_httpd(_server, _why=nil)
    Log.logger.info("Starting httpd on #{_server}: #{_why}")
    self.httpd_control(_server, 'start')
    httpd = httpd_process_name(_server)
    ssh(_server) {|ss|
      ss.watch(30, 1, 'pgrep '+httpd+' >/dev/null')
    }
  end

  def restart_httpd(_server, _why=nil)
    Log.logger.info("Restarting httpd on #{_server}: #{_why}")
    self.stop_httpd(_server, _why)
    self.start_httpd(_server, _why)
  end

  def httpd_control(_server, _arg, _why=nil)
    init_script = httpd_process_name(_server)
    self.service_control(_server, init_script, _arg, _why)
  end
  def httpd_process_name(_server)
    return 'apache2'
  end

  # Run a command on a remote server, printing debug messages,
  # and return exit code, stdout, and stderr
  def remote_exec(server, command, why = nil)
    exit_code = 0
    sout = ''
    serr = ''
    Log.logger.debug("Executing #{command} on #{server}: #{why}")
    ssh(server) do |s|
      s.run("logger -t ah-system-test -p daemon.notice 'executing #{command}: #{why}'")
      (exit_code, sout, serr) = s.attempt(command)
    end
    Log.logger.debug("exit: #{exit_code}, sout: #{sout}, serr: #{serr}")
    return exit_code, sout, serr
  end

  # Stops the puppet process on _server.
  #
  # Puppet now runs via cron, so this just locks the executable and stops the
  # process.
  def stop_puppet(_server, _why=nil)
    Log.logger.info("Stopping puppet on #{_server}: #{_why}")
    remote_exec(_server, "killall puppetd && /usr/local/bin/puppetd --disable", _why)
  end

  # Starts the puppet process on _server.
  #
  # Puppet now runs via cron, so this unlocks the executable and then runs it once.
  def start_puppet(_server, _why=nil)
    Log.logger.info("Starting puppet on #{_server}: #{_why}")
    remote_exec(_server, "/usr/local/bin/puppetd --enable", _why)
    remote_exec(_server, "nohup /usr/local/bin/puppetd --onetime --no-daemonize --logdest syslog > /dev/null 2>&1 &", _why)
  end

  def restart_puppet(_server, _why=nil)
    Log.logger.info("Restarting puppet on #{_server}: #{_why}")
    self.stop_puppet(_server, _why)
    self.start_puppet(_server, _why)
  end

  # Return the version of the codebase that the given server claims to be
  # running, based on the HOSTING_VERSION file that gets distributed by the
  # puppet manifest.
  def hosting_version_on_server(server)
    version = ''
    ssh(server) do |s|
      (ec, sout, serr) = s.attempt("cat /var/acquia/HOSTING_VERSION")
      version = sout.chomp
    end
    version
  end

  # general service control wrapper
  def service_control(_server, _service, _arg, _why=nil)
    cmd = '/etc/init.d/' + _service
    case _arg
    when /start/i
      arg = 'start'
    when /stop/
      arg = 'stop'
    else
      raise "#{_state} is not a valid state for #{_service} to switch to"
    end

    full_cmd  = cmd + ' ' + arg
    exit_code = 0
    sout=''
    serr=''
    Log.logger.debug("Executing #{full_cmd} on #{_server}: #{_why}")
    ssh(_server){|s|
      s.run("logger -t ah-system-test -p daemon.notice 'executing #{full_cmd}: #{_why}'")
      (exit_code,sout,serr) = s.attempt(full_cmd)
    }
    Log.logger.debug("exit: #{exit_code}, sout: #{sout}, serr: #{serr}")
    return exit_code,sout,serr
  end

  # checks to see that the db server is live and more or less properly configured
  # looks for mysqld process, my.cnf, a curl ofthe local web server is happy
  # the web servers fields-config-web.php exists and is not 0 size
  # crontab has an entry for the web server
  def svnserver_up?(_server)
    server_up = true
    state = Acquia::FieldsStatus.new()
    ssh(_server){|s|
      Log.logger.info("Verifying svn running")
      cmd = 'ps -ef'
      pattern = 'svnserv'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail('No svnserv process')
      end
      #      Log.logger.info("Verifying svn svnserve.conf")
      #      cnf_path = '/var/lib/svn/conf/svnserve.conf'
      #      cmd = 'test -f '+ cnf_path +' -a -s '+ cnf_path+' && echo '+ cnf_path + ' ok'
      #      pattern = cnf_path.gsub(/\//,'\/') + ' ok'
      #      if (!action_complete?(s, cmd,pattern))
      #        state.add_fail('Bad svn svnserve.conf')
      #      end
      Log.logger.info("Verifying svn connection")
      setupcmd = 'svnadmin create /var/lib/svn/test'
      # this command will fail if test repor already exists but that is ok
      s.exec!(setupcmd)
      cmd = 'svn info svn://localhost/test'
      pattern = 'Path:.+ test|URL:.+svn:\/\/localhost\/test'
      if (!action_complete?(s, cmd, pattern))
        state.add_fail('Cannot connect to svnserver')
      end
      Log.logger.info("Verifying svnserver fields config php")
      config_bin_path = '/usr/local/sbin/'
      config_bin = 'fields-config-svn.php'
      config_full_path = config_bin_path+config_bin
      cmd = 'test -f '+ config_full_path +' -a -s '+ config_full_path+' && echo '+ config_bin + ' script ok'
      pattern = config_bin + ' script ok'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail(config_bin +' not ok')
      end
      Log.logger.info("Verifying svn/puppet crontab")
      cmd = 'crontab -l'
      pattern = config_full_path.gsub(/\//, '\/')
      if (!action_complete?(s, cmd,pattern))
        state.add_fail(config_bin +' crontab not ok')
      end
    }
    unless (state.passed)
      Log.logger.info("Problem with svnserver configuration: #{state.get_fail_message}")
    end
    return state
  end

  def get_svn_users(_site_info)
    users = self.get_users_data(_site_info)
    d_type  = 'name'
    s_attr = Array.new
    users.each{|id,val|
      if (val[d_type])
        s_attr.push(val[d_type])
      end
    }
    if (0 == s_attr.size)
      raise "No attribute #{d_type} for user was found"
    end
    return s_attr
  end

  # for a given user, get the password
  def get_svn_pass(_site_info, _user)
    users = self.get_users_data(_site_info)
    d_type  = 'pass'
    s_attr = ''
    users.each{|id,val|
      if (val['name'] == _user)
        if (val[d_type])
          s_attr = val[d_type]
        end
      end
    }
    if (0 == s_attr.size)
      raise "No attribute #{d_type} for user #{_user} was found"
    end
    return s_attr
  end

  # Add an SVN user to the hosting master database
  def add_svn_user(_sitename=site, _user=nil, _pass=nil)
    site_info  = self.get_site_info(self.fields_xmlrpc, _sitename)
    unless (_pass)
      _pass = 'D2bESEPTW!'
    end
    data = {'site_name' => _sitename, 'name' => _user, 'pass' => _pass}
    _client = self.fields_xmlrpc
    user_id  = _client.call2('acquia.fields.set.site_user', data)
    site_info  = self.get_site_info(self.fields_xmlrpc, _sitename)
    self.svn_password_set?(_sitename, _user, _pass)
    users = get_users_data(site_info)
    return users
  end

  # returns the info from a fields site by name
  def set_svn_pass(_sitename=site, _user=nil, _pass=nil)
    site_info  = self.get_site_info(self.fields_xmlrpc, _sitename)
    unless (_pass)
      _pass = 'D2bESEPTW!'
    end
    data = {'site_name' => _sitename, 'name' => _user, 'pass' => _pass}
    _client = fields_xmlrpc
    status  = _client.call2('acquia.fields.chpass.site_user', data)
    site_info  = self.get_site_info(self.fields_xmlrpc, _sitename)
    self.svn_password_set?(_sitename, _user, _pass)
    users = get_users_data(site_info)
    return users
  end

  # ensures that the passwrod is set by trying to access svn
  def svn_password_set?(_sitename, _user, _pass)
    site_info  = self.get_site_info(self.fields_xmlrpc, _sitename)
    svn = SvnCommand.new
    svn.user = _user
    svn.password = _pass
    svn.url = site_info['svn']['internal']
    svn_servers = self.get_svn_servers(site_info)

    svn_servers.each{|server|
      ssh(server){|ss|
        Log.logger.info("Verifying password set for user #{_user} on svn server #{server}")
        # Force the password through.
        ss.attempt('/usr/local/sbin/fields-config-svn.php')
        puts "cmd = #{svn.info} | grep -q #{svn.url}"
        ss.watch(120,1,"#{svn.info} | grep -q #{svn.url}", true)
      }
    }
  end

  def get_svn_url(_site_info)
    url = Array.new
    url = get_repo_data(_site_info, 'svn', 'external')
    return url[0]
  end
  # checks to see that the db server is live and more or less properly configured
  # looks for mysqld process, my.cnf, a curl ofthe local web server is happy
  # the web servers fields-config-web.php exists and is not 0 size
  # crontab has an entry for the web server
  def balancer_up?(_server)
    server_up = true
    state = Acquia::FieldsStatus.new()
    ssh(_server){|s|
      Log.logger.info("Verifying balancer running")
      cmd = 'ps -ef'
      pattern = 'nginx'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail('No nginx process')
      end
      Log.logger.info("Verifying nginx ping")
      cmd = 'curl -I http://localhost:80'
      pattern = 'Server:.+nginx'
      if (!action_complete?(s, cmd, pattern))
        state.add_fail('Cannot verify nginx is serving content')
      end
      Log.logger.info("Verifying balancer fields config php")
      config_bin_path = '/usr/local/sbin/'
      config_bin = 'fields-config-bal.php'
      config_full_path = config_bin_path+config_bin
      cmd = 'test -f '+ config_full_path +' -a -s '+ config_full_path+' && echo '+ config_bin + ' script ok'
      pattern = config_bin + ' script ok'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail(config_bin +' not ok')
      end
      Log.logger.info("Verifying nginx/puppet crontab")
      cmd = 'crontab -l'
      pattern = config_full_path.gsub(/\//, '\/')
      if (!action_complete?(s, cmd,pattern))
        state.add_fail(config_bin +' crontab not ok')
      end
    }
    unless (state.passed)
      Log.logger.warn("Problem with svnserver configuration: #{state.get_fail_message}")
    end
    return state
  end

  # this will look at the fields-site.conf and determine if the conf looks
  # reasonable.  this depends on the internal names and fqhns
  def balancer_conf_ok?(_site_info)
    site_name = self.get_site_name(_site_info)
    site_id = self.get_site_id(_site_info)
    site_port = self.get_site_vhost_port(_site_info)
    fields_conf_path = "/etc/nginx/conf.d/#{site_name}-#{site_id}.conf"
    pub_host_domain = self.sites_domain
    balancer_ok = true
    state = Acquia::FieldsStatus.new()
    balancers = self.get_balancers(_site_info)
    web_names = self.get_web_servers_name(_site_info)
    web_internal_ips = self.get_web_servers_internal_ip(_site_info)
    balancers.each{|balancer|
      cmd  = "test -f #{fields_conf_path}"
      pattern = "#{site_name}-#{site_id}.conf"
      waittime = 600
      real_conf = ''
      ssh(balancer){|s|
        begin
          s.watch(waittime, 5, cmd, false)
        rescue
          state.add_fail("The sites balancer config #{fields_conf_path} was not written in #{waittime} minutes")
        end
        (ec, sout, serr) = s.attempt("cat #{fields_conf_path}")
        real_conf = sout
      }
      # check that upstream looks "ok"
      if (real_conf !~/upstream.+#{site_name}/)
        Log.logger.warn("Balancer #{balancer} upstream is misconfigured")
        state.add_fail("Balancer #{balancer} upstream is misconfigured")

      end
      #check that the map from server to ip looks ok
      web_names.each_index{|i|
        if (real_conf !~/server.+#{web_internal_ips[i]}:#{site_port}.+;.+#{web_names[i]}/)
          msg = "Balancer #{balancer} ip address map is misconfigured: missing #{web_internal_ips[i]}:#{site_port} for #{web_names[i]}"
          Log.logger.warn(msg)
          state.add_fail(msg)
        end
      }
      #check that we have a "server_name" that looks good
      pattern = "server_name.+#{site_name}.#{self.fields_stage}.#{pub_host_domain}"
      if (real_conf !~/#{pattern}/)
        Log.logger.warn("Balancer #{balancer} server name is misconfigured")
        state.add_fail("Balancer #{balancer} server name is misconfigured does not contain " + pattern )
      end

      # check that ssl seems "ok" this will miss a misconfigured server_name becasue it does not
      # parse the stanza

      #      listen 443;
      #  ssl    on;
      #  ssl_certificate    /etc/ssl/certs/acquia-hosting.crt;
      #  ssl_certificate_key    /etc/ssl/private/acquia-hosting.key;
      ssl_port = '443'
      pattern = "listen.+#{ssl_port}"
      if (real_conf !~/#{pattern}/)
        Log.logger.warn("Balancer #{balancer} is not configured to listen on SSL #{ssl_port}")
        state.add_fail("Balancer #{balancer} ssl port is misconfigured does not contain " + pattern )
      end

      pattern = "ssl.+on"
      if (real_conf !~/#{pattern}/)
        Log.logger.warn("Balancer #{balancer} does not have ssl on")
        state.add_fail("Balancer #{balancer} is not configured to use ssl")
      end
      cert_path  = '/etc/ssl/certs/acquia-sites_com.pem'
      priv_key_path  = '/etc/ssl/private/acquia-sites_com.key'
      pattern = "ssl_certificate.+#{cert_path}"
      if (real_conf !~/#{pattern}/)
        Log.logger.warn("Balancer #{balancer} ssl cert path is misconfigured")
        state.add_fail("Balancer #{balancer} ssl_certificate is misconfigured does not contain " + pattern )
      end
      pattern = "ssl_certificate_key.+#{priv_key_path}"
      if (real_conf !~/#{pattern}/)
        Log.logger.warn("Balancer #{balancer} private key is misconfigured")
        state.add_fail("Balancer #{balancer} ssl_certificate_key is misconfigured does not contain " + pattern )
      end

      # verify that the keys are in place
      ssh(balancer){|s|
        (ec, sout, serr) = s.attempt("cat #{cert_path}")
        cert = sout
        if (cert.size == 0 )
          Log.logger.warn("Balancer #{balancer} has no SSL cert at #{cert_path}")
          state.add_fail("Balancer #{balancer} No SSL cert was found at #{cert_path}")
        end
        (ec, sout, serr) = s.attempt("cat #{priv_key_path}")
        priv_key = sout
        if (priv_key.size == 0 )
          Log.logger.warn("Balancer #{balancer} has no SSL key at #{priv_key_path}")
          state.add_fail("Balancer #{balancer} No SLL private key was found at #{priv_key_path}")
        end
      }

    }
    unless(state.passed)
      Log.logger.warn("Problem with balancer configuration: #{state.get_fail_message}")
    end
    return state
  end

  # HEADER: This file was autogenerated at Wed Jul 29 08:07:28 -0400 2009 by puppet.
  # HEADER: While it can still be managed manually, it is definitely not recommended.
  # HEADER: Note particularly that the comments starting with 'Puppet Name' should
  # HEADER: not be deleted, as doing so could cause duplicate cron jobs.
  # Puppet Name: master-drupal-cron
  # */5 * * * * wget -O - -q http://#{master.i.smoke-buildbot.f.e2a.us}/cron.php
  # Puppet Name: fields-backup-hourly
  # 0 * * * * /usr/local/bin/backup-snapshot hourly 2>&1 1>/dev/null | grep -v INFO | logger -t BACKUP
  # Puppet Name: fields-config-hosts
  # * * * * * /usr/bin/php /usr/local/sbin/fields-config-hosts.php
  # Puppet Name: fields-backup
  # PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin
  # 15,30,45 * * * * /usr/local/bin/backup-snapshot 2>&1 1>/dev/null | grep -v INFO | logger -t BACKUP

  # the master needs to have puppet, puppetmaster,  the above crontab entries,
  # no errors in the syslog, a DB that is up and at least has the fields_master DB
  # successful XML RPC communication
  #
  def master_up?(_server)
    status = Acquia::FieldsStatus.new
    p_state = self.puppet_up?(_server)
    if (!p_state.passed)
      status.add_fail("Master puppetd is not up or configured correctly\n" + p_state.get_fail_message)
    end

    pm_state = self.puppetmaster_up?(_server)
    if (!pm_state.passed)
      status.add_fail("Master puppet master is not up or configured correctly\n" + pm_state.get_fail_message)
    end

    mweb_state = self.master_webserver_up?(_server)
    if (!mweb_state.passed)
      status.add_fail("Master web server is not up and serving data\n" + mweb_state.get_fail_message)
    end

    xml_rpc_state = self.xml_rpc_up?
    if (!xml_rpc_state.passed)
      status.add_fail("XML-RPC communication seems to be down\n" + xml_rpc_state.get_fail_message)
    end
    mdb_state = self.master_db_up?(_server)
    if (!mdb_state.passed)
      status.add_fail("Master DB is not up and running and partially happy\n" + mdb_state.get_fail_message)
    end

    log_state = self.log_has_errors?(_server, ['master_php_error_log'])
    if (!log_state.passed)
      status.add_fail("Master has errors in the syslog\n" + log_state.get_fail_message)
    end
    unless(status.passed)
      Log.logger.warn("Problem with master: #{status.get_fail_message}")
    end
    return status
  end

  def master_db_up?(_server)
    db_up = true
    state = Acquia::FieldsStatus.new
    ssh(_server){|s|
      Log.logger.info("Verifying mysqld running")
      cmd = 'ps -ef'
      pattern = 'mysqld'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail('No mysqld process')
      end
      Log.logger.info("Verifying mysqld my.cnf")
      my_cnf_path = '/etc/mysql/my.cnf'
      cmd = 'test -f '+ my_cnf_path +' -a -s '+ my_cnf_path+' && echo '+ my_cnf_path + ' ok'
      pattern = my_cnf_path.gsub(/\//,'\/') + ' ok'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail('Bad dbserver my.cnf')
      end
      Log.logger.info("Verifying mysqld connection")
      cmd = 'mysqlshow -u root fields_master'
      pattern = 'acquia_fields'
      if (!action_complete?(s, cmd,pattern,20))
        state.add_fail('Cannot connect to mysqld')
      end
    }
    unless (state.passed)
      Log.logger.warn("Master DB has a problem: #{state.get_fail_message}")
    end
    return state
  end

  # quick and dirty test of the xmlrpc.  IF the connection fails, and exception is thrown
  # and serverinfo is nil otherwise server info is just empty
  def xml_rpc_up?
    state = Acquia::FieldsStatus.new
    begin
      serverinfo = self.fields_xmlrpc.call2('acquia.fields.get.server.info',0)
    rescue
      Log.logger.warn("Could not successfully connect to XMLRPC")
      state.add_fail("Could not successfully connect to XMLRPC")
    ensure
      if(!serverinfo.nil?)
        state.passed = true
      end
    end
    return state
  end

  def master_webserver_up?(_server)
    ws_string = 'apache2'
    state = Acquia::FieldsStatus.new
    ssh(_server){|s|
      Log.logger.info("Verifying httpd running")
      cmd = 'ps -ef'
      pattern = "#{ws_string}"
      if (!action_complete?(s, cmd,pattern))
        state.add_fail("No #{ws_string} process")
      end
      Log.logger.info("Verifying httpd app.conf")
      cmd = "cat /etc/#{ws_string}/conf.d/app.conf"
      pattern = 'VirtualHost.*:443'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail('Bad webserver app.conf Virtual Host')
      end
      cmd = "cat /etc/#{ws_string}/conf.d/app.conf"
      pattern = 'DocumentRoot.+\/fields-master'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail('Bad webserver app.conf Doc root')
      end
      cmd = "cat /etc/#{ws_string}/conf.d/app.conf"
      pattern = 'AuthName.+xmlrpc'
      if (!action_complete?(s, cmd,pattern))
        state.add_fail('Bad webserver: app.conf,  AuthName "xmlrpc" coudl not be found')
      end

    }
    unless (state.passed)
      Log.logger.info("Problem with webserver configuration: #{state.get_fail_message}")
    end
    return state
  end

  # checks to see that puppet is live
  def puppetmaster_up?(_server, waittime = 600)
    if (_server =~ /master/)
      Log.logger.info("examining messages")
      syslog = 'messages'
    else
      Log.logger.info("examining syslog")
      syslog = 'syslog'
    end
    pingtime = 5
    lastloglines = 500
    #ubuntu is syslog, fedora is messages
    status = Acquia::FieldsStatus.new()
    ssh(_server){|s|
      Log.logger.info("Verifying Passenger running")
      cmd = 'ps -ef'
      pattern = 'passenger'
      if (!action_complete?(s, cmd, pattern, waittime, pingtime))
        status.add_fail("Passenger should be running but is not")
      end
    }
    unless (status.passed)
      Log.logger.warn("Problem with puppetmaster configuration: #{status.get_fail_message}")
    end
    return status
  end


  # verify's that some command checkable via a string regex completes in
  # the allotted time
  # s = ssh session
  # _cmd  = command to execute
  # _pattern = string of pattern to match
  # returns a boolean true if executed successfully in the timout window
  # will wait for 10 minutes by default
  def action_complete?(s, _cmd, _pattern, _waittime=600, _pingtime=5)
    start = Time.now()
    timeout = start + _waittime
    action_done = false
    action_passed = false
    while (!action_done)
      output = s.exec!(_cmd)
      Log.logger.debug output
      if (output =~ /#{_pattern}/)
        action_done = true
        action_passed = true
      end
      if (!action_done)
        sleep(_pingtime)
      end
      if (timeout < Time.now())
        action_passed = false
        action_done = true
      end
    end
    return action_passed
  end

  # Modify the Acquia::SSH#ssh_connect method to retry on failure.
  module Acquia::SSH
    alias_method :real_ssh_connect, :ssh_connect

    def ssh_connect(host, user, params, &block)
      max_tries = 3
      waittime = 5
      t = 0
      begin
        t += 1
        result = real_ssh_connect(host, user, params, &block)
        return result
      rescue RuntimeError, Timeout::Error => ssh_e
        Log.logger.debug("SSH Attempt #{t} failed, trying again in #{waittime} seconds")
        if (t < max_tries)
          sleep waittime
          retry
        else
          Log.logger.warn("SSH command failed after #{t} tries.  The last error was #{ssh_e.class}")
          Log.logger.warn("The last message was #{ssh_e.message}")
          raise ssh_e
        end
      end
    end

  end
end
