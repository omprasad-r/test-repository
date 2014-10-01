Feature: Flag

  Background:
    Given a fresh gardens installation
    And I am logged in as our testuser
    And Flag is enabled

  Scenario: As a user, I can create a new flag
    When I view my flags
    And I choose to add a new flag with valid information
    And A visitor with the proper permissions visits my site
    Then She can flag and unflag content in accordance to the setup

  Scenario: As a user, I can edit a flag
    Given I have a flag configured
    And I change the settings
    Then A visitor with the proper permissions visits my site
    Then She can flag and unflag
    And The config changes are reflected

  Scenario: As a user, I can create a flag action
    Given I have a flag configured
    And I add an action
    And A visitor with the proper permissions visits my site
    Then She can flag and unflag content in accordance to the setup
    And The flag action is executed

  Scenario: As a user, I can edit a flag action
    Given I have a flag action configured
    And I edit the action
    And A visitor with the proper permissions visits my site
    Then She can flag and unflag content in accordance to the setup
    And The flag action is executed
