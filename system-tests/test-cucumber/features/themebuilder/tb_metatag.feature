Feature: Theme Builder metatag configuration

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  @selenium
  Scenario: As a user, I can see that when I toggle the checkbox the update button, textfield, and control veil are also toggled
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to the theme builder tab "Advanced"
    When I switch to vertical theme builder tab "Viewport settings"
    Then I should see that the "Enable viewport metatag" checkbox is unchecked
    And the "Viewport metatag" field should be disabled
    And the update button should be disabled
    When I check the "Enable viewport metatag" checkbox
    Then the "Viewport metatag" field should not be disabled
    And the update button should be enabled
    And the control veil should be enabled
    When I uncheck the "Enable viewport metatag" checkbox
    Then the "Viewport metatag" field should be disabled
    And the update button should be disabled
    And the control veil should be disabled

  @smoke
  Scenario: As a user, I can see that the default value for the viewport meta tag is disabled
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to the theme builder tab "Advanced"
    When I switch to vertical theme builder tab "Viewport settings"
    Then I should see that the "Enable viewport metatag" checkbox is unchecked
    And the "Viewport metatag" field should be disabled
    And I should not see the "viewport" meta element

  @selenium
  Scenario: As a user, when I enable the viewport meta tag, the viewport meta tag should be enabled
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to the theme builder tab "Advanced"
    And I switch to the vertical theme builder tab "Viewport settings"
    And I check the "Enable viewport metatag" checkbox
    When I click "Update"
    Then I should see themebuilder-loader appear and disappear
    When I save and reload the current theme
    And I switch to the theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Viewport settings"
    Then I should see that the "Enable viewport metatag" checkbox is checked
    When I close the theme builder
    Then I should see the "viewport" meta element with "width=device-width, initial-scale=1.0" content

  @selenium
  Scenario: As a user, when I change the viewport meta tag, I can see it change in the source
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to the theme builder tab "Advanced"
    And I switch to the vertical theme builder tab "Viewport settings"
    And I check the "Enable viewport metatag" checkbox
    And I fill in "Viewport metatag" with "width=device-width, initial-scale=1.0, maximum-scale=1.0"
    When I click "Update"
    Then I should see themebuilder-loader appear and disappear
    When I save and reload the current theme
    And I switch to the theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Viewport settings"
    Then I should see that the "Enable viewport metatag" checkbox is checked
    When I close the theme builder
    Then I should see the "viewport" meta element with "width=device-width, initial-scale=1.0, maximum-scale=1.0" content

  @selenium
  Scenario: As a user, I can undo and redo changes to the viewport meta tag settings
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to the theme builder tab "Advanced"
    And I switch to the vertical theme builder tab "Viewport settings"
    And I check the "Enable viewport metatag" checkbox
    And I fill in "Viewport metatag" with "width=device-width, initial-scale=1.0, maximum-scale=1.0"
    When I click "Update"
    Then I should see themebuilder-loader appear and disappear
    And I should see that the "Viewport metatag" field has "width=device-width, initial-scale=1.0, maximum-scale=1.0" content
    When I click "Undo"
    Then I should see that the "Viewport metatag" field has "width=device-width, initial-scale=1.0" content
    When I click "Redo"
    Then I should see that the "Viewport metatag" field has "width=device-width, initial-scale=1.0, maximum-scale=1.0" content

  @selenium
  Scenario: As a user, I can undo changes to the viewport meta tag settings after saving my theme
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to the theme builder tab "Advanced"
    And I switch to the vertical theme builder tab "Viewport settings"
    And I check the "Enable viewport metatag" checkbox
    And I fill in "Viewport metatag" with "width=device-width, initial-scale=1.0, maximum-scale=1.0"
    When I click "Update"
    Then I should see themebuilder-loader appear and disappear
    When I save and reload the current theme
    And I switch to the theme builder tab "Advanced"
    And I switch to vertical theme builder tab "Viewport settings"
    Then I should see that the "Viewport metatag" field has "width=device-width, initial-scale=1.0, maximum-scale=1.0" content
    When I click "Undo"
    Then I should see that the "Viewport metatag" field has "width=device-width, initial-scale=1.0" content

  @selenium
  Scenario: As a user, I can save the viewport meta tag settings after navigating away from the tab
    Given I am on "the homepage"
    And I open the theme builder
    And I switch to the theme builder tab "Advanced"
    And I switch to the vertical theme builder tab "Viewport settings"
    And I check the "Enable viewport metatag" checkbox
    And I fill in "Viewport metatag" with "width=device-width, initial-scale=1.0, maximum-scale=1.0"
    And I switch to the vertical theme builder tab "Styles CSS"
    And I save and reload the current theme
    And I switch to the theme builder tab "Advanced"
    When I switch to vertical theme builder tab "Viewport settings"
    Then I should see that the "Enable viewport metatag" checkbox is checked
    Then I should see that the "Viewport metatag" field has "width=device-width, initial-scale=1.0, maximum-scale=1.0" content

