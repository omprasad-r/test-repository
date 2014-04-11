Feature: Email invite

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser

  Scenario: As a user, I send a basic email invite
    When I visit the people page
    And I choose to invite users
    And I enter a list of e-mail addresses
    Then my invited users receive an invite e-mail
    And they can join my site

  Scenario: As a user, I can send a custom invite messages
    When I visit the people page
    And I edit the invite message to include the other tokens
    And I invite users
    Then my invited users get the custom invite message
    And they can join my site

  Scenario: As a user, I can't send more than 25 invites a day
    When I have a site that I have invited less than 25 users in a day
    And I invite more so that I exceed the 25 users per day threshold
    Then I am notified that I cannot do that
    And I do not spam the invitees

