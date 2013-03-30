#!/usr/bin/ruby

require 'net/http'
require 'rss/1.0'
require 'rss/2.0'
require 'open-uri'
require 'singleton'
require 'rubygems'
require 'hpricot'

def usage
  puts "usage: patchCheck.rb path/to/PATCHES.txt DATE_SINCE"
  exit
end

patches_txt = ARGV[0]
unless patches_txt
  usage
end

if ARGV[1]
  start_timestamp = Time.local(*ParseDate.parsedate(ARGV[1])).to_i
end

unless start_timestamp
  usage
end

puts start_timestamp

is_core_patch = false;
DrupalCvsLog = 'http://drupal.org/cvs?rss=true&nid=3060'

def parse_patch(line)
  match_data = /[-]? ([A-Z]{0,3}) ([0-9\/]+):(.*)http:\/\/drupal.org\/node\/([0-9]+)(?:#comment-([0-9]+))?[^\s]+/.match(line)
  unless match_data.nil?
    patch_info = {"patcher" => match_data[1], "message" => match_data[2], "date" => match_data[3], "nid" => match_data[4], "cid" => match_data[5]}
    return patch_info
  end
end

def get_comments(nid)
  issue_page = get_issue(nid)
  
  comments = [];
  doc= Hpricot(issue_page)
  
  # Note that the extra space here is intentional because drupal.org has a funny class name
  comment_nodes = doc.search("div.comment")
  comment_nodes.each{|n|
    has_patch = false;
    has_commit = false;
    author_node = n.search('div.submitted em')
    match_data = /(.*) at (.*)<\/em>$/.match(author_node.to_html())
    date = match_data[1]
    if /\[\[SimpleTest\]\]/.match(n.to_html())
      has_patch = true
    end
    if /fixed/.match(n.search('div.summary').to_html())
      has_commit = true
    end
    comments << {"date" => Time.local(*ParseDate.parsedate(date)).to_i, "content" =>  n.to_html(), 'has_patch' => has_patch, 'has_commit' => has_commit}
  }
  throw :cannotGetInfoError if !comments
  return comments
end

class CVSLogChecker
  include Singleton
  
  def initialize
    source = DrupalCvsLog # url or local file
    content = "" # raw content of rss feed will be loaded here
    open(source) do |s| content = s.read end
    @rss_parsed = RSS::Parser.parse(content, false)
  end
  
  def is_committed?(nid)
    @rss_parsed.items.each() {|i|
      if /#{nid}/.match(i.description)
        return true
      end
    }
    return false
  end
  
end

def get_issue (nid)
  url = URI.parse('http://drupal.org/node/' + nid)
  req = Net::HTTP::Get.new(url.path)
  res = Net::HTTP.start(url.host, url.port) {|http|
    http.request(req)
  }
  return res.body
end


# Loops throw the lines, finds the section of the file which is for core patches
File.open(patches_txt, 'r') { |fp|
  while line = fp.gets
    if (is_core_patch)
      # Get the author, the d.o. nid and cid
      patch_info = parse_patch(line)
      unless patch_info.nil?
        nid = patch_info['nid']
        cid = patch_info['cid']
        # Only show interesting items
        header = ''
        header << "\n\n"
        header << line
        header << "--------------------"
        
        if nid
          new_comments = 0
          new_patches = 0
          has_commit = false
          # Checks the status of the patch
          comments = get_comments(nid)
          
          comments.each{|c|
            if c['date'] > start_timestamp
              new_comments += 1
              if (c['has_patch'])
                new_patches += 1
              end
              if (c['has_commit'])
                has_commit = true;
              end
            end
          }
          
          header << "node: #{nid} comment: #{cid}"
          if cid
            if new_comments > 0
              puts header
              puts "* #{new_comments.to_s} new comments available on the issue *"
              if new_patches > 0
                puts "*** #{new_patches.to_s} new patches available on the issue ***"
              end
              if has_commit
                puts "*** Probably COMMITTED to core! ***"
              end
            end
          end
          
          if CVSLogChecker.instance.is_committed?(nid)
            puts "*** COMMITTED to core! ***"
          end
          
        end
      end
    end
    
    if line == "The following patches have been applied on top of Drupal 7 core:\n"
      puts "start of core section"
      is_core_patch = true;
    end
    
    if line == "The following patches have been applied on top of Drupal 7 contributed modules:\n"
      puts "end core section"
      exit
    end
  end
}
