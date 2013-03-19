require "rubygems"
require "acquia_qa/log"
require "selenium/webdriver"
require 'acquia_qa/ssh'
require 'tempfile'
require 'acquia_qa/qaapi'
require 'faker'
require 'acquia_qa/util'

# This module contains functionality to use in testing the ecommerce features on drupalgardens.com.

module GardenerEcommerce

  # Provides methods to extract information from Zuora via the gardner XML-RPC
  # Also contains some meta-data constants like the product skus.
  
  class ZuoraInformation
    attr_reader :product_skus
    def initialize(gardener)
      @gardener = gardener
      @product_skus = {
        'basic_monthly_sku' => 'DG4010-1m',
        'professional_monthly_sku' => 'DG4020-1m',
        'professional_annual_sku' => 'DG4020-1y'
      }
    end

    def get_account_for_user(uid)
      @gardener.gardener_xmlrpc.call('acquia.gardens.get_zuora_account_for_user', uid)
    end

    def get_subscription_for_node(nid)
      @gardener.gardener_xmlrpc.call('acquia.gardens.get_zuora_subscription_for_node', nid)
    end

    def zuora_query(type, where)
      @gardener.gardener_xmlrpc.call('acquia.gardens.zuora_query', type, where)
    end

    def zuora_query_raw(zsql)
      Log.logger.info("Calling acquia.gardens.zuora_query_raw: #{zsql}")
      @gardener.gardener_xmlrpc.call('acquia.gardens.zuora_query_raw', zsql)
    end

  end

  # Contains methods to drive the purchase process

  class SubscriptionUpgrader
    include Acquia::TestingUtil
    attr_reader :site, :gardener

    def initialize(_browser, gardener,_url=nil)
      @gardener = gardener
      @browser = _browser
      @pmgui = PaymentMethodGuiMap.new
      @sut_url = _url || $config['sut_url']
    end

    # Logs in as an administrator and updates the site's subscription level.
    def change_subscription_type_as_admin(site_nid, new_level, subsku, browser = @browser)
      wait = Selenium::WebDriver::Wait.new(:timeout => 15)
      @gardener.login_as_admin(browser)
      browser.get("#{@sut_url}node/#{site_nid}/edit")
      wait.until { browser.find_element(:xpath => "//select[@id='edit-field-subscription-product-nid-nid']") }.find_elements(:xpath => '//option').each { |e| 
        next unless e.text == new_level; e.click ; break ; 
      }
      ## This shouldn't be needed, but the field is there and required.
      temp = browser.find_element(:xpath => "//input[@id='edit-field-db-cluster-id-0-value']")
      temp.clear
      temp.send_keys("42")
      temp = @browser.find_element(:xpath => @pmgui.CreditCardCity)
      temp.clear
      temp.send_keys(generate_random_string + 'town')
      @browser.find_element(:css => @pmgui.CreditCardCountry).click
      @browser.find_element(:xpath => "//dd[@value='United States']").click
      wait.until { @browser.find_element(:css => @pmgui.CreditCardState) }.click
      # I don't know why we need to type in the value AND click the fake dropdown
      # but it seems to be required, and fixing this test has already cost too much
      # time to figure that out.
      @browser.find_element(:xpath => "//dd[@value='MA']").click
      temp = @browser.find_element(:id => "edit-CreditCardState")
      temp.clear
      temp.send_keys('MA')
      # @todo: test that the state select box goes away if we change to a non-North America country
      # and that it comes back if we select it again.
      temp = @browser.find_element(:xpath => @pmgui.CreditCardPostalCode)
      temp.clear
      temp.send_keys('01060')
      temp = @browser.find_element(:xpath => @pmgui.CreditCardHolderName)
      temp.clear
      temp.send_keys(generate_random_string)
      temp = @browser.find_element(:xpath => @pmgui.CreditCardNumber)
      temp.clear
      temp.send_keys('4111111111111111')
      @browser.find_element(:id => @pmgui.CreditCardExpirationMonthID).find_elements(:xpath => '//option').each { |e| next unless e.text == '01' ; e.click ; break ; }
      @browser.find_element(:id => @pmgui.CreditCardExpirationYearID).find_elements(:xpath => '//option').each { |e| next unless e.text == '2016' ; e.click ; break ; }
      temp = @browser.find_element(:xpath => @pmgui.CreditCardSecurityCode)
      temp.clear
      temp.send_keys('123')
      Log.logger.info("Submiting payment data")
      wait.until { @browser.find_element(:xpath => "//button[@id='edit-submit']") }.click
    end

    # Determines if a given upgrade is available on the purchase page.
    def upgrade_available?(user_info, nid, sku)
      @browser.get(@sut_url)
      @gardener.logout
      @gardener.login(user_info.login, user_info.password);
      @browser.get("#{@sut_url}purchase/#{nid}")
      return !@browser.find_elements(:xpath => "//a[contains(@href, '#{sku}')]").empty?
    end
  end
  
  class PaymentMethodGuiMap
    attr_reader :CreditCardAddress1, :CreditCardCity, :CreditCardStateCSS, :CreditCardCountryCSS,
      :CreditCardPostalCode, :CreditCardHolderName, :CreditCardNumber, :CreditCardExpirationMonthID,
      :CreditCardExpirationYearID, :CreditCardSecurityCode

    def initialize()
      @CreditCardAddress1 = "//input[@id='edit-CreditCardAddress1']"
      @CreditCardCity = "//input[@id='edit-CreditCardCity']"
      @CreditCardStateCSS = "#edit-us-wrapper dl.enhanced-select"
      @CreditCardCountryCSS = '#edit-CreditCardCountry-wrapper dl.enhanced-select'
      @CreditCardPostalCode = "//input[@id='edit-CreditCardPostalCode']"
      @CreditCardHolderName = "//input[@id='edit-CreditCardHolderName']"
      @CreditCardNumber = "//input[@id='edit-CreditCardNumber']"
      @CreditCardExpirationMonthID = 'edit-CreditCardExpirationMonth'
      @CreditCardExpirationYearID = 'edit-CreditCardExpirationYear'
      @CreditCardSecurityCode = "//input[@id='edit-CreditCardSecurityCode']"
    end
  end
end
