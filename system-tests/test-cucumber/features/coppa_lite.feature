Feature: Coppa lite

  Background:
    Given a fresh warner installation
    And I am logged in as our testuser
    And I have a coppa lite enabled

  Scenario: As a user, I can register a user who is older than 13 years
    When I visit a site as a new user
    And I sign up for the site
    And I enter a date that is COPPA compliant
    Then I can be registered on the site

  Scenario: As a user, I can't register a user who is younger than 13 years
    When I visit a site as a new user
    And I sign up for the site
    And I enter a date that is not COPPA compliant
    Then my regsitration is rejected

  Scenario: As a user, I can't register a user who is younger than 13 years and pretends to be older
    When I visit a site as a new user
    And I sign up for the site
    And I enter a date that is not COPPA compliant
    And my registration is rejected
    And I try to enter a COPPA complaint date
    Then I still cannot register as a new user
