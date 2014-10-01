require "rubygems"
require "node_builder.rb"

module Test000020LayoutSystemHelper
  # wrapper to create content in d7 since there is no robust devel module on gardens as far as a I know
  def create_content(_browser, _types, _count)
    nb = NodeBuilder.new(_browser)
    # add some content
    _count.times{
      _types.each{|t|
        nb.add_node(t)
      }
    }
  end
end
