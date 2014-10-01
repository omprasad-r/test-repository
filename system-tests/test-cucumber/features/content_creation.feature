Feature: Content creation

  Background:
    Given a fresh gardens installation
    And authenticated users can create basic page content
    And I have the navigation block visible
    And I am logged in as our testuser

  Scenario: As a user, I can create a basic page and update the page content
    Given that I have permission to create a basic page
    And I my site administrator has enabled the Navigation block
    And I add a basic page
    And my new content is displayed
    And I edit my new page
    Then I can see my edits

  Scenario: As a user, I can create custom content types
    Given that I have a custom content type
    And I have permission to create that content type
    And I add the custom type
    And my new content is displayed
    And I edit my content
    Then I can see my edits

