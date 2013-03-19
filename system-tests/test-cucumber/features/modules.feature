Feature: Modules

  Background:
    Given a fresh gardens installation
    Given I am logged in as our testuser

    @smoke @non-utest
    Scenario: As engineering, we want the user to see all of the whitelisted modules
      When I am on the modules page
      Then I should see all whitelisted modules

  @smoke @non-utest
  Scenario: As engineering, we want all of the visible modules to be on the whitelist
    When I am on the modules page
    Then I shouldn't see any non-whitelisted modules

  @smoke @non-utest
  Scenario: As engineering, I don't want to see any unexpected module categories
    When I am on the modules page
    Then I shouldn't see any unexpected module categories

  Scenario: As a user, I want to be able to randomly enable and disable modules
    When I am on the modules page
    Then I want to be able to randomly enable and disable modules

  @non-utest
  Scenario: As gardens eng, I want to be able to install and uninstall a new module
    Given I am on the modules page
    Then I should not see the "Newsletter" module
    When I install the module drush calls "enews"
    And I am on the modules page
    Then I should see the "Newsletter" module
    When I uninstall the module drush calls "enews"
    And I am on the modules page
    Then I should not see the "Newsletter" module

