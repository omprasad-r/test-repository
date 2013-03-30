# really stupid script to force stats to be run
#
# Note: This script does not work on white label Gardens!
require 'itlib/ssh'
include Acquia::SSH

ARGV.each { |site|
  puts "forcing stats on #{site}"
  ssh("managed-47.#{ENV['FIELDS_STAGE']}.hosting.acquia.com"){|s|
    result = s.exec! "cd /mnt/www/html/tangle001/docroot && ../gardens-drush -y --uri=http://#{site}.#{ENV['FIELDS_STAGE']}.acquia-sites.com vset gardens_stats_time 0"
    puts result
    result = s.exec! "cd /mnt/www/html/tangle001/docroot && ../gardens-drush --uri=http://#{site}.#{ENV['FIELDS_STAGE']}.acquia-sites.com et-phone-home"
    puts result
    result = s.exec! "cd /mnt/www/html/tangle001/docroot && ../gardens-drush --uri=http://#{site}.#{ENV['FIELDS_STAGE']}.acquia-sites.com cron"
    puts result
  }
}
ssh("web-41.#{ENV['FIELDS_STAGE']}.hosting.acquia.com"){|s|
  result = s.exec!"cd /mnt/www/html/gardener/docroot && drush --uri-http://gardener.#{ENV['FIELDS_STAGE']}.acquia-sites.com cron"
  puts result
}
