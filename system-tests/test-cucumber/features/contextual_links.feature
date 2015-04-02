Feature: Contextual links

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  Scenario: As a site owner, I can use the block roll over menu
    Given I have  a block configured on my site
    And that block is visible on the page
    And I mouse over the block
    Then I can use the links and edit the block

