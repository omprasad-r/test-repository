#!/usr/bin/env ruby

require 'rubygems'
require 'nokogiri'

# Usage: ./saucelabs_failure_counter.rb $folder

abort "Usage: #{$0} /path/to/log/folder" if ARGV.empty?

path = ARGV.first
error_cases = [
  /wrong status line/,
  /Connection reset by peer/,
  /Transport endpoint is not connected/
]

all_test_count = 0
failed_test_count = 0
aggregated_seconds = 0

Dir["#{path}/**/junitResult.xml"].each do |file|
  document = Nokogiri::XML(File.open(file));
  all_test_count += document.xpath('//case').size

  document.xpath('//case[child::errorStackTrace]').each do |match|
    errorStackTrace = match.xpath('./errorStackTrace').first.content

    if error_cases.find_all { |error_case| errorStackTrace =~ error_case}.length > 0
      failed_test_count += 1
      duration = match.xpath('./duration').first.content.to_f
      aggregated_seconds += duration
      puts "Found error in #{file} duration: #{duration}"
    end

  end

end

def round(number)
  (number * 100).round / 100.0
end

aggregated_minutes = Integer(aggregated_seconds / 60)
percentage_failed_tests = round(failed_test_count * 100 / all_test_count.to_f)

puts "Aggregated minutes: #{aggregated_minutes}"
puts "Failed tests: #{failed_test_count} of #{all_test_count} (#{percentage_failed_tests}%)" 
