class Acquia::GardenerStatus
  
  attr_accessor :passed
  
  def initialize()
    @passed = true
    @fails = Hash.new()
    @fails_ord = Array.new() #something to preserve the order if I need it.
  end
  
  # adds a fail and increments a count if that exact fail was seen before
  # will append to the ordered array if that fail is new
  def add_fail(_message)
    @passed = false
    if(!@fails.has_key?(_message))
      @fails[_message] = 1
      @fails_ord.push(_message)
    else
      @fails[_message] += 1
    end  
  end
  
  def get_fail_message
    if (@passed and @fails.size == 0)
      return "There are no failure messages and the status is pass"
    end
    out_string = "Gardener Test status: \n"
    if (@fails.size !=0)
      @fails_ord.each{|key|
        reason  = key
        count = @fails[key]
        if (count > 1)
          out_string += "#{key} occured #{count.to_s} times and caused a test failure\n"
        else
          out_string += "#{key}: caused a test failure\n"
        end
      }
      return out_string
    end
  end
  
end