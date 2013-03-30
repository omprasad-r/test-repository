require 'rubygems'
require 'net/netrc'

if ARGV.size < 1
  puts "Usage: get_netrc_creds cred_stanza [login|password]
  returns the login or password for a particular login stanza.
  no option returns the full stanza
  e.g. svn.acquia.com.client
  example:
  get_netrc_creds svn.acquia.com.client login 
  returns the login "
end

machine = ARGV.shift

if ARGV
  attribute = ARGV.shift
end

rc = Net::Netrc.locate(machine)

case attribute
  when "login"
    print rc.login
  when "password"
    print rc.password
  else
    print "machine: #{rc.machine}\n"
    print "login: #{rc.login}\n"
    print "password: #{rc.password}\n"
end
