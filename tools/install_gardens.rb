#!/usr/bin/ruby
stage = ARGV[0]
template = ARGV[1]

i = {
  "trunk" => {"db" => "gardens_trunk", "url" => "http://gardens.trunk", "path" => "~/work/engineering/gardens/trunk"},
  "prod" => {"db" => "gardens_prod", "url" => "http://gardens.prod", "path" => "~/work/engineering/gardens/branches/prod"}
}[stage]

`mysqladmin -f drop #{i['db']}; mysqladmin create #{i['db']}`;

if template
  puts "Installing with #{template}"
  template_str="site_template=#{template}"
end

`cd #{i['path']} && php -d memory_limit=64M \`pwd\`/install_gardens.php database="#{i['db']}" username="root" password="root" profile="gardens" url="#{i['url']}" #{template_str}`
puts "cd #{i['path']} && php -d memory_limit=64M \`pwd\`/install_gardens.php database=\"#{i['db']}\" username=\"root\" password=\"root\" profile=\"gardens\" url=\"#{i['url']}\" #{template_str}"

