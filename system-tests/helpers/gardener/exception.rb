# does nothing but have somewhat more desriptive exception names
  class InvalidSiteNameError < StandardError
    def initialize
      super
    end
  end
