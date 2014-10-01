class JQuery
  def self.wait_for_events_to_finish(browser)
    #wait for jQuery to be loaded
    #Log.logger.debug("Waiting for jQuery to be present and ready")
    wait = Selenium::WebDriver::Wait.new(:timeout => 30)
    retry_counter = 0
    script = JQuery.script
    begin
      wait.until { browser.execute_script(script) }
    rescue Exception => e
      Log.logger.info("Error while waiting for jquery: #{e.message}")
      sleep 2
      retry_counter += 1
      if retry_counter < 5
        retry
      else
        raise "Even after several retries: Error while waiting for jquery to finish work: #{e.message}"
      end
    end
    #wait for jQuery to be done doing its thing
  end
  
  private

  def self.script
    script = <<JQUERY
      if (typeof jQuery != 'undefined') {
         return (jQuery.active == 0);
      }
      return false;
JQUERY
    return script
  end
end
