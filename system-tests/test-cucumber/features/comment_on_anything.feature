Feature: Comment on anything

  Background:
    Given a fresh warner installation
    Given I am logged in as our testuser

  Scenario: As a user, I can configure comment on anything
    When I visit the Accounts settings page
    And I add a comment on anything field to the user
    And I give users permission to comment on users
    Then users on my site can comment on uer profiles

  Scenario: As a user, I can comment on a user profile
    Given user profile commenting is configured
    And I find another user
    And I comment on that user
    Then others can see my comment

  Scenario: As an administrator, I can comment on a user profile
    Given user profile commenting is configured
    And I am an administrator on the site
    When I find another user
    And I comment on that user
    Then others can see my comment

  Scenario: As a user, I can edit my own comments
    Given user profile commenting is configured
    And I have permission to edit my comments
    And I find another user
    And I comment on that user
    And others can see my comment
    And I change my comment
    Then others can see my changes

  Scenario: As a user, I can delete my own comments
    Given user profile commenting is configured
    And I have permission to delete my comments
    And I find another user
    And I comment on that user
    And others can see my comment
    And I delete my comment
    Then others cannot see my comment

  Scenario: As a user, I can moderate profile comments
    Given user profile commenting is configured to require moderated comments
    And I have permission to moderate comments
    And a user finds another user
    And a user comments on that other user
    And others cannot see the comment
    And I approve the users comment
    Then others can see the comment

  Scenario: As a user, I can comment on a profile with "open" comments
    Given user profile commenting is configured
    And I find another user
    And that user is open to comments
    And I comment on that user
    Then others can see my comment

  Scenario: As an administrator, I can close comments for a user profile
    Given user profile commenting is configured
    And user1 finds user2
    And user1 comments on user2
    And others can see user1's comment
    And I close commenting on user2
    Then user1 cannot comment on user2

  Scenario: As a user, I can comment on different content types
    Given this hasn't been automated yet

  Scenario: As a user, I can comment on different entity types
    Given this hasn't been automated yet

