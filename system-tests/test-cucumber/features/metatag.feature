Feature: Meta tags

  Background:
    Given a fresh gardens installation
    And the module metatag is enabled
    And I am logged in as our testuser

  @utest
  Scenario: As a user I want metatag's token dialog to be supressed
    Given I am on the metatag settings page
    When I press the element "li.edit.first a" with the text "Override"
    Then I should not see "Browse available tokens" within "#edit-metatags-token-group"

