#!/usr/bin/ruby

# Note: This script does not work on white label Gardens!

def run_cmd_on_server(server, cmd)
  return `ssh root@#{server} '#{cmd}'`
end

# update servers
update_servers = nil
if (ARGV[0])
  update_servers = ARGV[0].split(',');
end

unless update_servers.kind_of?(Array)
  puts "Invalid server list"
  exit
end

puts "Not Ready / Errors / Total"
update_servers.each {|num|
  server_name = "managed-#{num}.gardens.hosting.acquia.com"
  cmd = 'log=`find /var/log/drush  -maxdepth 1 -type f -regex ".*[0-9]+\.log" | xargs ls -1t| head -n1`; if [ -f $log ]; then not_ready=`grep ready $log | wc -l`;error=`grep error $log | wc -l`; total=`cat $log | wc -l`; echo "$not_ready / $error / $total"; fi;'
  puts "#{server_name}:"
  puts run_cmd_on_server(server_name, cmd);

}
