Feature: Site maintenance mode HTTP header

  Background:
    Given a fresh gardens installation

  Scenario:
    When I put the site into maintenance mode
    And I visit the site as an anonymous user
    Then I can see the X-SF-Maintenance HTTP response header with a value of 'enabled'

  Scenario:
    When I take the site out of maintenance mode
    And I visit the site as an anonymous user
    Then I can not see the X-SF-Maintenance HTTP response header
