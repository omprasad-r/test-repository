
module Test000040FontSystemHelper
  # returns a 2 sig fig 0.1 - 1.0 random value
  def rand_width
    srand()
    width = rand(90).to_f / 100 + 0.1
    return width
  end
end
