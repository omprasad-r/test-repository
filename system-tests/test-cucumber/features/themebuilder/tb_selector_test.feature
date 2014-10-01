Feature: Theme Builder selector test

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  Scenario: As a user, I can run the theme builder selector test
    Given the module themebuilder_test is enabled
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to theme builder tab "Styles"
    And I switch to vertical theme builder tab "Font"
    And I select element "site name link"
    And I run the theme builder selector test
    Then I should see that the theme builder selector test finished successfully

