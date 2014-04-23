Feature: Forklift to migrate a site into Site Factory
  So that developer can connect their imported database to their site factory.
  The acsf-connect-factory Drush command to run is found on the site node in the
  factory.

  Background:
    Given a fresh gardens installation
    And I am logged in on the factory as a user with the 'platform admin' role

  Scenario Outline: As a developer, I can connect my site to the factory
    When my source site is <source site>
    And the source database has been imported in the database
    And I run the acsf-connect-factory command
    And I click on the "Log in" link of my site in the factory
    Then I am logged in to my imported site
    Examples:
    | source site                                          |
    | a vanilla Drupal 7 with acsf enabled                 |
    | a vanilla Drupal 7 with acsf disabled                |
    | a site which had acsf enabled on a real site factory |
    | a site which had acsf enabled on drupalgardens.com   |
    | a site which had acsf enabled locally                |
    | a site with acsf disabled locally                    |
