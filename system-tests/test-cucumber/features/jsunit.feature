Feature: JSUnit

  Background:
    Given a fresh gardens installation
    And the module "jsunit" is enabled
    And the module "jsunit_example" is enabled
    And I am logged in as our testuser

  @jsunit @smoke @selenium @non-utest
  Scenario: As Engineering we want all of the JSunit tests to pass
    Then the jsunit tests should pass

