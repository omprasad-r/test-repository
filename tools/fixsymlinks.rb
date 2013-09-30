site = ARGV[0]
env = ARGV[1]
gardens_sites_dir = ARGV[2]

result = []
Dir.foreach(gardens_sites_dir) do |filename|
  # sanity check: this should be g12345
  next unless filename.match('^g[0-9]+$')
  gardens_site_id = filename
  cmd = "sudo -u #{site} drush5 --root=../docroot --include=. fix-symlinks #{site} #{env} #{gardens_site_id}"
  puts cmd
  output = `#{cmd}`
  unless $?.exitstatus == 0
    puts output
    puts ''
    result.push(gardens_site_id)
  end
end

puts "The following nodes failed to set their symlinks:"
puts result.join("\n")
