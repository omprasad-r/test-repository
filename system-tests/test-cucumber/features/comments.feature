Feature: Comments

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  Scenario: As a user, I can set the default comment settings
    When I chose to edit a content type
    And I change the default comment settings
    Then for new content that is created the setting are respected

  Scenario: As a user, I can modify the comment entry attributes
    When I chose to edit a content type
    And I change the the comment entry attributes
    And I add a new comment
    Then the form properties are properly resepected

  Scenario: As a user, I can display comments in different ways
    When I chose to edit a content type
    And I change the comment display attributes
    And I add a new comment
    Then the comments are displayed according to my wishes

   Scenario: As a user, I can see the correct creation even if I have javascript disabled
    Given I chose to edit a content type
    And I change the comment display to use Timeago formats
    And I make a comment
    Then the comment is displayed nicely
    And I disable javascript
    Then my fallback comment date is displayed

